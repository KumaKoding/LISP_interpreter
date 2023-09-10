#include <stdio.h>

#include "vector.h"
#include "lex.h"
#include "parse.h"
#include "runtime.h"

int main(void)
{
    vector *v_char = init_vector();
    struct token *tokens;
    node *c_AST;

    do
    {
        v_append(v_char, getchar());
    } while(v_char->data[v_char->len - 1] != '\n');

    tokens = lex(v_char);
    c_AST = parse(tokens);

    { // pretty print enum tokens
        struct token *curr = tokens;

        while(1)
        {
            switch (curr->type)
            {
            case O_paren:
                printf("(");
                break;
            case C_paren:
                printf(")");
                break;
            case Space:
                printf(" ");
                break;
            case Number:
                printf("n");
                break;
            case Add:
                printf("+");
                break;
            case Sub:
                printf("-");
                break;
            case Mul:
                printf("*");
                break;
            case Div:
                printf("/");
                break;
            case Start:
                printf("{");
                break;
            case End:
                printf("}");
                break;
            case Unknown:
                printf("?");
                break;
            }

            if(curr->type == End)
            {
                break;
            }

            curr = curr->next_token;
        }
    }

    v_destruct(v_char);
    destruct_tokens(tokens);

    return 0;
}