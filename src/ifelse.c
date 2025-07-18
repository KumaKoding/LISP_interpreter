#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "types.h"

Expr *handle_ifelse(struct StackFrame frame)
{
	IfElse *function = frame.fn->car.data.ifE;

	if(frame.params[0]->car.type != Fls && frame.params[0]->car.type != Nil)
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
	new_ifelse->car.type = IfE;
	new_ifelse->car.data.ifE = malloc(sizeof(IfElse));

	Expr *inner = e->car.data.lst;

	new_ifelse->car.data.ifE->branch_true = new_copy(inner->cdr->cdr, NO_REPLACE, EXCLUDE_CDR);
	new_ifelse->car.data.ifE->branch_false = new_copy(inner->cdr->cdr->cdr, NO_REPLACE, EXCLUDE_CDR);

	return new_ifelse;
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

