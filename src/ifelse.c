#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "types.h"

// #include "eval.h"
// #include "callstack.h"
//
// Expr *handle_ifelse(struct CallStack *cs)
// {
// 	struct StackFrame frame = cs->stack[cs->len - 1];
// 	IfElse *function = frame.fn->car.data.ifE;
//
// 	if(frame.params[0]->car.type != Fls && frame.params[0]->car.type != Nil)
// 	{
// 		return function->branch_true;
// 	}
// 	else 
// 	{
// 		return function->branch_false;
// 	}
// }

void create_ifelse(Expr *copy, Expr *orig)
{
	copy->car.type = Lst;
	copy->car.data.lst = malloc(sizeof(Expr));

	copy->car.data.lst->car.type = IfE;
	copy->car.data.lst->car.data.ifE = malloc(sizeof(IfElse));

	copy->car.data.lst->car.data.ifE->branch_true = malloc(sizeof(Expr));
	copy->car.data.lst->car.data.ifE->branch_false = malloc(sizeof(Expr));

	copy->car.data.lst->cdr = malloc(sizeof(Expr));
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

