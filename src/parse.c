#include <stdio.h>
#include <stdlib.h>

#include "parse.h"
#include "types.h"

Expr *parse(char *buf, int len)
{
    int depth = 0;
    Expr *expr = malloc(sizeof(Expr));

    for(int c = 0; c < len; c++)
    {
        if(buf[c] == '(')
        {

        }
        if(buf[c] == ')')
        {
            
        }
    }
}