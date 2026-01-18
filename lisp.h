#ifndef LISP_H
#define LISP_H

typedef struct value value;
typedef struct env env;

void print_value(value *val);
void println_value(value *val);
// environments, TODO: make this use hashtable
// symbol->val
typedef struct env {
	const char *symbol;
	value *value;
	env *next;
	env *parent;
} env;

env *env_create(env *parent);
env *env_define(env *e, const char *sym, value *val);
value *env_lookup(env *e, const char *sym);

extern env *global_env;

value *eval(env *e, value *val);
value *eval_pair(env *e, value *val);

value *apply(value *lambda, value *args);

value *builtin_cons(env *e, value *args);
value *builtin_car(env *e, value *args);
value *builtin_cdr(env *e, value *args);
value *builtin_reverse(env *e, value *args);
value *builtin_isnull(env *e, value *args);
value *builtin_equal(env *e, value *args);
value *builtin_add(env *e, value *args);
value *builtin_sub(env *e, value *args);
value *builtin_mul(env *e, value *args);
value *builtin_div(env *e, value *args);
value *builtin_lt(env *e, value *args);
value *builtin_load_module(env *e, value *args);

int is_integer(double x);

#endif
