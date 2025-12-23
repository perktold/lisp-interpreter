%{
#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

int yylex(void);
int yyerror(const char *s);

void eval_and_print_REPL(value *v) {
        if(!v) { return; }
        value *ve = eval(global_env, v);
        printf(";> ");
        println_value(ve);
}
%}

%union {
        double dval;
        const char *strval;
        value *val;
}
%token <dval> NUMBER
%token <strval> STRING SYMBOL
%token LPAREN RPAREN DOT QUOTE

%type <val> expr atom list list_items

%start exprs

%%
exprs: /* nothing */
     | expr { eval_and_print_REPL($1); }
     | exprs expr { eval_and_print_REPL($2); }

expr:  atom
     | list
     | LPAREN expr DOT expr RPAREN { $$ = cons($2, $4); }
     | QUOTE expr { $$ = cons(make_symbol("quote"), cons($2, make_nil())); }

list: LPAREN list_items RPAREN { $$ = $2; };

list_items: /* empty list */ { $$ = make_nil(); }
          | expr list_items { $$ = cons($1, $2); }
          | expr DOT expr { $$ = cons($1, $3); };

atom:   NUMBER
    {
      if(is_integer($1)) {
        $$ = make_int($1);
      } else {
        $$ = make_double($1);
      }
    }
    |   STRING { $$ = make_string($1); };
    |   SYMBOL { $$ = make_symbol($1); };
%%

int yyerror(const char *s) { 
  printf("%s\n",s); 
}
