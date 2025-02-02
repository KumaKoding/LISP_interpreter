#ifndef PARSE_H
#define PARSE_H

#include "types.h"
#include "lexer.h"

Expr *parse(char *buf, int len, struct TokenBuffer *tokens);

#endif