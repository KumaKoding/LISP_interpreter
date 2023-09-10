#ifndef VECTOR_H
#define VECTOR_H

typedef struct
{
    char *data;
    int len, max;
} vector;

vector *init_vector();
int v_append(vector *v, char value);
void v_destruct(vector *v);

#endif