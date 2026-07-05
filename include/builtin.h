#ifndef BUILTIN_H
#define BUILTIN_H

#include "main.h"
#include "env.h"

/* builtin fn type declared in main (as part of Lval specification) */
void builtin_eq(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_gt(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_lt(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_ge(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_le(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_if(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_or(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_and(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_not(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_add(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_sub(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_mul(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_div(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_list(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_head(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_tail(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_join(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_eval(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_define(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_assign(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_lambda(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_call(Lval *restrict args, Lval *restrict result, Lval *restrict fn, Env *env);
void builtin_load(Lval *restrict args, Lval *restrict result, Env *env);
void builtin_print(Lval *restrict args, Lval *restrict result, Env *env);

#endif
