#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "lexer.h"
#include "parse.h"
#include "eval.h"

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
    FILE *f;
    int size;

    if(argc < 2)
	{
        return -1;
    }

    f = fopen(argv[1], "r");
    size = fsize(f);

    char *buf = malloc(size);
    fread(buf, sizeof(char), size, f);

    struct TokenBuffer *t_buf = lex(buf, size);
	Expr *expr = parse(buf, size, t_buf);

	e_print(expr);
	printf("\n");

	Expr *output = eval(expr);

	e_print(output);
	printf("\n");

	free(buf);
    delete_tokens(t_buf);
	new_destruct(expr, INCLUDE_CDR);

    return 0;
}
