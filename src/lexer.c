#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "input.h"
#include "my_malloc.h"

#if TESTING
	#define malloc(X) check_malloc(X, __FILE__, __LINE__, __FUNCTION__)
	#define realloc(X, Y) check_realloc(X, Y, __FILE__, __LINE__, __FUNCTION__)
	#define free(X) check_free(X, __FILE__, __LINE__, __FUNCTION__)
#endif


void append_token(struct TokenBuffer *buf, Token t)
{
    if (buf->len >= buf->max)
    {
        buf->tokens = realloc(buf->tokens, buf->max * 2 * sizeof(Token));
        buf->max *= 2;
    }

    buf->tokens[buf->len] = t;
    buf->len++;

}

void print_token(Token t)
{
	switch (t) 
	{
		case O_Paren:
			printf("(");
			break;
		case C_Paren:
			printf(")");
			break;
		case Space:
			printf("_");
			break;
		case Number:
			printf("#####");
			break;
		case String:
			printf("\"...\"");
			break;
		case Ident:
			printf("*****");
			break;
		case Undef:
			printf("?");
			break;
		case End:
			printf("<END>");
	}
}

int count_num(struct safe_string no_fmt, int c)
{
	int i = 1;

	while(within_bounds(no_fmt.data[c + i], '0', '9') && c + i < no_fmt.len)
	{
		i++;
	}

	return i;
}

int count_idr(struct safe_string no_fmt, int c)
{
	int i = 1;

	while((is_legal_identifier(no_fmt.data[c + i]) || within_bounds(no_fmt.data[c + i], '0', '9')) && c + i < no_fmt.len) 
	{
		i++;
	}

	return i;
}

int count_str(struct safe_string no_fmt, int c)
{
	int i = 1;

	while(no_fmt.data[c + i] != '\"' && c + i < no_fmt.len)
	{
		i++;

		if(c + i > no_fmt.len)
		{
			printf("LEX-ERROR: String failed to terminate. Aborting.");
			abort();
		}
	}

	i++;

	return i;
}

int count_space(struct safe_string no_fmt, int c)
{
	return count_copies(no_fmt.data, c, no_fmt.len) + 1;
}

struct TokenBuffer lex(struct safe_string no_fmt)
{
	struct TokenBuffer tokens;

	tokens.len = 0;
	tokens.max = 8;
	tokens.tokens = malloc(sizeof(Token) * tokens.max);

	int c = 0;

	while(c < no_fmt.len)
	{
		if(no_fmt.data[c] == '(')
		{
			append_token(&tokens, O_Paren);
		}
		else if(no_fmt.data[c] == ')')
		{
			append_token(&tokens, C_Paren);
		}
		else if(no_fmt.data[c] == ' ')
		{
			append_token(&tokens, Space);
			c += count_space(no_fmt, c) - 1;
		}
		else if(within_bounds(no_fmt.data[c], '0', '9'))
		{
			append_token(&tokens, Number);
			c += count_num(no_fmt, c) - 1;
		}
		else if(no_fmt.data[c] == '\"')
		{
			append_token(&tokens, String);
			c += count_str(no_fmt, c) - 1;
		}
		else if(is_legal_identifier(no_fmt.data[c]))
		{
			append_token(&tokens, Ident);
			c += count_idr(no_fmt, c) - 1;
		}
		else 
		{
			append_token(&tokens, Undef);
		}

		c++;
	}

	append_token(&tokens, End);

	return tokens;
}

int is_superfluous(struct TokenBuffer tokens, int t)
{
	if(t - 1 >= 0)
	{
		if(tokens.tokens[t - 1] == O_Paren)
		{
			return 1;
		}
	}

	if(t + 1 <= tokens.len - 1)
	{
		if(tokens.tokens[t + 1] == C_Paren || tokens.tokens[t + 1] == End)
		{
			return 1;
		}
	}

	return 0;
}

int count_superfluous_whitespace(struct TokenBuffer tokens, struct safe_string no_fmt)
{
	// duplicate whitespace
	// bounding whitespace Ex: "( " or "   )"

	int superfluous_whitespace = 0;
	int c = 0;

	for(int t = 0; t < tokens.len - 1; t++)
	{
		if(tokens.tokens[t] == Space)
		{
			int n_remove = 0; 

			if(is_superfluous(tokens, t))
			{
				n_remove = count_space(no_fmt, c);
			}
			else 
			{
				n_remove = count_space(no_fmt, c) - 1;
			}

			superfluous_whitespace += n_remove;
			c += n_remove;
		}
		else if(tokens.tokens[t] == Number)
		{
			c += count_num(no_fmt, c) - 1;
		}
		else if(tokens.tokens[t] == String)
		{
			c += count_str(no_fmt, c) - 1;
		}
		else if(tokens.tokens[t] == Ident)
		{
			c += count_idr(no_fmt, c) - 1;
		}

		c++;
	}

	return superfluous_whitespace;
}

int clean_whitespace_for_parse(struct TokenBuffer *tokens, struct safe_string *no_fmt)
{
	struct TokenBuffer new_tokens;
	struct safe_string new_buffer;

	new_tokens.len = 0;
	new_tokens.max = 8;
	new_tokens.tokens = malloc(sizeof(Token) * new_tokens.max);

	int remove_whitespace = count_superfluous_whitespace(*tokens, *no_fmt);

	new_buffer.len = 0;
	new_buffer.data = malloc(sizeof(char) * (no_fmt->len - remove_whitespace));

	int c = 0;
	int t = 0;

	while(tokens->tokens[t] != End)
	{
		if(tokens->tokens[t] == O_Paren)
		{
			append_token(&new_tokens, O_Paren);

			new_buffer.data[new_buffer.len] = '(';
			new_buffer.len++;
		}
		else if(tokens->tokens[t] == C_Paren)
		{
			append_token(&new_tokens, C_Paren);

			new_buffer.data[new_buffer.len] = ')';
			new_buffer.len++;
		}
		else if(tokens->tokens[t] == Space)
		{
			if(!is_superfluous(*tokens, t))
			{
				append_token(&new_tokens, Space);

				new_buffer.data[new_buffer.len] = ' ';
				new_buffer.len++;
			}

			c += count_space(*no_fmt, c) - 1;
		}
		else if(tokens->tokens[t] == Number)
		{
			append_token(&new_tokens, Number);

			int n_chars = count_num(*no_fmt, c);

			for(int i = 0; i < n_chars; i++)
			{
				new_buffer.data[new_buffer.len] = no_fmt->data[c + i];
				new_buffer.len++;
			}

			c += n_chars - 1;
		}
		else if(tokens->tokens[t] == String)
		{
			append_token(&new_tokens, String);

			int n_chars = count_str(*no_fmt, c);

			for(int i = 0; i < n_chars; i++)
			{
				new_buffer.data[new_buffer.len] = no_fmt->data[c + i];
				new_buffer.len++;
			}

			c += n_chars - 1;
		}
		else if(tokens->tokens[t] == Ident)
		{
			append_token(&new_tokens, Ident);

			int n_chars = count_idr(*no_fmt, c);

			for(int i = 0; i < n_chars; i++)
			{
				new_buffer.data[new_buffer.len] = no_fmt->data[c + i];
				new_buffer.len++;
			}

			c += n_chars - 1;
		}
		else if(tokens->tokens[t] == Undef)
		{
			append_token(&new_tokens, Undef);

			new_buffer.data[new_buffer.len] = no_fmt->data[c];
			new_buffer.len++;
		}

		c++;
		t++;
	}

	free(tokens->tokens);
	free(no_fmt->data);

	append_token(&new_tokens, End);

	tokens->tokens = new_tokens.tokens;
	tokens->max = new_tokens.max;
	tokens->len = new_tokens.len;

	no_fmt->data = new_buffer.data;
	no_fmt->len = new_buffer.len;

	return 1;
}

