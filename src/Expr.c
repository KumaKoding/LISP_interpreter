#include <stdio.h>
#include <stdlib.h>

#include "expr.h"
#include "callstack.h"
#include "garbage.h"
#include "my_malloc.h"

#if TESTING
	#define malloc(X) check_malloc(X, __FILE__, __LINE__, __FUNCTION__)
	#define realloc(X, Y) check_realloc(X, Y, __FILE__, __LINE__, __FUNCTION__)
	#define free(X) check_free(X, __FILE__, __LINE__, __FUNCTION__)
#endif

#define INITIAL_EXPR_VECTOR_SIZE 32

struct ExprVector *init_e_vec()
{
	struct ExprVector *e_vec = malloc(sizeof(struct ExprVector));

	e_vec->max = INITIAL_EXPR_VECTOR_SIZE;
	e_vec->exprs = malloc(sizeof(Expr *) * INITIAL_EXPR_VECTOR_SIZE);
	e_vec->n_exprs = 0;

	return e_vec;
}

void e_vec_push(struct ExprVector *e_vec, Expr *e)
{
	if(e_vec->n_exprs == e_vec->max)
	{
		e_vec->exprs = realloc(e_vec->exprs, sizeof(Expr *) * e_vec->max * 2);
		e_vec->max *= 2;
	}	

	e_vec->exprs[e_vec->n_exprs] = e;
	e_vec->n_exprs++;
}

void es_push(struct ExprStack *es, Expr *e)
{
    if(es->len >= MAX_EXPR_STACK_SIZE)
    {
        printf("ERROR: Stack has exceeded maximum depth");
        abort();
    }

    es->stack[es->len] = e;
    es->len++;
}

Expr *es_pop(struct ExprStack *es)
{
    if(es->len <= 0)
    {
        printf("ERROR: Expected item on stack, unable");
    }

    Expr *e = es->stack[es->len - 1];
    es->stack[es->len - 1] = NULL;
    es->len--;

    return e;
}

struct stackPairs
{
	struct ExprStack orig;
	struct ExprStack copy;
};

void copy_single_expr(Expr *orig, Expr *copy, struct stackPairs *stacks, struct Collector *gc)
{
	gc_push(gc, copy);

	switch (orig->car.type) {
		case Num:
			copy->car.type = Num;
			copy->car.data.num = orig->car.data.num;
			
			break;
		case Str:
			copy->car.type = Str;
			copy->car.data.str = v_init();

			v_copy(copy->car.data.str, orig->car.data.str);

			break;
		case Idr:
		{
			copy->car.type = Idr;
			copy->car.data.str = v_init();

			v_copy(copy->car.data.str, orig->car.data.str);

			break;
		}
		case Lam:
			copy->car.type = Lam;
			copy->car.data.lam = malloc(sizeof(Lambda));
			copy->car.data.lam->n_args = orig->car.data.lam->n_args;
			copy->car.data.lam->n_filled = orig->car.data.lam->n_filled;
			copy->car.data.lam->p_keys = malloc(sizeof(Vector) * copy->car.data.lam->n_args);
			copy->car.data.lam->params = malloc(sizeof(Expr *) * copy->car.data.lam->n_args);

			for (int i = 0; i < orig->car.data.lam->n_args; i++)
			{
				copy->car.data.lam->p_keys[i] = v_init();
				v_copy(copy->car.data.lam->p_keys[i], orig->car.data.lam->p_keys[i]);

				if(i < orig->car.data.lam->n_filled)
				{
					copy->car.data.lam->params[i] = malloc(sizeof(Expr));

					es_push(&stacks->orig, orig->car.data.lam->params[i]);
					es_push(&stacks->copy, copy->car.data.lam->params[i]);
				}
			}

			copy->car.data.lam->instructions = malloc(sizeof(Expr));

			es_push(&stacks->orig, orig->car.data.lam->instructions);
			es_push(&stacks->copy, copy->car.data.lam->instructions);

			break;
		case IfE:
			copy->car.type = IfE;
			copy->car.data.ifE = malloc(sizeof(IfElse));

			copy->car.data.ifE->branch_true = malloc(sizeof(Expr));
			copy->car.data.ifE->branch_false = malloc(sizeof(Expr));

			es_push(&stacks->orig, orig->car.data.ifE->branch_true);
			es_push(&stacks->copy, copy->car.data.ifE->branch_true);
			es_push(&stacks->orig, orig->car.data.ifE->branch_false);
			es_push(&stacks->copy, copy->car.data.ifE->branch_false);
			break;
		case Nat:
			copy->car.type = Nat;
			copy->car.data.nat = malloc(sizeof(Native));
			copy->car.data.nat->n_args = orig->car.data.nat->n_args;
			copy->car.data.nat->n_filled = orig->car.data.nat->n_filled;

			copy->car.data.nat->key = v_init();
			v_copy(copy->car.data.nat->key, orig->car.data.nat->key);

			if(orig->car.data.nat->params)
			{
				copy->car.data.nat->params = malloc(sizeof(Expr) * orig->car.data.nat->n_args);

				for(int i = 0; i < orig->car.data.nat->n_filled; i++)
				{
					copy->car.data.nat->params[i] = malloc(sizeof(Expr));

					es_push(&stacks->orig, orig->car.data.nat->params[i]);
					es_push(&stacks->copy, copy->car.data.nat->params[i]);
				}
			}
			else 
			{
				copy->car.data.nat->params = NULL;
			}

			break;
		case Def:
			copy->car.type = Def;

			copy->car.data.str = v_init();
			v_copy(copy->car.data.str, orig->car.data.str);

			break;
		case Lst:
			copy->car.type = Lst;

			if(orig->car.data.lst)
			{
				copy->car.data.lst = malloc(sizeof(Expr));
				es_push(&stacks->orig, orig->car.data.lst);
				es_push(&stacks->copy, copy->car.data.lst);
			}
			else
			{
				copy->car.data.lst = NULL;
			}

			break;
		case Nil:
			copy->car.type = Nil;

			break;
		case Tru:
			copy->car.type = Tru;
			
			break;
		case Fls:
			copy->car.type = Fls;

			break;
		default:
			break;
	}
}

Expr *new_copy(Expr *e, int CDR_OPTION, struct Collector *gc)
{
	struct stackPairs stack_pair;

	stack_pair.copy.len = 0;
	stack_pair.orig.len = 0;

	Expr *final_copy = malloc(sizeof(Expr));
	final_copy->cdr = NULL;

	Expr *orig = e;
	Expr *copy = final_copy;

	if(!e)
	{
		return NULL;
	}

	if(CDR_OPTION == EXCLUDE_CDR)
	{
		copy_single_expr(orig, copy, &stack_pair, gc);
	}
	else if(CDR_OPTION == INCLUDE_CDR)
	{
		es_push(&stack_pair.orig, e);
		es_push(&stack_pair.copy, copy);
	}
	else 
	{
		printf("not an option for copy\n");
	}


	while(stack_pair.copy.len > 0)
	{
		orig = es_pop(&stack_pair.orig);
		copy = es_pop(&stack_pair.copy);

		// TODO: ensure that .cdr is always NULL

		while(orig != NULL)
		{
			copy_single_expr(orig, copy, &stack_pair, gc);

			if(orig->cdr != NULL)
			{
				copy->cdr = malloc(sizeof(Expr));
				copy = copy->cdr;
				copy->cdr = NULL;
			}

			orig = orig->cdr;
		}
	}

	return final_copy;
}

void destroy_single_expr(Expr *e, struct ExprStack *trace)
{
	// e_print(e);
	// if(e->car.type == Lst)
	// {
	// 	printf(" - %p [%p]\n", e, e->car.data.lst);
	// }
	// else 
	// {
	// 	printf(" - %p\n", e);
	// }
	//
	switch(e->car.type)
	{
		case Num:
			break;
		case Str:
			v_destruct(e->car.data.str);
			break;
		case Idr:
			v_destruct(e->car.data.str);
			break;
		case Lam:
			for(int i = 0; i < e->car.data.lam->n_args; i++)
			{
				v_destruct(e->car.data.lam->p_keys[i]);

				if(i < e->car.data.lam->n_filled)
				{
					es_push(trace, e->car.data.lam->params[i]);
				}
			}

			es_push(trace, e->car.data.lam->instructions);

			free(e->car.data.lam->p_keys);
			free(e->car.data.lam->params);
			free(e->car.data.lam);
			break;
		case IfE:
			es_push(trace, e->car.data.ifE->branch_true);
			es_push(trace, e->car.data.ifE->branch_false);

			free(e->car.data.ifE);
			break;
		case Nat:
			v_destruct(e->car.data.nat->key);

			if(e->car.data.nat->params)
			{
				for(int i = 0; i < e->car.data.lam->n_args; i++)
				{
					es_push(trace, e->car.data.nat->params[i]);
				}
			}

			free(e->car.data.nat->params);
			free(e->car.data.nat);
			break;
		case Def:
			v_destruct(e->car.data.str);
			break;
		case Lst:
			es_push(trace, e->car.data.lst);
			break;
		case Nil:
			break;
		case Tru:
			break;
		case Fls:
			break;
		default:
			break;
	}

	free(e);
}

void new_destruct(Expr *expr, int CDR_OPTION)
{
	if(!expr)
	{
		return;
	}

	struct ExprStack trace;
	trace.len = 0;

	if(CDR_OPTION == EXCLUDE_CDR)
	{
		destroy_single_expr(expr, &trace);
	}
	else if(CDR_OPTION == INCLUDE_CDR)
	{
		es_push(&trace, expr);
	}
	else 
	{
		printf("Not an option for destruct.\n");
	}

	while(trace.len > 0)
	{
		Expr *e_curr = es_pop(&trace);

		while(e_curr != NULL)
		{
			Expr *next = e_curr->cdr;
			
			destroy_single_expr(e_curr, &trace);

			e_curr = next;
		}
	}
}

void e_print(Expr *expr)
{
    Expr *curr = expr;

    while (curr != NULL)
    {
        switch (curr->car.type)
        {
        case Num:
            printf("%d", curr->car.data.num);
            break;
        case Str:
            printf("\"");
            v_print(curr->car.data.str);
            printf("\"");
            break;
        case Idr:
            printf("*");
            v_print(curr->car.data.str);
            break;
        case Lam:
            printf("(");
            for (int i = 0; i < curr->car.data.lam->n_args; i++)
            {
				if(i < curr->car.data.lam->n_filled)
				{
					e_print(curr->car.data.lam->params[i]);
				}
				else 
				{	
					v_print(curr->car.data.lam->p_keys[i]);
				}

                if (i < curr->car.data.lam->n_args - 1)
                {
                    printf(" ");
                }
            }
            printf(") -> ");
            e_print(curr->car.data.lam->instructions);
            break;
		case IfE:
			printf("if { ");
			e_print(curr->car.data.ifE->branch_true);
			printf(" else ");
			e_print(curr->car.data.ifE->branch_false);
			printf(" }");
			break;
        case Nat:
            printf("__");
            v_print(curr->car.data.nat->key);
            printf("__");
            printf("(");
            for (int i = 0; i < curr->car.data.nat->n_args; i++)
            {
				if(curr->car.data.nat->params)
				{
					if(curr->car.data.nat->params[i])
					{
						e_print(curr->car.data.nat->params[i]);
					}
					else
					{	
						printf("%c", ('a' + i));
					}
				}
				else 
				{
					printf("%c", ('a' + i));
				}

                if (i < curr->car.data.nat->n_args - 1)
                {
                    printf(" ");
                }
            }
            printf(")");
            break;
		case Def:
			printf("__define (");
			v_print(curr->car.data.str);
			printf(")__");
			break;
        case Lst:
            printf("(");
            e_print(curr->car.data.lst);
            printf(")");
            break;
        case Nil:
            printf("nil");
            break;
		case Tru:
			printf("#t");
			break;
		case Fls:
			printf("#f");

        default:
            break;
        }

		if (curr->cdr)
		{
			// if(curr->cdr->cdr == NULL && curr->cdr->car.type != Nil)
			// {
			// 	printf(" .");
			// }
			printf(" ");
		}

        curr = curr->cdr;
    }
}
