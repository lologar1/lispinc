#include "main.h"

#include <editline/readline.h>
#include <editline/history.h>

Rpnval internal_rpneval(mpc_ast_t *ast);
u64 internal_checkrpnval(Rpnval val);

c_int main(c_int args, char *argv[]) {
	/* lispinc entry point */

	/* mpc initialization */
	mpc_parser_t *Number = mpc_new("number");
	mpc_parser_t *Operator = mpc_new("operator");
	mpc_parser_t *Expression = mpc_new("expression");
	mpc_parser_t *Rpn = mpc_new("rpn");
	mpca_lang(MPCA_LANG_DEFAULT,
		"number: /-?[0-9]+/;"
		"operator: '+' | '-' | '*' | '/' | '%';"
		"expression: <number> | '(' <operator> <expression>+ ')';"
		"rpn: /^/ <operator> <expression>+ /$/;",
		Number, Operator, Expression, Rpn);

	char *input; /* user input */
	while ((input = readline(">"))) {
		add_history(input);

		mpc_result_t res;
		if (mpc_parse("stdin", input, Rpn, &res)) {
			mpc_ast_print(res.output); fputc('\n', stdout);

			u64 value = internal_checkrpnval(internal_rpneval((mpc_ast_t *) res.output));
			printf(AESC_COLOR_FG_CYAN"%"PRIu64"\n"AESC_RESET_ALL, value); /* REPL */

			mpc_ast_delete(res.output);
		} else {
			mpc_err_print(res.error);
			mpc_err_delete(res.error);
		}

		free(input); /* alloc'd by editline */
	}

	printf("\n"AESC_COLOR_SET_BOLD"Program end."AESC_RESET_ALL"\n");
	mpc_cleanup(4, Number, Operator, Expression, Rpn);

	return 0;
}

Rpnval internal_rpneval(mpc_ast_t *ast) {
	/* Evaluate AST as an RPN (+ - * /) expression and return value */

	Rpnval retval;
	if (strstr(ast->tag, "number")) {
		retval.type = RPNVAL_VALUE;
		retval.val = strtou64(ast->contents, NULL, 10); /* base case */
		return retval; /* stack copy */
	}

	mpc_ast_t *operator = ast->children[1]; /* skip start of file or '(' */
	u64 op0 = internal_checkrpnval(internal_rpneval(ast->children[2])),
		op1 = internal_checkrpnval(internal_rpneval(ast->children[3]));

	retval.type = RPNVAL_VALUE; /* reset on error */
	switch (operator->contents[0]) { /* operator always single-char */
		default: retval.val = 0; printf("Unknown operator %c.\n", operator->contents[0]); break;
		case '+': retval.val = op0 + op1; break;
		case '-': retval.val = op0 - op1; break;
		case '*': retval.val = op0 * op1; break;
		case '/': {
			if (op1) retval.val = op0 / op1; /* safe */
			else retval.err = "Division by 0", retval.type = RPNVAL_ERROR;
			break;
		} case '%': {
			if (op1) retval.val = op0 % op1; /* safe */
			else retval.err = "Modulo by 0", retval.type = RPNVAL_ERROR;
		}
	}

	return retval;
}

u64 internal_checkrpnval(Rpnval val) {
	/* Check RPN evaluation return value for error and return numerical value (0 if err) */

	if (val.type == RPNVAL_ERROR) printf(AESC_COLOR_FG_RED"error during evaluation: %s\n", val.err);
	else return val.val;
	return 0; /* if error */
}
