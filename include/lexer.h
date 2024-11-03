#ifndef LEXER_H
#define LEXER_H

typedef enum 
{
    o_paren,
    c_paren,
    string,
    number,
    ident,
    space,
    end
} Token;

struct TokenBuffer
{
    Token *tokens;
    int len;
    int max;
};

struct TokenBuffer *lex(char *buf, int len);

#endif LEXER_H
