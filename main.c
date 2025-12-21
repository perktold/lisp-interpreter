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
	// START REPL
	char* line;
	while ((line = readline("λ > ")) != NULL) {
		if (*line) {
			add_history(line);
		}
		yy_scan_string(line);
		yyparse();
		free(line);
	}
	return 0;
}
