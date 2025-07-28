#ifndef GARBAGE_H
#define GARBAGE_H

#include <stdint.h>

typedef struct Expr Expr;
struct CallStack;

struct ExprVector
{
	Expr **exprs;
	int n_exprs;
	int max;
};

struct Collector 
{
	int allocs;
	struct CallStack *roots;
	struct ExprVector e_vec;
	uint8_t *marks;
};

struct Collector init_gc(struct CallStack *cs);
void gc_push(struct Collector *gc, Expr *e);
void mark(struct Collector *gc);
void sweep(struct Collector *gc);

#endif
