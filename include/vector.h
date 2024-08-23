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
void v_print(vector *v);
int match_str_to_vec(const char *str, int len, vector *vec);

#endif