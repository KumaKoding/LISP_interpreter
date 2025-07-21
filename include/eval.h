#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include "callstack.h"

#define DEFINE_ARGS 1
#define IFELSE_ARGS 1

// Expr *handle_lambda(struct CallStack *cs);
// int identify_lambda(Expr *e);
// Expr *create_lambda(Expr *e);

Expr *handle_native(struct CallStack *cs);
Expr *run_native(Native function);
void init_natives(struct CallStack *cs);

// Expr *handle_ifelse(struct CallStack *cs);
// Expr *create_ifelse(Expr *e);
// int identify_ifelse(Expr *e);
//
// Expr *handle_define(struct CallStack *cs);
// Expr *create_define(Expr *e);
// int identify_define(Expr *e);

Expr *eval(Expr *e, struct CallStack *cs);

#endif
