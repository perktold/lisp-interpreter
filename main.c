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

	int parens = 0;
	char line[1024];
	char buffer[4096] = "";
	while (fgets(line, sizeof(line), f)) {
		if (*line == '\n' || *line == ';') continue; // skip empty lines or comments
		for (char *c = line; *c != '\0'; c++) {
			if (*c == '(') {
				parens++;
			} else if (*c == ')') {
				parens--;
			}
		}
		//append line to buffer
		strcat(buffer, line);
		strcat(buffer, "\n");
		if (parens <= 0) {
			//if parens are balanced: parse
			yy_scan_string(buffer);
			yyparse();
			//reset buffer
			buffer[0] = '\0';
		}
	}
	fclose(f);
}

char *env_token_gen(const char *text, int state) {
	static int len;
	static env *nth_env;
	if (state == 0) {
		nth_env = global_env->next;
		len = strlen(text);
	}

	while (nth_env) {
		const char *cur = nth_env->symbol;
		nth_env = nth_env->next;

		if (strncmp(cur, text, len) == 0) {
			return strdup(cur);
		}
	}

	return NULL;
}

/* Dispatch function: called by readline to get all matches */
char **my_completion(const char *text, int start, int end) {
    /* Prevent default filename completion */
	rl_attempted_completion_over = 1;
	return rl_completion_matches(text, env_token_gen);
}

void init_readline() {
	rl_attempted_completion_function = my_completion;
}

int main(int argc, char **argv) {
	// init global env
	global_env = env_create(NULL);
	env_define(global_env, "cons", make_builtin(builtin_cons));
	env_define(global_env, "car", make_builtin(builtin_car));
	env_define(global_env, "cdr", make_builtin(builtin_cdr));
	env_define(global_env, "reverse", make_builtin(builtin_reverse));
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
	init_readline();

	int parens = 0;
	char *line;
	char buffer[4096] = "";
	char *prompt = "λ> ";
	while ((line = readline(prompt)) != NULL) {
		for (char *c = line; *c != '\0'; c++) {
			if (*c == '(') {
				parens++;
			} else if (*c == ')') {
				parens--;
			}
		}
		//append line to buffer
		strcat(buffer, line);
		if (parens <= 0) {
			//if parens are balanced parse
			yy_scan_string(buffer);
			yyparse();
			add_history(buffer);
			//reset buffer and prompt string
			buffer[0] = '\0';
			prompt = "λ> ";
		} else {
			strcat(buffer, "\n");
			prompt = "   ";
		}
		free(line);
	}
	return 0;
}
