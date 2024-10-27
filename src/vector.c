#include <stdio.h>
#include <stdlib.h>

#include "types.h"

Vector *v_init()
{
    Vector *v = (Vector*)malloc(sizeof(Vector));

    v->len = 0;
    v->max = VECTOR_INIT_SIZE;

    v->data = (char*)malloc(VECTOR_INIT_SIZE);

    return v;
}

void v_copy(Vector *targ, Vector *orig)
{
    for(int i = 0; i < orig->len; i++)
    {
        v_append(targ, orig->data[i]);
    }
}

void v_destruct(Vector *v)
{
    free(v->data);
    free(v);
}

void v_append(Vector *v, char c)
{
    if (v->len >= v->max)
    {
        v->data = realloc(v->data, v->max * 2);
        v->max *= 2;
    }

    v->data[v->len] = c;
    v->len++;
}

int v_match_with_string(Vector *v, char *str, int len)
{
    if(v->len != len)
    {
        return 0;
    }

    for(int i = 0; i < v->len; i++)
    {
        if(v->data[i] != str[i])
        {
            return 0;
        }
    }

    return 1;
}

void v_append_str(Vector *v, char *str, int len)
{
    for(int i = 0; i < len; i++)
    {
        v_append(v, str[i]);
    }
}

void v_print(Vector *v)
{
    // printf("%d %d\n", v->len, v->max);
    for(int i = 0; i < v->len; i++)
    {
        printf("%c", v->data[i]);
    }

    fflush(stdout);
}

void v_println(Vector *v)
{
    v_print(v);
    printf("\n");
}

int vec_cmp_vec(Vector *v1, Vector *v2)
{
    if(v1->len != v2->len)
    {
        return 0;
    }

    for(int i = 0; i < v1->len; i++)
    {
        if(v1->data[i] != v2->data[i])
        {
            return 0;
        }
    }

    return 1;
}

int vec_cmp_str(Vector *v, char *str, int len)
{
    if(v->len != len)
    {
        return 0;
    }

    for(int i = 0; i < v->len; i++)
    {
        if(v->data[i] != str[i])
        {
            return 0;
        }
    }

    return 1;
}
