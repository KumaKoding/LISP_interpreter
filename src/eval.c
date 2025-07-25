#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "callstack.h"
#include "types.h"

void shadow_variables(Vector *v, Expr *e, LocalMap *lm)
{
	for(int i = 0; i < lm->len; i++)
	{
		if(vec_cmp_vec(v, lm->map[i].v))
		{
			lm->map[i].v = v;
			lm->map[i].e = e;

			return;
		}
	}

	map_push(lm, init_map_pair(v, e));
}

Expr *evaluate_frame(struct StackFrame f_curr, struct CallStack *cs)
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
				add_fn(instructions, f_curr.return_addr, cs);

				for(int i = 0; i < f_curr.local_references->len; i++)
				{
					map_push(cs->stack[cs->len - 1].local_references, f_curr.local_references->map[i]);
				}

				for(int i = 0; i < f_curr.fn->car.data.lam->n_args; i++)
				{
					if(i < f_curr.fn->car.data.lam->n_filled)
					{
						shadow_variables(f_curr.fn->car.data.lam->p_keys[i], f_curr.fn->car.data.lam->params[i], cs->stack[cs->len - 1].local_references);
					}
					else 
					{
						shadow_variables(f_curr.fn->car.data.lam->p_keys[i], f_curr.params[i - f_curr.fn->car.data.lam->n_filled], cs->stack[cs->len - 1].local_references);
					}
				}
			}
			else 
			{
				for(int i = 0; i < f_curr.params_evaluated; i++)
				{
					f_curr.fn->car.data.lam->params[f_curr.fn->car.data.lam->n_filled] = f_curr.params[i];
					f_curr.fn->car.data.lam->n_filled++;
				}

				return_value = f_curr.fn;
			}

			break;
		case Nat:
			for(int i = 0; i < f_curr.params_evaluated; i++)
			{
				f_curr.fn->car.data.nat->params[f_curr.fn->car.data.nat->n_filled] = f_curr.params[i];
				f_curr.fn->car.data.nat->n_filled++;
			}

			if(f_curr.fn->car.data.nat->n_args == f_curr.fn->car.data.nat->n_filled)
			{
				return_value = run_native(*f_curr.fn->car.data.nat);
			}
			else
			{
				return_value = f_curr.fn;
			}

			break;
		case IfE:
			if(f_curr.params[0]->car.type != Fls && f_curr.params[0]->car.type != Nil)
			{
				add_fn(f_curr.fn->car.data.ifE->branch_true, f_curr.return_addr, cs);
			}
			else 
			{
				add_fn(f_curr.fn->car.data.ifE->branch_false, f_curr.return_addr, cs);
			}

			return_value = NULL;

			break;
		case Def:
			return_value = malloc(sizeof(Expr));
			return_value->car.type = Nil;
			return_value->cdr = NULL;

			map_push(cs->stack[cs->len - 1].local_references, init_map_pair(f_curr.fn->car.data.str, f_curr.params[0]));

			break;
		default:
			return_value = f_curr.fn;

			break;
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
		e_print(new_copy(cs.stack[cs.len - i - 1].fn, NO_REPLACE, EXCLUDE_CDR));
		printf("\t%d / %d params: ", cs.stack[cs.len - i - 1].params_evaluated, cs.stack[cs.len - i - 1].params_available);

		for(int p = 0; p < cs.stack[cs.len - i - 1].params_available; p++)
		{
			e_print(new_copy(cs.stack[cs.len - i - 1].params[p], NO_REPLACE, EXCLUDE_CDR));
			printf(" ");
		}

		printf("\t{ ");

		for(int l = 0; l < cs.stack[cs.len - i - 1].local_references->len; l++)
		{
			v_print(cs.stack[cs.len - i - 1].local_references->map[l].v);
			printf(" => ");
			e_print(new_copy(cs.stack[cs.len - i - 1].local_references->map[l].e, NO_REPLACE, EXCLUDE_CDR));

			if(l + 1 < cs.stack[cs.len - i - 1].local_references->len)
			{
				printf(", ");
			}
		}

		printf(" }\n");
	}
}

Expr* eval(Expr *e, struct CallStack *cs)
{
	Expr *return_value = NULL;

	if(e->car.type == Lst)
	{
		if(e->car.data.lst)
		{
			add_fn(e->car.data.lst, &return_value, cs);
		}
		else 
		{
			add_fn(e, &return_value, cs);
		}
	}
	else
	{
		add_fn(e, &return_value, cs);
	}

	int cycles = 0;

	while(cs->len > 1)
	{
		cycles++;
		// print_cs(*cs);
		// printf("\n");

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

			Expr *frame_output = evaluate_frame(*f_curr, cs);

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

					*return_addr = nil;
				}
				else
				{
					add_fn(e_curr->car.data.lst, return_addr, cs);
				}
			}
			else if(e_curr->car.type == Idr)
			{
				Expr *search = relative_stack_search(e_curr->car.data.str, cs, cs->len - 1);

				if(search)
				{
					*return_addr = new_copy(search, NO_REPLACE, EXCLUDE_CDR);
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
						map_push(cs->stack[cs->len - 2].local_references, cs->stack[cs->len - 1].local_references->map[i]);
					}
				}

				*return_addr = e_curr;
			}
		}
	}

	printf("CYCLES = %d\n", cycles);

	return return_value;
}

