#include <stdio.h>
#include <stdlib.h>

#include "types.h"

struct ExprQueue
{
    Expr *buf[256];
    int len;
};

Expr *eq_pop(struct ExprQueue *queue)
{
    if (queue->len <= 0)
    {
        printf("nothing in queue\n");
        return NULL;
    }

    Expr *first = queue->buf[0];

    if (queue->len == 1)
    {
        queue->buf[0] = NULL;
        queue->len--;
    }
    else if (queue->len > 1)
    {
        for (int i = 0; i < queue->len - 1; i++)
        {
            queue->buf[i] = queue->buf[i + 1];

            if (i == queue->len - 1)
            {
                queue->buf[i + 1] = NULL;
            }
        }

        queue->len--;
    }

    return first;
}

void eq_append(struct ExprQueue *queue, Expr *expr)
{
    if (queue->len >= 256)
    {
        printf("ERROR: copy has exceeded maximum depth");
    }

    queue->buf[queue->len] = expr;
    queue->len++;
}

Expr *e_copy(Expr *expr, Vector *replace_keys[], Expr *replace_expr[], int n_replace)
{
    struct ExprQueue copy_queue;
    struct ExprQueue orig_queue;
    copy_queue.len = 0;
    orig_queue.len = 0;

    Expr *copy = malloc(sizeof(Expr));

    eq_append(&orig_queue, expr);
    eq_append(&copy_queue, copy);

    while (copy_queue.len > 0)
    {
        Expr *curr_orig = eq_pop(&orig_queue);
        Expr *curr_copy = eq_pop(&copy_queue);

        while (curr_orig != NULL)
        {
            switch (curr_orig->car.type)
            {
            case Num:
                curr_copy->car.type = Num;
                curr_copy->car.data.num = curr_orig->car.data.num;

                break;
            case Str:
                curr_copy->car.type = Str;
                curr_copy->car.data.str = v_init();
                v_copy(curr_copy->car.data.str, curr_orig->car.data.str);

                break;
            case Idr:
            {
                int found_match = 0;

                for (int i = 0; i < n_replace; i++)
                {
                    if (vec_cmp_vec(curr_orig->car.data.str, replace_keys[i]))
                    {
                        found_match = 1;

                        eq_append(&orig_queue, replace_expr[i]);
                        eq_append(&copy_queue, curr_copy);
                    }
                }

                if (!found_match)
                {
                    curr_copy->car.type = Idr;
                    curr_copy->car.data.str = v_init();
                    v_copy(curr_copy->car.data.str, curr_orig->car.data.str);
                }

                break;
            }
            case Lam:
                curr_copy->car.type = Lam;
                curr_copy->car.data.lam = malloc(sizeof(Lambda));
                curr_copy->car.data.lam->n_args = curr_orig->car.data.lam->n_args;
                curr_copy->car.data.lam->p_keys = malloc(sizeof(Vector) * curr_copy->car.data.lam->n_args);

                for (int i = 0; i < curr_orig->car.data.lam->n_args; i++)
                {
                    curr_copy->car.data.lam->p_keys[i] = v_init();
                    v_copy(curr_copy->car.data.lam->p_keys[i], curr_orig->car.data.lam->p_keys[i]);
                }

                curr_copy->car.data.lam->instructions = malloc(sizeof(Expr));

                eq_append(&orig_queue, curr_orig->car.data.lam->instructions);
                eq_append(&copy_queue, curr_copy->car.data.lam->instructions);
                break;
            case Nat:
                curr_copy->car.type = Nat;
                curr_copy->car.data.nat = malloc(sizeof(Native));
                curr_copy->car.data.nat->n_args = curr_orig->car.data.nat->n_args;
                
                curr_copy->car.data.nat->key = v_init();
                v_copy(curr_copy->car.data.nat->key, curr_orig->car.data.nat->key);
                
                if(curr_orig->car.data.nat->params)
                {
                    curr_copy->car.data.nat->params = malloc(sizeof(Expr) * curr_orig->car.data.nat->n_args);
                    
                    for(int i = 0; i < curr_orig->car.data.nat->n_args; i++)
                    {
                        eq_append(&orig_queue, curr_orig->car.data.nat->params[i]);
                        eq_append(&copy_queue, curr_copy->car.data.nat->params[i]);
                    }
                }
                else
                {
                    curr_copy->car.data.nat->params = NULL;
                }
                break;
            case Lst:
                curr_copy->car.type = Lst;
                curr_copy->car.data.lst = malloc(sizeof(Expr));

                eq_append(&orig_queue, curr_orig->car.data.lst);
                eq_append(&copy_queue, curr_copy->car.data.lst);

                break;
            case Nil:
                curr_copy->car.type = Nil;

                break;
            default:
                break;
            }

            if (curr_orig->cdr)
            {
                curr_copy->cdr = malloc(sizeof(Expr));
            }

            curr_orig = curr_orig->cdr;
            curr_copy = curr_copy->cdr;
        }
    }

    return copy;
}

void e_destruct(Expr *expr)
{
    if (expr == NULL)
    {
        return;
    }

    struct ExprQueue trace;
    trace.len = 0;

    eq_append(&trace, expr);

    while (trace.len > 0)
    {
        Expr *curr = eq_pop(&trace);

        while (curr != NULL)
        {
            switch (curr->car.type)
            {
            case Lam:
                for(int i = 0; i < curr->car.data.lam->n_args; i++)
                {
                    v_destruct(curr->car.data.lam->p_keys[i]);
                }

                eq_append(&trace, curr->car.data.lam->instructions);

                free(curr->car.data.lam);
                break;
            case Nat:
                v_destruct(curr->car.data.nat->key);

                if(curr->car.data.nat->params)
                {
                    for(int i = 0; i < curr->car.data.nat->n_args; i++)
                    {
                        eq_append(&trace, curr->car.data.nat->params[i]);
                    }
    
                    free(curr->car.data.nat);
                }
                
                break;
            case Lst:
                eq_append(&trace, curr->car.data.lst);
                break;
            default:
                break;
            }

            Expr *prev = curr;
            curr = curr->cdr;

            free(prev);
        }
    }
}

void e_print(Expr *expr)
{
    Expr *curr = expr;

    while (curr != NULL)
    {
        switch (curr->car.type)
        {
        case Num:
            printf("%d", curr->car.data.num);
            break;
        case Str:
            printf("\"");
            v_print(curr->car.data.str);
            printf("\"");
            break;
        case Idr:
            printf("*");
            v_print(curr->car.data.str);
            break;
        case Lam:
            printf("(");
            for (int i = 0; i < curr->car.data.lam->n_args; i++)
            {
                v_print(curr->car.data.lam->p_keys[i]);

                if (i < curr->car.data.lam->n_args - 1)
                {
                    printf(" ");
                }
            }
            printf(") -> (");
            e_print(curr->car.data.lam->instructions);
            printf(")");
            break;
        case Nat:
            printf("__");
            v_print(curr->car.data.nat->key);
            printf("__");
            printf("(");
            for (int i = 0; i < curr->car.data.nat->n_args; i++)
            {
                printf("%c", ('a' + i));

                if (i < curr->car.data.nat->n_args - 1)
                {
                    printf(" ");
                }
            }
            printf(")");
            break;
        case Lst:
            printf("(");
            e_print(curr->car.data.lst);
            printf(")");
            break;
        case Nil:
            printf("nil");
            break;

        default:
            break;
        }

        if (curr->cdr != NULL)
        {
            printf(" ");
        }

        curr = curr->cdr;
    }
}
