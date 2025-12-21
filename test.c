#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int main(void) {
	test_make_int();
	test_cons_car_cdr();
	test_env_define_lookup();
	test_env_parent_lookup();
	test_eval_literal();
	test_eval_symbol();
	test_define();

	printf("\nTests run: %d\n", tests_run);
	printf("Tests failed: %d\n", tests_failed);

	return tests_failed ? 1 : 0;
}
