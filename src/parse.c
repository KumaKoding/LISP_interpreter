#include <stdio.h>
#include <stdlib.h>

#include "vector.h"
#include "parse.h"

node *parse(struct token *tokens)
{
    struct token *start = tokens;

    node *AST;
    AST->node_type = Start;
    AST->real_chars = "";
    AST->len = 0;

    struct token *curr = start->next_token;

    int scope = 0;

    while(curr->type != End)
    {
        if(curr->type == O_paren) { scope++; }
        if(curr->type == C_paren) { scope--; }        
    }
};
