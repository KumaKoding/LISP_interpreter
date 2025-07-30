#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "callstack.h"
#include "garbage.h"
#include "expr.h"
#include "input.h"
#include "lexer.h"
#include "parse.h"
#include "eval.h"
#include "my_malloc.h"

#if TESTING
	#define malloc(X) check_malloc(X, __FILE__, __LINE__, __FUNCTION__)
	#define realloc(X, Y) check_realloc(X, Y, __FILE__, __LINE__, __FUNCTION__)
	#define free(X) check_free(X, __FILE__, __LINE__, __FUNCTION__)
#endif


int fsize(FILE *f)
{
    fseek(f, 0L, SEEK_END);
    int size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    return size;
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

	struct safe_string clean_input = cleanse_formatting(buf, size);
	struct TokenBuffer t_buf = lex(clean_input);
	clean_whitespace_for_parse(&t_buf, &clean_input);

	struct ExprVector *exprs = parse(clean_input, t_buf);

	for(int i = 0; i < exprs->n_exprs; i++)
	{
		Expr *first_pass = exprs->exprs[i];
		exprs->exprs[i] = parse_special_forms(exprs->exprs[i]);

		new_destruct(first_pass, INCLUDE_CDR);
	}

	struct CallStack cs;
	cs.len = 1;
	cs.stack[0] = (struct StackFrame){0};
	cs.stack[0].local_references = init_map();

	struct Collector gc = init_gc(&cs);

	init_natives(&cs, &gc);

	for(int e = 0; e < exprs->n_exprs; e++)
	{
		Expr *output = eval(exprs->exprs[e], &cs, &gc);

		e_print(output);
		printf("\n");

		mark(&gc);
		sweep(&gc);
	}

	for(int i = 0; i < cs.stack[0].local_references->len; i++)
	{
		v_destruct(cs.stack[0].local_references->map[i].v);
	}

	free(cs.stack[0].local_references);

	cs.len--;

	mark(&gc);
	sweep(&gc);

	free(buf);
	free(clean_input.data);
	free(t_buf.tokens);

	for(int e = 0; e < exprs->n_exprs; e++)
	{
		new_destruct(exprs->exprs[e], EXCLUDE_CDR);
	}

	free(exprs->exprs);
	free(exprs);

	free(gc.e_vec->exprs);
	free(gc.e_vec);

    return 0;
}
