#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "vector.h"
#include "builtins.h"

struct Function
{
    int n_params;
    struct Item **params;
    struct Expr *instructions;
};

struct Item
{
    vector *key;
    char type; // 'f' or 'v'

    union 
    {
        struct Expr *expr;
        struct Function *func;
    };
};

struct Hashtable 
{
    struct Item **items;
    int len;
    int max;
};

struct Hashtable *init(int initial_size);
void insert(struct Hashtable *ht, struct Item *i);
struct Item *create_expr_item(vector *key, struct Expr *expr);
struct Item *create_func_item(vector *key, struct Function *func);
int eval(struct Expr *exprs);

#endif