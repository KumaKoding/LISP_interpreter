#include <stdio.h>

#include "vector.h"
#include "lex.h"
#include "parse.h"
#include "runtime.h"
#include "builtins.h"

int main(void)
{
    vector *v_char = init_vector();
    struct Token_vec *tokens;
    struct Expr *c_AST;

    do
    {
        v_append(v_char, getchar());
    } while (v_char->data[v_char->len - 1] != '\n');

    tokens = lex(v_char);

    c_AST = parse(tokens);
    
    print_ast(c_AST);
    printf("\n");

    eval(c_AST);

    v_destruct(v_char);
    destruct_tokens(tokens);

    return 0;
}