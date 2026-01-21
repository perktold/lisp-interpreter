#ifndef LISP_API_H
#define LISP_API_H

typedef struct value value;
typedef struct env env;

value *eval(env *e, value *val);

void print_value(value *val);
void println_value(value *val);

typedef enum {
	VT_INT,
	VT_DOUBLE,
	VT_SYMBOL,
	VT_STRING,
	VT_NIL,
	VT_PAIR,
	VT_LAMBDA,
	VT_PROCEDURE,
	VT_ERROR,
	VT_THUNK
} val_type;

struct value {
	val_type type;
	union {
		int i;
		double d;
		const char *sym;
		const char *str;
		const char *err;
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
		} procedure;
		struct {
			value *expr;
			value *cached;
			env *env;
		} thunk;
	} as;
};

value *make_int(int i);
value *make_double(double d);
value *make_symbol(const char *sym);
value *make_string(const char *str);
value *make_nil();
value *make_lambda(env *e, value *params, value *body);
value *make_procedure(value *(*fn) (env *, value *));
value *make_error(const char *str);
value *make_thunk(env *e, value *expr);
value *force_thunk(value *v);
value *cons(value *car, value *cdr);
value *car(value *cons);
value *cdr(value *cons);
value *reverse(value *list);

typedef struct {
    const char *name;
    value *v;
} module_export;

module_export *module_init();

int is_integer(double x);

#endif
