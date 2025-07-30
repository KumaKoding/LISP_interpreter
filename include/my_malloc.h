#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <stddef.h>

#define TESTING 1

struct allocation
{
	void *ptr;
	struct allocation *next;
	const char *file;
	int line;
	const char *fn;
};

struct allocation_lst
{
	struct allocation *ll;
	struct allocation *head;
	int allocs;
};

void *check_malloc(size_t size, const char *const file, const int line, const char *const fn);
void *check_realloc(void *ptr, size_t size, const char *const file, const int line, const char *const fn);
void check_free(void *ptr, const char *const file, const int line, const char *const fn);
void check_memory();

#endif
