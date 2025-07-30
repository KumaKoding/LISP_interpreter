#include <stdio.h>
#include <stdlib.h>

#include "callstack.h"
#include "expr.h"
#include "garbage.h"
#include "my_malloc.h"

#if TESTING
	#define malloc(X) check_malloc(X, __FILE__, __LINE__, __FUNCTION__)
	#define realloc(X, Y) check_realloc(X, Y, __FILE__, __LINE__, __FUNCTION__)
	#define free(X) check_free(X, __FILE__, __LINE__, __FUNCTION__)
#endif

int count_exprs(Expr *e)
{
	int n = 0;
	Expr *e_curr  = e;

	while(e_curr)
	{
		n++;

		e_curr = e_curr->cdr;
	}

	return n;
}

void cs_push(int n_params, Expr **return_addr, struct CallStack *cs)
{
	if(n_params > 0)
	{
		cs->stack[cs->len].params = malloc(sizeof(Expr*) * n_params);
	}
	else 
	{
		cs->stack[cs->len].params = NULL;
	}

	cs->stack[cs->len].params_available = n_params;
	cs->stack[cs->len].params_evaluated = 0;

	cs->stack[cs->len].fn = NULL;
	cs->stack[cs->len].function_evaluated = 0;

	cs->stack[cs->len].local_references = init_map();

	cs->stack[cs->len].return_addr = return_addr;

	cs->len++;
}

void add_fn(Expr *e, Expr **return_addr, struct CallStack *cs, struct Collector *gc)
{
	if(!e)
	{
		printf("ERROR: NULL expression cannot be turned into a function. Aborting.");
		abort();
	}

	int n_params = count_exprs(e) - 1;

	cs_push(n_params, return_addr, cs);

	cs->stack[cs->len - 1].fn = new_copy(e, EXCLUDE_CDR, gc);

	Expr *e_curr = e->cdr;

	for(int i = 0; i < n_params; i++)
	{
		cs->stack[cs->len - 1].params[i] = new_copy(e_curr, EXCLUDE_CDR, gc);

		e_curr = e_curr->cdr;
	}
}

LocalMap *init_map()
{
	LocalMap *lm = malloc(sizeof(LocalMap));
	lm->len = 0;

	return lm;
}

Expr *map_search(Vector *key, LocalMap lm)
{
	for(int i = 0; i < lm.len; i++)
	{
		if(vec_cmp_vec(key, lm.map[i].v))
		{
			return lm.map[i].e;
		}
	}

	return NULL;
}

MapPair init_map_pair(Vector *v, Expr *e, struct Collector *gc)
{
	MapPair mp;

	mp.v = v_init();
	v_copy(mp.v, v);

	mp.e = new_copy(e, EXCLUDE_CDR, gc);

	return mp;
}

Expr *relative_stack_search(Vector *key, struct CallStack *cs, int idx)
{
	for(int i = idx; i >= 0; i--)
	{
		Expr *search = map_search(key, *cs->stack[i].local_references);

		if(search)
		{
			return search;
		}
	}

	return NULL;
}

void map_push(LocalMap *lm, MapPair mp)
{
	if(lm->len == MAX_MAP_SIZE)
	{
		printf("ERROR: Too many identifiers defined in one scope. Aborting.");
		abort();
	}

	lm->map[lm->len] = mp;
	lm->len++;
}

