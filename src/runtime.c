#include "runtime.h"

struct Hashtable *init(int initial_size)
{
    struct Hashtable *ht = malloc(sizeof(struct Hashtable));

    ht->items = malloc(initial_size * sizeof(struct Item *));
    ht->max = initial_size;
    ht->len = 0;

    return ht;
}

int search(struct Hashtable *ht, vector *key)
{
    int is_equal = 0;
    int index = 0;

    while (!is_equal)
    {
        if (index >= ht->len)
        {
            return -1;
        }

        int temp_is_equal = 1;

        for (int i = 0; i < ht->items[index]->key->len; i++)
        {
            if (ht->items[index]->key->data[i] != key->data[i])
            {
                temp_is_equal = 0;
            }
        }

        is_equal = temp_is_equal;

        index++;
    }

    return index - 1;
}

void insert(struct Hashtable *ht, struct Item *i)
{
    if (ht->len >= ht->max)
    {
        ht->items = realloc(ht->items, 2 * ht->max * sizeof(struct Item *));
        ht->max *= 2;
    }

    ht->items[ht->len] = i;

    ht->len++;
}

void delete(struct Hashtable *ht, vector *key)
{
    int index = search(ht, key);

    v_destruct(ht->items[index]->key);
    free(ht->items[index]);

    for (int i = 0; i < ht->len - index - 1; i++)
    {
        ht->items[index + i] = ht->items[index + i + 1];
    }

    ht->items[ht->len - 1] = 0x00000000;
}

struct Item *create_expr_item(vector *key, struct Expr *expr)
{
    struct Item *new_item = malloc(sizeof(struct Item));

    new_item->key = key;
    new_item->expr = expr;
    new_item->type = 'v';

    return new_item;
}

struct Item *create_func_item(vector *key, struct Function *func)
{
    struct Item *new_item = malloc(sizeof(struct Item));

    new_item->key = key;
    new_item->func = func;
    new_item->type = 'f';

    return new_item;
}

struct Function *create_function(int n_params, vector **params, struct Expr *instructions)
{
    struct Function *f = malloc(sizeof(struct Function));

    f->n_params = n_params;
    f->instructions = instructions;

    f->params = malloc(sizeof(struct Item *) * n_params);
    for(int i = 0; i < f->n_params; i++)
    {
        f->params[i] = malloc(sizeof(struct Item));
        f->params[i]->key = params[i];
    }

    return f;
}

void rec_replace(struct Expr* ref, struct Expr *mod, struct Hashtable *vars, struct Hashtable *params)
{
    // print_ast(ref); printf("\n");
    if(ref->curr_type == Lst)
    {
        mod->curr_type = Lst;
        mod->content.expressions = malloc(sizeof(struct Expr_list));
        mod->content.expressions->len = ref->content.expressions->len;
        mod->content.expressions->list = malloc(sizeof(struct Expr*) * mod->content.expressions->len);

        for(int i = 0; i < mod->content.expressions->len; i++)
        {
            mod->content.expressions->list[i] = malloc(sizeof(struct Expr));
        }

        for(int i = 0; i < ref->content.expressions->len; i++)
        {
            rec_replace(ref->content.expressions->list[i], mod->content.expressions->list[i], vars, params);
        }
    }
    else if(ref->curr_type == Str)
    {
        mod->curr_type = Str;
        mod->content.string = init_vector();

        for(int i = 0; i < ref->content.string->len; i++)
        {
            mod->content.string->data[i] = ref->content.string->data[i];
        }

        return;
    }
    else if(ref->curr_type == Num)
    {
        mod->curr_type = Num;
        mod->content.number = ref->content.number;
        return;
    }
    else if(ref->curr_type == Nul)
    {
        mod->curr_type = Nul;
        return;
    }
    else if(ref->curr_type == Idr)
    {
        int v_index = search(vars, ref->content.string);
        int p_index = search(params, ref->content.string);

        if(v_index >= 0)
        {
            if(!match_to_builtin(ref->content.string))
            {
                mod->curr_type = vars->items[v_index]->expr->curr_type;
                mod->content = vars->items[v_index]->expr->content;
            }
            else
            {
                mod->curr_type = ref->curr_type;
                mod->content.string = init_vector();

                for(int i = 0; i < ref->content.string->len; i++)
                {
                    v_append(mod->content.string, ref->content.string->data[i]);
                }
            }
        }
        if(p_index >= 0)
        {
            mod->curr_type = params->items[p_index]->expr->curr_type;
            mod->content = params->items[p_index]->expr->content;
        }
        if(p_index < 0 && v_index < 0)
        {
            mod->curr_type = Idr;
            mod->content.string = init_vector();

            for(int i = 0; i < ref->content.string->len; i++)
            {
                v_append(mod->content.string, ref->content.string->data[i]);
            }
        }

        return;
    }
}

struct Expr* replace_params(struct Function *f, struct Hashtable *vars, struct Hashtable *params)
{
    struct Expr *new_instructions = malloc(sizeof(struct Expr));
    new_instructions->prev_expr = f->instructions->prev_expr;
    new_instructions->curr_type = f->instructions->curr_type;

    rec_replace(f->instructions, new_instructions, vars, params);

    return new_instructions;
}

void print_ht(struct Hashtable *ht)
{
    for(int i = 0; i < ht->len; i++)
    {
        if(ht->items[i]->type == 'f')
        {
            printf("function: ");
            for(int j = 0; j < ht->items[i]->key->len; j++)
            {
                printf("%c", ht->items[i]->key->data[j]);
            }
            printf("\n");

            // for(int p = 0; p < ht->items[i]->func->n_params; p++)
            // {
            //     for(int p_char = 0; p_char < ht->items[i]->func->params[p]->key->len; p)
            // }

            print_ast(ht->items[i]->func->instructions);

            printf("\n\n");
        }

        if(ht->items[i]->type == 'v')
        {
            printf("variable: ");
            for(int j = 0; j < ht->items[i]->key->len; j++)
            {
                printf("%c", ht->items[i]->key->data[j]);
            }
            printf("\n");

            print_ast(ht->items[i]->expr);

            printf("\n\n");
        }
    }

    printf("--------------------------------------------------\n\n");
}

struct Expr *eval_rec(struct Expr *curr_expr, struct Hashtable *vars)
{
    // print_ast(curr_expr);
    // printf("\n");

    if(curr_expr->curr_type == Idr)
    {
        int i = search(vars, curr_expr->content.string);

        if(i >= 0)
        {
            // make work for function
            if(vars->items[i]->type == 'v')
            {
                curr_expr->content = vars->items[i]->expr->content;
                curr_expr->curr_type = vars->items[i]->expr->curr_type;
                curr_expr->prev_expr = vars->items[i]->expr->prev_expr;
            }
            else if (vars->items[i]->type == 'f')
            {

            }
        }
        else
        {
            return curr_expr;
        }
    }
    else if(curr_expr->curr_type == Lst)
    {
        struct Expr *new_lst; 

        if(curr_expr->content.expressions->list[0]->curr_type == Idr)
        {
            if(match_str_to_vec("define", strlen("define"), curr_expr->content.expressions->list[0]->content.string))
            {
                // (define x 10)
                struct Item *new_var_item = create_expr_item(curr_expr->content.expressions->list[1]->content.string, eval_rec(curr_expr->content.expressions->list[2], vars));
                insert(vars, new_var_item);

                new_lst = new_var_item->expr;
            }
            else if(match_str_to_vec("defun", strlen("defun"), curr_expr->content.expressions->list[0]->content.string))
            {
                // (defun f (x y) (add x y))

                vector **params = malloc(sizeof(vector *) * curr_expr->content.expressions->list[2]->content.expressions->len);
                for(int i = 0; i < curr_expr->content.expressions->list[2]->content.expressions->len; i++)
                {
                    params[i] = init_vector();
                    
                    for(int c = 0; c < curr_expr->content.expressions->list[2]->content.expressions->list[i]->content.string->len; c++)
                    {
                        v_append(params[i], curr_expr->content.expressions->list[2]->content.expressions->list[i]->content.string->data[c]);
                    }
                }
                struct Function *new_fun = create_function(curr_expr->content.expressions->list[2]->content.expressions->len, params, curr_expr->content.expressions->list[3]);

                insert(vars, create_func_item(curr_expr->content.expressions->list[1]->content.string, new_fun));
            }
            else if(match_to_builtin(curr_expr->content.expressions->list[0]->content.string) >= 0)
            {
                int n_params = vars->items[search(vars, curr_expr->content.expressions->list[0]->content.string)]->func->n_params;
                struct Expr **params = malloc(sizeof(struct Expr *) * n_params);

                for(int i = 0; i < n_params; i++)
                {
                    params[i] = malloc(sizeof(struct Expr));
                    params[i] = eval_rec(curr_expr->content.expressions->list[i+1], vars);
                }

                return eval_rec(exec_builtin(curr_expr->content.expressions->list[0]->content.string, n_params, params), vars);
            }
            else if(search(vars, curr_expr->content.expressions->list[0]->content.string) >= 0)
            {
                int v_index = search(vars, curr_expr->content.expressions->list[0]->content.string);

                if(vars->items[v_index]->type == 'f')
                {
                    struct Hashtable *params = init(vars->items[v_index]->func->n_params);
                    
                    for(int i = 0; i < vars->items[v_index]->func->n_params; i++)
                    {
                        struct Expr *param = malloc(sizeof(struct Expr));
                        param = eval_rec(curr_expr->content.expressions->list[i+1], vars);
                        insert(params, create_expr_item(vars->items[v_index]->func->params[i]->key, param));
                    }
                    // (defun f (x) (println x)) (f "hello world")
                    // print_ast(replace_params(vars->items[v_index]->func, vars, params));
                    // print_ast(vars->items[v_index]->func->instructions);
                    // printf("\n");

                    return eval_rec(replace_params(vars->items[v_index]->func, vars, params), vars);
                }
                else
                {
                    return vars->items[v_index]->expr;
                }
            }
            else
            {
                new_lst = malloc(sizeof(struct Expr));
                new_lst->prev_expr = curr_expr->prev_expr;
                new_lst->curr_type = Lst;
                new_lst->content.expressions->len = 0;
                
                for(int i = 1; i < curr_expr->content.expressions->len; i++)
                {
                    new_lst->content.expressions->list = realloc(new_lst->content.expressions->list, new_lst->content.expressions->len+1);
                    new_lst->content.expressions->len++;

                    if(curr_expr->content.expressions->list[i]->curr_type == Lst)
                    {
                        new_lst->content.expressions->list[i] = eval_rec(curr_expr->content.expressions->list[i], vars);
                    }
                }

                return new_lst;
            }

            return curr_expr;
        }
    }

    return curr_expr;
}

int eval(struct Expr *exprs)
{
    // (defun f (x) x) (println (f 10))
    // (defun f (x) (println x)) (f "hello world")
    // (defun f (a) a) (f 10)
    // print_ast(exprs);
    // printf("\n");

    struct Hashtable *vars = init(16);
    add_builtins(vars);

    if(exprs->curr_type != Lst)
    {
        return -1;
    }
    
    for(int i = 0; i < exprs->content.expressions->len; i++)
    {
        eval_rec(exprs->content.expressions->list[i], vars);
    }

    return 0;
}