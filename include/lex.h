#ifndef LEX_H
#define LEX_H

#include "vector.h"

enum type
{
    O_paren,
    C_paren,
    Space,
    Number,
    Identifier,
    String,
    End
};

struct Token_vec
{
    enum type *toks;
    vector **strs;
    
    int len, max;
};

struct Token_vec *lex(vector *input);
int destruct_tokens(struct Token_vec *tokens);

#endif