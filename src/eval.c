#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"

struct ExprStack
{
    int len;
    Expr *stack[256];
};

struct ExprQueue
{
    Expr *buf[256];
    int len;
};

static Expr *es_pop(struct ExprStack *es)
{
    if (es->len == 0)
    {
        return NULL;
    }

    Expr *e = es->stack[es->len - 1];
    es->stack[es->len - 1] = NULL;
    es->len--;

    return e;
}

static void es_append(struct ExprStack *es, Expr *expr)
{
    if (es->len >= 256)
    {
        printf("ERROR: Evaluator has exceeded maximum depth");
        abort();
    }

    es->stack[es->len] = expr;
    es->len++;
}

void run_native(Expr *f, Expr *params, struct ExprStack *visit_stack, struct ExprStack *value_stack, PairTable *pt)
{
    Expr *nil = malloc(sizeof(Expr));
    nil->car.type = Nil;
    nil->cdr = NULL;

    int n_filled = 0;
    Expr *fill_ptr = f->car.data.nat->params;

    while (fill_ptr != NULL)
    {
        n_filled++;
        fill_ptr = fill_ptr->cdr;
    }

    int n_params = 0;

    Expr *param_ptr = params;

    while (param_ptr != NULL)
    {
        n_params++;
        param_ptr = param_ptr->cdr;
    }

    // number of arguments already filled, not number of args in params

    if (n_filled + n_params < f->car.data.nat->n_args)
    {
        Expr *incomplete_native = malloc(sizeof(Expr));
        incomplete_native->car.data.nat = malloc(sizeof(Native));
        incomplete_native->car.type = Nat;

        incomplete_native->car.data.nat->key = v_init();
        v_copy(incomplete_native->car.data.nat->key, f->car.data.nat->key);

        incomplete_native->car.data.nat->params = e_copy(f->car.data.nat->params, NULL, NULL, 0);

        Expr *incomplete_ptr = incomplete_native->car.data.nat->params;

        for (int i = 0; i < n_filled; i++)
        {
            incomplete_ptr = incomplete_ptr->cdr;
        }

        incomplete_native->car.data.nat->params->cdr = e_copy(incomplete_ptr, NULL, NULL, 0);

        es_append(value_stack, incomplete_native);
    }

    if (v_match_with_string(f->car.data.nat->key, "define", strlen("define")))
    {
        if (params->cdr->car.type == Idr)
        {
            pt_insert(pt, params->car.data.str, e_copy(pt_find(pt, params->car.data.str)->instructions, NULL, NULL, 0));
        }
        else if (params->cdr->car.type == Lst)
        {
            pt_insert(pt, params->car.data.str, e_copy(es_pop(value_stack), NULL, NULL, 0));
        }
        else
        {
            pt_insert(pt, params->car.data.str, e_copy(params->cdr, NULL, NULL, 0));
        }

        es_append(value_stack, nil);
    }
    if (v_match_with_string(f->car.data.nat->key, "print", strlen("print")))
    {
        if (params->car.type == Idr)
        {
            e_print(pt_find(pt, params->car.data.str)->instructions);
            printf("\n");
        }
        else if (params->car.type == Lst)
        {
            e_print(es_pop(value_stack));
            printf("\n");
        }
        else
        {
            e_print(params);
            printf("\n");
        }

        es_append(value_stack, nil);
    }
}

Expr *create_native(char *name, int name_len, int n_args)
{
    Expr *new_native = malloc(sizeof(Expr));
    new_native->car.data.nat = malloc(sizeof(Native));
    new_native->car.type = Nat;

    new_native->car.data.nat->key = v_init();
    v_append_str(new_native->car.data.nat->key, name, name_len);

    new_native->car.data.nat->n_args = n_args;
    new_native->car.data.nat->params = NULL;

    return new_native;
}

void add_natives(PairTable *pt)
{
    Expr *define = create_native("define", strlen("define"), 2);
    Expr *print = create_native("print", strlen("print"), 2);

    pt_insert(pt, define->car.data.nat->key, define);
    pt_insert(pt, print->car.data.nat->key, print);
}

int identify_lambda(Expr *e)
{
    if (e->car.type == Idr)
    {
        if (v_match_with_string(e->car.data.str, "lambda", strlen("lambda")))
        {
            if (e->cdr->car.type == Lst && e->cdr->cdr->car.type == Lst)
            {
                return 1;
            }
        }
    }

    return 0;
}

Expr *create_lambda(Expr *e)
{
    Expr *lambda = malloc(sizeof(Expr));
    lambda->car.data.lam = malloc(sizeof(Lambda));
    lambda->car.type = Lam;

    Expr *ptr = e->cdr->car.data.lst;
    int n_args = 0;

    while (ptr != NULL)
    {
        ptr = ptr->cdr;
        n_args++;
    }

    lambda->car.data.lam->p_keys = malloc(sizeof(Vector) * n_args);
    lambda->car.data.lam->n_args = n_args;

    ptr = e->cdr->car.data.lst;

    for (int i = 0; i < n_args; i++)
    {
        lambda->car.data.lam->p_keys[i] = v_init();
        v_copy(lambda->car.data.lam->p_keys[i], ptr->car.data.str);

        ptr = ptr->cdr;
    }

    lambda->car.data.lam->instructions = e_copy(e->cdr->cdr, NULL, NULL, 0); // will be a list

    return lambda;
}

void idr_loop(Expr *e, struct ExprStack *visit_stack, struct ExprStack *value_stack, PairTable *pt)
{
    Expr *f = e;

    e_print(f);
    printf("\n");

    if (f->car.type == Idr)
    {
        f = pt_find(pt, e->car.data.str)->instructions;
    }

    if (f->car.type == Num || f->car.type == Str || f->car.type == Nil)
    {
        es_append(value_stack, f);
        return;
    }

    if (f->car.type == Nat)
    {
        run_native(f, e->cdr, visit_stack, value_stack, pt);
        return;
    }

    Expr *e_curr = e->cdr;
    int n = 0;

    Expr **params = malloc(sizeof(Expr) * f->car.data.lam->n_args);

    while (e_curr != NULL)
    {
        if (e_curr->car.type == Idr)
        {
            params[n] = pt_find(pt, e_curr->car.data.str)->instructions;
        }
        else if (e_curr->car.type == Lst)
        {
            params[n] = es_pop(value_stack);
        }
        else
        {
            params[n] = e_curr;
        }

        e_curr = e_curr->cdr;
        n++;
    }

    if (n < (f->car.data.lam->n_args - 1))
    {
        Expr *incomplete_lam = malloc(sizeof(Expr));
        incomplete_lam->car.data.lam = malloc(sizeof(Lambda));
        incomplete_lam->car.type = Lam;

        incomplete_lam->car.data.lam->instructions = e_copy(f, f->car.data.lam->p_keys, params, n);

        incomplete_lam->car.data.lam->n_args = f->car.data.lam->n_args - n;
        incomplete_lam->car.data.lam->p_keys = malloc(incomplete_lam->car.data.lam->n_args);

        for (int i = 0; i < incomplete_lam->car.data.lam->n_args; i++)
        {
            incomplete_lam->car.data.lam->p_keys[i] = v_init();
            v_copy(incomplete_lam->car.data.lam->p_keys[i], f->car.data.lam->p_keys[i + n]);
        }

        es_append(value_stack, incomplete_lam);
    }
    else
    {
        es_append(visit_stack, e_copy(f->car.data.lam->instructions, f->car.data.lam->p_keys, params, f->car.data.lam->n_args));
    }
}

void list_loop(Expr *e, struct ExprStack *visit_stack, struct ExprStack *value_stack, PairTable *pt)
{
    Expr *e_curr = e;

    if (e->car.type == Idr)
    {
        // check for special forms first
        if (identify_lambda(e_curr))
        {
            es_append(value_stack, create_lambda(e_curr));

            return;
        }
        else
        {
            es_append(visit_stack, e);
            e_curr = e->cdr;
        }
    }

    while (e_curr != NULL)
    {
        if (e_curr->car.type == Lst)
        {
            es_append(visit_stack, e_curr);
        }

        e_curr = e_curr->cdr;
    }
}

void eval(Expr *e)
{
    struct PairTable *pt = pt_init();
    struct ExprStack visit_stack; // already evaluated items too feed into function
    struct ExprStack value_stack; // which things to visit

    add_natives(pt);

    visit_stack.len = 0;
    value_stack.len = 0;

    // EVALUATE LAST ITEM IN THE LIST FIRST

    Expr *base_expr = e;
    int n_base = 0;

    while (base_expr != NULL)
    {
        es_append(&visit_stack, NULL);

        base_expr = base_expr->cdr;
        n_base++;
    }

    base_expr = e;

    for (int i = 0; i < n_base; i++)
    {
        visit_stack.stack[n_base - i - 1] = base_expr;
        base_expr = base_expr->cdr;
    }

    while (visit_stack.len > 0)
    {
        Expr *e_init = es_pop(&visit_stack);
        Expr *e_curr = e_init;

        if (e_init->car.type != Lst) // if it's lambda, loop, idr, builtin, etc.
        {                            // builtin functions stored as lam (a b) (define a b)
            // runs functions, combines all parts of function
            idr_loop(e_curr, &visit_stack, &value_stack, pt);
        }
        else
        {
            // combines special forms (lambda, loop), appends to queue & stack
            e_curr = e_curr->car.data.lst;

            list_loop(e_curr, &visit_stack, &value_stack, pt);
        }
    }

    // printf("\n");
}
