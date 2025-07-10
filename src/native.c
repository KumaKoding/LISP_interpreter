#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"
#include "types.h"

#define IS_NATIVE(function, target) vec_cmp_str(function.key, target, strlen(target))

Expr *run_native(Native function)
{
	Expr *placeholder = malloc(sizeof(Expr));
	placeholder->car.type = Nil;
	placeholder->cdr = NULL;

	if(IS_NATIVE(function, "print"))
	{
		e_print(function.params[0]);
		printf("\n");
		
		return placeholder;
	}
	else if(IS_NATIVE(function, "add"))
	{
		placeholder->car.type = Num;
		placeholder->car.data.num = function.params[0]->car.data.num + function.params[1]->car.data.num;

		return placeholder;
	}
	else if(IS_NATIVE(function, "sub"))
	{
		placeholder->car.type = Num;
		placeholder->car.data.num = function.params[0]->car.data.num - function.params[1]->car.data.num;

		return placeholder;
	}
	else if(IS_NATIVE(function, "mul"))
	{
		placeholder->car.type = Num;
		placeholder->car.data.num = function.params[0]->car.data.num * function.params[1]->car.data.num;

		return placeholder;
	}
	else if(IS_NATIVE(function, "div"))
	{
		placeholder->car.type = Num;
		placeholder->car.data.num = function.params[0]->car.data.num / function.params[1]->car.data.num;

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

void add_native(char *key, int n_args, PairTable *pt)
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

	pt_insert(pt, nat->car.data.nat->key, nat);
}

void init_natives(PairTable *pt)
{
	add_native("print", 1, pt);
	add_native("add", 2, pt);
	add_native("sub", 2, pt);
	add_native("mul", 2, pt);
	add_native("div", 2, pt);
}

#undef IS_NATIVE

