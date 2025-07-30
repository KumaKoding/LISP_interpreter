#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "input.h"
#include "lexer.h"
#include "expr.h"
#include "my_malloc.h"


#if TESTING
	#define malloc(X) check_malloc(X, __FILE__, __LINE__, __FUNCTION__)
	#define realloc(X, Y) check_realloc(X, Y, __FILE__, __LINE__, __FUNCTION__)
	#define free(X) check_free(X, __FILE__, __LINE__, __FUNCTION__)
#endif

struct OptionStack
{
	int len;
	char options[MAX_EXPR_STACK_SIZE];
};

struct ParseNum
{
	int n;
	int offset;
};

struct ParseVec
{
	Vector *v;
	int offset;
};

char os_pop(struct OptionStack *os)
{
	if(os->len == 0)
	{
		return 0;
	}

	char c = os->options[os->len - 1];
	os->options[os->len - 1] = 0;
	os->len--;

	return c;
}

void os_push(struct OptionStack *os, char option)
{
    if(os->len >= MAX_EXPR_STACK_SIZE)
    {
        printf("PARSE-ERROR: Parsing has exceeded maximum depth. Aborting.");
        abort();
    }

	os->options[os->len] = option;
	os->len++;
}

#define INT_FROM_CHAR(c) (c - '0')

struct ParseNum parse_num(struct safe_string str, int c)
{
	struct ParseNum p;
	
	p.offset = 0;
	p.n = 0;

	while(within_bounds(str.data[c + p.offset], '0', '9') && c + p.offset < str.len)
	{
		p.n *= 10;
		p.n += INT_FROM_CHAR(str.data[c + p.offset]);

		p.offset++;
	}

	return p;
}

#undef INT_FROM_CHAR

struct ParseVec parse_idr(struct safe_string str, int c)
{
	struct ParseVec p;

	p.offset = 0;
	p.v = v_init();

	while((is_legal_identifier(str.data[c + p.offset]) || within_bounds(str.data[c + p.offset], '0', '9')) && c + p.offset < str.len)
	{
		v_append(p.v, str.data[c + p.offset]);
		p.offset++;
	}

	return p;
}

struct ParseVec parse_str(struct safe_string str, int c)
{
	struct ParseVec p;

	p.offset = 1; // for the quote
	p.v = v_init();

	while(str.data[c + p.offset] != '\"')
	{
		v_append(p.v, str.data[c + p.offset]);

		p.offset++;
	}

	p.offset++; // for the quote

	return p;
}

int len_til_close(struct TokenBuffer tokens, int t)
{
	if(tokens.tokens[t] != O_Paren)
	{
		printf("ERROR: token %d is not an opening parenthesis at t = %d. Aborting.\n", tokens.tokens[t], t);
		abort();
	}

	int i = 1;
	int depth = 1;
	int len = 0;

	while(depth > 0 && t + i < tokens.len)
	{
		switch (tokens.tokens[t + i]) 
		{
			case O_Paren:
				if(depth == 1)
				{
					len++;
				}

				depth++;
				i++;

				break;
			case C_Paren:
				depth--;
				i++;

				break;
			case Space:
				i++;
				break;
			default:
				len++;
				i++;
				break;
		}
	}

	return len;
}

int validate_delimitation(struct TokenBuffer tokens, int t)
{
	if(tokens.tokens[t] == C_Paren || tokens.tokens[t] == Space)
	{
		return 1;
	}

	int r_offset = 1;

	if(tokens.tokens[t] == O_Paren)
	{
		int depth = 1;
		int i = 1;

		while(depth > 0 && t + i < tokens.len)
		{
			switch(tokens.tokens[t + i])
			{
				case O_Paren:
					depth++;
					break;
				case C_Paren:
					depth--;
					break;
				default:
					break;
			}

			i++;
		}

		if(depth > 0)
		{
			return 0;
		}

		r_offset = i;
	}

	if(tokens.tokens[t + r_offset] != C_Paren && tokens.tokens[t + r_offset] != Space && tokens.tokens[t + r_offset] != End)
	{
		return 0;
	}

	if(t != 0)
	{
		if(tokens.tokens[t - 1] != O_Paren && tokens.tokens[t - 1] != Space)
		{
			return 0;
		}
	}


	return 1;
}

#define PARSE_ERROR(str, c) \
	do\
	{\
		printf("PARSE-ERROR: Syntax error at character %d:\n", c);\
		printf("\t");\
		if(c - 1 >= 0)\
		{\
			printf("%c", str.data[c - 1]);\
		}\
		\
		printf("%c", str.data[c]);\
		\
		if(c + 1 < str.len)\
		{\
			printf("%c", str.data[c + 1]);\
		}\
		\
		printf("\n");\
		abort();\
		\
	} while(0)\

int check_next_alloc(struct TokenBuffer tokens, int t)
{
	int peek = 1;
	Token next_significant_token = tokens.tokens[t + peek];

	if(tokens.tokens[t] == Space)
	{
		return 0;
	}

	if(next_significant_token == Space)
	{
		peek++;

		next_significant_token = tokens.tokens[t + peek];
	}

	if(next_significant_token == C_Paren)
	{
		return 0;
	}

	if(next_significant_token == End)
	{
		return 0;
	}

	return 1;
}

int check_alloc_next(struct TokenBuffer tokens, int t)
{
	if(t + 1 >= tokens.len - 1)
	{
		return 0;
	}

	if(tokens.tokens[t + 1] == Space)
	{
		return 0;
	}

	if(tokens.tokens[t + 1] == C_Paren)
	{
		return 0;
	}

	if(tokens.tokens[t + 1] == End)
	{
		return 0;
	}

	return 1;
}

Expr *parse_special_forms(Expr *e)
{
	struct OptionStack options;
	struct ExprStack orig;
	struct ExprStack copy;

	Expr *final = malloc(sizeof(Expr));

	options.len = 0;
	orig.len = 0;
	copy.len = 0;

	os_push(&options, 'I');
	es_push(&orig, e);
	es_push(&copy, final);

	while(orig.len > 0)
	{
		char option = os_pop(&options);
		Expr *o = es_pop(&orig);
		Expr *c = es_pop(&copy);

		while(o)
		{
			switch (o->car.type)
			{
				case Lst:
					if(identify_lambda(o))
					{
						create_lambda(c, o);

						os_push(&options, 'I');
						es_push(&orig, o->car.data.lst->cdr->cdr);
						es_push(&copy, c->car.data.lam->instructions);
					}
					else if(identify_define(o))
					{
						create_define(c, o);

						os_push(&options, 'X');
						es_push(&orig, o->car.data.lst->cdr->cdr);
						es_push(&copy, c->car.data.lst->cdr);
					}
					else if(identify_ifelse(o))
					{
						create_ifelse(c, o);

						os_push(&options, 'X');
						es_push(&orig, o->car.data.lst->cdr);
						es_push(&copy, c->car.data.lst->cdr);
						os_push(&options, 'X');
						es_push(&orig, o->car.data.lst->cdr->cdr);
						es_push(&copy, c->car.data.lst->car.data.ifE->branch_true);
						os_push(&options, 'X');
						es_push(&orig, o->car.data.lst->cdr->cdr->cdr);
						es_push(&copy, c->car.data.lst->car.data.ifE->branch_false);
					}
					else 
					{
						c->car.type = o->car.type;

						if(o->car.data.lst)
						{
							c->car.data.lst = malloc(sizeof(Expr));

							os_push(&options, 'I');
							es_push(&orig, o->car.data.lst);
							es_push(&copy, c->car.data.lst);
						}
						else 
						{
							c->car.data.lst = NULL;
						}
					}

					break;
				case Num:
					c->car.type = o->car.type;
					c->car.data.num = o->car.data.num;

					break;
				case Str:
					c->car.type = o->car.type;
					c->car.data.str = v_init();
					v_copy(c->car.data.str, o->car.data.str);

					break;
				case Idr:
					c->car.type = o->car.type;
					c->car.data.str = v_init();
					v_copy(c->car.data.str, o->car.data.str);

					break;
				default:
					printf("PARSE-ERROR: Unknown expression when parsing special forms. Aborting.\n");
					abort();

					break;
			}

			c->cdr = NULL;

			if(option == 'I')
			{
				if(o->cdr)
				{
					c->cdr = malloc(sizeof(Expr));
					c = c->cdr;
				}

				o = o->cdr;
			}
			else if(option == 'X')
			{
				break;
			}
		}
	}

	return final;
}

struct Indices
{
	int c;
	int t;
};

Expr *parse_single(struct safe_string clean_input, struct TokenBuffer tokens, struct Indices *indices)
{
	Expr *origin = malloc(sizeof(Expr));
	origin->cdr = NULL;

	struct ExprStack trace;
	trace.len = 0;

	Expr *e_curr = origin;
	int depth = 0;

	while(tokens.tokens[indices->t] != End)
	{
		if(!validate_delimitation(tokens, indices->t))
		{
			PARSE_ERROR(clean_input, indices->c);
		}

		if(tokens.tokens[indices->t] == O_Paren)
		{
			e_curr->car.type = Lst;
			e_curr->car.data.lst = NULL;
			e_curr->cdr = NULL;

			es_push(&trace, e_curr);
			depth++;

			if(len_til_close(tokens, indices->t))
			{
				e_curr->car.data.lst = malloc(sizeof(Expr));
				e_curr = e_curr->car.data.lst;
				e_curr->cdr = NULL;
			}
		}
		else if(tokens.tokens[indices->t] == C_Paren)
		{
			e_curr = es_pop(&trace);
			depth--;
		}
		else 
		{
			switch(tokens.tokens[indices->t]) 
			{
				case O_Paren:
					break;
				case C_Paren:
					break;
				case Space:
					e_curr->cdr = malloc(sizeof(Expr));
					e_curr = e_curr->cdr;
					break;
				case Number:
					e_curr->car.type = Num;
					struct ParseNum num = parse_num(clean_input, indices->c);

					e_curr->car.data.num = num.n;
					e_curr->cdr = NULL;

					indices->c += num.offset - 1;
					break;
				case String:
					e_curr->car.type = Str;
					struct ParseVec str = parse_str(clean_input, indices->c);

					e_curr->car.data.str = str.v;
					e_curr->cdr = NULL;

					indices->c += str.offset - 1;
					break;
				case Ident:
					e_curr->car.type = Idr;
					struct ParseVec idr = parse_idr(clean_input, indices->c);

					e_curr->car.data.str = idr.v;
					e_curr->cdr = NULL;

					indices->c += idr.offset - 1;
					break;
				case Undef:
					PARSE_ERROR(clean_input, indices->c);
					break;
				case End:
					break;
			}
		}

		indices->t++;
		indices->c++;

		if(depth == 0)
		{
			break;
		}
	}

	return origin;
}

struct ExprVector *parse(struct safe_string clean_input, struct TokenBuffer tokens)
{
	struct ExprVector *parsed_exprs = init_e_vec();

	struct Indices indices = {0};

	while(tokens.tokens[indices.t] != End)
	{
		if(tokens.tokens[indices.t] != Space)
		{
			e_vec_push(parsed_exprs, parse_single(clean_input, tokens, &indices));
		}
		else 
		{
			indices.t++;
			indices.c++;
		}
	}

	return parsed_exprs;
}
