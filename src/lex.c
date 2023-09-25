#include <stdio.h>
#include <stdlib.h>

#include "lex.h"

int destruct_tokens(struct Token_vec *tokens)
{
    for (int i = 0; i < tokens->len; i++)
    {
        v_destruct(tokens->strs[i]);
    }

    free(tokens->strs);
    free(tokens->toks);

    free(tokens);

    return 0;
}

void append_token(struct Token_vec *tokens, enum type token, char *str, int len)
{
    if (tokens->len >= tokens->max)
    {
        tokens->toks = realloc(tokens->toks, tokens->max * 2 * sizeof(enum type));
        tokens->strs = realloc(tokens->strs, tokens->max * 2 * sizeof(vector *));

        tokens->max *= 2;
    }

    tokens->toks[tokens->len] = token;

    tokens->strs[tokens->len] = init_vector();

    for (int i = 0; i < len; i++)
    {
        v_append(tokens->strs[tokens->len], str[i]);
    }

    tokens->len++;
}

struct Token_vec *lex(vector *input)
{
    struct Token_vec *tokens = malloc(sizeof(struct Token_vec));

    tokens->toks = malloc(8 * sizeof(enum type));
    tokens->strs = malloc(8 * sizeof(vector *));

    tokens->max = 8;
    tokens->len = 0;

    for (int i = 0; i < input->len; i++)
    {
        // printf("%d %c\n", i, input->data[i]);
        if (input->data[i] == '(')
        {
            append_token(tokens, O_paren, "(", 1);
        }
        if (input->data[i] == ')')
        {
            append_token(tokens, C_paren, ")", 1);
        }
        if (input->data[i] == ' ')
        {
            append_token(tokens, Space, " ", 1);
        }
        if (input->data[i] == '\n')
        {
            append_token(tokens, End, "", 0);
        }
        if (input->data[i] <= '9' && input->data[i] >= '0')
        {
            vector *tmp_str = init_vector();

            int j = 0;

            while (input->data[i + j] <= '9' && input->data[i + j] >= '0')
            {
                v_append(tmp_str, input->data[i + j]);

                j++;
            }

            i += j - 1;

            append_token(tokens, Number, tmp_str->data, tmp_str->len);

            v_destruct(tmp_str);
        }
        if (input->data[i] <= 'z' && input->data[i] >= 'a')
        {
            vector *tmp_str = init_vector();

            int j = 0;

            while (input->data[i + j] <= 'z' && input->data[i + j] >= 'a')
            {
                v_append(tmp_str, input->data[i + j]);

                j++;
            }

            i += j - 1;

            append_token(tokens, Identifier, tmp_str->data, tmp_str->len);

            v_destruct(tmp_str);
        }
        if (input->data[i] == '\"')
        {
            vector *tmp_str = init_vector();

            int j = 0;

            do 
            {
                v_append(tmp_str, input->data[i + j]);
                j++;
            }while(input->data[i + j] != '\"');

            v_append(tmp_str, input->data[i + j]);
            i += j;

            append_token(tokens, String, tmp_str->data, tmp_str->len);

            v_destruct(tmp_str);
        }
    }
    
    return tokens;
}