#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MONGA_DO_NOT_DEFINE_MACROS
#include "monga_utils.h"

#ifdef MONGA_DEBUG
struct monga_amb_list {
	void* mem;
	const char* file;
	int line;
	size_t size;
	struct monga_amb_list* next;
};

struct monga_amb_list* monga_amb_list_head;
#endif

/* Number of allocated memory spaces */
static size_t allocated_cnt = 0;

void* monga_memdup(
#ifdef MONGA_DEBUG
	const char* file,
	int line,
#endif
	void* mem,
	size_t size)
{
	void* new_mem = monga_malloc(
#ifdef MONGA_DEBUG
		file,
		line,
#endif
		size);
	memcpy(new_mem, mem, size);
	return new_mem;
}

#ifdef MONGA_DEBUG
static void monga_add_node(void* mem, const char* file, int line, size_t size)
{
	struct monga_amb_list* new_node = malloc(sizeof *new_node);
	if (!new_node) {
		fprintf(stderr, "Could not allocate memory.\n");
		exit(MONGA_ERR_MALLOC);
	}
	new_node->mem = mem;
	new_node->file = file;
	new_node->line = line;
	new_node->size = size;
	new_node->next = monga_amb_list_head;
	monga_amb_list_head = new_node;
}
#endif

void* monga_malloc(
#ifdef MONGA_DEBUG
	const char* file,
	int line,
#endif
	size_t size
)
{
	void* mem = NULL;
#ifndef MONGA_FAIL_MALLOC
	mem = malloc(size);
#endif
	if (!mem) {
		fprintf(stderr, "Could not allocate memory.\n");
		exit(MONGA_ERR_MALLOC);
	}
#ifdef MONGA_DEBUG
	monga_add_node(mem, file, line, size);
#endif
	++allocated_cnt;
	return mem;
}

#ifdef MONGA_DEBUG
void monga_remove_node(void* mem)
{
	struct monga_amb_list* node, * prev = NULL;
	for (node = monga_amb_list_head; node; node = node->next) {
		if (node->mem == mem) {
			if (prev == NULL) {
				monga_amb_list_head = node->next;
			} else {
				prev->next = node->next;
			}
			free(node);
			return;
		}
		prev = node;
	}
	fprintf(stderr, "WARNING: Could not find node %p in allocated memory block list.\n", mem);
}
#endif

void monga_free(void* mem)
{
	if (!mem) {
		fprintf(stderr, "Could not deallocate memory.\n");
		exit(MONGA_ERR_FREE);
	}
#ifdef MONGA_DEBUG
	monga_remove_node(mem);
#endif
	--allocated_cnt;
	free(mem);
}

void monga_clean_amb()
{
	struct monga_amb_list* node;
	for (node = monga_amb_list_head; node; node = node->next) {
		fprintf(stderr, "Leaked %p (%zuB) allocated at %s:%d.\n",
			node->mem, node->size, node->file, node->line);
		free(node->mem);
	}
}

size_t monga_get_allocated_cnt()
{
	return allocated_cnt;
}

void monga_unreachable_func(
#ifdef MONGA_DEBUG
	const char* file,
	int line
#endif
)
{
#ifdef MONGA_DEBUG
	fprintf(stderr, "Unexpectedly reached %s:%d\n", file, line);
#else
	fprintf(stderr, "Reached the unexpected\n");
#endif
	exit(MONGA_ERR_UNREACHABLE);
}