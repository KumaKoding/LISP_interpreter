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

	Expr *first_pass = parse(clean_input, t_buf);
	Expr *second_pass = parse_special_forms(first_pass);

	struct CallStack cs;
	cs.len = 1;
	cs.stack[0] = (struct StackFrame){0};
	cs.stack[0].local_references = init_map();

	struct Collector gc = init_gc(&cs);

	init_natives(&cs, &gc);

	Expr *temp = second_pass->cdr;
	second_pass->cdr = NULL;

	while(second_pass)
	{
		Expr *output = eval(second_pass, &cs, &gc);

		e_print(output);
		printf("\n\n");

		second_pass = temp;

		if(temp)
		{
			temp = second_pass->cdr;
			second_pass->cdr = NULL;
		}
	}

	free(buf);
	free(clean_input.data);
	free(t_buf.tokens);
	new_destruct(first_pass, INCLUDE_CDR);
	new_destruct(second_pass, INCLUDE_CDR);
	//
    return 0;
}
