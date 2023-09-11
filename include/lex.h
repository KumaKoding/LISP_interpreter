#ifndef LEX_H
#define LEX_H

#include "vector.h"

enum type
{
    O_paren,
    C_paren,
    Space,
    Number,
    Add,
    Sub,
    Mul,
    Div,
    Start,
    End,
    Word
};

struct token
{
    enum type type;
    char *str;
    int len;

    struct token *next_token;
};

struct token* lex(vector *input);
int destruct_tokens(struct token *head);

#endif