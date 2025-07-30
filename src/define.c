#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"
#include "parse.h"
#include "my_malloc.h"

#if TESTING
	#define malloc(X) check_malloc(X, __FILE__, __LINE__, __FUNCTION__)
	#define realloc(X, Y) check_realloc(X, Y, __FILE__, __LINE__, __FUNCTION__)
	#define free(X) check_free(X, __FILE__, __LINE__, __FUNCTION__)
#endif


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

