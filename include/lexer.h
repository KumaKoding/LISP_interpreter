#ifndef LEXER_H
#define LEXER_H

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

struct TokenBuffer *lex(char *buf, int len);
void delete_tokens(struct TokenBuffer *buf);

#endif
