#ifndef TYPES_H
#define TYPES_H

#define VECTOR_INIT_SIZE 8
#define PTABLE_INIT_SIZE 16

typedef struct Vector Vector;
typedef struct PairTable PairTable;
typedef struct Pair Pair;
typedef struct ExprData ExprData;
typedef struct Expr Expr;
typedef struct Lambda Lambda;

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
        Lam, // Function (lambda)
        Nat,  // (print ...)
        Lst, // (x y z ...)
        Nil, // nil
    } type;
    union
    {
        Vector *str;
        int num;
        Lambda *lam;

        Expr *lst;
    } data;
};

struct Expr
{
    ExprData car;
    Expr *cdr;
};

struct Lambda
{
    Vector **p_keys;
    Expr *instructions;
    int n_args;
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

PairTable *pt_init();
int pt_hash(Vector *key, int table_size);
void pt_insert(PairTable *pt, Vector *key, Expr *instruction);
Pair *pt_find(PairTable *pt, Vector *key);
void pt_delete(PairTable *pt, Vector *key);
void pt_print(PairTable *pt);
void pt_destruct(PairTable *pt);

void e_print(Expr* expr);
Expr *e_copy(Expr *expr, Vector *replace_keys[], Expr *replace_expr[], int n_replace);
void e_destruct(Expr *expr);

#endif