#include <stdio.h>
#include <stdlib.h>

#include "parse.h"
#include "lexer.h"
#include "types.h"

struct ExprStack
{
    int len;
    Expr *stack[MAX_EXPR_STACK_SIZE];
};

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

static Expr *es_pop(struct ExprStack *es)
{
    if(es->len == 0)
    {
        return NULL;
    }

    Expr *e = es->stack[es->len - 1];
    es->stack[es->len - 1] = NULL;
    es->len--;

    return e;
}

static void es_append(struct ExprStack *es, Expr *expr)
{
    if(es->len >= MAX_EXPR_STACK_SIZE)
    {
        printf("ERROR: Parsing has exceeded maximum depth");
        abort();
    }

    es->stack[es->len] = expr;
    es->len++;
}

void parse_num(Expr *e, char *buf, int *c)
{
    e->car.type = Num;

    int i = 0;

#define INT_FROM_CHAR(a) (a - '0')

    while(within_bounds(buf[*c + i], '0', '9'))
    {
        e->car.data.num *= 10;
        e->car.data.num += INT_FROM_CHAR(buf[*c + i]);

        i++;
    }

#undef INT_FROM_CHAR

    *c += i;
}

void parse_idr(Expr *e, char *buf, int *c)
{
    e->car.type = Idr;
    e->car.data.str = v_init();

    int len = 0;

    while(within_bounds(buf[*c + len], 'a', 'z') || within_bounds(buf[*c + len], 'A', 'Z') || buf[*c + len] == '_')
    {
        v_append(e->car.data.str, buf[*c + len]);
        len++;
    }

    *c += len;
}

void parse_str(Expr *e, char *buf, int *c)
{
    e->car.type = Str;
    e->car.data.str = v_init();

    int len = 0;
    
    len++; // leading quote

    while(buf[*c + len] != '\"')
    {
        v_append(e->car.data.str, buf[*c + len]);
        len++;
    }

    *c += len + 1;
}

Expr *parse(char *buf, int len, struct TokenBuffer *tokens)
{
    struct ExprStack es;
    es.len = 0;

    if(len <= 0)
    {
        return NULL;
    }

    Expr *origin = malloc(sizeof(Expr));
    Expr *e_curr = origin;

    int t = 0;
    int c = 0;

    while(tokens->tokens[t] != End)
    {
        if(tokens->tokens[t] == O_Paren)
        {
            if(tokens->tokens[t+1] != C_Paren)
            {
                e_curr->car.type = Lst;
                e_curr->car.data.lst = malloc(sizeof(Expr));

                es_append(&es, e_curr);

                e_curr = e_curr->car.data.lst;
            }
            else
            {

				// FOR REWRITE: this is not needed
                e_curr->car.type = Nil;

                c++;
                t++;
            }

            c++;
        }
        else if(tokens->tokens[t] == C_Paren)
        {
            //
            // Need to pre-cleanse the tokens 
            //

            e_curr = es_pop(&es);

            c++;

            if(tokens->tokens[t+1] != Space && tokens->tokens[t+1] != C_Paren && tokens->tokens[t+1] != Undef && tokens->tokens[t+1] != End)
            {
                e_curr->cdr = malloc(sizeof(Expr));
                e_curr = e_curr->cdr;
            }
        }
        else
        {
            if(tokens->tokens[t] == Space)
            {
                c++;
            }
            if(tokens->tokens[t] == Undef)
            {
				c++;
                // printf("ERROR: Undefined Token at %d\n", c);
                // abort();
            }
            if(tokens->tokens[t] == Number)
            {
                parse_num(e_curr, buf, &c);
            }
            if(tokens->tokens[t] == Ident)
            {
                parse_idr(e_curr, buf, &c);
            }
            if(tokens->tokens[t] == String)
            {
                parse_str(e_curr, buf, &c);
            }

            if(tokens->tokens[t+1] != Space && tokens->tokens[t+1] != C_Paren && tokens->tokens[t+1] != Undef && tokens->tokens[t+1] != End)
            {
                e_curr->cdr = malloc(sizeof(Expr));
                e_curr = e_curr->cdr;
            }
        }

        t++;
    }

    return origin;
}
