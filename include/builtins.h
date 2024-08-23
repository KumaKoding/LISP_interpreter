#ifndef BUILTINS_H
#define BUILTINS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"

#define BI_PRINT "println"
#define BI_ADD "add"

#define PRINT_ARGS 1
#define ADD_ARGS 2

enum expression_type;

struct Hashtable;

int match_to_builtin(vector *v);
struct Expr *exec_builtin(vector *key, int n_params, struct Expr **params);
void add_builtins(struct Hashtable *ht);

#endif