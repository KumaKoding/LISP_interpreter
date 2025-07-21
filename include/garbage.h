#ifndef GARB_H
#define GARB_H

#include "callstack.h"
#include "types.h"

#define INITIAL_EXPR_VECTOR_SIZE 32

struct ExprVector
{
	Expr **exprs;
	int n_exprs;
	int max;
};

struct Collector 
{
	struct CallStack *roots;
	struct ExprVector e_vec;
};

void mark(struct Collector *gc);
void sweep(struct Collector *gc);

#endif
