#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "callstack.h"
#include "expr.h"
#include "garbage.h"
#include "my_malloc.h"

#if TESTING
	#define malloc(X) check_malloc(X, __FILE__, __LINE__, __FUNCTION__)
	#define realloc(X, Y) check_realloc(X, Y, __FILE__, __LINE__, __FUNCTION__)
	#define free(X) check_free(X, __FILE__, __LINE__, __FUNCTION__)
#endif

#define IS_NATIVE(function, target) vec_cmp_str(function.key, target, strlen(target))

Expr *run_native(Native function, struct Collector *gc)
{
	Expr *placeholder = malloc(sizeof(Expr));
	placeholder->car.type = Nil;
	placeholder->cdr = NULL;
	gc_push(gc, placeholder);

	if(IS_NATIVE(function, "display"))
	{
		e_print(function.params[0]);
		
		return placeholder;
	}
	else if(IS_NATIVE(function, "+"))
	{
		placeholder->car.type = Num;
		placeholder->car.data.num = function.params[0]->car.data.num + function.params[1]->car.data.num;

		return placeholder;
	}
	else if(IS_NATIVE(function, "-"))
	{
		placeholder->car.type = Num;
		placeholder->car.data.num = function.params[0]->car.data.num - function.params[1]->car.data.num;

		return placeholder;
	}
	else if(IS_NATIVE(function, "*"))
	{
		placeholder->car.type = Num;
		placeholder->car.data.num = function.params[0]->car.data.num * function.params[1]->car.data.num;

		return placeholder;
	}
	else if(IS_NATIVE(function, "/"))
	{
		placeholder->car.type = Num;
		placeholder->car.data.num = function.params[0]->car.data.num / function.params[1]->car.data.num;

		return placeholder;
	}	
	else if(IS_NATIVE(function, "eq"))
	{
		if(function.params[0]->car.data.num == function.params[1]->car.data.num)
		{
			placeholder->car.type = Tru;
		}
		else 
		{
			placeholder->car.type = Fls;
		}

		return placeholder;
	}
	else if(IS_NATIVE(function, "cons"))
	{
		placeholder->car.type = Lst;

		placeholder->car.data.lst = new_copy(function.params[0], EXCLUDE_CDR, gc);
		placeholder->car.data.lst->cdr = new_copy(function.params[1], EXCLUDE_CDR, gc);

		return placeholder;
	}
	else if(IS_NATIVE(function, "car"))
	{
		placeholder = new_copy(function.params[0]->car.data.lst, EXCLUDE_CDR, gc);

		return placeholder;
	}
	else if(IS_NATIVE(function, "cdr"))
	{
		placeholder = new_copy(function.params[0]->car.data.lst->cdr, EXCLUDE_CDR, gc);

		return placeholder;
	}
	else if(IS_NATIVE(function, "is-nil?"))
	{
		if(function.params[0]->car.type == Nil)
		{
			placeholder->car.type = Tru;
		}

		return placeholder;
	}
	else 
	{
		return placeholder;
	}
}

void add_native(char *key, int n_args, struct CallStack *cs, struct Collector *gc)
{
	Expr *nat = malloc(sizeof(Expr));
	nat->car.type = Nat;
	nat->car.data.nat = malloc(sizeof(Native));
	nat->cdr = NULL;

	nat->car.data.nat->key = v_init();
	v_append_str(nat->car.data.nat->key, key, strlen(key));

	nat->car.data.nat->n_args = n_args;
	nat->car.data.nat->n_filled = 0;
	nat->car.data.nat->params = malloc(sizeof(Expr*) * n_args);

	gc_push(gc, nat);
	map_push(cs->stack[0].local_references, init_map_pair(nat->car.data.nat->key, nat, gc));
}

void init_natives(struct CallStack *cs, struct Collector *gc)
{
	add_native("display", 1, cs, gc);
	add_native("+", 2, cs, gc);
	add_native("-", 2, cs, gc);
	add_native("*", 2, cs, gc);
	add_native("/", 2, cs, gc);
	add_native("eq", 2, cs, gc);
	add_native("cons", 2, cs, gc);
	add_native("car", 1, cs, gc);
	add_native("cdr", 1, cs, gc);
	add_native("is-nil?", 1, cs, gc);
}

#undef IS_NATIVE

