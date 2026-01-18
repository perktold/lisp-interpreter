#include <stdio.h>
#include "../lisp_api.h"

value *hello_fun(env *e, value *args) {
	printf("Hello from dynamically loaded module!\n");
	return make_nil();
}

module_export *module_init() {
	static module_export exports[2];

	exports[0].name = "hello";
	exports[0].v    = make_procedure(&hello_fun);

	exports[1].name = NULL;
	exports[1].v    = NULL;

	return exports;
}
