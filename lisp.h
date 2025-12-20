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

void *print_val(value *val);

#endif
