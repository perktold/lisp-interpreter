#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "lisp.h"
#include "lex.yy.h"
#include "lisp.tab.h"

env *global_env = NULL;


void eval_file(const char *filename) {
	FILE *f = fopen(filename, "r");
	if (!f) {
		fprintf(stderr, "could not open file: %s\n", filename);
		return;
	}

	char line[1024];
	while (fgets(line, sizeof(line), f)) {
		if (*line == '\n' || *line == ';') continue; // skip empty lines or comments
		yy_scan_string(line);
		yyparse();
	}
	fclose(f);
}

int main(int argc, char **argv) {
	// init global env
	global_env = env_create(NULL);
	env_define(global_env, "cons", make_builtin(builtin_cons));
	env_define(global_env, "car", make_builtin(builtin_car));
	env_define(global_env, "cdr", make_builtin(builtin_cdr));
	env_define(global_env, "null?", make_builtin(builtin_isnull));
	env_define(global_env, "eq?", make_builtin(builtin_eq));
	env_define(global_env, "+", make_builtin(builtin_add));
	env_define(global_env, "-", make_builtin(builtin_sub));
	env_define(global_env, "*", make_builtin(builtin_mul));
	env_define(global_env, "/", make_builtin(builtin_div));
	env_define(global_env, "<", make_builtin(builtin_lt));

	// evaluate files
	for(int i = 1; i < argc; i++) {
		eval_file(argv[i]);
	}

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
