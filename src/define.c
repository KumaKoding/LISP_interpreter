#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "types.h"

Expr *handle_define(struct StackFrame frame, PairTable *pt)
{
	pt_insert(pt, frame.fn->car.data.str, frame.params[0]);

	Expr *nil = malloc(sizeof(Expr));
	nil->car.type = Nil;
	nil->cdr = NULL;

	return nil;
}

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

Expr *create_define(Expr *e)
{
	Expr *new_define = malloc(sizeof(Expr));
	new_define->car.type = Def;
	new_define->cdr = NULL;

	new_define->car.data.str = v_init();
	v_copy(new_define->car.data.str, e->car.data.lst->cdr->car.data.str);

	return new_define;
}	

