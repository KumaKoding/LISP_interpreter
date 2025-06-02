#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eval.h"

typedef struct StackFrame StackFrame;

struct VisitStack
{
    int len;
    int max_len;
    Expr *stack[256];
};

struct StackFrame
{
    Expr *fn;

    Expr **params;
    int max_params;
    int occ_params;

    Expr *return_addr;
};

struct CallStack
{
    int len;
    int max_len;
    StackFrame stack[256];
};

void vs_push(struct VisitStack visit_stack, Expr *e)
{
    if(visit_stack.len >= visit_stack.max_len)
    {
        printf("ERROR: Evaluator has exceeded maximum depth");
        abort();
    }

    visit_stack.stack[visit_stack.len] = e;
    visit_stack.len++;
}

Expr *vs_pop(struct VisitStack visit_stack)
{
    if(visit_stack.len <= 0)
    {
        printf("ERROR: Expected item on stack, unable");
    }

    Expr *e = visit_stack.stack[visit_stack.len - 1];
    visit_stack.stack[visit_stack.len - 1] = NULL;
    visit_stack.len--;

    return e;
}

void cs_push(struct CallStack call_stack, Expr *fn, Expr *params[], int max_params, Expr *return_addr)
{
    StackFrame frame;

    frame.fn = fn;
    frame.params = params;
    frame.max_params = max_params;
    frame.occ_params = 0;
    frame.return_addr = return_addr;

    if(call_stack.len >= call_stack.max_len)
    {
        printf("ERROR: Evaluator has exceeded maximum depth");
        abort();
    }

    call_stack.stack[call_stack.len] = frame;
    call_stack.len++;
}

Expr *create_lambda(Expr *e)
{
    Expr *new_lambda = malloc(sizeof(struct Expr));
    new_lambda->car.type = Lam;
    new_lambda->car.data.lam = malloc(sizeof(Lambda));

    Expr *curr_expr = new_lambda->cdr->car.data.lst;
    int size = 0;

    while(curr_expr != NULL)
    {
        size++;
        
        curr_expr = curr_expr->cdr;
    }

    new_lambda->car.data.lam->n_args = size;
    new_lambda->car.data.lam->p_keys = malloc(sizeof(Vector*) * size);

    curr_expr = new_lambda->cdr->car.data.lst;

    for(int i = 0; i < size; i++)
    {
        new_lambda->car.data.lam->p_keys[i] = v_init();
        v_copy(new_lambda->car.data.lam->p_keys[i], curr_expr->car.data.str);

        curr_expr = curr_expr->cdr;
    }
    
    new_lambda->car.data.lam->instructions = e_copy(e->cdr->cdr, NULL, NULL, 0);


}

int identify_lambda(Expr *e)
{
    if(e->car.type != Idr)
    {
        return 0;
    }

    if(!vec_cmp_str(e->car.data.str, "lambda", strlen("lambda")))
    {
        return 0;
    }

    if(e->cdr->car.type != Lst && !e->cdr->cdr)
    {
        return 0;
    }

    return 1;
}

int identify_native(Expr *e)
{
    if(e->car.type != Idr)
    {
        return 0;
    }

    const char* const strings[] = {
        "print"
    };

    for(int i = 0; i < sizeof(strings); i++)
    {
        if(vec_cmp_str(e->car.data.str, strings[i], strlen(strings[i])))
        {
            return 1;
        }
    }
}

Expr *run_native(StackFrame f_curr)
{
    Vector *key = f_curr.fn->car.data.nat->key;

    Expr *return_value = malloc(sizeof(Expr));
    return_value->car.type = Nil;
    return_value->cdr = NULL;

    if (v_match_with_string(key, "print", strlen("print")))
    {
        e_print(f_curr.params[0]);
        printf("\n");

        return return_value;
    }
}

void run_fn(struct VisitStack visit_stack, struct CallStack call_stack)
{
    StackFrame f_curr = call_stack.stack[call_stack.len - 1];

    Expr *return_value = malloc(sizeof(Expr));

    if(f_curr.fn->car.type == Nat)
    {
        struct Native *function = f_curr.fn->car.data.nat;

        for(int i = 0; i < return_value->car.data.nat->n_args; i++)
        {
            return_value->car.data.nat->params[f_curr.occ_params + i] = f_curr.params[i];
        }

        if(f_curr.occ_params < f_curr.max_params)
        {
            return_value->car.type = Nat;
            return_value->car.data.nat = malloc(sizeof(Native));
            return_value->car.data.nat->key = v_init();
            v_copy(return_value->car.data.nat->key, function->key);
            return_value->car.data.nat->n_args = f_curr.max_params;
        }
        else
        {
            return_value = run_native(f_curr);
        }
    }
    else if(f_curr.fn->car.type == Lam)
    {
        struct Lambda *function = f_curr.fn->car.data.lam;

        if(f_curr.occ_params > function->n_args)
        {
            printf("it's wrong\n");
        }

        Expr *new_instructions = e_copy(function->instructions, function->p_keys, f_curr.params, f_curr.occ_params);

        if(f_curr.occ_params < f_curr.max_params)
        {
            return_value->car.type = Lam;
            return_value->car.data.lam = malloc(sizeof(Lambda));
            return_value->car.data.lam->instructions = new_instructions;
            return_value->car.data.lam->n_args = f_curr.max_params - f_curr.occ_params;
            return_value->car.data.lam->p_keys = malloc(sizeof(Vector) * return_value->car.data.lam->n_args);

            for(int i = 0; i < return_value->car.data.lam->n_args; i++)
            {
                return_value->car.data.lam->p_keys[i] = v_init();
                v_copy(return_value->car.data.lam->p_keys[i], function->p_keys[f_curr.occ_params + i]);
            }
        }
        else
        {
            return_value = new_instructions;
        }
    }

    f_curr.return_addr = return_value;
    e_destruct(f_curr.fn);

    for(int i = 0; i < f_curr.max_params; i++)
    {
        e_destruct(f_curr.params[i]);
    }

    free(f_curr.params);

    f_curr.fn = NULL;
    f_curr.params = NULL;
    f_curr.max_params = 0;
    f_curr.occ_params = 0;
    f_curr.return_addr = NULL;

    call_stack.len--;
}

void add_fn(Expr *e, struct CallStack call_stack)
{
    StackFrame f_curr = call_stack.stack[call_stack.len - 1];

    Expr *fn = NULL;
    Expr **params = NULL;
    int max_params = 0;
    Expr *return_addr = NULL;

    Expr *arg_ptr = e;

    while(arg_ptr != NULL)
    {
        max_params++;
        arg_ptr = arg_ptr->cdr;
    }

    params = calloc(max_params, sizeof(Expr));

    if(f_curr.fn == NULL)
    {
        f_curr.fn = malloc(sizeof(Expr));
        return_addr = f_curr.fn;
    }
    else
    {
        f_curr.params[f_curr.occ_params] = malloc(sizeof(Expr));
        return_addr = f_curr.params[f_curr.occ_params];
    }

    cs_push(call_stack, fn, params, max_params, return_addr);
}

void eval_loop(Expr *e, PairTable *pt, struct CallStack call_stack, struct VisitStack visit_stack)
{

}

void eval_inner(Expr *e, PairTable *pt, struct CallStack call_stack, struct VisitStack visit_stack)
{

}

void eval(Expr *e)
{
    PairTable *pt = pt_init();
    struct CallStack call_stack;
    struct VisitStack visit_stack;

    call_stack.len = 0;
    call_stack.max_len = 256;

    visit_stack.len = 0;
    visit_stack.max_len = 256;

    cs_push(call_stack, e, NULL, 0, NULL);
    vs_push(visit_stack, e);

    e_print(e);

    while(call_stack.len < 0)
    {
        Expr *e_curr = vs_pop(visit_stack);

        e_print(e_curr);
        printf("\n");

        while(e_curr != NULL)
        {
            if(e_curr->car.type == Idr)
            {
                vs_push(visit_stack, e_curr->cdr);
                e_curr = pt_find(pt, e_curr->car.data.str)->instructions; // NULL if none
            }

            if(e_curr->car.type == Lst)
            {
                if(e_curr->cdr)
                {
                    vs_push(visit_stack, e_curr->cdr);
                }

                e_curr = e_curr->car.data.lst;

                eval_inner(e_curr, pt, call_stack, visit_stack);
                // add_fn(e_curr, call_stack);
            }
            else
            {
                eval_inner(e_curr, pt, call_stack, visit_stack);




                // StackFrame f_curr = call_stack.stack[call_stack.len - 1];

                // f_curr.params[f_curr.occ_params] = e_copy(e_curr, NULL, NULL, 0);
                // f_curr.occ_params++;

                e_curr = e_curr->cdr;
            }

            if(e_curr == NULL)
            {
                run_fn(visit_stack, call_stack);
            }
        }
    }
}