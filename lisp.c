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

reg *global_reg = NULL;

value *make_int(int i) {
	value *val = malloc(sizeof(value));
	val->type = VT_INT;
	val->as.i = i;
	val->reachable = 0;
	reg_add(val);
	return val;
}

value *make_double(double d) {
	value *val = malloc(sizeof(value));
	val->type = VT_DOUBLE;
	val->as.d = d;
	val->reachable = 0;
	reg_add(val);
	return val;
}

value *make_symbol(const char *s) {
	value *val = malloc(sizeof(value));
	val->type = VT_SYMBOL;
	val->as.sym = strdup(s);
	val->reachable = 0;
	reg_add(val);
	return val;
}

value *make_string(const char *s) {
	value *val = malloc(sizeof(value));
	val->type = VT_STRING;
	val->as.str = strdup(s);
	val->reachable = 0;
	reg_add(val);
	return val;
}

value *NIL;
value *make_nil() {
	if (!NIL) {
		NIL = malloc(sizeof(value));
		NIL->type = VT_NIL;
		NIL->reachable = 1;
		NIL->as.pair.car = NIL; //might be bogus
		NIL->as.pair.cdr = NIL; //might be bogus
	}
	return NIL;
}

value *make_lambda(env *e, value *params, value *body) {
	value *l = malloc(sizeof(value));
	l->type = VT_LAMBDA;
	l->as.lambda.params = params;
	l->as.lambda.body = body;
	l->as.lambda.env = e;
	l->reachable = 0;
	reg_add(l);
	return l;
}

value *make_procedure(value *(*fn) (env *, value *)) {
	value *val = malloc(sizeof(value));
	val->type = VT_PROCEDURE;
	val->as.procedure.fn = fn;
	reg_add(val);
	return val;
}

value *make_error(const char *s) {
	value *val = malloc(sizeof(value));
	val->type = VT_ERROR;
	val->as.err = strdup(s);
	reg_add(val);
	return val;
}

value *make_thunk(env *e, value *expr) {
	// don't thunk self-evaluating expressions
	if (expr->type == VT_INT || 
            expr->type == VT_DOUBLE || 
            expr->type == VT_STRING ||
            expr->type == VT_NIL) {
		return expr;
	}

	value *val = malloc(sizeof(value));
	val->type = VT_THUNK;
	val->as.thunk.expr = expr;
	val->as.thunk.cached = NULL;
	val->as.thunk.env = e;
	reg_add(val);
	return val;
}

value *force_thunk(value *v) {
	if(v->type != VT_THUNK) {
		return v;
	}
	if(v->as.thunk.cached) {
		return v->as.thunk.cached;
	}
	
	value *evaled = eval(v->as.thunk.env, v->as.thunk.expr);
	evaled = force_thunk(evaled);
	v->as.thunk.cached = evaled;
	return evaled;
}

value *cons(value *car, value *cdr) {
	value *val = malloc(sizeof(value));
	val->type = VT_PAIR;
	val->as.pair.car = car;
	val->as.pair.cdr = cdr;
	reg_add(val);
	return val;
}

value *car(value *cons) {
	if (cons && cons->type == VT_PAIR) {
		return cons->as.pair.car;
	}
	printf("car of non-pair: ");
	println_value(cons);
	return make_error("car of non-pair: ");
}	

value *cdr(value *cons) {
	if (cons && cons->type == VT_PAIR) {
		return cons->as.pair.cdr;
	}
	printf("cdr of non-pair: ");
	println_value(cons);
	return make_error("cons of non-pair: ");
}

value *reverse(value *list) {
	value *reversed = make_nil();
	for (value *a = list; a->type == VT_PAIR; a = cdr(a)) {
		reversed = cons(car(a), reversed);
	}
	return reversed;
}

void println_value(value *val) {
	print_value(val);
	printf("\n");
}

void print_value(value *val) {
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
			printf("(");
			print_value(force_thunk(car(val)));
			value *p_cdr = cdr(val);
			p_cdr = force_thunk(p_cdr);
			while (p_cdr->type == VT_PAIR || p_cdr->type == VT_THUNK) {
				printf(" ");
				print_value(car(p_cdr));
				p_cdr = cdr(p_cdr);
				p_cdr = force_thunk(p_cdr);
			}

			if(p_cdr->type != VT_NIL) {
				printf(" . ");
				print_value(p_cdr);
			}
			printf(")");
			break;

		case VT_LAMBDA:
			print_value(
				cons(make_symbol("λ"),
					cons(val->as.lambda.params, val->as.lambda.body)
				)
			);
			//printf("(λ ");
			//print_value(val->as.lambda.params);
			//printf(" ");
			//print_value(car(val->as.lambda.body));
			//printf(")");
			break;

		case VT_PROCEDURE:
			printf("<procedure>");
			break;

		case VT_ERROR:
			printf("(error \"%s\")", val->as.err);
			break;

		case VT_THUNK:
			print_value(force_thunk(val));
			break;
		default:
			printf("\n<unknown>[as str:%s][as f:%f][as int:%d]\n", val->as.str, val->as.d, val->as.i);
			printf("<unknown>[as pair: (%s . %s)\n", val->as.pair.car, val->as.pair.cdr);
			break;
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
	//nil evaluates to itself
	if (!v || v->type == VT_NIL) {
		return make_nil();
	}

	//symbols get evaluated
	if (v->type == VT_SYMBOL) {
		value *found = env_lookup(e, v->as.sym);
		if (!found) {
			printf("symbol %s not found\n", v->as.sym);
			return make_error("symbol not found");
			///return list(make_symbol("quote"), v); //TODO: return error
		}
		return force_thunk(found);
	}

	//pairs get special treatment
	if (v->type == VT_PAIR) {
		return eval_pair(e, v);
	}
	
	// strings and errors etc do not get evaluated further, thunks get forced
	v = force_thunk(v);
	return v;
}

value *eval_pair(env *e, value *v) {
	value *head = car(v);
	value *tail = cdr(v);

	// special forms
	if (head->type == VT_SYMBOL && !strcmp(head->as.sym, "quote")) {
		return car(tail);
	}

	if (head->type == VT_SYMBOL && !strcmp(head->as.sym, "define")) {
		value *defargs = tail;
		value *defname = car(defargs);

		//define does not bind to evaluated arguemnts but is lazy
		value *defval = make_thunk(e, car(cdr(defargs)));

		if (defname->type != VT_SYMBOL) {
			printf("error, expected symbol, found: ");
			println_value(defname);
			return make_error("error, expected symbol, found: ");
		}
		env_define(e, defname->as.sym, defval);
		return defval;
	}

	if (head->type == VT_SYMBOL && (!strcmp(head->as.sym, "lambda") ||
					!strcmp(head->as.sym, "\\") ||
					!strcmp(head->as.sym, "λ"))) {
		value *params = car(tail);
		value *body = cdr(tail);

		return make_lambda(e, params, body);
	}

	if (head->type == VT_SYMBOL && !strcmp(head->as.sym, "if")) {
		value *test = force_thunk(eval(e, car(tail)));
		if(test->type != VT_NIL) {
			return eval(e, car(cdr(tail)));
		} else {
			return eval(e, car(cdr(cdr(tail))));
		}
	}

	if (head->type == VT_SYMBOL && !strcmp(head->as.sym, "print")) {
		value *v = eval(e, car(tail));
		println_value(v);
		return v;
	}

	value *head_eval = force_thunk(eval(e, head));
	if (head_eval->type == VT_PROCEDURE) {
		return head_eval->as.procedure.fn(e, tail);
	}

	if (head_eval->type == VT_LAMBDA) {
		//wrap arguments as thunks
		value *args_thunks = make_nil();
		for(value *a = tail; a->type == VT_PAIR; a = cdr(a)) {
			args_thunks = cons(make_thunk(e, car(a)), args_thunks);
		}
		return apply(head_eval, reverse(args_thunks));
	}
	
	// else evaluate the list recursively
	return cons(head_eval, eval(e, cdr(v)));
}

value *apply(value *lval, value *args) {

	if (lval->type != VT_LAMBDA) {
		//ERROR
		return make_error("not a lambda: ");
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

	// are all lambda parameters bound to respective args?
	if (params->type == VT_NIL) {
		value *lbody = lval->as.lambda.body;
		value *result = make_nil();

		while (lbody->type == VT_PAIR) {
			result = eval(sub_env, car(lbody));
			lbody = cdr(lbody);
		}

		// if arguments left, try applying them to evaluated lambda
		if (arg->type == VT_PAIR && result->type == VT_LAMBDA) {
			return apply(result, arg);
		}

		if (arg->type == VT_PAIR) {
			return make_error("not a lambda: ");
		}
		return result;
	}

	// if not all parameters bound, return partially applied lambda
	return make_lambda(sub_env, params, lval->as.lambda.body);
}

value *procedure_load_module(env *e, value *args) {
	value *fst_arg = eval(e, car(args));

	if (fst_arg->type != VT_STRING) {
		printf("error: not a string:");
		print_value(fst_arg);
		return make_error("not a string: ");
	}

	const char *path = fst_arg->as.str;

	void *handle = dlopen(path, RTLD_LAZY);
	if (!handle) {
		printf("dlopen error: %s\n", dlerror());
		return make_error("dlopen error: ");
	}

	module_export *(*init)(void) = dlsym(handle, "module_init");

	if (!init) {
		printf("not a valid module: %s\n", dlerror());
		return make_error("not a valid module: ");
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

reg *reg_add(value *val) {
	reg *new = malloc(sizeof(reg));
	new->value = val;
	new->next = global_reg;
	global_reg = new;
	return new;
}

int mark_val(value *val) {
	if(!val) {
		return 0;
	}
	//println_value(val);
	switch (val->type) {
		case VT_INT:
		case VT_DOUBLE:
		case VT_SYMBOL:
		case VT_STRING:
		case VT_ERROR:
		case VT_PROCEDURE:
			val->reachable = 1;
			return 1;

		case VT_PAIR:
			printf("inside pair\n");
			val->reachable = 1;
			return 1 + mark_val(val->as.pair.car)
			         + mark_val(val->as.pair.cdr);
			break;

		case VT_LAMBDA:
			printf("test\n");
			val->reachable = 1;
			return 1 + mark_val(val->as.lambda.params)
			         + mark_val(val->as.lambda.body);
			         + mark_env(val->as.lambda.env);
			break;

		case VT_THUNK:
			val->reachable = 1;
			return 1 + mark_val(val->as.thunk.expr)
			         + mark_val(val->as.thunk.cached);
			         + mark_env(val->as.thunk.env);
			break;

		case VT_NIL:
		default:
			return 0;
	}
}

int mark_env(env *envi) {
	int count=0;
	for (env *e = envi; e; e = e->next) {
		if (e->value) {
			count += mark_val(e->value);
		}
	}
	printf("mark_env count: %d\n", count);
	return count;
}

int sweep() {
	for (reg *r = global_reg; r; r = r->next) {
		if (r->value->reachable) {
			printf("reachable:");
			println_value(r->value);
		} else {
			printf("unreachable:");
			println_value(r->value);
		}
	}
}
