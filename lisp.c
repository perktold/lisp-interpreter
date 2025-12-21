#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "lisp.h"
#include "lex.yy.h"
#include "lisp.tab.h"

value *make_int(int i) {
	value *val = malloc(sizeof(value));
	val->type = VT_INT;
	val->as.i = i;
	return val;
}

value *make_double(double d) {
	value *val = malloc(sizeof(value));
	val->type = VT_DOUBLE;
	val->as.d = d;
	return val;
}

value *make_symbol(const char *s) {
	value *val = malloc(sizeof(value));
	val->type = VT_SYMBOL;
	val->as.sym = s;
	return val;
}

value *make_string(const char *s) {
	value *val = malloc(sizeof(value));
	val->type = VT_STRING;
	val->as.str = s;
	return val;
}

value *make_nil() {
	value *val = malloc(sizeof(value));
	val->type = VT_NIL;
	return val;
}

value *make_lambda(env *e, value *params, value *body) {
	printf("making lambda:\n");
	value *l = malloc(sizeof(value));
	l->type = VT_LAMBDA;
	l->as.lambda.params = params;
	printf("params: ");
	print_value(params);
	l->as.lambda.body = body;
	printf("\nbody: ");
	print_value(body);
	printf("\n");
	l->as.lambda.env = e;
	return l;
}

value *cons(value *car, value *cdr) {
	value *val = malloc(sizeof(value));
	val->type = VT_PAIR;
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
		case VT_INT:
			printf("%d", val->as.i);
			break;

		case VT_DOUBLE:
			printf("%f", val->as.d);
			break;

		case VT_SYMBOL:
			printf("%s", val->as.sym);
			break;

		case VT_STRING:
			printf("\"%s\"", val->as.str);
			break;

		case VT_NIL:
			printf("()");
			break;

		case VT_PAIR:
			printf("(");
			print_value(val->as.pair.car);
			printf(" . ");
			print_value(val->as.pair.cdr);
			printf(")");
			break;

		case VT_LAMBDA:
			printf("(λ ");
			print_value(val->as.lambda.params);
			printf(" ");
			print_value(val->as.lambda.body);
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

	if (v->type == VT_NIL) {
		return v;
	}

	if (v->type == VT_SYMBOL &&
		(found = env_lookup(e, v->as.sym))) {
		return found;
	}

	if (v->type == VT_SYMBOL) {
		printf("symbol %s not found\n", v->as.sym);
		return v;
	}

	if (v->type == VT_PAIR) {
		return eval_pair(e, v);
	}

	return v;
}

value *eval_pair(env *e, value *v) {
	value *head = car(v);
	value *tail = cdr(v);

	value *head_eval = eval(e, head);

	if (head_eval->type == VT_LAMBDA) {
		apply(head_eval, tail);
	}

	if (!strcmp(head->as.str, "define")) {
		value *defargs = tail;
		value *defname = car(defargs);
		value *defval = eval(e, car(cdr(defargs)));

		if (defname->type != VT_SYMBOL) {
			printf("error, expected symbol, found: ");
			print_value(defname);
			printf("\n");
			return v;
		}
		env_define(e, defname->as.sym, defval);
	}

	if (!strcmp(head->as.str, "lambda") ||
	    !strcmp(head->as.str, "\\") ||
	    !strcmp(head->as.str, "λ")) {
		value *params = car(tail);
		value *body = car(cdr(tail));

		return make_lambda(e, params, body);
	}
	return v;
}

value *apply(value *lval, value *args) {
	env *sub_env = env_create(lval->as.lambda.env);
	value *params = lval->as.lambda.params;
	value *arg = args;

	while (params->type == VT_PAIR && arg->type == VT_PAIR) {
		env_define(sub_env,
			   car(params)->as.sym,
			   eval(lval->as.lambda.env, car(arg)));
		params = cdr(params);
		arg = cdr(arg);
	}
	return eval(sub_env, lval->as.lambda.body);
}
