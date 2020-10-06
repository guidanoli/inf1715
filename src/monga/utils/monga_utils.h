#ifndef MONGA_UTILS_H
#define MONGA_UTILS_H

#include <stddef.h>

/* Program error codes */
typedef enum
{
	MONGA_ERR_OK,
	MONGA_ERR_MALLOC,
}
monga_error_t;

/* Utility functions */
void* monga_malloc(size_t size);
void* monga_memdup(void* mem, size_t size);

#endif
