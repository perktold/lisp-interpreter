#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "lisp.h"
#include "lex.yy.h"
#include "lisp.tab.h"

value *make_int(int i) {
	value *val = malloc(sizeof(value));
	val->type = VALTYPE_INT;
	val->as.i = i;
	return val;
}

value *make_double(double d) {
	value *val = malloc(sizeof(value));
	val->type = VALTYPE_DOUBLE;
	val->as.d = d;
	return val;
}

value *make_symbol(const char *s) {
	value *val = malloc(sizeof(value));
	val->type = VALTYPE_SYMBOL;
	val->as.sym = s;
	return val;
}

value *make_string(const char *s) {
	value *val = malloc(sizeof(value));
	val->type = VALTYPE_STRING;
	val->as.str = s;
	return val;
}

value *make_nil() {
	value *val = malloc(sizeof(value));
	val->type = VALTYPE_NIL;
	return val;
}

value *cons(value *car, value *cdr) {
	value *val = malloc(sizeof(value));
	val->type = VALTYPE_PAIR;
	val->as.pair.car = car;
	val->as.pair.cdr = cdr;
	return val;
}

void *print_val(value *val){
	char *str;
	switch (val->type) {
		case VALTYPE_INT:
			printf("%d", val->as.i);
			break;

		case VALTYPE_DOUBLE:
			printf("%f", val->as.d);
			break;

		case VALTYPE_SYMBOL:
			printf("%s", val->as.sym);
			break;

		case VALTYPE_STRING:
			printf("\"%s\"", val->as.str);
			break;

		case VALTYPE_NIL:
			printf("()");
			break;

		case VALTYPE_PAIR:
			printf("(");
			print_val(val->as.pair.car);
			printf(" . ");
			print_val(val->as.pair.cdr);
			printf(")");
		default:
	}

	return val;
}

int main(void) {
    char* line;
    while ((line = readline("λ > ")) != NULL) {
        if (*line) add_history(line);
        yy_scan_string(line); // feed input to Flex/Bison
        yyparse();
        free(line);
    }
    return 0;
}
