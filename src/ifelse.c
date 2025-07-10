#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "types.h"

Expr *handle_ifelse(struct StackFrame frame)
{
	IfElse *function = frame.fn->car.data.ifE;

	if(frame.params[0]->car.type != Fls)
	{
		return function->branch_true;
	}
	else 
	{
		return function->branch_false;
	}
}

Expr *create_ifelse(Expr *e)
{
	Expr *new_ifelse = malloc(sizeof(Expr));
	new_ifelse->cdr = NULL;

	Expr *inner = e->car.data.lst;
}

int identify_ifelse(Expr *e)
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

	if(!vec_cmp_str(inner->car.data.str, "if", strlen("if")))
	{
		return 0;
	}

	if(!inner->cdr)
	{
		return 0;
	}

	if(!inner->cdr->cdr)
	{
		return 0;
	}

	if(!inner->cdr->cdr->cdr)
	{
		return 0;
	}

	return 1;
}

