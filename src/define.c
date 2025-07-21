#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"

// #include "eval.h"
// #include "callstack.h"
//
// Expr *handle_define(struct CallStack *cs)
// {
// 	struct StackFrame frame = cs->stack[cs->len - 1];
// 	// put the variable into the previous stack frame
// 	map_push(cs->stack[cs->len - 2].local_references, init_map_pair(frame.fn->car.data.str, frame.params[0]));
//
// 	Expr *nil = malloc(sizeof(Expr));
// 	nil->car.type = Nil;
// 	nil->cdr = NULL;
//
// 	return nil;
// }

int identify_define(Expr *e)
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

	if(!vec_cmp_str(inner->car.data.str, "define", strlen("define")))
	{
		return 0;
	}

	if(!inner->cdr)
	{
		return 0;
	}

	if(inner->cdr->car.type != Idr)
	{
		return 0;
	}

	if(!inner->cdr->cdr)
	{
		return 0;
	}

	return 1;
}

void create_define(Expr *copy, Expr *orig)
{
	copy->car.type = Lst;
	copy->car.data.lst = malloc(sizeof(Expr));

	copy->car.data.lst->car.type = Def;

	copy->car.data.lst->car.data.str = v_init();
	v_copy(copy->car.data.lst->car.data.str, orig->car.data.lst->cdr->car.data.str);

	copy->car.data.lst->cdr = malloc(sizeof(Expr));
}	

