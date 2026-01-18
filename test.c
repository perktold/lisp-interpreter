#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp_api.h"
#include "lisp.h"
#include "lex.yy.h"
#include "lisp.tab.h"

static int tests_run = 0;
static int tests_failed = 0;

env *global_env = NULL;

#define ASSERT(cond, msg) do {			\
	tests_run++;				\
	if (!(cond)) {				\
		tests_failed++;			\
		printf("FAIL: %s\n", msg);	\
	} else {				\
		printf("PASS: %s\n", msg);	\
	}					\
} while (0)

void test_make_int() {
	value *v = make_int(42);
	ASSERT(v->type == VT_INT, "make_int sets type");
	ASSERT(v->as.i == 42, "make_int sets value");
}

void test_cons_car_cdr() {
	value *a = make_int(1);
	value *b = make_int(2);
	value *p = cons(a, b);

	ASSERT(p->type == VT_PAIR, "cons creates pair");
	ASSERT(car(p)->as.i == 1, "car works");
	ASSERT(cdr(p)->as.i == 2, "cdr works");
}


void test_env_define_lookup() {
	env *e = env_create(NULL);
	value *v = make_int(10);

	env_define(e, "x", v);
	value *found = env_lookup(e, "x");

	ASSERT(found != NULL, "env_lookup finds symbol");
	ASSERT(found->as.i == 10, "env_lookup returns correct value");
}

void test_env_parent_lookup() {
	env *parent = env_create(NULL);
	env_define(parent, "x", make_int(5));

	env *child = env_create(parent);
	value *found = env_lookup(child, "x");

	ASSERT(found != NULL, "lookup finds parent symbol");
	ASSERT(found->as.i == 5, "parent symbol value correct");
}

void test_eval_literal() {
	env *e = env_create(NULL);

	value *v = make_int(7);
	value *r = eval(e, v);

	ASSERT(r->as.i == 7, "eval of int returns itself");
}

void test_eval_symbol() {
	env *e = env_create(NULL);
	env_define(e, "x", make_int(99));

	value *sym = make_symbol("x");
	value *r = eval(e, sym);

	ASSERT(r->as.i == 99, "eval resolves symbol");
}


void test_define() {
	env *e = env_create(NULL);

	value *expr =cons(make_symbol("define"),
			cons(make_symbol("x"),
				cons(make_int(3), make_nil())));

	eval(e, expr);
	value *r = env_lookup(e, "x");

	ASSERT(r != NULL, "define creates binding");
	ASSERT(r->as.i == 3, "define sets correct value");
}


void test_lambda_identity() {
	env *e = env_create(NULL);

	// ((lambda (x) x) 5)
	value *expr =
		cons(
			cons(make_symbol("lambda"),
				cons(
					cons(make_symbol("x"), make_nil()),
					cons(make_symbol("x"), make_nil())
				)
			),
			cons(make_int(5), make_nil())
		);

	value *r = eval(e, expr);

	ASSERT(r->type == VT_INT, "lambda identity returns int");
	ASSERT(r->as.i == 5, "lambda identity returns argument");
}

void test_lambda_add() {
	env *e = env_create(NULL);

	// install +
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));
	

	value *expr =
		cons(
			cons(make_symbol("lambda"),
				cons(
					cons(make_symbol("x"), make_nil()),
					cons(
						cons(make_symbol("+"),
							cons(make_symbol("x"),
								cons(make_int(1), make_nil())
							)
						),
						make_nil()
					)
				)
			),
			cons(make_int(4), make_nil())
		);

	value *r = eval(e, expr);

	ASSERT(r->as.i == 5, "lambda body with + works");
}


void test_lambda_multiple_args() {
	env *e = env_create(NULL);
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));

	value *expr =
		cons(
			cons(make_symbol("lambda"),
				cons(
					cons(make_symbol("x"),
						cons(make_symbol("y"), make_nil())
					),
					cons(
						cons(make_symbol("+"),
							cons(make_symbol("x"),
								cons(make_symbol("y"), make_nil())
							)
						),
						make_nil()
					)
				)
			),
			cons(make_int(2),
				cons(make_int(3), make_nil())
			)
		);

	value *r = eval(e, expr);

	ASSERT(r->as.i == 5, "lambda with multiple parameters");
}


void test_lambda_closure() {

	//(define x 10)
	//((lambda (y) (+ x y)) 5)

	env *e = env_create(NULL);
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));
	
	env_define(e, "x", make_int(10));

	value *expr =
		cons(
			cons(make_symbol("lambda"),
				cons(
					cons(make_symbol("y"), make_nil()),
					cons(
						cons(make_symbol("+"),
							cons(make_symbol("x"),
								cons(make_symbol("y"), make_nil())
							)
						),
						make_nil()
					)
				)
			),
			cons(make_int(5), make_nil())
		);

	value *r = eval(e, expr);

	ASSERT(r->as.i == 15, "lambda closes over outer environment");
}

void test_nested_lambdas() {
	// (((lambda (x) (lambda (y) (+ x y))) 3) 4)
	env *e = env_create(NULL);
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));

	value *expr =
		cons(
			cons(
				cons(make_symbol("lambda"),
					cons(
						cons(make_symbol("x"), make_nil()),
						cons(
							cons(make_symbol("lambda"),
								cons(
									cons(make_symbol("y"), make_nil()),
									cons(
										cons(make_symbol("+"),
											cons(make_symbol("x"),
												cons(make_symbol("y"), make_nil())
											)
										),
										make_nil()
									)
								)
							),
							make_nil()
						)
					)
				),
				cons(make_int(3), make_nil())
			),
			cons(make_int(4), make_nil())
		);

	value *r = eval(e, expr);

	ASSERT(r->as.i == 7, "nested lambdas capture variables");
}

void test_lambda_does_not_modify_outer_env() {
	env *e = env_create(NULL);
	env_define(e, "x", make_int(1));

	value *expr =
		cons(
			cons(make_symbol("lambda"),
				cons(
					cons(make_symbol("x"), make_nil()),
					cons(make_symbol("x"), make_nil())
				)
			),
			cons(make_int(99), make_nil())
		);

	value *r = eval(e, expr);
	value *outer = env_lookup(e, "x");

	ASSERT(r->as.i == 99, "lambda returns inner x");
	ASSERT(outer->as.i == 1, "outer x unchanged");
}

void test_recursive_factorial() {
	env *e = env_create(NULL);

	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));

	/*
	(define fact
	    (lambda (n)
		(if (< n 2)
		    1
		    (* n (fact (- n 1))))))
	*/
	value *fact_def =
		cons(make_symbol("define"),
			cons(make_symbol("fact"),
				cons(
					cons(make_symbol("lambda"),
						cons(
							cons(make_symbol("n"), make_nil()),
							cons(
								cons(make_symbol("if"),
									cons(
										cons(make_symbol("<"),
											cons(make_symbol("n"),
												cons(make_int(2), make_nil())
											)
										),
										cons(
											make_int(1),
											cons(
												cons(make_symbol("*"),
													cons(make_symbol("n"),
														cons(
															cons(make_symbol("fact"),
																cons(
																	cons(make_symbol("-"),
																		cons(make_symbol("n"),
																			cons(make_int(1), make_nil())
																		)
																	),
																	make_nil()
																)
															),
															make_nil()
														)
													)
												),
												make_nil()
											)
										)
									)
								),
								make_nil()
							)
						)
					),
					make_nil()
				)
			)
		);

	eval(e, fact_def);

	// (fact 5)
	value *call =
		cons(make_symbol("fact"),
			cons(make_int(5), make_nil())
		);

	value *r = eval(e, call);
	ASSERT(r->as.i == 120, "recursive factorial works");
}

void test_eq_ints() {
	env *e = env_create(NULL);
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));
	

	value *expr =
		cons(make_symbol("equal?"),
			cons(make_int(3),
				cons(make_int(3), make_nil())
			)
		);

	value *r = eval(e, expr);
	ASSERT(r->type == VT_INT, "eq returns true as int");
	ASSERT(r->as.i == 1, "eq works on equal ints");
}


void test_eq_ints_false() {
	env *e = env_create(NULL);
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));
	

	value *expr =
		cons(make_symbol("equal?"),
			cons(make_int(3),
				cons(make_int(4), make_nil())
			)
		);

	value *r = eval(e, expr);
	ASSERT(r->type == VT_NIL, "eq returns nil on unequal ints");
}

void test_eq_symbols() {
	env *e = env_create(NULL);
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));
	

	value *expr =
		cons(make_symbol("equal?"),
			cons(make_symbol("foo"),
				cons(make_symbol("foo"), make_nil())
			)
		);

	value *r = eval(e, expr);
	ASSERT(r->as.i == 1, "eq works on symbols");
}

void test_eq_lists() {
	env *e = env_create(NULL);
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));
	

	value *list1 =
		cons(make_int(1),
			cons(make_int(2), make_nil())
		);

	value *list2 =
		cons(make_int(1),
			cons(make_int(2), make_nil())
		);

	value *expr =
		cons(make_symbol("equal?"),
			cons(list1,
				cons(list2, make_nil())
			)
		);

	value *r = eval(e, expr);
	ASSERT(r->as.i == 1, "eq works on structurally equal lists");
}

void test_null_true() {
	env *e = env_create(NULL);
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));
	

	value *expr =
		cons(make_symbol("null?"),
			cons(make_nil(), make_nil())
		);

	value *r = eval(e, expr);
	ASSERT(r->as.i == 1, "null? true on ()");
}

void test_null_false() {
	env *e = env_create(NULL);
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));
	

	value *expr =
		cons(make_symbol("null?"),
			cons(
				cons(make_int(1), make_nil()),
				make_nil()
			)
		);

	value *r = eval(e, expr);
	ASSERT(r->type == VT_NIL, "null? false on non-empty list");
}

void test_if_true() {
	env *e = env_create(NULL);
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));

	value *expr =
		cons(make_symbol("if"),
			cons(
				make_int(1),              // true (non-nil)
				cons(make_int(42),
					cons(make_int(0), make_nil())
				)
			)
		);

	value *r = eval(e, expr);
	ASSERT(r->as.i == 42, "if true branch evaluated");
}

void test_if_false() {
	env *e = env_create(NULL);

	value *expr =
		cons(make_symbol("if"),
			cons(
				make_nil(),               // false
				cons(make_int(1),
					cons(make_int(2), make_nil())
				)
			)
		);

	value *r = eval(e, expr);
	ASSERT(r->as.i == 2, "if false branch evaluated");
}

void test_if_short_circuit() {
	env *e = env_create(NULL);

	// (if 1 42 undefined-symbol)
	value *expr =
		cons(make_symbol("if"),
			cons(
				make_int(1),               // true
				cons(make_int(42),
					cons(make_symbol("nope"), make_nil())
				)
			)
		);

	value *r = eval(e, expr);
	ASSERT(r->as.i == 42, "if short-circuits unused branch");
}

void test_add_multiple_args() {
	env *e = env_create(NULL);
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));

	value *expr =
		cons(make_symbol("+"),
			cons(make_int(1),
				cons(make_int(2),
					cons(make_int(3), make_nil())
				)
			)
		);

	value *r = eval(e, expr);
	ASSERT(r->as.i == 6, "+ works with multiple args");
}

void test_lt_chain() {
	env *e = env_create(NULL);
	procedure_load_module(e, cons(make_string("modules/std_lib.so"), make_nil()));

	value *expr =
		cons(make_symbol("<"),
			cons(make_int(1),
				cons(make_int(2),
					cons(make_int(3), make_nil())
				)
			)
		);

	value *r = eval(e, expr);
	ASSERT(r->as.i == 1, "< works as chain comparison");
}

int main(void) {
	printf("lol 1\n");
	test_make_int();
	printf("lol 2\n");
	test_cons_car_cdr();
	printf("lol 3\n");
	test_env_define_lookup();
	printf("lol 4\n");
	test_env_parent_lookup();
	printf("lol 5\n");
	test_eval_literal();
	printf("lol 6\n");
	test_eval_symbol();
	printf("lol 7\n");
	test_define();
	printf("lol 8\n");
	test_lambda_identity();
	printf("lol 9\n");
	test_lambda_add();
	printf("lol 10\n");
	test_lambda_multiple_args();
	printf("lol 11\n");
	test_lambda_closure();
	printf("lol 12\n");
	test_nested_lambdas();
	printf("lol 13\n");
	test_lambda_does_not_modify_outer_env();
	printf("lol 14\n");
	test_recursive_factorial();
	printf("lol 15\n");
	test_eq_ints();
	printf("lol 16\n");
	test_eq_ints_false();
	printf("lol 17\n");
	test_eq_symbols();
	printf("lol 18\n");
	test_eq_lists();
	printf("lol 19\n");
	test_null_true();
	printf("lol 20\n");
	test_null_false();
	printf("lol 21\n");
	test_if_true();
	printf("lol 22\n");
	test_if_false();
	printf("lol 23\n");
	test_if_short_circuit();
	printf("lol 24\n");
	test_add_multiple_args();
	printf("lol 25\n");
	test_lt_chain();

	printf("\nTests run: %d\n", tests_run);
	printf("Tests failed: %d\n", tests_failed);

	return tests_failed ? 1 : 0;
}
