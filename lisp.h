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

value *procedure_cons(env *e, value *args);
value *procedure_car(env *e, value *args);
value *procedure_cdr(env *e, value *args);
value *procedure_reverse(env *e, value *args);
value *procedure_isnull(env *e, value *args);
value *procedure_equal(env *e, value *args);
value *procedure_add(env *e, value *args);
value *procedure_sub(env *e, value *args);
value *procedure_mul(env *e, value *args);
value *procedure_div(env *e, value *args);
value *procedure_lt(env *e, value *args);
value *procedure_load_module(env *e, value *args);

int is_integer(double x);

#endif
