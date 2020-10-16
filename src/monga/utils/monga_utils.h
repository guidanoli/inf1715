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
	MONGA_ERR_REDECLARATION,
	MONGA_ERR_UNDECLARED,
	MONGA_ERR_REFERENCE_KIND,
	MONGA_ERR_TYPE,
	MONGA_ERR_NO_RETURN,
	MONGA_ERR_SIGNATURE,
}
monga_error_t;

/* Utility functions */

void* monga_malloc(
#ifdef MONGA_DEBUG
	const char* file,
	int line,
#endif
	size_t size
);

void* monga_memdup(
#ifdef MONGA_DEBUG
	const char* file,
	int line,
#endif
	void* mem,
	size_t size);

#if defined(MONGA_DEBUG) && !defined(MONGA_DO_NOT_DEFINE_MACROS)
#define monga_malloc(size) monga_malloc(__FILE__, __LINE__, size)
#define monga_memdup(mem, size) monga_memdup(__FILE__, __LINE__, mem, size)
#endif

#ifdef MONGA_DEBUG
void monga_clean_amb();
#endif

void monga_free(void* mem);

size_t monga_get_allocated_cnt();

void monga_unreachable_func(
#ifdef MONGA_DEBUG
	const char* file,
	int line
#endif
);

#ifdef MONGA_DEBUG
#define monga_unreachable() monga_unreachable_func(__FILE__, __LINE__)
#else
#define monga_unreachable() monga_unreachable_func()
#endif

/* Scanner */
size_t monga_get_lineno();

#endif
