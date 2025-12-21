#ifndef LISP_H
#define LISP_H

typedef enum {
	VALTYPE_INT,
	VALTYPE_DOUBLE,
	VALTYPE_SYMBOL,
	VALTYPE_STRING,
	VALTYPE_NIL,
	VALTYPE_PAIR
} val_type;

typedef struct value value;
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
	} as;
};

value *make_int(int i);
value *make_double(double d);
value *make_symbol(const char *sym);
value *make_string(const char *str);
value *make_nil();
value *cons(value *car, value *cdr);
value *car(value *cons);
value *cdr(value *cons);

void *print_value(value *val);
// environments, TODO: make this use hashtable
// symbol->val
typedef struct env env;
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

#endif
