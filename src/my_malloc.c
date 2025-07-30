#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "my_malloc.h"

struct allocation_lst allocs;

void *check_malloc(size_t size, const char *const file, const int line, const char *const fn)
{
	void *ptr = malloc(size);

	// printf("%d:%s:%s\t%p\n", line, fn, file, ptr);
	
	if(allocs.allocs == 0)
	{
		allocs.ll = malloc(sizeof(struct allocation));
		allocs.head = allocs.ll;
		allocs.allocs = 0;
	}
	else 
	{
		allocs.head->next = malloc(sizeof(struct allocation));
		allocs.head = allocs.head->next;
	}

	allocs.head->ptr = ptr;
	allocs.head->file = file;
	allocs.head->line = line;
	allocs.head->fn = fn;
	allocs.head->next = NULL;

	allocs.allocs++;

	return ptr;
}

void *check_realloc(void *ptr, size_t size, const char *const file, const int line, const char *const fn)
{
	void *new_ptr = realloc(ptr, size);
	struct allocation *a_curr = allocs.ll;

	while(a_curr->ptr != ptr)
	{
		a_curr = a_curr->next;
	}

	// printf("%d:%s:%s\t%p\n", line, fn, file, new_ptr);

	a_curr->ptr = new_ptr;
	a_curr->file = file;
	a_curr->line = line;
	a_curr->fn = fn;

	return new_ptr;
}

void check_free(void *ptr, const char *const file, const int line, const char *const fn)
{
	struct allocation *a_curr = allocs.ll;
	struct allocation *a_prev = NULL;

	while(a_curr)
	{
		if(a_curr->ptr == ptr)
		{
			break;
		}

		a_prev = a_curr;
		a_curr = a_curr->next;
	}

	if(a_curr)
	{
		// printf("Freed pointer at %d:%s:%s.\t%p. From %d:%s:%s\n", line, fn, file, ptr, a_curr->line, a_curr->fn, a_curr->file);
		a_curr->file = NULL;
		a_curr->line = 0;
		a_curr->fn = NULL;

		if(allocs.allocs == 1)
		{
			free(ptr);
			free(allocs.ll);

			allocs.ll = NULL;
			allocs.head = NULL;
		}
		else if(a_curr == allocs.ll)
		{
			free(ptr);

			struct allocation *a_next = a_curr->next;

			free(a_curr);

			allocs.ll = a_next;
		}
		else if(a_curr == allocs.head)
		{
			free(ptr);
			free(a_curr);
			
			allocs.head = a_prev;
			allocs.head->next = NULL;
		}
		else 
		{
			free(ptr);

			a_prev->next = a_curr->next;

			free(a_curr);
		}
	}
	else 
	{
		printf("Freed untracked pointer at %d by %s in %s. %p.\n", line, fn, file, ptr);
		abort();
	}

	allocs.allocs--;
}

void check_memory()
{
	struct allocation *a_curr = allocs.ll;

	int n = 0;

	while(a_curr)
	{
		printf("%d: Allocation by %s on line %d in file %s: %p\n", n, a_curr->fn, a_curr->line, a_curr->file, a_curr->ptr);

		a_curr = a_curr->next;
		n++;
	}
	printf("\n");
}
