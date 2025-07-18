#include <stdio.h>
#include <stdlib.h>

#include "environment.h"
#include "types.h"

void map_push(LocalMap *lm, MapPair mp)
{
	if(lm->len >= MAX_MAP_SIZE)
	{
		printf("ERROR: Max depth reached for local map. Aborting.");
		abort();
	}

	lm->map[lm->len] = mp;

	lm->len++;
}

Expr *map_search(Vector *key, LocalMap lm)
{
	Expr *search = NULL;

	for(int i = 0; i < lm.len; i++)
	{
		if(vec_cmp_vec(key, lm.map[i].v))
		{
			search = lm.map[i].e;
		}
	}

	return search;
}

Expr *env_search(Vector *key, Environment env)
{
	Expr *search = NULL;

	for(int i = env.depth; i >= 0; i--)
	{
		search = map_search(key, env.env[i]);

		if(search)
		{
			return search;
		}
	}

	return search;
}

void env_pop(Environment *env)
{
	for(int i = 0; i < env->env[env->depth].len; i++)
	{
		env->env[env->depth].map[i].e = NULL;
		env->env[env->depth].map[i].v = NULL;
	}

	env->env[env->depth].len = 0;

	env->depth--;
}

MapPair init_map_pair(Vector *v, Expr *e)
{
	MapPair mp;

	mp.v = v_init();
	v_copy(mp.v, v);

	mp.e = new_copy(e, NO_REPLACE, EXCLUDE_CDR);

	return mp;
}

Environment init_environment()
{
	Environment env;

	env.depth = 0;

	env.env = malloc(sizeof(LocalMap) * MAX_ENV_DEPTH);

	env.env[0] = (LocalMap){0};

	return env;
}

