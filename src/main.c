#include <stdio.h>

#include "vector.h"
#include "lex.h"
#include "parse.h"
#include "runtime.h"

int main(void)
{
    vector *v_char = init_vector();
    struct Token_vec *tokens;
    struct Expr *c_AST;

    do
    {
        v_append(v_char, getchar());
    } while(v_char->data[v_char->len - 1] != '\n');

    tokens = lex(v_char);

    { // pretty print enum tokens
        for(int i = 0; i < tokens->len; i++)
        {
            switch(tokens->toks[i])
            {
            case O_paren:
                for(int j = 0; j < tokens->strs[i]->data[j]; j++) { printf("%c", tokens->strs[i]->data[j]); }
                break;
            case C_paren:
                for(int j = 0; j < tokens->strs[i]->data[j]; j++) { printf("%c", tokens->strs[i]->data[j]); }
                break;
            case Space:
                for(int j = 0; j < tokens->strs[i]->data[j]; j++) { printf("%c", tokens->strs[i]->data[j]); }
                break;
            case Number:
                for(int j = 0; j < tokens->strs[i]->data[j]; j++) { printf("%c", tokens->strs[i]->data[j]); }
                break;
            case Identifier:
                for(int j = 0; j < tokens->strs[i]->data[j]; j++) { printf("%c", tokens->strs[i]->data[j]); }
                break;
            case String:
                for(int j = 0; j < tokens->strs[i]->data[j]; j++) { printf("%c", tokens->strs[i]->data[j]); }
                break;
            default:
                break;
            }
        }
    }

    printf("\n");

    for(int i = 0; i < tokens->len; i++)
    {
        switch (tokens->toks[i])
        {
        case O_paren:
            printf("[ ");
            break;
        case C_paren:
            printf("] ");
            break;
        case Space:
            printf("Space ");
            break;
        case Number:
            printf("Number ");
            break;
        case Identifier:
            printf("Identifier ");
            break;
        case String:
            printf("String ");
            break;
        
        default:
            break;
        }
    }

    printf("\n");

    c_AST = parse(tokens);
    

    v_destruct(v_char);
    destruct_tokens(tokens);

    return 0;
}