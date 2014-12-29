#include "config.h"	// bool, stdint

#include "sgherm.h"	// emu_state
#include "print.h"	// error

#include <stdlib.h>	// malloc
#include <string.h>	// strerror
#include <errno.h>	// errno


#ifdef HAVE_MMAP

#include <sys/mman.h>	// mmap/munmap/msync
#include <sys/stat.h>	// open
#include <sys/types.h>	// open flags (some systems)
#include <fcntl.h>	// open flags (some systems)


typedef struct memmap_state_t
{
	char *path;
	size_t size;
	int flags;
} memmap_state;

static inline int _open_map(const char *path, size_t size)
{
	int fd;

	if((fd = open(path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) < 0)
	{
		return -1;
	}

	if(ftruncate(fd, size) < 0)
	{
		return -1;
	}

	return fd;
}


void * memmap_open(emu_state *restrict state, const char *path, size_t size, memmap_state **data)
{
	memmap_state *m_state = malloc(sizeof(memmap_state));
	int fd;
	void *map;

	if(path)
	{
		m_state->path = strdup(path);

		if((fd = _open_map(m_state->path, m_state->size)) < 0)
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
		fd = -1;
		m_state->path = NULL;
		m_state->flags = MAP_PRIVATE|MAP_ANONYMOUS;
	}

	if((map = mmap(NULL, size, PROT_READ | PROT_WRITE, m_state->flags, fd, 0)) == NULL)
	{
		error(state, "Could not mmap file: %s", strerror(errno));
		free(m_state);
		*data = NULL;
		return NULL;
	}

	// POSIX says it's okay to close the file
	close(fd);

	madvise(map, size, MADV_RANDOM);

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

	if(m_state->path)
	{
		if((fd = _open_map(m_state->path, m_state->size)) < 0)
		{
			error(state, "Could not open file for remap: %s", strerror(errno));
			return NULL;
		}
	}

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

	if(!map)
	{
		return;
	}

	munmap(map, m_state->size);

	free(m_state->path);
	free(m_state);
	*data = NULL;
}

void memmap_sync(emu_state *restrict state UNUSED, void *map, memmap_state **data)
{
	memmap_state *m_state = *data;

	msync(map, m_state->size, MS_ASYNC);
}

#else //HAVE_MMAP

// Shitty fallback implementation for lesser systems

#include <stdio.h>	// file functions


typedef struct memmap_state_t
{
	FILE *f;
	bool anonymous;
	size_t size;
} memmap_state;


void * memmap_open(emu_state *restrict state, const char *path, size_t size, memmap_state **data)
{
	memmap_state *m_state = (memmap_state *)malloc(sizeof(memmap_state));
	void *map = malloc(size);

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
