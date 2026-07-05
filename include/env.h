#ifndef ENV_H
#define ENV_H

#include "main.h"

typedef struct Env {
	usf_hashmap *map;
	struct Env *super;
} Env;

Env *env_newenv(void);
void env_freeenv(Env *env);
void env_envput(Env *env, const char *key, Lval *value);
void env_envtrans(Env *env, const char *key, Lval *value);
Lval *env_envget(Env *env, const char *key);
void env_ecopy(Env *restrict to, Env *restrict from);
void env_printenv(Env *env);

#endif
