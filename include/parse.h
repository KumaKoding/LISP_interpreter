#ifndef PARSE_H
#define PARSE_H

#include "input.h"
#include "types.h"
#include "lexer.h"

Expr *parse(struct safe_string clean_input, struct TokenBuffer tokens);

#endif
