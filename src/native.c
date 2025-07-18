#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "environment.h"
#include "types.h"

#define IS_NATIVE(function, target) vec_cmp_str(function.key, target, strlen(target))

Expr *run_native(Native function)
{
	Expr *placeholder = malloc(sizeof(Expr));
	placeholder->car.type = Nil;
	placeholder->cdr = NULL;

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

		placeholder->car.data.lst = new_copy(function.params[0], NO_REPLACE, EXCLUDE_CDR);
		placeholder->car.data.lst->cdr = new_copy(function.params[1], NO_REPLACE, EXCLUDE_CDR);

		return placeholder;
	}
	else if(IS_NATIVE(function, "car"))
	{
		placeholder = function.params[0]->car.data.lst;

		return placeholder;
	}
	else if(IS_NATIVE(function, "cdr"))
	{
		placeholder = function.params[0]->car.data.lst->cdr;

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

Expr *handle_native(struct StackFrame frame)
{
	Native *function = frame.fn->car.data.nat;

	if(function->n_args < function->n_filled + frame.params_evaluated)
	{
		printf("ERROR: Too many parameters applied to native function. Aborting.\n");
		abort();
	}

	for(int i = 0; i < frame.params_evaluated; i++)
	{
		function->params[function->n_filled] = frame.params[i];
		function->n_filled++;
	}

	if(function->n_args == function->n_filled)
	{
		return run_native(*function);
	}
	else 
	{
		return frame.fn;
	}
}

void add_native(char *key, int n_args, Environment *env)
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

	map_push(&env->env[env->depth], init_map_pair(nat->car.data.nat->key, nat));
}

void init_natives(Environment *env)
{
	add_native("display", 1, env);
	add_native("+", 2, env);
	add_native("-", 2, env);
	add_native("*", 2, env);
	add_native("/", 2, env);
	add_native("eq", 2, env);
	add_native("cons", 2, env);
	add_native("car", 1, env);
	add_native("cdr", 1, env);
	add_native("is-nil?", 1, env);
}
#undef IS_NATIVE

