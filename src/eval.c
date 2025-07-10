#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "types.h"

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
	cs->stack[cs->len].return_addr = return_addr;

	cs->stack[cs->len].fn = NULL;
	cs->stack[cs->len].function_evaluated = 0;

	cs->len++;
}

void add_fn(Expr *e, Expr **return_addr, struct CallStack *cs)
{
	if(!e)
	{
		printf("ERROR: NULL expression cannot be turned into a function. Aborting.");
		abort();
	}

	int n_params = count_exprs(e) - 1;

	cs_push(n_params, return_addr, cs);

	cs->stack[cs->len - 1].fn = e;

	Expr *e_curr = e->cdr;

	for(int i = 0; i < n_params; i++)
	{
		cs->stack[cs->len - 1].params[i] = e_curr;

		e_curr = e_curr->cdr;
	}
}

Expr *evaluate_frame(struct StackFrame f_curr, struct CallStack *cs, PairTable *pt)
{
	Expr *return_value = NULL;

	cs->stack[cs->len - 1].return_addr = NULL;
	cs->stack[cs->len - 1].params_evaluated = 0;
	cs->stack[cs->len - 1].params_available = 0;
	cs->stack[cs->len - 1].function_evaluated = 0;

	if(cs->stack[cs->len - 1].params_available > 0)
	{
		free(cs->stack[cs->len - 1].params);
	}
	
	cs->len--;

	switch (f_curr.fn->car.type) 
	{
		case Lam:
			return_value = handle_lambda(f_curr);

			if(return_value->car.type == Lst)
			{
				add_fn(return_value, f_curr.return_addr, cs);
			}

			break;
		case Nat:
			return_value = handle_native(f_curr);

			if(return_value->car.type == Lst)
			{
				add_fn(return_value, f_curr.return_addr, cs);
			}

			break;
		case IfE:
			return_value = handle_ifelse(f_curr);

			if(return_value->car.type == Lst)
			{
				add_fn(return_value, f_curr.return_addr, cs);
			}
			
			break;
		case Def:
			return_value = handle_define(f_curr, pt);

			break;
		default:
			return_value = f_curr.fn;

			break;
	}

	return return_value;
}

void print_cs(struct CallStack cs)
{
	for(int i = cs.len - 1; i >= 0; i--)
	{
		printf("%d\t", i);
		printf("(%d) fn: ", cs.stack[i].function_evaluated);
		Expr *fn_copy = new_copy(cs.stack[i].fn, NO_REPLACE, EXCLUDE_CDR);
		e_print(fn_copy);
		printf("\t");
		printf("%d / %d params: ", cs.stack[i].params_evaluated, cs.stack[i].params_available);
		
		for(int p = 0; p < cs.stack[i].params_available; p++)
		{
			Expr *param_copy = new_copy(cs.stack[i].params[p], NO_REPLACE, EXCLUDE_CDR);
			e_print(param_copy);
			printf(" ");
		}

		printf("\n");
	}
}

int init_stack(Expr *e, struct CallStack *cs, Expr **return_addr)
{
	Expr *e_curr = e;
	int n_exprs = count_exprs(e);

	if(n_exprs == 0)
	{
		return 0;
	}

	Expr **reverse = malloc(sizeof(Expr *) * n_exprs);

	for(int i = 0; i < n_exprs; i++)
	{
		reverse[n_exprs - i - 1] = e_curr;

		e_curr = e_curr->cdr;
	}

	for(int i = 0; i < n_exprs; i++)
	{
		cs_push(0, return_addr, cs);
		cs->stack[cs->len - 1].fn = reverse[i];
	}

	return 1;
}

Expr* eval(Expr *e)
{
	PairTable *pt = pt_init();
	struct CallStack cs;

	cs.len = 0;

	Expr *return_value;

	init_natives(pt);

	if(!init_stack(e, &cs, &return_value))
	{
		printf("ERROR: Failure to evaluate 0 exprs. Aborting.\n");
		abort();
	}

	while(cs.len > 0)
	{
		// print_cs(cs);
		// printf("\n");
		struct StackFrame *f_curr = &cs.stack[cs.len - 1];

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
			*return_addr = evaluate_frame(*f_curr, &cs, pt);
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
				else if(identify_lambda(e_curr))
				{
					*return_addr = create_lambda(e_curr);
				}
				else if(identify_define(e_curr))
				{
					cs_push(1, return_addr, &cs);
					cs.stack[cs.len - 1].fn = create_define(e_curr);
					cs.stack[cs.len - 1].params[0] = e_curr->car.data.lst->cdr->cdr;

					cs.stack[cs.len - 1].function_evaluated = 1;
				}
				else 
				{	
					add_fn(e_curr->car.data.lst, return_addr, &cs);
				}
			}
			else if(e_curr->car.type == Idr)
			{
				Pair *search = pt_find(pt, e_curr->car.data.str);

				if(search)
				{
					*return_addr = new_copy(search->instructions, NO_REPLACE, EXCLUDE_CDR);
				}
				else 
				{
					printf("ERROR: Unexpected identifier. Aborting.");
					abort();
				}
			}
			else 
			{
				*return_addr = e_curr;
			}
		}
	}

	return return_value;
}

