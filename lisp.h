#ifndef LISP_H
#define LISP_H

typedef struct value value;
typedef struct env env;

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

value *eval_pair(env *e, value *val);

value *apply(value *lambda, value *args);

value *procedure_load_module(env *e, value *args);

#endif
