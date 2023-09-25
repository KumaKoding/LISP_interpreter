#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "parse.h"

void print_ast(struct Expr *ast)
{
    if(ast->curr_type != 2)
    {
        return;
    }

    printf("[ ");

    for(int i = 0; i < ast->content.expressions->len; i++)
    {
        if(ast->content.expressions->list[i]->curr_type == 2)
        {
            print_ast(ast->content.expressions->list[i]);
        }
        else if(ast->content.expressions->list[i]->curr_type == 3)
        {
            printf("ident: ");
            for(int j = 0; j < ast->content.expressions->list[i]->content.string->len; j++)
            {
                printf("%c", ast->content.expressions->list[i]->content.string->data[j]);
            }
        }
        else if(ast->content.expressions->list[i]->curr_type == 1)
        {
            for(int j = 0; j < ast->content.expressions->list[i]->content.string->len; j++)
            {
                printf("%c", ast->content.expressions->list[i]->content.string->data[j]);
            }
        }
        else if(ast->content.expressions->list[i]->curr_type == 0)
        {
            printf("%d", ast->content.expressions->list[i]->content.number);
        }

        if(i != ast->content.expressions->len - 1)
        {
            printf(", ");
        }
    }

    printf("] ");
}

void add_expr(struct Expr_list *expr_list, struct Expr *expr)
{
    if(expr_list->len <= 0)
    {
        expr_list->list = malloc(sizeof(struct Expr*));
    }
    else
    {
        expr_list->list = realloc(expr_list->list, (expr_list->len + 1)  * sizeof(struct Expr*));
    }

    expr_list->list[expr_list->len] = expr;
    expr_list->len++;
}

struct Expr *parse(struct Token_vec *tokens)
{
    struct Expr *AST = malloc(sizeof(struct Expr));
    AST->prev_expr = NULL;
    AST->curr_type = 2;
    AST->content.expressions = malloc(sizeof(struct Expr_list));
    AST->content.expressions->len = 0;

    struct Expr *curr_ptr = AST;

    for(int i = 0; i < tokens->len; i++)
    {
        if(tokens->toks[i] == O_paren) 
        {
            struct Expr *new_expr = malloc(sizeof(struct Expr));
            new_expr->content.expressions = malloc(sizeof(struct Expr_list));
            new_expr->content.expressions->len = 0;
            new_expr->curr_type = 2;

            add_expr(curr_ptr->content.expressions, new_expr);

            new_expr->prev_expr = curr_ptr;
            curr_ptr = new_expr;
            curr_ptr->curr_type = 2;
        }
        else if(tokens->toks[i] == C_paren)
        {
            curr_ptr = curr_ptr->prev_expr;
        }
        else if(tokens->toks[i] == End)
        {
            break;
        }
        else if(tokens->toks[i] == Number)
        {
            struct Expr *new_expr = malloc(sizeof(struct Expr));

            int num = 0;

            for(int j = 0; j < tokens->strs[i]->len; j++)
            {
                num += (tokens->strs[i]->data[tokens->strs[i]->len - 1 - j] - '0') * pow(10, j);
            }

            printf("%d\n", num);

            new_expr->content.number = num;
            new_expr->prev_expr = curr_ptr;
            new_expr->curr_type = 0;

            add_expr(curr_ptr->content.expressions, new_expr);
        }
        else if(tokens->toks[i] == String)
        {
            struct Expr *new_expr = malloc(sizeof(struct Expr));
            new_expr->content.string = init_vector();
            printf("%s\n", tokens->strs[i]->data);

            for(int j = 0; j < tokens->strs[i]->len; j++)
            {
                v_append(new_expr->content.string, tokens->strs[i]->data[j]);
            }

            new_expr->prev_expr = curr_ptr;
            new_expr->curr_type = 1;

            add_expr(curr_ptr->content.expressions, new_expr);            
        }
        else if(tokens->toks[i] == Identifier)
        {
            struct Expr *new_expr = malloc(sizeof(struct Expr));
            new_expr->content.string = init_vector();

            for(int j = 0; j < tokens->strs[i]->len; j++)
            {
                v_append(new_expr->content.string, tokens->strs[i]->data[j]);
            }

            new_expr->prev_expr = curr_ptr;
            new_expr->curr_type = 3;

            add_expr(curr_ptr->content.expressions, new_expr);    
        }
    }

    printf("\n");

    print_ast(AST);
    printf("\n");

    return AST;
}