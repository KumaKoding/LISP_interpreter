#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "callstack.h"
#include "types.h"
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

	printf("%s\n\n", buf);

	struct safe_string clean_input = cleanse_formatting(buf, size);
    struct TokenBuffer t_buf = lex(clean_input);

	printf("%s\n\n", clean_input.data);

	for(int i = 0; i < t_buf.len; i++)
	{
		print_token(t_buf.tokens[i]);
	}

	printf("\n\n");

	clean_whitespace_for_parse(&t_buf, &clean_input);

	for(int i = 0; i < t_buf.len; i++)
	{
		print_token(t_buf.tokens[i]);
	}

	printf("\n\n");


	for(int i = 0; i < clean_input.len; i++)
	{
		printf("%c", clean_input.data[i]);
	}

	printf("\n\n");

	Expr *first_pass = parse(clean_input, t_buf);

	e_print(first_pass);
	printf("\n\n");

	Expr *second_pass = parse_special_forms(first_pass);

	e_print(second_pass);
	printf("\n\n");

	struct CallStack cs;
	cs.len = 1;
	cs.stack[0] = (struct StackFrame){0};
	cs.stack[0].local_references = init_map();
	init_natives(&cs);

	Expr *temp = second_pass->cdr;
	second_pass->cdr = NULL;

	while(second_pass)
	{
		Expr *output = eval(second_pass, &cs);

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
