#ifndef PARSE_H
#define PARSE_H

#include "lex.h"

typedef struct c_expr
{
    enum type node_type;

    char *real_chars;
    int len; 

    struct c_expr *leaf_1;
    struct c_expr *leaf_2;
} node;

node *parse(struct token *tokens);

#endif