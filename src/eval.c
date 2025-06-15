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

    Expr **return_addr;
};

struct CallStack
{
    int len;
    int max_len;
    StackFrame stack[256];
};

void vs_push(struct VisitStack *visit_stack, Expr *e)
{
    if(visit_stack->len >= visit_stack->max_len)
    {
        printf("ERROR: Evaluator has exceeded maximum depth");
        abort();
    }

    visit_stack->stack[visit_stack->len] = e;
    visit_stack->len++;
}

Expr *vs_pop(struct VisitStack *visit_stack)
{
    if(visit_stack->len <= 0)
    {
        printf("ERROR: Expected item on stack, unable");
    }

    Expr *e = visit_stack->stack[visit_stack->len - 1];
    visit_stack->stack[visit_stack->len - 1] = NULL;
    visit_stack->len--;

    return e;
}

void cs_push(struct CallStack *call_stack, Expr *fn, Expr *params[], int max_params, Expr **return_addr)
{
    StackFrame frame;

    frame.fn = fn;
    frame.params = params;
    frame.max_params = max_params;
    frame.occ_params = 0;
    frame.return_addr = return_addr;

    if(call_stack->len >= call_stack->max_len)
    {
        printf("ERROR: Evaluator has exceeded maximum depth");
        abort();
    }

    call_stack->stack[call_stack->len] = frame;
    call_stack->len++;
}

Expr *create_lambda(Expr *e)
{
    Expr *new_lambda = malloc(sizeof(struct Expr));
    new_lambda->car.type = Lam;
    new_lambda->car.data.lam = malloc(sizeof(Lambda));

    Expr *inner = e->car.data.lst;
    Expr *curr_expr = inner->cdr->car.data.lst;

    int size = 0;

    while(curr_expr != NULL)
    {
        size++;
        
        curr_expr = curr_expr->cdr;
    }

    new_lambda->car.data.lam->n_args = size;
    new_lambda->car.data.lam->p_keys = malloc(sizeof(Vector*) * size);

    curr_expr = inner->cdr->car.data.lst;

    for(int i = 0; i < size; i++)
    {
        new_lambda->car.data.lam->p_keys[i] = v_init();
        v_copy(new_lambda->car.data.lam->p_keys[i], curr_expr->car.data.str);
        
        curr_expr = curr_expr->cdr;
    }
    
    new_lambda->car.data.lam->instructions = e_copy(inner->cdr->cdr, NULL, NULL, 0);

    return new_lambda;
}

int identify_lambda(Expr *e)
{
    Expr *inner = e->car.data.lst;

    if(!inner)
    {
        return 0;
    }

    if(inner->car.type != Idr)
    {
        return 0;
    }

    if(!vec_cmp_str(inner->car.data.str, "lambda", strlen("lambda")))
    {
        return 0;
    }

    if(inner->cdr->car.type != Lst && !inner->cdr->cdr)
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

    char* const strings[] = {
        "print"
    };

    for(int i = 0; i < sizeof(strings); i++)
    {
        if(vec_cmp_str(e->car.data.str, strings[i], strlen(strings[i])))
        {
            return 1;
        }
    }

    return 0;
}

Expr *create_native(char *key, int n_args)
{
    Expr *new_native = malloc(sizeof(Expr));
    new_native->car.data.nat = malloc(sizeof(Native));
    new_native->car.type = Nat;

    new_native->car.data.nat->key = v_init();
    v_append_str(new_native->car.data.nat->key, key, strlen(key));

    new_native->car.data.nat->n_args = n_args;
    new_native->car.data.nat->params = NULL;

    return new_native;
}

void init_native(PairTable *pt)
{
    Expr *print = create_native("print", 1);
    Expr *add = create_native("add", 2);

    pt_insert(pt, print->car.data.nat->key, print);
    pt_insert(pt, add->car.data.nat->key, add);
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
        return return_value;
    }
    else if (v_match_with_string(key, "add", strlen("add")))
    {
        return_value->car.type = Num;
        return_value->car.data.num = f_curr.params[0]->car.data.num + f_curr.params[1]->car.data.num;

        return return_value;
    }

    return return_value;
}

void run_fn(struct VisitStack *vs, struct CallStack *cs)
{
    StackFrame f_curr = cs->stack[cs->len - 1];

    Expr *return_value = malloc(sizeof(Expr));
    
    if(f_curr.fn->car.type == Nat)
    {
        Native *function = f_curr.fn->car.data.nat;
        
        if(f_curr.occ_params > function->n_args)
        {
            printf("it's wrong\n");
        }

        if(f_curr.occ_params < f_curr.max_params)
        {
            return_value->car.type = Nat;
            return_value->car.data.nat = malloc(sizeof(Native));
            return_value->car.data.nat->key = v_init();
            v_copy(return_value->car.data.nat->key, function->key);
            return_value->car.data.nat->n_args = f_curr.max_params;

            return_value->car.data.nat->params = malloc(sizeof(Expr*) * f_curr.max_params);

            for(int i = 0; i < return_value->car.data.nat->n_args; i++)
            {
                return_value->car.data.nat->params[f_curr.occ_params + i] = f_curr.params[i];
            }
        }
        else
        {
            return_value = run_native(f_curr);
        }

        cs->len--;
        
        if(f_curr.return_addr)
        {
            if(cs->stack[cs->len - 1].fn) // the previous stackframe
            {
                cs->stack[cs->len - 1].occ_params++;
            }
            
            *(f_curr.return_addr) = return_value; // need to increment params by 1 if not function
        }

        e_destruct(f_curr.fn);

        e_destruct(f_curr.params[0]);
        free(f_curr.params);
        
    }
    else if(f_curr.fn->car.type == Lam)
    {
        Lambda *function = f_curr.fn->car.data.lam;
        Expr *new_instructions = e_copy(function->instructions, function->p_keys, f_curr.params, f_curr.occ_params);
        
        if(f_curr.occ_params > function->n_args)
        {
            printf("it's wrong\n");
        }

        if(f_curr.occ_params == f_curr.max_params)
        {
            if(function->instructions->car.type == Lst)
            {
                vs_push(vs, new_instructions->car.data.lst);
                cs->stack[cs->len - 1].fn = NULL;
                cs->stack[cs->len - 1].occ_params = 0;
                cs->stack[cs->len - 1].return_addr = f_curr.return_addr;
                
                Expr *e_curr = new_instructions->car.data.lst;

                cs->stack[cs->len - 1].max_params = 0;

                while(e_curr != NULL)
                {
                    cs->stack[cs->len - 1].max_params++;
                    
                    e_curr = e_curr->cdr;
                }

                cs->stack[cs->len - 1].max_params--; // accout for function
                
                cs->stack[cs->len - 1].params = malloc(sizeof(Expr*) * cs->stack[cs->len - 1].max_params);
            }
            else
            {
                return_value = new_instructions;

                cs->len--;

                if(f_curr.return_addr)
                {
                    if(cs->stack[cs->len - 1].fn) // the previous stackframe
                    {
                        cs->stack[cs->len - 1].occ_params++;
                    }

                    *(f_curr.return_addr) = return_value;

                }

                e_destruct(f_curr.fn);
                e_destruct(f_curr.params[0]);

                free(f_curr.params);
            }
        }
        else
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

            if(f_curr.return_addr)
            {
                *(f_curr.return_addr) = return_value;
            }

            e_destruct(f_curr.fn);

            for(int i = 0; i < f_curr.max_params; i++)
            {
                e_destruct(f_curr.params[i]);
            }

            free(f_curr.params);

            cs->len--;
        }
    }
}

void add_fn(Expr *e, struct CallStack *call_stack)
{
    StackFrame f_curr = call_stack->stack[call_stack->len - 1];

    Expr *fn = NULL;
    Expr **params = NULL;
    int max_params = 0;
    Expr **return_addr = NULL;

    Expr *arg_ptr = e;

    while(arg_ptr != NULL)
    {
        max_params++;
        arg_ptr = arg_ptr->cdr;
    }

    
    params = calloc(max_params, sizeof(Expr*));

    if(f_curr.fn == NULL)
    {
        f_curr.fn = malloc(sizeof(Expr));
        return_addr = &f_curr.fn;
    }
    else
    {
        f_curr.params[f_curr.occ_params] = malloc(sizeof(Expr));
        return_addr = &f_curr.params[f_curr.occ_params];
    }

    cs_push(call_stack, fn, params, max_params, return_addr);
}

void add_expr_to_fn(StackFrame *stack_frame, Expr *e)
{
    if(stack_frame->fn == NULL)
    {
        stack_frame->fn = e;
    }
    else
    {
        stack_frame->params[stack_frame->occ_params] = e;
        stack_frame->occ_params++;
    }
}

Expr *eval_inner(Expr *e, PairTable *pt, struct CallStack *cs, struct VisitStack *vs)
{
    StackFrame *curr_frame = &cs->stack[cs->len - 1];
    Expr *curr_expr = e;

    if(curr_expr->car.type == Idr)
    {
        vs_push(vs, curr_expr->cdr);

        Pair *search = pt_find(pt, curr_expr->car.data.str);

        if(search)
        {
            curr_expr = pt_find(pt, curr_expr->car.data.str)->instructions;
        }
        else
        {
            printf("ERROR: unknown identifier\n");
        }

        return curr_expr;
    }

    if(curr_expr->car.type == Lst)
    {
        if(!curr_expr->car.data.lst)
        {
            Expr *nil = malloc(sizeof(Expr));
            nil->car.type = Nil;
            nil->cdr = NULL;

            add_expr_to_fn(curr_frame, nil);

            return curr_expr->cdr;
        }
        else if(identify_lambda(curr_expr))
        {
            add_expr_to_fn(curr_frame, create_lambda(curr_expr));

            return curr_expr->cdr; // this
        }
        else
        {
            vs_push(vs, curr_expr->cdr);
            add_fn(curr_expr, cs);

            return curr_expr->car.data.lst;
        }
    }
    else
    {
        add_expr_to_fn(curr_frame, curr_expr);
        return curr_expr->cdr;
    }
}

void print_cs(struct CallStack cs)
{
    for(int i = cs.len - 1; i >= 0; i--)
    {
        printf("%d\t", i);

        printf("fn: ");
        if(cs.stack[i].fn)
        {
            e_print(cs.stack[i].fn);
        }
        else
        {
            printf("NULL");
        }

        printf("\t\t\t ");

        printf("%d/%d params: ", cs.stack[i].occ_params, cs.stack[i].max_params);

        for(int p = 0; p < cs.stack[i].max_params; p++)
        {
            if(cs.stack[i].params[p])
            {
                e_print(cs.stack[i].params[p]);
                printf(", ");
            }
            else
            {
                printf("NULL");
            }
        }

        printf("\n");
    }
}

void eval_curr(PairTable *pt, struct CallStack *cs, struct VisitStack *vs)
{
    while(vs->len > 0)
    {
        Expr *e_curr = vs_pop(vs);
        
        while(e_curr != NULL)
        {
            
            if(e_curr->car.type == Idr)
            {
                e_curr = eval_inner(e_curr, pt, cs, vs);
            }
            else if(e_curr->car.type == Lst)
            {
                e_curr = eval_inner(e_curr, pt, cs, vs);
            }
            else
            {
                add_expr_to_fn(&cs->stack[cs->len - 1], e_curr);
                e_curr = e_curr->cdr;
            }
        }
    }

    // printf("end of visit stack\n");
}

int init_eval(Expr *e, struct CallStack *cs, struct VisitStack *vs)
{
    Expr *e_curr = e;
    int size = 0;

    while(e_curr != NULL)
    {
        if(e_curr->car.type != Lst)
        {
            return 0;
        }

        e_curr = e_curr->cdr;
        size++;
    }

    e_curr = e;
    int offset = 0;

    while(e_curr != NULL)
    {
        if(e_curr->car.type != Lst)
        {
            return 0;
        }

        vs->stack[size - offset - 1] = e_curr->car.data.lst; // this doesn't work, because it adds an extra visit element while it hasn't moved onto the next function
        StackFrame *frame = &cs->stack[size - offset - 1];

        Expr *param = e_curr->car.data.lst;

        frame->max_params = 0;

        while(param != NULL)
        {
            frame->max_params++;
            param = param->cdr;
        }

        frame->max_params--; // to account for the first element, which is the function

        frame->fn = NULL;
        frame->occ_params = 0;
        frame->params = malloc(frame->max_params * sizeof(Expr*));
        frame->return_addr = NULL;

        e_curr = e_curr->cdr;
        offset++;

        vs->len++;
        cs->len++;
    }

    return 1;
}

void eval(Expr *e)
{
    // Fix the init function
    // Make two copy/destruct functions that copy/destruct the cdr expressions and don't or have it as an option paramter

    PairTable *pt = pt_init();
    struct CallStack call_stack;
    struct VisitStack visit_stack;

    call_stack.len = 0;
    call_stack.max_len = 256;

    visit_stack.len = 0;
    visit_stack.max_len = 256;

    int check = init_eval(e, &call_stack, &visit_stack);
    init_native(pt);

    if(!check)
    {
        printf("bad input\n");
        return;
    }
    
    while(call_stack.len > 0)
    {
        eval_curr(pt, &call_stack, &visit_stack);

        run_fn(&visit_stack, &call_stack);
    }
}