#ifndef LISP_H
#define LISP_H

typedef struct value value;
typedef struct env env;
typedef struct reg reg;

// environments, TODO: make this use hashtable
// symbol->val
typedef struct env {
	char *symbol;
	value *value;
	env *next;
	env *parent;
	int marked;
} env;

env *env_create(env *parent);
env *env_define(env *e, const char *sym, value *val);
value *env_lookup(env *e, const char *sym);
extern env *global_env;

value *eval_pair(env *e, value *val);

value *apply(value *lambda, value *args);

value *procedure_load_module(env *e, value *args);

typedef struct reg {
	value *value;
	reg *next;
} reg;

reg *reg_add(value *val);
extern reg *global_reg;
void mark_env(env *e);
void mark_val(value *v);
void sweep();
void free_env(env *e);
void free_value(value *v);
void reset_env_marks(env *e);
void reset_marks();
void print_register();

#endif
