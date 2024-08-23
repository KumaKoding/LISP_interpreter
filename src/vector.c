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
    if (v->len >= v->max)
    {
        void *tmp = realloc(v->data, v->max * 2);

        if (tmp == NULL)
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

void v_print(vector *v)
{
    for(int i = 0; i < v->len; i++)
    {
        printf("%c", v->data[i]);
    }
}

int match_str_to_vec(const char *str, int len, vector *vec)
{
    if (len != vec->len)
    {
        return 0;
    }

    for (int i = 0; i < len; i++)
    {
        if (vec->data[i] != str[i])
        {
            return 0;
        }
    }

    return 1;
}
