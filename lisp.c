#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <math.h>
#include <dlfcn.h>
#include "lisp_api.h"
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
	val->as.sym = strdup(s);
	return val;
}

value *make_string(const char *s) {
	value *val = malloc(sizeof(value));
	val->type = VT_STRING;
	val->as.str = strdup(s);
	return val;
}

value *NIL;
value *make_nil() {
	if (!NIL) {
		NIL = malloc(sizeof(value));
		NIL->type = VT_NIL;
	}
	return NIL;
}

value *make_lambda(env *e, value *params, value *body) {
	value *l = malloc(sizeof(value));
	l->type = VT_LAMBDA;
	l->as.lambda.params = params;
	l->as.lambda.body = body;
	l->as.lambda.env = e;
	return l;
}

value *make_procedure(value *(*fn) (env *, value *)){
	value *val = malloc(sizeof(value));
	val->type = VT_PROCEDURE;
	val->as.procedure.fn = fn;
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
	if (cons && cons->type == VT_PAIR) {
		return cons->as.pair.car;
	}
	printf("car of non-pair: ");
	println_value(cons);
	return make_nil();
}	

value *cdr(value *cons) {
	if (cons && cons->type == VT_PAIR) {
		return cons->as.pair.cdr;
	}
	printf("cdr of non-pair: ");
	println_value(cons);
	return make_nil();
}

value *reverse(value *list) {
	value *reversed = make_nil();
	for(value *a = list; a->type == VT_PAIR; a = cdr(a)) {
		reversed = cons(car(a), reversed);
	}
	return reversed;
}

void println_value(value *val){
	print_value(val);
	printf("\n");
}
void print_value(value *val){
	if(!val) { return; }
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

		case VT_PROCEDURE:
			printf("<procedure>\n");
			break;
		default:
			printf("<unknown>\n");
	}
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
	//if symbol has been found, redefine
	if (e->symbol && !strcmp(e->symbol, sym)) {
		e->value = val;
		return e;
	}
	//if not, find end of list and add new definition
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
	
	if (!v || v->type == VT_NIL) {
		return make_nil();
	}

	if (v->type == VT_PAIR) {
		//printf("pair found: ");
		//println_value(v);
		return eval_pair(e, v);
	}

	if (v->type == VT_SYMBOL) {
		value *found = env_lookup(e, v->as.sym);
		if (!found) {
			printf("symbol %s not found\n", v->as.sym);
			return cons(make_symbol("quote"),
				cons(v, make_nil())); //TODO: return error
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
			return make_nil();
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
	if (head_eval->type == VT_PROCEDURE) {
		return head_eval->as.procedure.fn(e, tail);
	}

	if (head_eval->type == VT_LAMBDA) {
		//evaluate arguments first, then apply
		value *args_eval = make_nil();
		for(value *a = tail; a->type == VT_PAIR; a = cdr(a)) {
			args_eval = cons(eval(e, car(a)), args_eval);
		}
		return apply(head_eval, reverse(args_eval));
	}
	
	// else evaluate the list recursively
	return cons(head_eval, eval(e, cdr(v)));
}

value *apply(value *lval, value *args) {

	if (lval->type != VT_LAMBDA) {
		//ERROR
		return make_nil();
	}

	env *sub_env = env_create(lval->as.lambda.env);
	value *params = lval->as.lambda.params;
	value *arg = args;

	// bind all available args in subenv
	while (params->type == VT_PAIR && arg->type == VT_PAIR) {
		env_define(sub_env, car(params)->as.sym, car(arg));
		params = cdr(params);
		arg = cdr(arg);
	}

	// all lambda parameters bound to respective args
	if (params->type == VT_NIL) {
		value *result = eval(sub_env, lval->as.lambda.body);

		// if arguments left, try applying them to evaluated lambda
		if (arg->type == VT_PAIR && result->type == VT_LAMBDA) {
			return apply(result, arg);
		}
		// if arguments left but result is not a lambda, explicitely return false TODO: return an error
		if (arg->type == VT_PAIR) {
			return make_nil();
		}
		return result;
	}

	// if not all parameters bound, return partially applied lambda
	return make_lambda(sub_env, params, lval->as.lambda.body);
}

value *procedure_cons(env *e, value *args) {
	value *v = eval(e, car(args));
	return cons(v, eval(e, car(cdr(args))));
}

value *procedure_car(env *e, value *args) {
	return car(eval(e, car(args)));
}

value *procedure_cdr(env *e, value *args) {
	return cdr(eval(e, car(args)));
}

value *procedure_reverse(env *e, value *args) {
	return reverse(eval(e, car(args)));
}

value *procedure_isnull(env *e, value *args) {
	value *v = eval(e, car(args));
	if(v->type == VT_NIL) {
		return make_int(1);
	}
	return make_nil();
}

int value_eq(value *a, value *b) {
	if(a->type != b->type){
		return 0;
	}
	if(a == b){
		return 1;
	}
	switch (a->type) {
		case VT_INT: return a->as.i == b->as.i;
		case VT_DOUBLE: return a->as.d == b->as.d;
		case VT_SYMBOL: return !strcmp(a->as.sym, b->as.sym);
		case VT_STRING: return !strcmp(a->as.sym, b->as.sym);
		case VT_NIL: return 1;
		case VT_PAIR:
			return value_eq(car(a), car(b)) &&
				value_eq(cdr(a), cdr(b));
		case VT_LAMBDA:
		case VT_PROCEDURE:
		default:
			return 0;
	}
	return 1;
}

value *procedure_equal(env *e, value *args) {
	value *v_prev = eval(e, car(args));

	args = cdr(args);
	while (args->type == VT_PAIR) {
		value *v_cur = eval(e, car(args));
		if (!value_eq(v_prev, v_cur)) {
			return make_nil();
		}
		v_prev = v_cur;
		args = cdr(args);
	}

	return make_int(1);
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
value *procedure_add(env *e, value *args) {
	return apply_to_nums(e, args, add_fn);
}

double sub_fn(double x, double y) { return x-y; }
value *procedure_sub(env *e, value *args) {
	return apply_to_nums(e, args, sub_fn);
}

double mul_fn(double x, double y) { return x*y; }
value *procedure_mul(env *e, value *args) {
	return apply_to_nums(e, args, mul_fn);
}

double div_fn(double x, double y) { return x/y; }
value *procedure_div(env *e, value *args) {
	return apply_to_nums(e, args, div_fn);
}

value *procedure_lt(env *e, value *args) {
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
		if(prev_num >= cur_num) {
			return make_nil();
		}
		prev_num = cur_num;
		args = cdr(args);
	}
	return make_int(1);
}

value *procedure_load_module(env *e, value *args) {
	value *fst_arg = eval(e, car(args));

	if (fst_arg->type != VT_STRING) {
		printf("error: not a string:");
		print_value(fst_arg);
		return make_nil(); //TODO: make_err
	}

	const char *path = fst_arg->as.str;

	void *handle = dlopen(path, RTLD_LAZY);
	if (!handle) {
		printf("dlopen error: %s\n", dlerror());
		return make_nil(); //TODO: make_err
	}

	module_export *(*init)(void) = dlsym(handle, "module_init");

	if (!init) {
		printf("not a valid module: %s\n", dlerror());
		return make_nil(); //TODO: make_err
	}

	module_export *exports = init();
	for (int i = 0; exports[i].name; i++) {
		env_define(e, exports[i].name, exports[i].v);
	}

	return make_int(1);
}

int is_integer(double x) {
	return floor(x) == x && isfinite(x);
}
