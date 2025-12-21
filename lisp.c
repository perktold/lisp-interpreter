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

void *print_value(value *val){
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
			print_value(val->as.pair.car);
			printf(" . ");
			print_value(val->as.pair.cdr);
			printf(")");
		default:
	}

	return val;
}

env *env_create(env *parent) {
	env *e = malloc(sizeof(env));
	e->symbol = NULL;
	e->value = NULL;
	e->next = NULL;
	e->parent = parent;
	return e;
}

env *env_define(env *e, const char *sym, value *val) {
	if (e->next) {
		return env_define(e->next, sym, val);
	}
	env *new = malloc(sizeof(env));
	new->symbol = strdup(sym);
	new->value = val;
	new->next = NULL;
	new->parent = e->parent;
	e->next = new;
	return new;
}

value *env_lookup(env *e, const char *sym) {
	if (e->symbol && !strcmp(e->symbol, sym)) {
		return e->value;
	}
	if (e->next) {
		return env_lookup(e->next, sym);
	}
	if (e->parent) {
		return env_lookup(e->parent, sym);
	}
	return NULL;
}

int main(void) {
	/* TEST; DANKE CHATGPT
	// create global environment
	env *global = env_create(NULL);
	// define some variables
	env_define(global, "x", make_int(10));
	env_define(global, "y", make_int(42));
	
	// lookup variables
	value *vx = env_lookup(global, "x");
	value *vy = env_lookup(global, "y");
	value *vz = env_lookup(global, "z"); // undefined
	//
	if (vx) {
		printf("x = ");
		print_value(vx);
		printf("\n");
	}
	if (vx) {
		printf("y = ");
		print_value(vy);
		printf("\n");
	}
	if (vz) {
		printf("z = ");
		print_value(vz);
		printf("\n");
	}

	// create nested environment
	env *local = env_create(global);
	env_define(local, "x", make_int(99)); // shadow global x
	
	value *lx = env_lookup(local, "x"); // should find local
	value *ly = env_lookup(local, "y"); // should find global
	
	printf("local x = %d\n", lx ? lx->as.i : -1);
	printf("local y = %d\n", ly ? ly->as.i : -1);
	*/
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
