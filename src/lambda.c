#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "types.h"

Expr *handle_lambda(struct StackFrame frame)
{
	Lambda *function = frame.fn->car.data.lam;
	
	if(function->n_args == frame.params_evaluated)
	{
		return new_copy(function->instructions, REPLACE(function->p_keys, frame.params, frame.params_evaluated), EXCLUDE_CDR);
	}
	else if(function->n_args > frame.params_evaluated)
	{
		Expr *new_lambda = malloc(sizeof(Expr));
		new_lambda->car.type = Lam;

		new_lambda->car.data.lam = malloc(sizeof(Lambda));
		new_lambda->car.data.lam->p_keys = malloc(sizeof(Vector) * (function->n_args - frame.params_evaluated));
		new_lambda->car.data.lam->instructions = new_copy(function->instructions, REPLACE(function->p_keys, frame.params, frame.params_evaluated), EXCLUDE_CDR);

		new_lambda->car.data.lam->n_args = function->n_args - frame.params_evaluated;

		for(int i = 0; i < new_lambda->car.data.lam->n_args; i++)
		{
			new_lambda->car.data.lam->p_keys[i] = v_init();
			v_copy(new_lambda->car.data.lam->p_keys[i], function->p_keys[frame.params_evaluated + i]);
		}

		return new_lambda;
	}
	else 
	{
		printf("ERROR: Too many parameters applied to lambda function. Aborting.");
		abort();
	}
}

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
		return 0;
	}

	return 1;
}

Expr *create_lambda(Expr *e)
{
	Expr *new_lambda = malloc(sizeof(Expr));
	new_lambda->car.type = Lam;
	new_lambda->car.data.lam = malloc(sizeof(Lambda));

	Expr *inner = e->car.data.lst;

	Expr *arg_ptr = inner->cdr->car.data.lst;
	int n_args = 0;

	while(arg_ptr != NULL)
	{
		n_args++;

		arg_ptr = arg_ptr->cdr;
	}

	new_lambda->car.data.lam->n_args = n_args;
	new_lambda->car.data.lam->p_keys = malloc(sizeof(Vector) * n_args);

	arg_ptr = inner->cdr->car.data.lst;

	for(int i = 0; i < n_args; i++)
	{
		if(arg_ptr->car.type != Idr)
		{
			printf("ERROR: Lambda arguments must be type identifier. Aborting.");
			abort();
		}

		new_lambda->car.data.lam->p_keys[i] = v_init();
		v_copy(new_lambda->car.data.lam->p_keys[i], arg_ptr->car.data.str);

		arg_ptr = arg_ptr->cdr;
	}

	new_lambda->car.data.lam->instructions = new_copy(inner->cdr->cdr, NO_REPLACE, EXCLUDE_CDR);

	return new_lambda;
}

