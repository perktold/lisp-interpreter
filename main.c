#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "lisp.h"
#include "lex.yy.h"
#include "lisp.tab.h"


env *global_env = NULL;

int main(void) {
	// init global env
	global_env = env_create(NULL);
	env_define(global_env, "cons", make_builtin(builtin_cons));
	env_define(global_env, "car", make_builtin(builtin_car));
	env_define(global_env, "cdr", make_builtin(builtin_cdr));
	env_define(global_env, "null?", make_builtin(builtin_isnull));
	env_define(global_env, "+", make_builtin(builtin_add));
	env_define(global_env, "-", make_builtin(builtin_sub));
	env_define(global_env, "*", make_builtin(builtin_mul));
	env_define(global_env, "/", make_builtin(builtin_div));
	env_define(global_env, "<=", make_builtin(builtin_le));
	// START REPL
	char* line;
	while ((line = readline("λ> ")) != NULL) {
		if (*line) {
			add_history(line);
		}
		yy_scan_string(line);
		yyparse();
		free(line);
	}
	return 0;
}
