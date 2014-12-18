#ifndef __BIP_BUFFER_H__
#define __BIP_BUFFER_H__

/*
 * Copyright (c) 2003 Simon Cooke, All Rights Reserved
 *
 * Licensed royalty-free for commercial and non-commercial
 * use, without warranty or guarantee of suitability for any purpose.
 * All that I ask is that you send me an email
 * telling me that you're using my code. It'll make me
 * feel warm and fuzzy inside. spectecjr@gmail.com
 *
 * 17 December 2014 - converted to standard C by Elizabeth Myers
*/

#include <stdlib.h>	// calloc/free
#include <stdbool.h>	// true/false
#include <stdint.h>	// integer constants

typedef struct bip_buffer_t
{
	char *buffer;
	size_t buflen;
	size_t index_a, size_a;
	size_t index_b, size_b;
	size_t index_resv, size_resv;
} bip_buffer;

/*!
 * Allocate a bip buffer from memory.
 * @param buffer the buffer to initalise (must already be allocated!)
 * @param size size of the buffer to use
 * @return true if successful, false on failure
 */
static inline bool bip_buffer_init(bip_buffer *buffer, size_t size)
{
	if(size <= 0)
	{
		return false;
	}

	buffer->buffer = calloc(size, 1);
	if(!buffer->buffer)
	{
		return false;
	}

	buffer->buflen = size;
	return true;
}

/*!
 * Clear the bip buffer's data, without freeing the memory (it can be reused)
 * @param buffer the buffer to clear
 */
static inline void bip_buffer_clear(bip_buffer *buffer)
{
	buffer->index_a = buffer->size_a = buffer->index_b = buffer->size_b =
		buffer->index_resv = buffer->size_resv = 0;
}

/*!
 * Free the bip buffer, clearing all values
 * @param buffer the buffer to free
 * @note doesn't free buffer itself, just the internal data structures!
 */
static inline void bip_buffer_free(bip_buffer *buffer)
{
	bip_buffer_clear(buffer);
	buffer->buflen = 0;

	free(buffer->buffer);
	buffer->buffer = NULL;
}

/*!
 * Reserve a chunk of memory in the buffer.
 * @param buffer the buffer to allocate memory from
 * @param size the size of the allocation to make
 * @return NULL if insufficent free space, otherwise the pointer to the reserved block
 * @note you must call bip_buffer_commit after you're done to push it into the buffer!
 */
static inline void *bip_buffer_reserve(bip_buffer *buffer, size_t size)
{
	size_t free_space;

	// We always allocate on B if B exists; this means we have two blocks and our buffer is filling.
	if(buffer->size_b)
	{
		free_space = buffer->index_a - buffer->index_b - buffer->size_b;
		if(size > free_space)
		{
			return NULL;
		}

		free_space = size;

		buffer->size_resv = free_space;
		buffer->index_resv = buffer->index_b + buffer->size_b;
		return (void *)(buffer->buffer + buffer->index_resv);
	}
	else
	{
		// Block b does not exist, so we can check if the space AFTER a is bigger than the space
		// before A, and allocate the bigger one.
		free_space = buffer->buflen - buffer->index_a - buffer->size_a;
		if(free_space >= buffer->index_a)
		{
			if(size > free_space)
			{
				return NULL;
			}

			free_space = size;

			buffer->size_resv = free_space;
			buffer->index_resv = buffer->index_a + buffer->size_a;
			return (void *)(buffer->buffer + buffer->index_resv);
		}
		else
		{
			if(size > buffer->index_a)
			{
				return NULL;
			}

			buffer->index_resv = size;
			buffer->index_resv = 0;
			return (void *)buffer->buffer;
		}
	}
}

/*!
 * Commit the last reservation (a given range of bytes)
 * @param buffer the buffer to use
 * @param size the size of the commit range, must be <= the reserved range, 0 to decommit
 */
static inline void bip_buffer_commit(bip_buffer *buffer, size_t size)
{
	if(size == 0)
	{
		// decommit any reservation
		buffer->size_resv = buffer->index_resv = 0;
		return;
	}

	// If we try to commit more space than we asked for, clip to the size we asked for.
	if(size > buffer->size_resv)
	{
		size = buffer->size_resv;
	}

	// If we have no blocks being used currently, we create one in A.
	if(!buffer->size_a && !buffer->size_b)
	{
		buffer->index_a = buffer->index_resv;
		buffer->size_a = size;

		buffer->index_resv = buffer->size_resv = 0;
		return;
	}

	if(buffer->index_resv == (buffer->size_a + buffer->index_a))
	{
		buffer->size_a += size;
	}
	else
	{
		buffer->size_b += size;
	}

	buffer->index_resv = buffer->size_resv = 0;
}

/*!
 * Cancel a reservation (alias for bip_buffer_commit(buffer, NULL))
 * @param buffer Buffer to cancel reservation on
 */
static inline void bip_buffer_cancel(bip_buffer *buffer)
{
	bip_buffer_commit(buffer, 0);
}

/*!
 * Point to the next data in the buffer, decommit whatever you're done with.
 * @param buffer the buffer to use
 * @param size size of the data returned (NULL if you don't care)
 * @return pointer to data
 */
static inline void * bip_buffer_get(bip_buffer *buffer, size_t *size)
{
	if(!buffer->size_a)
	{
		if(size)
		{
			*size = 0;
		}
		return NULL;
	}

	if(size)
	{
		*size = buffer->size_a;
	}

	return (void *)(buffer->buffer + buffer->index_a);
}

/*!
 * Decommit the given data, returning it as free/read space in the buffer
 * @param buffer buffer to return data to
 * @param size size of data to free up
 * @note Only decommit committed data!
 */
static inline void bip_buffer_decommit(bip_buffer *buffer, size_t size)
{
	if(size >= buffer->size_a)
	{
		buffer->index_a = buffer->index_b;
		buffer->size_a = buffer->size_b;
		buffer->index_b = buffer->size_b = 0;
	}
	else
	{
		buffer->size_a -= size;
		buffer->index_a += size;
	}
}

/*!
 * Get the amount of committed data in the buffer
 * @param buffer to get size of committed data
 * @return size of committed data
 */
static inline size_t bip_buffer_get_committed(bip_buffer *buffer)
{
	return buffer->size_a + buffer->size_b;
}

/*!
 * Get the amount of reserved memory in the buffer
 * @param buffer buffer to get size of reserved memory
 * @return size of reserved data
 */
static inline size_t bip_buffer_get_reserved(bip_buffer *buffer)
{
	return buffer->size_resv;
}

/*!
 * Get the size of the buffer
 * @param buffer buffer to get the size of
 * @return size of the buffer
 */
static inline size_t bip_buffer_get_size(bip_buffer *buffer)
{
	return buffer->buflen;
}

#endif //__BIP_BUFFER_H__
