#ifndef LISP_H
#define LISP_H

typedef struct value value;
typedef struct env env;

typedef enum {
	VT_INT,
	VT_DOUBLE,
	VT_SYMBOL,
	VT_STRING,
	VT_NIL,
	VT_PAIR,
	VT_LAMBDA,
	VT_BUILTIN
} val_type;

struct value {
	val_type type;
	union {
		int i;
		double d;
		const char *sym;
		const char *str;
		struct {
			value *car;
			value *cdr;
		} pair;
		struct {
			value *params;
			value *body;
			env *env;
		} lambda;
		struct {
			value *(*fn) (env *, value *);
		} builtin;
	} as;
};

value *make_int(int i);
value *make_double(double d);
value *make_symbol(const char *sym);
value *make_string(const char *str);
value *make_nil();
value *make_lambda(env *e, value *params, value *body);
value *make_builtin(value *(*fn) (env *, value *));
value *cons(value *car, value *cdr);
value *car(value *cons);
value *cdr(value *cons);

void *print_value(value *val);
void *println_value(value *val);
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

value *apply(env *e, value *lambda, value *args);

value *builtin_cons(env *e, value *args);
value *builtin_car(env *e, value *args);
value *builtin_cdr(env *e, value *args);
value *builtin_isnull(env *e, value *args);
value *builtin_eq(env *e, value *args);
value *builtin_add(env *e, value *args);
value *builtin_sub(env *e, value *args);
value *builtin_mul(env *e, value *args);
value *builtin_div(env *e, value *args);
value *builtin_le(env *e, value *args);

int is_integer(double x);

#endif
