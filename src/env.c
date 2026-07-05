#include "env.h"

Env *env_newenv(void) {
	/* Create and return a new environment */

	Env *env = malloc(sizeof(Env));
	env->map = usf_newhm();
	env->super = NULL;

	return env;
}

void env_freeenv(Env *env) {
	/* Free an environment */

	usf_freehmfunc(env->map, (void (*)(void *)) lisp_freelval);
	free(env);
}

void env_envput(Env *env, const char *key, Lval *value) {
	/* Put a copy of value in the env at position given by key */

	Lval *copy = malloc(sizeof(Lval));
	lisp_lcopy(copy, value); /* copy */

	lisp_freelval((Lval *) usf_strhmget(env->map, key).p);
	usf_strhmput(env->map, key, USFDATAP(copy));
}

void env_envtrans(Env *env, const char *key, Lval *value) {
	/* Destructively transfer value in the env at position given by key */

	Lval *trans = malloc(sizeof(Lval));
	lisp_ltrans(trans, value); /* transfer */

	lisp_freelval((Lval *) usf_strhmget(env->map, key).p);
	usf_strhmput(env->map, key, USFDATAP(trans));
}

Lval *env_envget(Env *env, const char *key) {
	/* Return a copy of the Lval associated with this key in given environment if present, else NULL */

	Lval *template = (Lval *) usf_strhmget(env->map, key).p;
	if (template == NULL) {
		if (env->super == NULL) return NULL; /* nowhere */
		else return env_envget(env->super, key); /* search up */
	}

	Lval *copy = malloc(sizeof(Lval));
	lisp_lcopy(copy, template);
	return copy;
}

void env_ecopy(Env *restrict to, Env *restrict from) {
	/* Deep copies an environment to another */

	usf_hashiter iter;
	for (usf_hmiterbegin(from->map, &iter); usf_hmiternext(&iter);)
		env_envput(to, (const char *) iter.entry->key.p, (Lval *) iter.entry->value.p);
	to->super = from->super; /* share parent */
}

void env_printenv(Env *env) {
	/* Prints environment to stdout */

	usf_hashiter iter;
	for (usf_hmiterbegin(env->map, &iter); usf_hmiternext(&iter);) {
		printf("%s:", (char *) iter.entry->key.p);
		lisp_printlval((Lval *) iter.entry->value.p);
		fputc(' ', stdout);
	} if (env->map->size) fputc('\b', stdout); /* remove last space */
}
