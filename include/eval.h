#ifndef EVAL_H
#define EVAL_H

#include "types.h"

struct StackFrame
{
	Expr *fn;
	int function_evaluated;

	Expr **params;
	int params_available;
	int params_evaluated;

	Expr **return_addr;
};

struct CallStack
{
	int len;
	struct StackFrame stack[MAX_CALL_STACK_SIZE];
};

Expr *handle_lambda(struct StackFrame frame);
int identify_lambda(Expr *e);
Expr *create_lambda(Expr *e);

Expr *run_native(Native function);
Expr *handle_native(struct StackFrame frame);
void init_natives(PairTable *pt);

Expr *handle_ifelse(struct StackFrame frame);
Expr *create_ifelse(Expr *e);
int identify_ifelse(Expr *e);

Expr *handle_define(struct StackFrame frame, PairTable *pt);
Expr *create_define(Expr *e);
int identify_define(Expr *e);


Expr *eval(Expr *e);

#endif
