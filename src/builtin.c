#include "builtin.h"

/* All builtin primitives assume well-formed arguments */
#define ARG(_I) ((Lval *) usf_listptrget(args->arr, (_I)))
#define EXPTEST(_COND, _LVAL, _ERRFMT, ...) /* expect given Ltype, or bubble error up; use as guard clause */ \
	if (_COND) { \
		result->type = LTYPE_ERROR; \
		if ((_LVAL)->type == LTYPE_ERROR) { \
			result->err = (_LVAL)->err; /* transfer */ \
			(_LVAL)->err = NULL; /* safe free */ \
		} else result->err = lisp_errmsg(_ERRFMT __VA_OPT__(,) __VA_ARGS__); \
		return; /* args free'd upstream */ \
	}
#define ARGTYPETEST(_CMP, _LTYPE, _FNAME) \
	for (u64 i_ = 0; i_ < args->arr->size; i_++) { \
		Lval *element = ARG(i_); \
		EXPTEST(element->type _CMP (_LTYPE), element, "function \""_FNAME"\" argument %"PRIu64" is %s, not %s", \
				i_, lisp_lttostr(element->type), lisp_lttostr(_LTYPE)); \
	}
#define ARGCOUNTTEST(_CMP, _EXP, _FNAME) \
	if (args->arr->size _CMP (_EXP)) { \
		result->type = LTYPE_ERROR; \
		result->err = lisp_errmsg("function \""_FNAME"\" expected %"PRIu64" arguments, but got %"PRIu64, \
				(_EXP), args->arr->size); \
		return; \
	}

void builtin_eq(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin equal primitive: check equality of arguments */

	ARGTYPETEST(==, LTYPE_ERROR, "");
	(void) env;

	result->type = LTYPE_NUMBER; result->num = 0; /* early return */
	Lval *cmp = ARG(0); /* compare-to */

	for (u64 i = 0; i < args->arr->size; i++) if (!lisp_lvalcmp(cmp, ARG(i))) return; /* test */
	result->num = 1; /* all match */
}

void builtin_gt(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin greater than primitive: check if first argument is greater than second */

	ARGTYPETEST(==, LTYPE_ERROR, "");
	ARGCOUNTTEST(!=, 2, ">");
	(void) env;

	result->type = LTYPE_NUMBER;
	result->num = ARG(0)->num > ARG(1)->num;
}
void builtin_lt(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin less than primitive: check if first argument is less than second */

	ARGTYPETEST(==, LTYPE_ERROR, "");
	ARGCOUNTTEST(!=, 2, "<");
	(void) env;

	result->type = LTYPE_NUMBER;
	result->num = ARG(0)->num < ARG(1)->num;
}

void builtin_ge(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin greater or equal primitive: check if first argument is greater or equal to second */

	ARGTYPETEST(==, LTYPE_ERROR, "");
	ARGCOUNTTEST(!=, 2, ">=");
	(void) env;

	result->type = LTYPE_NUMBER;
	result->num = ARG(0)->num >= ARG(1)->num;
}

void builtin_le(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin less than or equal primitive: check if first argument is less than or equal to second */

	ARGTYPETEST(==, LTYPE_ERROR, "");
	ARGCOUNTTEST(!=, 2, "<=");
	(void) env;

	result->type = LTYPE_NUMBER;
	result->num = ARG(0)->num <= ARG(1)->num;
}

void builtin_if(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin if primitive: if first argument is true (not 0), evaluate to second argument, else third */

	ARGTYPETEST(==, LTYPE_ERROR, "");
	ARGCOUNTTEST(!=, 3, "if");
	EXPTEST(ARG(0)->type != LTYPE_NUMBER, ARG(0), "function \"if\" first argument is %s, not number",
			lisp_lttostr(ARG(0)->type));
	EXPTEST(ARG(1)->type != LTYPE_QEXPR, ARG(1), "function \"if\" second argument is %s, not Q-expression",
			lisp_lttostr(ARG(1)->type));
	EXPTEST(ARG(2)->type != LTYPE_QEXPR, ARG(2), "function \"if\" third argument is %s, not Q-expression",
			lisp_lttostr(ARG(2)->type));

	Lval **arg = (Lval **) (ARG(0)->num ? &args->arr->array[1] : &args->arr->array[2]);
	(*arg)->type = LTYPE_SEXPR; *arg = lisp_lvaleval(*arg, env); /* evaluate and replace */
	lisp_ltrans(result, *arg); /* transfer */
}

void builtin_or(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin or primitive: return logical OR of arguments */

	ARGTYPETEST(==, LTYPE_ERROR, "");
	(void) env;

	result->type = LTYPE_NUMBER;
	for (u64 i = 0; i < args->arr->size; i++) result->num |= ARG(i)->num;
}

void builtin_and(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin and primitive: return logical AND of arguments */

	ARGTYPETEST(==, LTYPE_ERROR, "");
	(void) env;

	result->type = LTYPE_NUMBER; result->num = -1; /* AND identity */
	for (u64 i = 0; i < args->arr->size; i++) result->num &= ARG(i)->num;
}

void builtin_not(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin and primitive: return logical NOT of argument */

	ARGTYPETEST(==, LTYPE_ERROR, "");
	ARGCOUNTTEST(!=, 1, "!");
	(void) env;

	result->type = LTYPE_NUMBER;
	result->num = !ARG(0)->num;
}

void builtin_add(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin add primitive: sums all arguments */

	ARGTYPETEST(!=, LTYPE_NUMBER, "+");
	(void) env; i64 sum = 0;

	for (u64 i = 0; i < args->arr->size; i++) sum += ((Lval *) usf_listptrget(args->arr, i))->num;
	result->type = LTYPE_NUMBER;
	result->num = sum;
}

void builtin_sub(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin sub primitive: sequentually subtracts from the first argument with all others */

	ARGTYPETEST(!=, LTYPE_NUMBER, "-");
	(void) env; i64 sum = ARG(0)->num;

	for (u64 i = 1; i < args->arr->size; i++) sum -= ((Lval *) usf_listptrget(args->arr, i))->num;
	result->type = LTYPE_NUMBER;
	result->num = (args->arr->size > 1 ? sum : -sum); /* negate if single argument */
}

void builtin_mul(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin mul primitive: multiplies all arguments */

	ARGTYPETEST(!=, LTYPE_NUMBER, "*");
	(void) env; i64 prod = 1;

	for (u64 i = 0; i < args->arr->size; i++) prod *= ((Lval *) usf_listptrget(args->arr, i))->num;
	result->type = LTYPE_NUMBER;
	result->num = prod;
}

void builtin_div(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin div primitive: sequentially divides first argument by all others */

	ARGTYPETEST(!=, LTYPE_NUMBER, "/");
	(void) env; i64 prod = ARG(0)->num;

	for (u64 i = 1; i < args->arr->size; i++) {
		i64 div = ((Lval *) usf_listptrget(args->arr, i))->num;
		EXPTEST(div == 0, args, "Division by 0"); /* args never err; no bubble up */
		prod /= div;
	}
	result->type = LTYPE_NUMBER;
	result->num = prod;
}

void builtin_list(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin list primitive: make a Q-expression out of arguments */

	ARGTYPETEST(==, LTYPE_ERROR, ""); /* errmsg unreachable; errors always bubble up */
	(void) env;

	lisp_ltrans(result, args);
	result->type = LTYPE_QEXPR; /* set type */
}

void builtin_head(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin head primitive: return the first element of a Q-expression (first and only argument) */

	(void) env; Lval *qexpr = (Lval *) ARG(0);
	EXPTEST(qexpr->type != LTYPE_QEXPR, qexpr, "function \"head\" argument is %s, not Q-expression",
			lisp_lttostr(qexpr->type));
	lisp_ltrans(result, qexpr);
	if (result->arr->size) for (u64 i = result->arr->size - 1; i > 0; i--)
		lisp_freelval(usf_listptrdel(result->arr, i));
}

void builtin_tail(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin tail primitive: return a Q-expression (first and only argument) with its first element removed */

	(void) env; Lval *qexpr = (Lval *) ARG(0);
	EXPTEST(qexpr->type != LTYPE_QEXPR, qexpr, "function \"tail\" argument is %s, not Q-expression",
			lisp_lttostr(qexpr->type));
	lisp_ltrans(result, qexpr);
	if (result->arr->size) lisp_freelval(usf_listptrdel(result->arr, 0)); /* remove first; keep rest */
}

void builtin_join(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin join primitive: concatenates Q-expressions (arguments) into a single one */

	ARGTYPETEST(!=, LTYPE_QEXPR, "join");
	(void) env;

	result->type = LTYPE_QEXPR;
	result->arr = usf_newlistptr(); /* concatenation recipient */
	for (u64 i = 0; i < args->arr->size; i++) {
		Lval *subqexpr = (Lval *) usf_listptrget(args->arr, i);
		while (subqexpr->arr->size) usf_listptradd(result->arr, usf_listptrdel(subqexpr->arr, 0));
	}
}

void builtin_eval(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin eval primitive: evaluates a Q-expression as if it were an S-expression */

	Lval *expr = (Lval *) ARG(0);
	EXPTEST(expr->type != LTYPE_QEXPR, expr, "function \"eval\" argument is %s, not Q-expression",
			lisp_lttostr(expr->type));

	expr->type = LTYPE_SEXPR; /* make sexpr for execution */
	usf_listptrset(args->arr, 0, (void *) lisp_lvaleval(expr, env)); /* replace expr by computed result */
	lisp_ltrans(result, ARG(0)); /* transfer updated expr to result */
}

void builtin_define(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin define primitive: associates new values with given symbols in toplevel environment */

	(void) env;

	builtin_assign(args, result, genv_); /* assign */
}

void builtin_assign(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin assign primitive: associates new values with given symbols in this environment */

	/* symbol checks */
	Lval *symbols = ARG(0);
	EXPTEST(symbols->type != LTYPE_QEXPR, symbols, "assignment argument 0 is %s, not Q-expression",
			lisp_lttostr(ARG(0)->type));
	for (u64 i = 0; i < symbols->arr->size; i++) {
		Lval *symbol = (Lval *) usf_listptrget(symbols->arr, i);
		EXPTEST(symbol->type != LTYPE_SYMBOL, symbol, "assignment has non-symbol (%s) at argument 0, "
				"position %"PRIu64, lisp_lttostr(symbol->type), i);
	}

	/* argument check */
	ARGTYPETEST(==, LTYPE_ERROR, ""); /* avoid storing errors */
	EXPTEST(symbols->arr->size != args->arr->size - 1, symbols, "assignment number of arguments "
			"mismatch (%"PRIu64", whilst expecting %"PRIu64")", args->arr->size, symbols->arr->size + 1);

	/* definition association */
	for (u64 i = 0; i < symbols->arr->size; i++) /* transfer from arguments */
		env_envtrans(env, ((Lval *) usf_listptrget(symbols->arr, i))->sym, ARG(i + 1));

	lisp_lsetnil(result);
}

void builtin_lambda(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin lambda primitive: makes a user-defined function */

	/* parameters check */
	(void) env; Lval *params = ARG(0), *body = ARG(1);
	ARGCOUNTTEST(!=, 2, "lambda");
	EXPTEST(params->type != LTYPE_QEXPR, params, "function \"lambda\" argument 0 is %s, not Q-expression",
			lisp_lttostr(params->type));
	for (u64 i = 0; i < params->arr->size; i++) {
		Lval *symbol = (Lval *) usf_listptrget(params->arr, i);
		EXPTEST(symbol->type != LTYPE_SYMBOL, symbol, "function \"lambda\" has non-symbol (%s) at argument 0, "
				"position %"PRIu64, lisp_lttostr(symbol->type), i);
	}
	EXPTEST(body->type != LTYPE_QEXPR, body, "function \"lambda\" argument 1 is %s, not Q-expression",
			lisp_lttostr(body->type));

	result->type = LTYPE_FN;
	result->bin = NULL; /* user-defined */
	result->env = env_newenv();
	Lval *p = malloc(sizeof(Lval)); lisp_ltrans(p, params); result->params = p;
	Lval *b = malloc(sizeof(Lval)); lisp_ltrans(b, body); result->body = b;

}

void builtin_call(Lval *restrict args, Lval *restrict result, Lval *restrict fn, Env *env) {
	/* Binds and calls user-defined function fn (allows partial initialization and variadics) */

	usf_listptr *params = fn->params->arr;
	for (u64 i = 0; i < args->arr->size; i++) { /* all fields assumed valid from function declaration */
		if (params->size == 0) {
			result->type = LTYPE_ERROR;
			result->err = lisp_errmsg("Too many arguments for function evaluation");
			return;
		} else {
			Lval *arg = ((Lval *) usf_listptrdel(params, 0));
			if (usf_sendswith(arg->sym, "...")) {
				Lval valist; valist.type = LTYPE_QEXPR; /* stack alloc */
				for (valist.arr = usf_newlistptr(); i < args->arr->size; i++) /* implicit break */
					usf_listptradd(valist.arr, ARG(i)); /* copy */
				args->arr->size -= valist.arr->size; /* transfer */
				env_envtrans(fn->env, arg->sym, &valist);
			} else env_envtrans(fn->env, arg->sym, ARG(i));
			lisp_freelval(arg); /* consumed (removed from list) */
		}
	}

	if (params->size) { /* half-evaluation or variadics nil (if last) */
		Lval *arg = (Lval *) usf_listptrget(params, 0);
		if (params->size == 1 && usf_sendswith(arg->sym, "...")) { /* last vaarg */
			Lval vanil; lisp_lsetnil(&vanil); /* stack alloc */
			env_envtrans(fn->env, arg->sym, &vanil);
			lisp_freelval(usf_listptrdel(params, 0)); /* free pending parameter */
		} else {
			lisp_ltrans(result, fn); /* incomplete function */
			return; /* do not execute */
		}
	} /* execute */

	fn->env->super = env; /* hook to caller env */
	fn->body->type = LTYPE_SEXPR; /* for evaluation */
	fn->body = lisp_lvaleval(fn->body, fn->env); /* replace with result */
	lisp_ltrans(result, fn->body); /* transfer */
}

void builtin_load(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin load primitive: load and execute a file */

	ARGCOUNTTEST(!=, 1, "load");
	ARGTYPETEST(!=, LTYPE_STRING, "load");

	mpc_result_t res;
	if (mpc_parse_contents(ARG(0)->sym, LParser_, &res)) { /* parse file */
		Lval *topexpr = lisp_asttolval((mpc_ast_t *) res.output); /* toplevel sexpr */

		for (u64 i = 0; i < topexpr->arr->size; i++) { /* eval individually */
			Lval *subexpr = (Lval *) usf_listptrget(topexpr->arr, i);
			printf(AESC_COLOR_FG_BRIGHT_BLACK); lisp_printlval(subexpr); fputc('\n', stdout);
			subexpr = lisp_lvaleval(subexpr, env); /* execute */
			printf(AESC_COLOR_FG_YELLOW); lisp_printlval(subexpr); printf(AESC_COLOR_FG_DEFAULT"\n");
			lisp_freelval(subexpr);
		}

		usf_freelistptr(topexpr->arr); /* contents free'd by evaluation */
		free(topexpr); /* free here since alloc'd by load */
		mpc_ast_delete((mpc_ast_t *) res.output);
	} else { /* error */
		mpc_err_print(res.error);
		mpc_err_delete(res.error);
	}

	lisp_lsetnil(result);
}

void builtin_print(Lval *restrict args, Lval *restrict result, Env *env) {
	/* Builtin print primitive: print an expression to stdout */

	(void) env;

	printf(AESC_COLOR_SET_BOLD);
	for (u64 i = 0; i < args->arr->size; i++) lisp_printlval(ARG(i)), fputc('\n', stdout);
	printf(AESC_COLOR_RESET_BOLD);

	lisp_lsetnil(result);
}

#undef ARG
#undef EXPTEST
#undef ARGTYPETEST
#undef ARGCOUNTTEST
