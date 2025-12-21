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

value *car(value *cons) {
	//TODO: more robust implementation with error catching
	return cons->as.pair.car;
}	

value *cdr(value *cons) {
	//TODO: more robust implementation with error catching
	return cons->as.pair.cdr;
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

value *eval(env *e, value *v) {
	value *found;

	if (v->type == VALTYPE_NIL) {
		return v;
	}

	if (v->type == VALTYPE_SYMBOL &&
		(found = env_lookup(e, v->as.sym))) {
		return found;
	}

	if (v->type == VALTYPE_SYMBOL) {
		printf("symbol %s not found\n", v->as.sym);
		return v;
	}

	if (v->type == VALTYPE_PAIR) {
		return eval_pair(e, v);
	}

	return v;
}

value *eval_pair(env *e, value *v) {
	value *head = v->as.pair.car;
	value *tail = v->as.pair.cdr;

	if (head->type == VALTYPE_SYMBOL &&
	    tail->type == VALTYPE_PAIR) {
		if(!strcmp(head->as.str, "define")) {
			value *defargs = cdr(v);
			value *defname = car(defargs);
			value *defval = eval(e, car(cdr(defargs)));

			if (defname->type != VALTYPE_SYMBOL) {
				printf("error, expected symbol, found: ");
				print_value(defname);
				printf("\n");
				return v;
			}
			env_define(e, defname->as.sym, defval);
		}
	}
	return v;
}

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
