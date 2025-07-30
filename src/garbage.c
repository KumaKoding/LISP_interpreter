#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "expr.h"
#include "garbage.h"
#include "callstack.h"
#include "my_malloc.h"

#if TESTING
	#define malloc(X) check_malloc(X, __FILE__, __LINE__, __FUNCTION__)
	#define realloc(X, Y) check_realloc(X, Y, __FILE__, __LINE__, __FUNCTION__)
	#define free(X) check_free(X, __FILE__, __LINE__, __FUNCTION__)
#endif

struct Collector init_gc(struct CallStack *cs)
{
	struct Collector gc;

	gc.allocs = 0;
	gc.e_vec = init_e_vec();
	gc.roots = cs;

	return gc;
}

void gc_push(struct Collector *gc, Expr *e)
{
	e_vec_push(gc->e_vec, e);
	e->mark = 0;
	gc->allocs++;
}

void mark_root(Expr *root)
{
	struct ExprStack trace;

	trace.len = 0;
	es_push(&trace, root);

	while(trace.len > 0)
	{
		Expr *e_curr = es_pop(&trace);

		while(e_curr != NULL)
		{
			e_curr->mark = 1;

			switch (e_curr->car.type)
			{
				case Lst:
					if(e_curr->car.data.lst)
					{
						es_push(&trace, e_curr->car.data.lst);
					}

					break;
				case Lam:
					for(int p = 0; p < e_curr->car.data.lam->n_filled; p++)
					{
						es_push(&trace, e_curr->car.data.lam->params[p]);
					}

					es_push(&trace, e_curr->car.data.lam->instructions);

					break;
				case Nat:
					for(int p = 0; p < e_curr->car.data.nat->n_filled; p++)
					{
						es_push(&trace, e_curr->car.data.nat->params[p]);
					}

					break;
				case IfE:
					es_push(&trace, e_curr->car.data.ifE->branch_true);
					es_push(&trace, e_curr->car.data.ifE->branch_false);

					break;
				case Def:
					break;
				default:
					break;
			}

			e_curr = e_curr->cdr;
		}
	}
}

void mark(struct Collector *gc)
{
	for(int i = 0; i < gc->roots->len; i++)
	{
		if(gc->roots->stack[i].fn)
		{
			mark_root(gc->roots->stack[i].fn);
		}

		// need to integrate variable map into callstack
		for(int p = 0; p < gc->roots->stack[i].params_available; p++)
		{
			mark_root(gc->roots->stack[i].params[p]);
		}

		for(int e = 0; e < gc->roots->stack[i].local_references->len; e++)
		{
			mark_root(gc->roots->stack[i].local_references->map[e].e);
		}
	}
}

void sweep(struct Collector *gc)
{
	struct ExprVector *e_vec = init_e_vec();

	for(int i = 0; i < gc->e_vec->n_exprs; i++)
	{
		if(gc->e_vec->exprs[i]->mark)
		{
			gc->e_vec->exprs[i]->mark = 0;
			e_vec_push(e_vec, gc->e_vec->exprs[i]);
		}
		else 
		{
			switch (gc->e_vec->exprs[i]->car.type) {
				case Lst:
					break;
				case Lam:
					for(int p = 0; p < gc->e_vec->exprs[i]->car.data.lam->n_args; p++)
					{
						v_destruct(gc->e_vec->exprs[i]->car.data.lam->p_keys[p]);
					}

					free(gc->e_vec->exprs[i]->car.data.lam->params);
					free(gc->e_vec->exprs[i]->car.data.lam->p_keys);
					free(gc->e_vec->exprs[i]->car.data.lam);

					break;
				case Nat:
					v_destruct(gc->e_vec->exprs[i]->car.data.nat->key);
					free(gc->e_vec->exprs[i]->car.data.nat->params);
					free(gc->e_vec->exprs[i]->car.data.nat);

					break;
				case IfE:
					free(gc->e_vec->exprs[i]->car.data.ifE);
					break;
				case Def:
					v_destruct(gc->e_vec->exprs[i]->car.data.str);

					break;
				case Str:
					v_destruct(gc->e_vec->exprs[i]->car.data.str);

					break;
				case Idr:
					v_destruct(gc->e_vec->exprs[i]->car.data.str);

					break;
				default:
					break;
			}

			free(gc->e_vec->exprs[i]);
			gc->allocs--;
		}
	}

	free(gc->e_vec->exprs);
	free(gc->e_vec);

	gc->e_vec = e_vec;
}

