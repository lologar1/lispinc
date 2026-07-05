#ifndef MAIN_H
#define MAIN_H

/* LISPINC standard program-wide conventions and includes */
#include <stddef.h>
#include <setjmp.h>

#include "usfstd.h"
#include "usfaesc.h"
#include "usflist.h"
#include "usfhashmap.h"
#include "usfstring.h"
#include "usfio.h"
#include "mpc.h"

#define LISP_MAXERRSTRSZ 512

typedef struct Lval Lval;
typedef struct Env Env;
typedef void Builtinfn(Lval *restrict args, Lval *restrict result, Env *env);

typedef enum Ltype {
	LTYPE_ERROR,
	LTYPE_NUMBER,
	LTYPE_SYMBOL,
	LTYPE_STRING,
	LTYPE_SEXPR,
	LTYPE_QEXPR,
	LTYPE_FN,
} Ltype;

typedef struct Lval {
	Ltype type;
	union {
		char *err; /* error string */
		i64 num; /* number primitive */
		char *sym; /* symbol string */
		usf_listptr *arr; /* list of Lval * */
		Builtinfn *bin; /* builtin function */
	};

	/* lambda (bin == NULL) */
	Env *env;
	Lval *params; /* qexpr */
	Lval *body; /* qexpr */
} Lval;

extern Env *genv_;
extern mpc_parser_t *LParser_;

const char *lisp_lttostr(Ltype type);
const char *lisp_bttostr(Builtinfn *fn);
char *lisp_errmsg(const char *fmt, ...);
void lisp_printlval(Lval *lval);

void lisp_lcopy(Lval *restrict to, Lval *restrict from);
void lisp_ltrans(Lval *restrict to, Lval *restrict from);
void lisp_lsetnil(Lval *to);
void lisp_freelval(Lval *lval);

Lval *lisp_lvaleval(Lval *lval, Env *env);
i32 lisp_lvalcmp(const Lval *a, const Lval *b);
Lval *lisp_asttolval(mpc_ast_t *ast);

#endif
