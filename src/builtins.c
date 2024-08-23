#include "builtins.h"
#include "vector.h"
#include "runtime.h"
#include "parse.h"

void bi_print(struct Expr *expr)
{
    switch (expr->curr_type)
    {
    case Num:
        printf("%d", expr->content.number);
        break;
    case Str:
        v_print(expr->content.string);
        break;
    case Lst:
        printf("(");
        for(int i = 0; i < expr->content.expressions->len; i++)
        {
            bi_print(expr->content.expressions->list[i]);
            
            if(i < expr->content.expressions->len - 1)
            {
                printf(" ");
            }
        }
        printf(")");
        break;
    case Nul:
        printf("nul");
        break;
    case Idr:
        v_print(expr->content.string);
        break;
    default:
        break;
    }

    fflush(stdout);
}

int bi_add(struct Expr *expr1, struct Expr *expr2)
{
    return expr1->content.number + expr2->content.number;
}

int match_to_builtin(vector *v)
{
    /* returns number of args in function */

    if(match_str_to_vec(BI_PRINT, strlen(BI_PRINT), v))
    {
        return 1;
    }
    if(match_str_to_vec(BI_ADD, strlen(BI_ADD), v))
    {
        return 1;
    }

    return -1;
}

struct Expr *exec_builtin(vector *key, int n_params, struct Expr **params)
{
    struct Expr *null_expr = malloc(sizeof(struct Expr));
    null_expr->curr_type = Nul;

    if(match_str_to_vec(BI_PRINT, strlen(BI_PRINT), key) && n_params == 1)
    {
        bi_print(params[0]);

        return null_expr;
    }
    else if(match_str_to_vec(BI_ADD, strlen(BI_ADD), key) && n_params == 2)
    {
        null_expr->curr_type = Num;
        null_expr->content.number = bi_add(params[0], params[1]);
        return null_expr;
    }

    return null_expr;
}

void add_to_vec(const char *str, vector *v)
{
    for(int i = 0; i < strlen(str); i++)
    {
        v_append(v, str[i]);
    }
}

void add_builtins(struct Hashtable *ht)
{
    vector *key_println = init_vector();
    add_to_vec(BI_PRINT, key_println);
    struct Function *println = malloc(sizeof(struct Function));
    println->n_params = PRINT_ARGS;
    insert(ht, create_func_item(key_println, println));

    vector *key_add = init_vector();
    add_to_vec(BI_ADD, key_add);
    struct Function *add = malloc(sizeof(struct Function));
    add->n_params = ADD_ARGS;
    insert(ht, create_func_item(key_add, add));
}
