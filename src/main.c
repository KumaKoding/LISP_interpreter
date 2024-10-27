#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "types.h"

int fsize(FILE *f)
{
    fseek(f, 0L, SEEK_END);
    int size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    return size;
}

Expr *test_num(int n)
{
    Expr *e = malloc(sizeof(Expr));
    
    e->car.type = Num;
    e->car.data.num = n;
    e->cdr = NULL;

    return e;
}

Expr *test_str(char *str)
{
    Expr *e = malloc(sizeof(Expr));

    e->car.type = Str;
    e->car.data.str = v_init();
    v_append_str(e->car.data.str, str, strlen(str));
    e->cdr = NULL;

    return e;
}

Expr *test_idr(char *str)
{
    Expr *e = malloc(sizeof(Expr));

    e->car.type = Idr;
    e->car.data.str = v_init();
    v_append_str(e->car.data.str, str, strlen(str));
    e->cdr = NULL;

    return e;
}

Expr *test_lam(char *earg[], int nargs, Expr *ins)
{
    Expr *e = malloc(sizeof(Expr));

    e->car.type = Lam;
    e->car.data.lam = malloc(sizeof(Lambda));
    e->car.data.lam->n_args = nargs;
    e->car.data.lam->p_keys = malloc(sizeof(Vector) * nargs);
    for(int i = 0; i < nargs; i++)
    {
        e->car.data.lam->p_keys[i]= v_init();
        v_append_str(e->car.data.lam->p_keys[i], earg[i], strlen(earg[i]));
    }

    e->car.data.lam->instructions = ins;

    e->cdr = NULL;

    return e;
}

Expr *test_nat(char *earg[], int nargs, Expr *ins)
{
    Expr *e = malloc(sizeof(Expr));

    e->car.type = Nat;
    e->car.data.lam = malloc(sizeof(Lambda));
    e->car.data.lam->n_args = nargs;
    e->car.data.lam->p_keys = malloc(sizeof(Vector) * nargs);
    for(int i = 0; i < nargs; i++)
    {
        e->car.data.lam->p_keys[i]= v_init();
        v_append_str(e->car.data.lam->p_keys[i], earg[i], strlen(earg[i]));
    }
    
    e->car.data.lam->instructions = ins;

    e->cdr = NULL;

    return e;
}

Expr *test_lst(Expr *ins[], int len)
{
    Expr *e = malloc(sizeof(Expr));

    e->car.type = Lst;

    Expr **head;

    if(len > 0)
    {
        head = &e->car.data.lst;
        *head = ins[0];
    }

    for(int i = 1; i < len; i++)
    {
        (*head)->cdr = ins[i];
        head = &(*head)->cdr;
    }

    e->cdr = NULL;

    return e;
}


int main(int argc, const char *argv[])
{
    // if(argc != 2)
    // {
    //     return 0;
    // }

    // FILE *input = fopen(argv[1], "r");
    // int size = fsize(input);
    // char *buf = malloc(size);
    // fread(buf, sizeof(char), size, input);

    PairTable *pt = pt_init();

#define LEN 200

    Vector **tests = malloc(sizeof(Vector*) * LEN);

    srand(time(NULL));

    for(int i = 0; i < LEN; i++)
    {
        tests[i] = v_init();

        for(int j = 0; j < 8; j++)
        {
            v_append(tests[i], 'a' + (rand() % ('z' - 'a')));
        }

        pt_insert(pt, tests[i], NULL);
    }

    Expr *test = 
        test_lst(
            (Expr*[]){
                test_lst(
                    (Expr*[]){
                        test_idr("Hello"),
                        test_num(156),
                        test_nat(
                            (char*[]){
                                "x",
                                "y",
                                "z"
                            }, 3,
                            test_num(8)
                        )
                    }, 3
                ),
                test_lam(
                    (char*[]){
                        "arg1",
                        "arg2"
                    }, 2,
                    test_lst(
                        (Expr*[]){
                            test_idr("x"),
                            test_num(12931)
                        }, 2
                    )
                )
            }, 2
        );

    Expr *copy = e_copy(test, NULL, NULL, 0);
    e_print(copy);

    e_destruct(test);
    e_destruct(copy);
    pt_destruct(pt);

    return 0;
}