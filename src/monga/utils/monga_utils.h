#ifndef MONGA_UTILS_H
#define MONGA_UTILS_H

#include <stddef.h>

/* Program error codes */
typedef enum
{
	MONGA_ERR_OK,
	MONGA_ERR_MALLOC,
	MONGA_ERR_FREE,
	MONGA_ERR_LEAK,
	MONGA_ERR_UNREACHABLE,
}
monga_error_t;

/* Utility functions */
void* monga_malloc(size_t size);
void monga_free(void* mem);
void* monga_memdup(void* mem, size_t size);
size_t monga_get_allocated_cnt();
void monga_unreachable();

/* Scanner */
size_t monga_get_lineno();

#endif
