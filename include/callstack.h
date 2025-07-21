#ifndef CALLSTACK_H
#define CALLSTACK_H

#include "types.h"

#define MAX_ENV_DEPTH 256
#define MAX_MAP_SIZE 512

typedef struct LocalMap LocalMap;
typedef struct MapPair MapPair;

struct StackFrame
{
	Expr *fn;
	int function_evaluated;

	Expr **params;
	int params_available;
	int params_evaluated;

	LocalMap *local_references;

	Expr **return_addr;
};

struct CallStack
{
	int len;
	struct StackFrame stack[MAX_CALL_STACK_SIZE];
};

struct MapPair
{
	Vector *v;
	Expr *e;
};

struct LocalMap
{
	MapPair map[MAX_MAP_SIZE];
	int len;
};

void cs_push(int n_params, Expr **return_addr, struct CallStack *cs);
void add_fn(Expr *e, Expr **return_addr, struct CallStack *cs);
Expr *evaluate_frame(struct StackFrame f_curr, struct CallStack *cs);

void map_push(LocalMap *lm, MapPair mp);
LocalMap *init_map();
MapPair init_map_pair(Vector *v, Expr *e);
Expr *relative_stack_search(Vector *key, struct CallStack *cs, int idx);

#endif
