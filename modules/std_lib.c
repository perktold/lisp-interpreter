#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "../lisp_api.h"

value *procedure_cons(env *e, value *args) {
	value *v = eval(e, car(args));
	return cons(v, make_thunk(e, car(cdr(args))));
}

value *procedure_car(env *e, value *args) {
	value *v = eval(e, car(args));
	while (v->type == VT_THUNK) {
		v = force_thunk(v);
	}
	return car(v);
}

value *procedure_cdr(env *e, value *args) {
	value *v = eval(e, car(args));
	while (v->type == VT_THUNK) {
		v = force_thunk(v);
	}
	return cdr(v);
}

value *procedure_reverse(env *e, value *args) {
	return reverse(eval(e, car(args)));
}

value *procedure_isnull(env *e, value *args) {
	value *v = eval(e, car(args));
	v = force_thunk(v); 
	if(v->type == VT_NIL) {
		return make_int(1);
	}
	return make_nil();
}

int value_eq(value *a, value *b) {
	a = force_thunk(a); 
	b = force_thunk(b); 
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
	v_prev = force_thunk(v_prev); 

	args = cdr(args);
	while (args->type == VT_PAIR) {
		value *v_cur = eval(e, car(args));
		v_cur = force_thunk(v_cur); 
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
	v = force_thunk(v);
	if (v->type == VT_INT) {
		result = v->as.i;
	} else if (v->type == VT_DOUBLE) {
		result = v->as.d;
	} else {
		printf("not a number: ");
		println_value(v);
		return make_error("not a number: ");
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
			return make_error("not a number: ");
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
	v = force_thunk(v);

	if (v->type == VT_INT) {
		prev_num = v->as.i;
	} else if (v->type == VT_DOUBLE) {
		prev_num = v->as.d;
	} else {
		printf("not a number: ");
		println_value(v);
		return make_error("not a number: ");
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
			return make_error("not a number: ");
		}
		if(prev_num >= cur_num) {
			return make_nil();
		}
		prev_num = cur_num;
		args = cdr(args);
	}
	return make_int(1);
}

value *procedure_list(env *e, value *args) {
	return eval(e, args);
}


module_export *module_init() {
	//exports[i].name = "xxx"
	//exports[i].v = make_procedure(xxx);
	static module_export exports[13];

	exports[0].name = "cons";
	exports[0].v = make_procedure(procedure_cons);
	exports[1].name = "car";
	exports[1].v = make_procedure(procedure_car);
	exports[2].name = "cdr";
	exports[2].v = make_procedure(procedure_cdr);
	exports[3].name = "reverse";
	exports[3].v = make_procedure(procedure_reverse);
	exports[4].name = "null?";
	exports[4].v = make_procedure(procedure_isnull);
	exports[5].name = "equal?";
	exports[5].v = make_procedure(procedure_equal);
	exports[6].name = "+";
	exports[6].v = make_procedure(procedure_add);
	exports[7].name = "-";
	exports[7].v = make_procedure(procedure_sub);
	exports[8].name = "*";
	exports[8].v = make_procedure(procedure_mul);
	exports[9].name = "/";
	exports[9].v = make_procedure(procedure_div);
	exports[10].name = "<";
	exports[10].v = make_procedure(procedure_lt);
	exports[11].name = "list";
	exports[11].v = make_procedure(procedure_list);

	exports[12].name = NULL;
	exports[12].v    = NULL;

	return exports;
}
