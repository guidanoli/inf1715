#ifndef MONGA_UTILS_H
#define MONGA_UTILS_H

#include <stddef.h>
#include <stdbool.h>

/* Size of static C array */
#define sizeofv(v) (sizeof(v)/sizeof(*v))

/* Program error codes */
typedef enum
{
	MONGA_ERR_OK,
	MONGA_ERR_MALLOC,
	MONGA_ERR_FREE,
	MONGA_ERR_LEAK,
	MONGA_ERR_UNREACHABLE,
	MONGA_ERR_ASSERT,
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
	const void* mem,
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

void monga_assert_func(
#ifdef MONGA_DEBUG
	const char* file,
	int line,
#endif
	bool condition,
	const char* condition_str
);

#ifdef MONGA_DEBUG
#define monga_assert(cond) monga_assert_func(__FILE__, __LINE__, cond, #cond)
#else
#define monga_assert(cond) monga_assert_func(cond, #cond)
#endif

/* Use this for unused arguments in a function definition to silence compiler
 * warnings. For example:
 * int func(int a, int monga_unused(b)) { return a; } */
#if defined(__GNUC__) || defined(__clang__)
#  define monga_unused(name) _unused_ ## name __attribute__((unused))
#else
#  define monga_unused(name) _unused_ ## name
#endif

/* Scanner */
size_t monga_get_lineno();

#endif
