#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "lisp_api.h"
#include "lisp.h"
#include "lex.yy.h"
#include "lisp.tab.h"

#define DEFAULT_PROMPT "λ> "

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

char **my_completion(const char *text, int start, int end) {
	rl_attempted_completion_over = 1;
	return rl_completion_matches(text, env_token_gen);
}

int caught_sigint = 0;
void handle_sigint(int sig) {
	printf("^C\n");

	caught_sigint = 1;
	rl_set_prompt(DEFAULT_PROMPT);
	rl_replace_line("", 0);
	rl_on_new_line();
	rl_redisplay();
}

void init_readline() {
	rl_attempted_completion_function = my_completion;

	rl_catch_signals = 0; /* disable readline's own handlers */
	struct sigaction sa = {0};
	sa.sa_handler = handle_sigint;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
}

int main(int argc, char **argv) {
	// init global value register
	global_reg = NULL;
	// init global env
	global_env = env_create(NULL);
	env_define(global_env, "load_module", make_procedure(procedure_load_module));
	procedure_load_module(global_env, cons(make_string("modules/std_lib.so"), make_nil()));

	// evaluate files
	for(int i = 1; i < argc; i++) {
		eval_file(argv[i]);
	}

	// START REPL
	init_readline();

	int parens = 0;
	char *line;
	char buffer[4096] = "";
	char *prompt = DEFAULT_PROMPT;
	while ((line = readline(prompt)) != NULL) {
		if (caught_sigint) {
			parens = 0;
			buffer[0] = '\0';
			prompt = DEFAULT_PROMPT;
			caught_sigint = 0;
		}
		//printf("DEBUG: %s\n", line);
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
			prompt = DEFAULT_PROMPT;
		} else {
			strcat(buffer, "\n");
			prompt = "   ";
		}
		free(line);
		int marked = mark_env(global_env); //maybe do this somewhere else?
		printf("marked: %d\n", marked);
		sweep();
	}
	return 0;
}
