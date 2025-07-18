#ifndef LOCAL_H
#define LOCAL_H

#include "types.h"

#define MAX_ENV_DEPTH 256
#define MAX_MAP_SIZE 512

typedef struct MapPair
{
	Vector *v;
	Expr *e;
} MapPair;

typedef struct LocalMap
{
	MapPair map[MAX_MAP_SIZE];
	int len;
} LocalMap;

typedef struct Environment
{
	LocalMap *env;
	int depth;
} Environment;

void map_push(LocalMap *lm, MapPair mp);
Expr *env_search(Vector *key, Environment env);
void env_pop(Environment *env);
MapPair init_map_pair(Vector *v, Expr *e);
Environment init_environment();

#endif

