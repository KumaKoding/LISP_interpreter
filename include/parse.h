#ifndef PARSE_H
#define PARSE_H

#include "vector.h"
#include "lex.h"

enum expression_type
{
    Num,
    Str,
    Idr,
    Lst,
    Nul
};

struct Expr_list
{
    struct Expr **list;
    int len;
};

struct Expr
{
    enum expression_type curr_type;

    struct Expr *prev_expr;

    union
    {
        int number;
        vector *string;
        struct Expr_list *expressions;
    } content;
};

struct Expr *parse(struct Token_vec *tokens);
void print_ast(struct Expr *ast);

#endif // PARSE_H