#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

void append_token(struct TokenBuffer *buf, Token t)
{
    if (buf->len >= buf->max)
    {
        buf->tokens = realloc(buf->tokens, buf->max * 2 * sizeof(Token));
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

static int within_bounds(int value, int a, int b)
{
    if(value >= a && value <= b)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

struct TokenBuffer *lex(char *buf, int len)
{
    struct TokenBuffer *t_buf = malloc(sizeof(struct TokenBuffer));
    t_buf->len = 0;
    t_buf->max = 8;
    t_buf->tokens = malloc(sizeof(Token) * t_buf->max);

    int i = 0;

    while(i < len)
    {
        if (within_bounds(buf[i], '0', '9'))
        {
            int j = 1;
            while(within_bounds(buf[i + j], '0', '9'))
            {
                j++;
            }

            append_token(t_buf, Number);
            
            i += j;
        }
        else if (within_bounds(buf[i], 'a', 'z') || within_bounds(buf[i], 'A', 'Z') || buf[i] == '_')
        {
            int j = 1;

            while(within_bounds(buf[i + j], 'a', 'z') || within_bounds(buf[i + j], 'A', 'Z') || buf[i + j] == '_')
            {
                j++;
            }

            append_token(t_buf, Ident);

            i += j;
        }
        else if(buf[i] == '\"')
        {
            int j = 1;
            while(buf[i + j] != '\"')
            {
                // printf("%c\n", buf[i+j]);
                j++;
            }

            append_token(t_buf, String);
            
            i += j + 1;
        }
        else if (buf[i] == ' ')
        {
            append_token(t_buf, Space);
            i++;
        }
        else if (buf[i] == '(')
        {
            append_token(t_buf, O_Paren);
            i++;
        }
        else if (buf[i] == ')')
        {
            append_token(t_buf, C_Paren);
            i++;
        }
        else
        {
            append_token(t_buf, Undef);
            i++;
        }
    }

    append_token(t_buf, End);

    return t_buf;
}
