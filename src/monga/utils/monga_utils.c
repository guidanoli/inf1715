#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "monga_utils.h"

void* monga_memdup(void* mem, size_t size)
{
	void* new_mem = monga_malloc(size);
	memcpy(new_mem, mem, size);
	return new_mem;
}

void* monga_malloc(size_t size)
{
	void* mem = NULL;
#ifndef MONGA_FAIL_MALLOC
	mem = malloc(size);
#endif
	if (!mem) {
		fprintf(stderr, "Could not allocate memory.\n");
		exit(MONGA_ERR_MALLOC);
	}
	return mem;
}
