#include <stdio.h>
#include <stdlib.h>

#include "types.h"

struct ExprQueue
{
    Expr *buf[256];
    int len;
};

Expr *pop(struct ExprQueue *queue)
{
    if(queue->len <= 0)
    {
        printf("nothing in queue\n");
        return NULL;
    }

    Expr *first = queue->buf[0];

    if(queue->len == 1)
    {
        queue->buf[0] = NULL;
        queue->len--;
    }
    else if(queue->len > 1)
    {
        for(int i = 0; i < queue->len - 1; i++)
        {
            queue->buf[i] = queue->buf[i+1];

            if(i == queue->len - 1)
            {
                queue->buf[i+1] = NULL;
            }
        }

        queue->len--;
    }

    return first;

}

void append(struct ExprQueue *queue, Expr *expr)
{
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

    append(&orig_queue, expr);
    append(&copy_queue, copy);

    while(copy_queue.len > 0)
    {
        Expr *curr_orig = pop(&orig_queue);
        Expr *curr_copy = pop(&copy_queue);

        while(curr_orig != NULL)
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

                for(int i = 0; i < n_replace; i++)
                {
                    if(vec_cmp_vec(curr_orig->car.data.str, replace_keys[i]))
                    {
                        found_match = 1;

                        append(&orig_queue, replace_expr[i]);
                        append(&copy_queue, curr_copy);
                    }
                }

                if(!found_match)
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

                for(int i = 0; i < curr_orig->car.data.lam->n_args; i++)
                {
                    curr_copy->car.data.lam->p_keys[i] = v_init();
                    v_copy(curr_copy->car.data.lam->p_keys[i], curr_orig->car.data.lam->p_keys[i]);
                }

                curr_copy->car.data.lam->instructions = malloc(sizeof(Expr));

                append(&orig_queue, curr_orig->car.data.lam->instructions);
                append(&copy_queue, curr_copy->car.data.lam->instructions);
                break;
            case Nat:
                curr_copy->car.type = Nat;
                curr_copy->car.data.lam = malloc(sizeof(Lambda));
                curr_copy->car.data.lam->n_args = curr_orig->car.data.lam->n_args;
                curr_copy->car.data.lam->p_keys = malloc(sizeof(Vector) * curr_copy->car.data.lam->n_args);

                for(int i = 0; i < curr_orig->car.data.lam->n_args; i++)
                {
                    curr_copy->car.data.lam->p_keys[i] = v_init();
                    v_copy(curr_copy->car.data.lam->p_keys[i], curr_orig->car.data.lam->p_keys[i]);
                }

                curr_copy->car.data.lam->instructions = malloc(sizeof(Expr));

                append(&orig_queue, curr_orig->car.data.lam->instructions);
                append(&copy_queue, curr_copy->car.data.lam->instructions);
                break;
            case Lst:
                curr_copy->car.type = Lst;
                curr_copy->car.data.lst = malloc(sizeof(Expr));

                append(&orig_queue, curr_orig->car.data.lst);
                append(&copy_queue, curr_copy->car.data.lst);
                break;
            
            default:
                break;
            }

            if(curr_orig->cdr)
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
    if(expr == NULL)
    {
        return;
    }

    struct ExprQueue trace;
    trace.len = 0;

    append(&trace, expr);

    while(trace.len > 0)
    {
        Expr *curr = pop(&trace);

        while(curr != NULL)
        {
            switch(curr->car.type)
            {
                case Lam:
                    append(&trace, curr->car.data.lam->instructions);
                    break;
                case Nat:
                    append(&trace, curr->car.data.lam->instructions);
                    break;
                case Lst: 
                    append(&trace, curr->car.data.lst);
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

void e_print(Expr* expr)
{
    Expr *curr = expr;
    
    while(curr != NULL)
    {
        switch (curr->car.type)
        {
        case Num:
            printf("%d", curr->car.data.num);
            break;
        case Str:
            v_print(curr->car.data.str);
            break;
        case Idr:
            printf("*");
            v_print(curr->car.data.str);
            break;
        case Lam:
            printf("(");
            for(int i = 0; i < curr->car.data.lam->n_args; i++)
            {
                v_print(curr->car.data.lam->p_keys[i]);

                if(i < curr->car.data.lam->n_args - 1)
                {
                    printf(" ");
                }
            }
            printf(") -> (");
            e_print(curr->car.data.lam->instructions);
            printf(")");
            break;
        case Nat:
            printf("(");
            for(int i = 0; i < curr->car.data.lam->n_args; i++)
            {
                v_print(curr->car.data.lam->p_keys[i]);

                if(i < curr->car.data.lam->n_args - 1)
                {
                    printf(" ");
                }
            }
            printf(") -> (");
            e_print(curr->car.data.lam->instructions);
            printf(")");
            break;
        case Lst:
            printf("(");
            e_print(curr->car.data.lst);
            printf(")");
            break;
        
        default:
            break;
        }
        
        if(curr->cdr != NULL)
        {
            printf(" ");
        }

        curr = curr->cdr;
    }
}
