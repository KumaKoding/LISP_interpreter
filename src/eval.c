#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "callstack.h"
#include "expr.h"
#include "garbage.h"
#include "my_malloc.h"

#if TESTING
	#define malloc(X) check_malloc(X, __FILE__, __LINE__, __FUNCTION__)
	#define realloc(X, Y) check_realloc(X, Y, __FILE__, __LINE__, __FUNCTION__)
	#define free(X) check_free(X, __FILE__, __LINE__, __FUNCTION__)
#endif



void shadow_variables(Vector *v, Expr *e, LocalMap *lm, struct Collector *gc)
{
	for(int i = 0; i < lm->len; i++)
	{
		if(vec_cmp_vec(v, lm->map[i].v))
		{
			// memory leak
			lm->map[i] = init_map_pair(v, e, gc);

			return;
		}
	}

	map_push(lm, init_map_pair(v, e, gc));
}

Expr *evaluate_frame(struct StackFrame f_curr, struct CallStack *cs, struct Collector *gc)
{
	Expr *return_value = NULL;
	cs->stack[cs->len - 1].return_addr = NULL;
	cs->stack[cs->len - 1].params_evaluated = 0;
	cs->stack[cs->len - 1].params_available = 0;
	cs->stack[cs->len - 1].function_evaluated = 0;

	cs->len--;

	switch (f_curr.fn->car.type) 
	{
		case Lam:
			if(f_curr.fn->car.data.lam->n_args == f_curr.params_evaluated + f_curr.fn->car.data.lam->n_filled)
			{
				Expr *instructions = f_curr.fn->car.data.lam->instructions;
				return_value = instructions;
				add_fn(instructions, f_curr.return_addr, cs, gc);

				for(int i = 0; i < f_curr.local_references->len; i++)
				{
					// rewriting memory in the same place probably
					map_push(cs->stack[cs->len - 1].local_references, init_map_pair(f_curr.local_references->map[i].v, f_curr.local_references->map[i].e, gc));
				}

				for(int i = 0; i < f_curr.fn->car.data.lam->n_args; i++)
				{
					if(i < f_curr.fn->car.data.lam->n_filled)
					{
						shadow_variables(f_curr.fn->car.data.lam->p_keys[i], f_curr.fn->car.data.lam->params[i], cs->stack[cs->len - 1].local_references, gc);
					}
					else 
					{
						shadow_variables(f_curr.fn->car.data.lam->p_keys[i], f_curr.params[i - f_curr.fn->car.data.lam->n_filled], cs->stack[cs->len - 1].local_references, gc);
					}
					// for(int sf = 0; sf < cs->len; sf++) { if(cs->stack[sf].local_references->len > 30) { printf("HERE\n"); } }
				}
			}
			else 
			{
				for(int i = 0; i < f_curr.params_evaluated; i++)
				{
					f_curr.fn->car.data.lam->params[f_curr.fn->car.data.lam->n_filled] = new_copy(f_curr.params[i], EXCLUDE_CDR, gc);
					f_curr.fn->car.data.lam->n_filled++;
				}

				return_value = f_curr.fn;
			}

			break;
		case Nat:
			for(int i = 0; i < f_curr.params_evaluated; i++)
			{
				f_curr.fn->car.data.nat->params[f_curr.fn->car.data.nat->n_filled] =  new_copy(f_curr.params[i], EXCLUDE_CDR, gc);
				f_curr.fn->car.data.nat->n_filled++;
			}

			if(f_curr.fn->car.data.nat->n_args == f_curr.fn->car.data.nat->n_filled)
			{
				return_value = run_native(*f_curr.fn->car.data.nat, gc);
			}
			else
			{
				return_value = f_curr.fn;
			}

			break;
		case IfE:
			if(f_curr.params[0]->car.type != Fls && f_curr.params[0]->car.type != Nil)
			{
				add_fn(f_curr.fn->car.data.ifE->branch_true, f_curr.return_addr, cs, gc);
			}
			else 
			{
				add_fn(f_curr.fn->car.data.ifE->branch_false, f_curr.return_addr, cs, gc);
			}

			return_value = NULL;

			break;
		case Def:
			return_value = malloc(sizeof(Expr));
			return_value->car.type = Nil;
			return_value->cdr = NULL;
			gc_push(gc, return_value);

			map_push(cs->stack[cs->len - 1].local_references, init_map_pair(f_curr.fn->car.data.str, f_curr.params[0], gc));

			break;
		default:
			return_value = f_curr.fn;

			break;
	}

	for(int i = 0; i < f_curr.local_references->len; i++)
	{
		v_destruct(f_curr.local_references->map[i].v);
	}

	free(f_curr.local_references);

	if(f_curr.params_available > 0)
	{
		free(f_curr.params);
	}


	return return_value;
}

void print_cs(struct CallStack cs)
{
	for(int i = 0; i < cs.len; i++)
	{
		printf("%d (%d)", cs.len - i - 1, cs.stack[cs.len - i - 1].function_evaluated);
		e_print(cs.stack[cs.len - i - 1].fn);
		printf("\t%d / %d params: ", cs.stack[cs.len - i - 1].params_evaluated, cs.stack[cs.len - i - 1].params_available);

		for(int p = 0; p < cs.stack[cs.len - i - 1].params_available; p++)
		{
			e_print(cs.stack[cs.len - i - 1].params[p]);
			printf(" ");
		}

		printf("\t{ ");
		printf("(L = %d) ", cs.stack[cs.len - i -1].local_references->len);

		for(int l = 0; l < cs.stack[cs.len - i - 1].local_references->len; l++)
		{
			v_print(cs.stack[cs.len - i - 1].local_references->map[l].v);
			printf(" => ");
			e_print(cs.stack[cs.len - i - 1].local_references->map[l].e);

			if(l + 1 < cs.stack[cs.len - i - 1].local_references->len)
			{
				printf(", ");
			}
		}

		printf(" }\n");
	}
}

Expr* eval(Expr *e, struct CallStack *cs, struct Collector *gc)
{
	Expr *return_value = NULL;

	if(e->car.type == Lst)
	{
		if(e->car.data.lst)
		{
			add_fn(e->car.data.lst, &return_value, cs, gc);
		}
		else 
		{
			add_fn(e, &return_value, cs, gc);
		}
	}
	else
	{
		add_fn(e, &return_value, cs, gc);
	}

	while(cs->len > 1)
	{
		struct StackFrame *f_curr = &cs->stack[cs->len - 1];

		Expr *e_curr = NULL;
		Expr **return_addr = NULL;

		if(!f_curr->function_evaluated)
		{
			e_curr = f_curr->fn;
			return_addr = &f_curr->fn;
			f_curr->function_evaluated = 1;
		}
		else if(f_curr->params_evaluated < f_curr->params_available)
		{
			e_curr = f_curr->params[f_curr->params_evaluated];
			return_addr = &f_curr->params[f_curr->params_evaluated];
			f_curr->params_evaluated++;
		}
		else
		{
			return_addr = f_curr->return_addr;

			Expr *frame_output = evaluate_frame(*f_curr, cs, gc);

			if(frame_output)
			{
				*return_addr = frame_output;
			}
		}

		if(e_curr)
		{
			if(e_curr->car.type == Lst)
			{
				if(!e_curr->car.data.lst)
				{
					Expr *nil = malloc(sizeof(Expr));
					nil->car.type = Nil;
					nil->cdr = NULL;
					gc_push(gc, nil);

					*return_addr = nil;
				}
				else
				{
					add_fn(e_curr->car.data.lst, return_addr, cs, gc);
				}
			}
			else if(e_curr->car.type == Idr)
			{
				Expr *search = relative_stack_search(e_curr->car.data.str, cs, cs->len - 1);

				if(search)
				{
					*return_addr = new_copy(search, EXCLUDE_CDR, gc);
				}
				else 
				{
					printf("ERROR: Unexpected identifier *");
					v_print(e_curr->car.data.str);
					printf(". Aborting.\n");
					abort();
				}
			}
			else 
			{
				if(e_curr->car.type == Lam) // is closure
				{

					for(int i = 0; i < cs->stack[cs->len - 1].local_references->len; i++)
					{
						map_push(cs->stack[cs->len - 2].local_references, init_map_pair(cs->stack[cs->len - 1].local_references->map[i].v, cs->stack[cs->len - 1].local_references->map[i].e, gc));

					}
				}

				*return_addr = e_curr;
			}
		}
	}

	// print_cs(*cs);
	// printf("\n");

	return return_value;
}

