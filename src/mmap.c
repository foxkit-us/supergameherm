#include "config.h"	// bool, stdint

#include "sgherm.h"	// emu_state
#include "print.h"	// error

#include <stdlib.h>	// malloc
#include <string.h>	// strerror
#include <errno.h>	// errno
#include <assert.h>	// assert


#ifdef HAVE_MMAP

// Smooth over MAP_ANON and MAP_ANONYMOUS differences
#ifdef HAVE_MAP_ANONYMOUS
#	ifndef MAP_ANONYMOUS
#		define MAP_ANONYMOUS MAP_ANON
#	endif
#endif

#include <sys/mman.h>	// mmap/munmap/msync
#include <unistd.h>	// open/close
#include <sys/stat.h>	// open flags (some systems)
#include <sys/types.h>	// open flags (some systems)
#include <fcntl.h>	// open flags (some systems)


struct memmap_state_t
{
	char *path;
	size_t size, f_size;
	int flags;
};


static inline int _open_map(const char *path, size_t size)
{
	int fd;
	int64_t filesize;

	if((filesize = get_file_size(path)) < 0)
	{
		return -1;
	}

	if((fd = open(path, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR)) < 0)
	{
		return -1;
	}

	if((size_t)filesize < size)
	{
		// Pad out remaining size by writing 0's (to avoid fragmentation)
		unsigned i;
		char ch = '\0';

		for(i = 0; i < (size - filesize); i++)
		{
			int ret;
			if((ret = write(fd, &ch, 1)) < 0)
			{
				close(fd);
				return -1;
			}
			else if(ret == 0)
			{
				// write again
				i--;
			}
		}
	}
	else if((size_t)filesize > size && ftruncate(fd, size) < 0)
	{
		// Truncate the fie or fail
		close(fd);
		return -1;
	}

	lseek(fd, 0, SEEK_SET);

	return fd;
}

static inline uint64_t _round_nearest(uint64_t num, uint32_t multiple)
{
	uint64_t rem;

	assert(multiple);

	if((rem = num % multiple) == 0)
	{
		return num;
	}

	return num + multiple - rem;
}

void * memmap_open(emu_state *restrict state, const char *path, size_t size, memmap_state **data)
{
	memmap_state *m_state = malloc(sizeof(memmap_state));
	int fd;
	void *map;

	if(!size)
	{
		free(m_state);
		*data = NULL;
		return NULL;
	}

	if(path)
	{
		m_state->path = strdup(path);

		if((fd = _open_map(m_state->path, size)) < 0)
		{
			error(state, "Could not open file for mmap: %s", strerror(errno));
			free(m_state);
			*data = NULL;
			return NULL;
		}

		m_state->flags = MAP_SHARED;
	}
	else
	{
		// Anonymous mapping
#ifdef HAVE_MAP_ANONYMOUS
		fd = -1;
		m_state->path = NULL;
		m_state->flags = MAP_PRIVATE | MAP_ANONYMOUS;
#else
		m_state->path = "/dev/zero";
		if((fd = _open_map(m_state->path, size)) < 0)
		{
			error(state, "Could not open /dev/zero for mmap: %s", strerror(errno));
			free(m_state);
			*data = NULL;
			return NULL;
		}

		m_state->flags = MAP_PRIVATE;
#endif
	}

	// Round up to nearest page size
	m_state->f_size = size;
	m_state->size = size = _round_nearest(size, sysconf(_SC_PAGESIZE));

	if(!(map = mmap(NULL, size, PROT_READ | PROT_WRITE, m_state->flags, fd, 0)))
	{
		error(state, "Could not mmap file: %s", strerror(errno));

		if(fd > -1)
		{
			close(fd);
		}

		free(m_state);
		*data = NULL;
		return NULL;
	}

	// POSIX says it's okay to close the file
	if(fd > -1)
	{
		close(fd);
	}

	madvise(map, size, MADV_RANDOM);

	debug(state, "Allocated %ld bytes", size);

	m_state->size = size;
	*data = m_state;
	return map;
}

#if defined(HAVE_MREMAP) && !defined(__NetBSD__)
// Use a better implementation (NetBSD's is not compatible)

#ifndef MREMAP_MAYMOVE
#	define MREMAP_MAYMOVE 0
#endif //!MREMAP_MAYMOVE

void * memmap_resize(emu_state *restrict state, void *map, size_t size, memmap_state **data)
{
	memmap_state *m_state = *data;
	void *map_new;

	assert(m_state);

	if(size == 0)
	{
		memmap_close(state, map, data);
		return;
	}

	m_state->f_size = size;
	size = _round_nearest(size, sysconf(_SC_PAGESIZE));

	if(!(map_new = mremap(map, m_state->size, size, MREMAP_MAYMOVE)))
	{
		error(state, "Could not remap file: %s", strerror(errno));
		return NULL;
	}

	return map_new;
}

#else //defined(HAVE_MREMAP) && !defined(__NetBSD__)
// Fall back to POSIXly correct method

void * memmap_resize(emu_state *restrict state, void *map, size_t size, memmap_state **data)
{
	memmap_state *m_state = *data;
	void *map_new;
	int fd = -1;

	assert(m_state);

	if(size == 0)
	{
		memmap_close(state, map, data);
		return NULL;
	}

	if(m_state->path)
	{
		if((fd = _open_map(m_state->path, m_state->size)) < 0)
		{
			error(state, "Could not open file for remap: %s", strerror(errno));
			return NULL;
		}
	}

	m_state->f_size = size;
	size = _round_nearest(size, sysconf(_SC_PAGESIZE));

	if((map_new = mmap(NULL, size, PROT_READ | PROT_WRITE, m_state->flags, fd, 0)) == NULL)
	{
		error(state, "Could not resize mmap file: %s", strerror(errno));
		return NULL;
	}

	madvise(map_new, size, MADV_RANDOM);

	// Remove old mapping
	munmap(map, m_state->size);

	m_state->size = size;
	return map_new;
}

#endif //defined(HAVE_MREMAP) && !defined(__NetBSD__)

void memmap_close(emu_state *restrict state UNUSED, void *map, memmap_state **data)
{
	memmap_state *m_state = *data;
	int ret;

	assert(m_state);

	if(!map)
	{
		return;
	}

	munmap(map, m_state->size);

	// Truncate if possible
	ret = truncate(m_state->path, m_state->f_size);
	(void)ret;

	free(m_state->path);
	free(m_state);
	*data = NULL;
}

void memmap_sync(emu_state *restrict state UNUSED, void *map, memmap_state **data)
{
	memmap_state *m_state = *data;

	assert(m_state);

	msync(map, m_state->size, MS_ASYNC);
}

#else //HAVE_MMAP

// Shitty fallback implementation for lesser systems

#include <stdio.h>	// file functions


struct memmap_state_t
{
	FILE *f;
	bool anonymous;
	size_t size;
};


void * memmap_open(emu_state *restrict state, const char *path, size_t size, memmap_state **data)
{
	memmap_state *m_state = (memmap_state *)malloc(sizeof(memmap_state));
	void *map = malloc(size);

	*data = m_state;

	if(path)
	{
		m_state->anonymous = false;

		if(!(m_state->f = fopen(path, "r+b")))
		{
			error(state, "Could not open file for fake mmap: %s", strerror(errno));
			free(m_state);
			free(map);
			*data = NULL;
			return NULL;
		}

		if((fread(map, 1, size, m_state->f) < size) && ferror(m_state->f))
		{
			error(state, "Could not read file for fake mmap: %s", strerror(errno));
			fclose(m_state->f);
			free(m_state);
			free(map);
			*data = NULL;
			return NULL;
		}

		fseek(m_state->f, 0, SEEK_SET);
	}
	else
	{
		m_state->anonymous = true;
	}

	return map;
}

void * memmap_resize(emu_state *restrict state, void *map, size_t size, memmap_state **data)
{
	memmap_state *m_state = *data;
	void *map_new;

	// Write back all data
	memmap_sync(state, map, data);

	if(!(map_new = realloc(map, size)))
	{
		error(state, "Could not resize fake mmap: %s", strerror(errno));
		return NULL;
	}

	m_state->size = size;
	return map_new;
}

void memmap_close(emu_state *restrict state, void *map, memmap_state **data)
{
	memmap_state *m_state = *data;

	if(!(m_state->anonymous))
	{
		// Write back data
		memmap_sync(state, map, data);

		fclose(m_state->f);
	}

	free(m_state);
	free(map);
	*data = NULL;
}

void memmap_sync(emu_state *restrict state UNUSED, void *map, memmap_state **data)
{
	memmap_state *m_state = *data;

	if(!(m_state->anonymous))
	{
		// Write all data (if it fails there's not much we can do...)
		fwrite(map, m_state->size, 1, m_state->f);
		if(ferror(m_state->f))
		{
			error(state, "Could not write back file for fake mmap: %s", strerror(errno));
		}

		fseek(m_state->f, 0, SEEK_SET);
	}
}

#endif //HAVE_MMAP
