#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <math.h>
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
	value *l = malloc(sizeof(value));
	l->type = VT_LAMBDA;
	l->as.lambda.params = params;
	l->as.lambda.body = body;
	l->as.lambda.env = e;
	return l;
}

value *make_builtin(value *(*fn) (env *, value *)){
	value *val = malloc(sizeof(value));
	val->type = VT_BUILTIN;
	val->as.builtin.fn = fn;
	return val;
}

value *cons(value *car, value *cdr) {
	value *val = malloc(sizeof(value));
	val->type = VT_PAIR;
	val->as.pair.car = car;
	val->as.pair.cdr = cdr;
	return val;
}

value *car(value *cons) {
	if (cons->type == VT_PAIR) {
		return cons->as.pair.car;
	}
	printf("not a pair: ");
	println_value(cons);
	return make_nil();
}	

value *cdr(value *cons) {
	if (cons->type == VT_PAIR) {
		return cons->as.pair.cdr;
	}
	printf("not a pair: ");
	println_value(cons);
	return make_nil();
}	

void *println_value(value *val){
	print_value(val);
	printf("\n");
}
void *print_value(value *val){
	char *str;
	switch (val->type) {
		case VT_INT:
			printf("%d", val->as.i);
			break;

		case VT_DOUBLE:
			printf("%g", val->as.d);
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
			// DOTTED:
			//printf("(");
			//print_value(val->as.pair.car);
			//printf(" . ");
			//print_value(val->as.pair.cdr);
			//printf(")");
			
			// LISTS:
			printf("(");
			print_value(car(val));
			value *p_cdr = cdr(val);
			while (p_cdr->type == VT_PAIR) {
				printf(" ");
				print_value(car(p_cdr));
				p_cdr = cdr(p_cdr);
			}
			if(p_cdr->type != VT_NIL) {
				printf(" . ");
				print_value(p_cdr);
			}
			printf(")");
			break;

		case VT_LAMBDA:
			printf("(λ ");
			print_value(val->as.lambda.params);
			printf(" ");
			print_value(val->as.lambda.body);
			printf(")");
			break;

		case VT_BUILTIN:
			printf("<builtin>\n");
			break;
		default:
			printf("<unknown>\n");
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
	//printf("evaluating: ");
	//print_value(v);
	//printf("\n");

	if (v->type == VT_NIL) {
		return v;
	}

	if (v->type == VT_PAIR) {
		return eval_pair(e, v);
	}

	if (v->type == VT_SYMBOL) {
		value *found = env_lookup(e, v->as.sym);
		if (!found) {
			printf("symbol %s not found\n", v->as.sym);
			return make_nil();
		}
		return found;
	}

	return v;
}

value *eval_pair(env *e, value *v) {
	value *head = car(v);
	value *tail = cdr(v);

	if (head->type == VT_SYMBOL && !strcmp(head->as.sym, "quote")) {
		return car(tail);
	}

	if (head->type == VT_SYMBOL && !strcmp(head->as.sym, "define")) {
		value *defargs = tail;
		value *defname = car(defargs);
		value *defval = eval(e, car(cdr(defargs)));

		if (defname->type != VT_SYMBOL) {
			printf("error, expected symbol, found: ");
			println_value(defname);
			return v;
		}
		env_define(e, defname->as.sym, defval);
		return defval;
	}

	if (head->type == VT_SYMBOL && (!strcmp(head->as.sym, "lambda") ||
		!strcmp(head->as.sym, "\\") ||
		!strcmp(head->as.sym, "λ"))) {
		value *params = car(tail);
		value *body = car(cdr(tail));

		return make_lambda(e, params, body);
	}

	if (head->type == VT_SYMBOL && !strcmp(head->as.sym, "if")) {
		value *test = eval(e, car(tail));
		if(test->type != VT_NIL) {
			return eval(e, car(cdr(tail)));
		} else {
			return eval(e, car(cdr(cdr(tail))));
		}
		return make_nil();
	}

	if (head->type == VT_SYMBOL && !strcmp(head->as.sym, "print")) {
		value *v = eval(e, car(tail));
		println_value(v);
		return v;
	}

	value *head_eval = eval(e, head);
	if (head_eval->type == VT_BUILTIN) {
		return head_eval->as.builtin.fn(e, tail);
	}

	if (head_eval->type == VT_LAMBDA) {
		return apply(head_eval, tail);
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

value *builtin_cons(env *e, value *args) {
	value *v = eval(e, car(args));
	return cons(v, eval(e, car(cdr(args))));
}

value *builtin_car(env *e, value *args) {
	return car(eval(e, car(args)));
}

value *builtin_cdr(env *e, value *args) {
	return cdr(eval(e, car(args)));
}

value *builtin_isnull(env *e, value *args) {
	value *v = eval(e, car(args));
	if(v->type == VT_NIL) {
		return make_int(1);
	}
	return make_nil();
}

value *apply_to_nums(env *e, value *args, double (*fn)(double, double)) {
	double result;
	value *v = eval(e, car(args));
	if (v->type == VT_INT) {
		result = v->as.i;
	} else if (v->type == VT_DOUBLE) {
		result = v->as.d;
	} else {
		printf("not a number: ");
		println_value(v);
		return make_nil();
	}

	args = cdr(args);
	while (args->type == VT_PAIR) {
		v = eval(e, car(args));
		if (v->type == VT_INT) {
			result = fn(result, v->as.i);
		} else if (v->type == VT_DOUBLE) {
			result = fn(result, v->as.d);
		} else {
			printf("not a number: ");
			println_value(v);
			return make_nil();
		}
		args = cdr(args);
	}

	if(is_integer(result)){
		return make_int((int) result);
	}
	return make_double(result);
}

double add_fn(double x, double y) { return x+y; }
value *builtin_add(env *e, value *args) {
	return apply_to_nums(e, args, add_fn);
}

double sub_fn(double x, double y) { return x-y; }
value *builtin_sub(env *e, value *args) {
	return apply_to_nums(e, args, sub_fn);
}

double mul_fn(double x, double y) { return x*y; }
value *builtin_mul(env *e, value *args) {
	return apply_to_nums(e, args, mul_fn);
}

double div_fn(double x, double y) { return x/y; }
value *builtin_div(env *e, value *args) {
	return apply_to_nums(e, args, div_fn);
}

value *builtin_le(env *e, value *args) {
	double prev_num;
	value *v = eval(e, car(args));

	if (v->type == VT_INT) {
		prev_num = v->as.i;
	} else if (v->type == VT_DOUBLE) {
		prev_num = v->as.d;
	} else {
		printf("not a number: ");
		println_value(v);
		return make_nil();
	}

	args = cdr(args);
	while (args->type == VT_PAIR) {
		v = eval(e, car(args));
		double cur_num;
		if (v->type == VT_INT) {
			cur_num = v->as.i;
		} else if (v->type == VT_DOUBLE) {
			cur_num = v->as.d;
		} else {
			printf("not a number: ");
			println_value(v);
			return make_nil();
		}
		if(prev_num > cur_num) {
			return make_nil();
		}
		prev_num = cur_num;
		args = cdr(args);
	}
	return make_int(1);
}

int is_integer(double x) {
    return floor(x) == x && isfinite(x);
}
