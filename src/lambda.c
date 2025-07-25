#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "expr.h"

int identify_lambda(Expr *e)
{
	if(!e->car.data.lst)
	{
		return 0;
	}

	Expr *inner = e->car.data.lst;

	if(inner->car.type != Idr)
	{
		return 0;
	}

	if(!vec_cmp_str(inner->car.data.str, "lambda", strlen("lambda")))
	{
		return 0;
	}

	if(!inner->cdr)
	{
		return 0;
	}

	if(inner->cdr->car.type != Lst)
	{
		return 0;
	}

	if(!inner->cdr->cdr)
	{
//
		return 0;
	}

	return 1;
}

void create_lambda(Expr *copy, Expr *orig)
{
	copy->car.type = Lam;
	copy->car.data.lam = malloc(sizeof(Lambda));

	Expr *inner = orig->car.data.lst;

	Expr *arg_ptr = inner->cdr->car.data.lst;
	int n_args = 0;

	while(arg_ptr != NULL)
	{
		n_args++;

		arg_ptr = arg_ptr->cdr;
	}

	copy->car.data.lam->n_filled = 0;
	copy->car.data.lam->n_args = n_args;
	copy->car.data.lam->p_keys = malloc(sizeof(Vector) * n_args);
	copy->car.data.lam->params = malloc(sizeof(Expr *) * n_args);

	arg_ptr = inner->cdr->car.data.lst;

	for(int i = 0; i < n_args; i++)
	{
		if(arg_ptr->car.type != Idr)
		{
			printf("ERROR: Lambda arguments must be type identifier. Aborting.");
			abort();
		}

		copy->car.data.lam->p_keys[i] = v_init();
		v_copy(copy->car.data.lam->p_keys[i], arg_ptr->car.data.str);

		arg_ptr = arg_ptr->cdr;
	}

	copy->car.data.lam->instructions = malloc(sizeof(Expr));
}

