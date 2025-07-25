#ifndef TYPES_H
#define TYPES_H

#define VECTOR_INIT_SIZE 8
#define PTABLE_INIT_SIZE 16

#define MAX_EXPR_STACK_SIZE 256

#define INCLUDE_CDR 1
#define EXCLUDE_CDR 0

typedef struct Vector Vector;
typedef struct PairTable PairTable;
typedef struct Pair Pair;
typedef struct ExprData ExprData;
typedef struct Expr Expr;
typedef struct Lambda Lambda;
typedef struct IfElse IfElse;
typedef struct Native Native;
typedef struct Define Define;

struct Collector;

struct Vector
{
    int len;
    int max;
    
    char *data;
};

struct PairTable
{
    int max;
    int len;

    Pair **pairs;
};

struct Pair
{
    Vector *key;
    Expr *instructions;

    Pair *next_item;
    int n_next;
};

struct ExprData
{
    enum
    {
        Num, // Number (1 2 3...)
        Str, // String ("...")
        Idr, // Identifier (def __) or (f (a 10))
        Lam, // Function (lambda) only for storage
        Nat, // Function w/ name but not instructions
        Lst, // (x y z ...)
		IfE, // (if T (print 5) (print "False???"))
		Def, // (define x 100)
        Nil, // ni
		Tru, // #t
		Fls, // #f
    } type;
    union
    {
        Vector *str;
        int num;
        Lambda *lam;
        Native *nat;
		IfElse *ifE;

        Expr *lst;
    } data;
};

struct ExprStack
{
    int len;
    Expr *stack[MAX_EXPR_STACK_SIZE];
};

struct Expr
{
    ExprData car;
    Expr *cdr;

	int mark;
};

struct Lambda
{
    Vector **p_keys;
	Expr **params;
    Expr *instructions;
    int n_args;
	int n_filled;
};

struct IfElse
{
	Expr *branch_true;
	Expr *branch_false;
};

struct Native
{
    Vector *key;
    Expr **params;
    int n_args;
	int n_filled;
};

struct replacements
{
	Vector **replace_keys;
	Expr **replace_exprs;
	int n_replace;
};

Vector *v_init();
void v_copy(Vector *targ, Vector *orig);
void v_destruct(Vector *v);
void v_append(Vector *v, char c);
int v_match_with_string(Vector *v, char *str, int len);
void v_append_str(Vector *v, char *str, int len);
void v_print(Vector *v);
void v_println(Vector *v);
int vec_cmp_vec(Vector *v1, Vector *v2);
int vec_cmp_str(Vector *v, char *str, int len);

void es_push(struct ExprStack *es, Expr *e);
Expr *es_pop(struct ExprStack *es);

void e_print(Expr* expr);
Expr *new_copy(Expr *e, int CDR_OPTION, struct Collector *gc);
void new_destruct(Expr *expr, int CDR_OPTION);

#endif
