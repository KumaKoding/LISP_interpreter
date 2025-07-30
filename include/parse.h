#ifndef PARSE_H
#define PARSE_H

typedef struct Expr Expr;
struct ExprVector;
struct TokenBuffer;
struct safe_string;

struct ExprVector *parse(struct safe_string clean_input, struct TokenBuffer tokens);
Expr *parse_special_forms(Expr *e);

void create_define(Expr *copy, Expr *orig);
int identify_define(Expr *e);

void create_lambda(Expr *copy, Expr *orig);
int identify_lambda(Expr *e);

void create_ifelse(Expr *copy, Expr *orig);
int identify_ifelse(Expr *e);

#endif
