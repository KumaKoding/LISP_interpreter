#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "garbage.h"

struct ExprVector init_e_vec()
{
	struct ExprVector e_vec;

	e_vec.max = INITIAL_EXPR_VECTOR_SIZE;
	e_vec.exprs = malloc(sizeof(Expr *) * INITIAL_EXPR_VECTOR_SIZE);
	e_vec.n_exprs = 0;

	return e_vec;
}

void e_vec_push(struct ExprVector *e_vec, Expr *e)
{
	if(e_vec->n_exprs == e_vec->max)
	{
		e_vec->exprs = realloc(e_vec->exprs, e_vec->max * 2);
		e_vec->max *= 2;
	}

	e_vec->exprs[e_vec->n_exprs] = e;
	e_vec->n_exprs++;
}

void mark_root(Expr *root)
{
	// Expr *e_curr = root;
}

void mark(struct Collector *gc)
{
	for(int i = 0; i < gc->roots->len; i++)
	{
		mark_root(gc->roots->stack[i].fn);

		// need to integrate variable map into callstack
		for(int p = 0; p < gc->roots->stack[i].params_available; p++)
		{
			mark_root(gc->roots->stack[i].params[p]);
		}
	}
}

void sweep(struct Collector *gc)
{
	struct ExprVector e_vec = init_e_vec();

	for(int i = 0; i < gc->e_vec.n_exprs; i++)
	{
		if(gc->e_vec.exprs[i]->mark == 0)
		{
			gc->e_vec.exprs[i]->mark = 1;
			e_vec_push(&e_vec, gc->e_vec.exprs[i]);
		}
		else 
		{
			free(gc->e_vec.exprs[i]);
		}
	}

	free(gc->e_vec.exprs);
	gc->e_vec = e_vec;
}

