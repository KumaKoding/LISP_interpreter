#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

vector *init_vector()
{
    vector *v = malloc(sizeof(vector));

    v->data = malloc(8);

    v->len = 0;
    v->max = 8;

    return v;
}

int v_append(vector *v, char value)
{
    if(v->len >= v->max)
    {
        void *tmp = realloc(v->data, v->max * 2);
        
        if(tmp == NULL)
        {
            return -1;
        }

        v->data = tmp;
        v->max *= 2;
    }

    v->data[v->len] = value;
    v->len++;

    return 0;
}

void v_destruct(vector *v)
{
    free(v->data);
    free(v);
}