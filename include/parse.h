#ifndef PARSE_H
#define PARSE_H

#include "vector.h"
#include "lex.h"

struct Expr_list
{
    struct Expr **list;
    int len;
};

struct Expr
{
    int curr_type;

    struct Expr *prev_expr;
    
    union
    {
        int number;
        vector *string;
        struct Expr_list *expressions;
    } content;
};

struct Expr *parse(struct Token_vec *tokens);

#endif // PARSE_H