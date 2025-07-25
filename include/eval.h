#ifndef EVAL_H
#define EVAL_H

#include "expr.h"
#include "callstack.h"

#define DEFINE_ARGS 1
#define IFELSE_ARGS 1

Expr *run_native(Native function, struct Collector *gc);
void init_natives(struct CallStack *cs, struct Collector *gc);

Expr *eval(Expr *e, struct CallStack *cs, struct Collector *gc);

#endif
