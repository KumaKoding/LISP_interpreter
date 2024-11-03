#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

void append_token(struct TokenBuffer *buf, Token t)
{
    if(buf->len >= buf->max)
    {
        buf->tokens = realloc(buf->tokens, buf->max * 2);
        buf->max *= 2;
    }

    buf->tokens[buf->len] = t;

    buf->len++;
}

void delete_tokens(struct TokenBuffer *buf)
{
    free(buf->tokens);
    free(buf);
}

struct TokenBuffer *lex(char *buf, int len)
{
    for(int i = 0; i < len; i++)
    {
        
    }
}

