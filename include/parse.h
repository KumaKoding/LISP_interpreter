#ifndef PARSE_H
#define PARSE_H

#include "lex.h"

typedef struct
{
    enum type node_type;

    int len;
    void *value;
} term;

typedef struct c_expr
{
    enum type node_type;

    char *real_chars;
    int len; 

    union
    {
        struct c_expr *intermediary;
        term *end;
    } *contents;
    
    int node_amount;
} node;

node *parse(struct token *tokens);

#endif