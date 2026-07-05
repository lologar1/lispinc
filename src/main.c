#include <editline/readline.h>
#include <editline/history.h>

#include "main.h"
#include "builtin.h"
#include "env.h"

Env *genv_; /* global environment */
mpc_parser_t *LParser_;

c_int main(void) {
	/* lispinc entry point */

	/* mpc initialization */
	mpc_parser_t *Number = mpc_new("number");
	mpc_parser_t *Symbol = mpc_new("symbol");
	mpc_parser_t *String = mpc_new("string");
	mpc_parser_t *Comment = mpc_new("comment");
	mpc_parser_t *Sexpr = mpc_new("sexpr");
	mpc_parser_t *Qexpr = mpc_new("qexpr");
	mpc_parser_t *Gexpr = mpc_new("gexpr");
	LParser_ = mpc_new("lisp");
	mpca_lang(MPCA_LANG_DEFAULT,
		"number: /-?[0-9]+/;"
		"symbol: /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&|]+(\\.\\.\\.)?/;"
		"string: /\"(\\\\.|[^\"])*\"/;"
		"comment: /;[^\\r\\n]*/;"
		"sexpr: '(' <gexpr>* ')';"
		"qexpr: '{' <gexpr>* '}';"
		"gexpr: <number> | <symbol> | <string> | <sexpr> | <qexpr> | <comment> ;"
		"lisp: /^/ <gexpr>*/$/;",
		Number, Symbol, String, Comment, Sexpr, Qexpr, Gexpr, LParser_);

	genv_ = env_newenv(); /* global environment */
#define ADDBUILTIN(_SYM, _NAME) \
	do { \
		Lval builtin = {.type = LTYPE_FN, .bin = builtin_##_NAME}; \
		env_envtrans(genv_, _SYM, &builtin); \
	} while (0);
	ADDBUILTIN("==", eq); ADDBUILTIN(">", gt); ADDBUILTIN("<", lt); ADDBUILTIN(">=", ge); ADDBUILTIN("<=", le);
	ADDBUILTIN("if", if); ADDBUILTIN("+", add); ADDBUILTIN("-", sub); ADDBUILTIN("*", mul); ADDBUILTIN("/", div);
	ADDBUILTIN("|", or); ADDBUILTIN("&", and); ADDBUILTIN("!", not); ADDBUILTIN("list", list);
	ADDBUILTIN("head", head); ADDBUILTIN("tail", tail); ADDBUILTIN("join", join); ADDBUILTIN("eval", eval);
	ADDBUILTIN("define", define); ADDBUILTIN("=", assign); ADDBUILTIN("lambda", lambda);
	ADDBUILTIN("load", load); ADDBUILTIN("print", print);

	ADDBUILTIN("def", define); ADDBUILTIN("\\", lambda); /* for book compatibility */
#undef ADDBUILTIN

	mpc_result_t res;
	mpc_ast_t *ast;

	u64 rclen; char **lisprc = usf_ftost("lisprc", &rclen);
	if (lisprc) {
		for (u64 i = 0; i < rclen; i++) { /* startup */
			if (mpc_parse("lisprc", lisprc[i], LParser_, &res)) {
				ast = (mpc_ast_t *) res.output;
				Lval *cmd = lisp_asttolval(ast);
				lisp_freelval(lisp_lvaleval(cmd, genv_));
				mpc_ast_delete(ast);
			} else {
				printf(AESC_COLOR_FG_RED"error during lisprc execution\n"AESC_COLOR_FG_DEFAULT);
				mpc_err_print(res.error);
				mpc_err_delete(res.error);
				return 0;
			}
		}
		printf(AESC_COLOR_FG_BRIGHT_GREEN"lisprc: execution finished successfully\n"AESC_COLOR_FG_DEFAULT);
		usf_freetxt(lisprc, 1);
	}

	char *input; /* user input */
	while ((input = readline(">"))) {
		add_history(input);
		if (mpc_parse("stdin", input, LParser_, &res)) {
			ast = (mpc_ast_t *) res.output;
			// mpc_ast_print(ast); fputc('\n', stdout);

			Lval *lval = lisp_asttolval(ast); /* parse (result always sexpr) */
			printf(AESC_COLOR_FG_BRIGHT_BLACK); lisp_printlval(lval); printf(AESC_COLOR_FG_DEFAULT"\n");
			lval = lisp_lvaleval(lval, genv_); /* evaluate */
			printf(AESC_COLOR_FG_CYAN); lisp_printlval(lval); printf(AESC_COLOR_FG_DEFAULT"\n");/* display */
			lisp_freelval(lval); /* free */

			mpc_ast_delete(ast);
		} else {
			mpc_err_print(res.error);
			mpc_err_delete(res.error);
		}
		free(input); /* alloc'd by editline */
	}

	/* program end */
	printf("\n"AESC_COLOR_SET_BOLD"Program end."AESC_RESET_ALL"\n");
	env_freeenv(genv_);
	mpc_cleanup(6, Number, Symbol, String, Comment, Sexpr, Qexpr, Gexpr, LParser_);
	clear_history(); /* free editline memory */

	return 0;
}

const char *lisp_lttostr(Ltype type) {
	/* Return prettyname string associated with Ltype */

	switch (type) {
		case LTYPE_ERROR: return "error";
		case LTYPE_NUMBER: return "number";
		case LTYPE_SYMBOL: return "symbol";
		case LTYPE_STRING: return "string";
		case LTYPE_SEXPR: return "S-expression";
		case LTYPE_QEXPR: return "Q-expression";
		case LTYPE_FN: return "function";
	} unreachable();
}

const char *lisp_bttostr(Builtinfn *fn) {
	/* Return prettyname string associated with builtin function */

	usf_hashiter iter; /* look through genv_ for builtins */
	for (usf_hmiterbegin(genv_->map, &iter); usf_hmiternext(&iter);) /* prettystring never written to */
		if (((Lval *) iter.entry->value.p)->bin == fn) return (const char *) iter.entry->key.p;
	return "unknown";
}

char *lisp_errmsg(const char *fmt, ...) {
	/* Allocate new formatted string for error reporting */

	va_list args; va_start(args, fmt);

	char *msg = malloc(LISP_MAXERRSTRSZ);
	vsnprintf(msg, LISP_MAXERRSTRSZ, fmt, args);

	va_end(args);
	return msg;
}

void lisp_printlval(Lval *lval) {
	/* Print parsed Lval to stdout */

	switch (lval->type) {
		case LTYPE_ERROR: printf(AESC_COLOR_FG_RED"error: %s"AESC_COLOR_FG_DEFAULT, lval->err); break;
		case LTYPE_NUMBER: printf("%"PRId64, lval->num); break;
		case LTYPE_SYMBOL: printf("%s", lval->sym); break;
		case LTYPE_STRING: printf("\"%s\"", lval->sym); break;
		case LTYPE_FN:
			if (lval->bin) printf("<builtin function %s>", lisp_bttostr(lval->bin));
			else {
				printf("<function, params: "); lisp_printlval(lval->params);
				printf(" body: "); lisp_printlval(lval->body);
				printf(" env: "); env_printenv(lval->env);
				printf("> ");
			} break;
		case LTYPE_QEXPR: /* fallthrough */
		case LTYPE_SEXPR:
			fputc(lval->type == LTYPE_SEXPR ? '(' : '{', stdout);
			for (u64 i = 0; i < lval->arr->size; i++) {
				lisp_printlval((Lval *) usf_listptrget(lval->arr, i));
				fputc(' ', stdout); /* separator */
			}
			if (lval->arr->size) fputc('\b', stdout); /* delete extra space if non-empty */
			fputc(lval->type == LTYPE_SEXPR ? ')' : '}', stdout);
			break;
	}
}

void lisp_lcopy(Lval *restrict to, Lval *restrict from) {
	/* Deep copies an lval to another */

	memcpy(to, from, sizeof(Lval)); /* shallow */
	switch (from->type) {
		case LTYPE_NUMBER: return; /* only shallow */
		case LTYPE_FN:
			if (from->bin == NULL) { /* user defined */
				env_ecopy(to->env = env_newenv(), from->env);
				lisp_lcopy(to->params = malloc(sizeof(Lval)), from->params);
				lisp_lcopy(to->body = malloc(sizeof(Lval)), from->body);
			} return;
		case LTYPE_ERROR: /* fallthrough */
		case LTYPE_SYMBOL: /* fallthrough */
		case LTYPE_STRING: to->sym = strdup(from->sym); return;
		case LTYPE_SEXPR: /* fallthrough */
		case LTYPE_QEXPR: to->arr = usf_newlistptr();
			for (u64 i = 0; i < from->arr->size; i++) {
				Lval *clone = malloc(sizeof(Lval)); lisp_lcopy(clone, usf_listptrget(from->arr, i));
				usf_listptradd(to->arr, clone); /* add copy */
			} return;
	}
}

void lisp_ltrans(Lval *restrict to, Lval *restrict from) {
	/* Destructively transfer (copy) an Lval from from to to */

	memcpy(to, from, sizeof(Lval));
	from->type = LTYPE_NUMBER; /* free only affects struct */
}

void lisp_lsetnil(Lval *to) {
	/* Initialize an empty list */

	to->type = LTYPE_QEXPR;
	to->arr = usf_newlistptr();
}

void lisp_freelval(Lval *lval) {
	/* Recursively free Lval structure */

	if (lval == NULL) return;

	switch (lval->type) {
		default: break; /* only struct to be free'd */
		case LTYPE_SEXPR: /* fallthrough */
		case LTYPE_QEXPR: usf_freelistptrfunc(lval->arr, (void (*)(void *)) lisp_freelval); break;
		case LTYPE_ERROR: /* fallthrough */
		case LTYPE_SYMBOL: /* fallthrough */
		case LTYPE_STRING: free(lval->sym); break;
		case LTYPE_FN: if (lval->bin == NULL) {
			env_freeenv(lval->env);
			lisp_freelval(lval->params);
			lisp_freelval(lval->body);
		}
	} free(lval);
}

Lval *lisp_lvaleval(Lval *lval, Env *env) {
	/* Evaluate given Lval and return it */

	switch (lval->type) {
		case LTYPE_FN: /* fallthrough */
		case LTYPE_ERROR: /* fallthrough */
		case LTYPE_NUMBER: /* fallthrough */
		case LTYPE_STRING: /* fallthrough */
		case LTYPE_QEXPR: return lval;
		case LTYPE_SYMBOL: { /* symbol lookup always copies */
			char *sym = lval->sym;
			Lval *template = env_envget(env, sym);
			if (template) lisp_ltrans(lval, template), free(template); /* transfer and free */
			else lval->type = LTYPE_ERROR, lval->err = lisp_errmsg("Unbound symbol %s", sym);

			free(sym); /* free string memory */
			return lval;
		}
		case LTYPE_SEXPR: break;
	}
	
	/* s-expression (errors bubble up primitives) */
	for (u64 i = 0; i < lval->arr->size; i++) /* evaluate before execution */
		usf_listptrset(lval->arr, i, lisp_lvaleval(usf_listptrget(lval->arr, i), env));

	if (lval->arr->size == 0) return lval; /* empty */
	else if (lval->arr->size == 1) { /* unpack and delete */
		/* 0-argument functions pass through to call */
		Lval *child = (Lval *) usf_listptrget(lval->arr, 0);
		if (!(child->type == LTYPE_FN && child->bin == NULL && child->params->arr->size == 0)) {
			usf_freelistptr(lval->arr); /* child taken */
			free(lval); /* free outer list */
			return child;
		}
	}

	/* execution */
	Lval *result = calloc(1, sizeof(Lval)), *operator = usf_listptrdel(lval->arr, 0); /* pop */
	if (operator->type != LTYPE_FN) {
		result->type = LTYPE_ERROR;
		if (operator->type == LTYPE_ERROR) result->err = operator->err, operator->err = NULL;
		else result->err = lisp_errmsg("S-expression operator is %s, not function",lisp_lttostr(operator->type));
	} else if (operator->bin) operator->bin(lval, result, env); /* primitive */
		   else builtin_call(lval, result, operator, env); /* user defined */

	lisp_freelval(operator); /* operator (copy) always free'd */
	lisp_freelval(lval); /* arguments always free'd, or placeholder swapped */
	return result;
}

i32 lisp_lvalcmp(const Lval *a, const Lval *b) {
	/* Compare two Lval, and return 1 if they are the same, else 0 */

	if (!a || !b) return a == b; /* NULL */
	if (a->type != b->type) return 0; /* type */

	switch (a->type) { /* compare */
		case LTYPE_NUMBER: return (a->num == b->num ? 1 : 0);
		case LTYPE_ERROR: /* fallthrough */
		case LTYPE_SYMBOL: /* fallthrough */
		case LTYPE_STRING: return !strcmp(a->sym, b->sym);
		case LTYPE_SEXPR: /* fallthrough */
		case LTYPE_QEXPR:
			for (u64 i = 0; i < a->arr->size; i++) /* recurse */
				if (!lisp_lvalcmp(usf_listptrget(a->arr, i), usf_listptrget(b->arr, i))) return 0;
			return 1;
		case LTYPE_FN: if (a->bin) return (void *) a->bin == (void *) b->bin;
			else return lisp_lvalcmp(a->body, b->body);
	} unreachable();
}

Lval *lisp_asttolval(mpc_ast_t *ast) {
	/* Convert given AST to LISPINC Lval */

	if (strstr(ast->tag, "comment")) return NULL; /* ignored by caller */
	Lval *lval = calloc(1, sizeof(Lval)); /* zero all fields as default */

	if (strstr(ast->tag, "number")) lval->type = LTYPE_NUMBER, lval->num = strtoi64(ast->contents, NULL, 10);
	else if (strstr(ast->tag, "symbol")) lval->type = LTYPE_SYMBOL, lval->sym = strdup(ast->contents);
	else if (strstr(ast->tag, "string")) { /* unescape */
		u64 strsz = strlen(ast->contents) - 1; /* without "", incl. \0 */
		char *unquoted = malloc(strsz); memcpy(unquoted, ast->contents + 1, --strsz); unquoted[strsz] = '\0';
		lval->type = LTYPE_STRING; lval->sym = mpcf_unescape(unquoted); /* frees unquoted */
	} if (lval->type) return lval; /* leaf node complete; LTYPE_ERROR is 0 (unset state) */

	/* s-expression or q-expression */
	lval->arr = usf_newlistptr();
	lval->type = (strstr(ast->tag, "qexpr") ? LTYPE_QEXPR : LTYPE_SEXPR); /* default for root: sexpr */
	for (u64 i = 1; i < (u64) ast->children_num - 1; i++) { /* skip separators (root, sexpr, qexpr) */
		Lval *element = lisp_asttolval(ast->children[i]);
		if (element == NULL) continue; /* comment */
		else usf_listptradd(lval->arr, element);
	}

	return lval;
}
