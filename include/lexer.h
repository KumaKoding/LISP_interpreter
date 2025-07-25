#ifndef LEXER_H
#define LEXER_H

struct safe_string;

typedef enum 
{
    O_Paren,
    C_Paren,
    String,
    Ident,
    Number,
    Space,
    Undef,
    End
} Token;

struct TokenBuffer
{
    Token *tokens;
    int len;
    int max;
};

struct TokenBuffer lex(struct safe_string no_fmt);
int clean_whitespace_for_parse(struct TokenBuffer *tokens, struct safe_string *no_fmt);
void delete_tokens(struct TokenBuffer buf);
void print_token(Token t);

#endif
