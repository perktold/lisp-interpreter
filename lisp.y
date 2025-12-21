%{
#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

int yylex(void);
int yyerror(const char *s);
%}

%union {
        int ival;
        double dval;
        const char *strval;
        value *val;
}
%token <dval> NUMBER
%token <strval> STRING SYMBOL
%token <ival> LPAREN RPAREN DOT
%token <ival> PLUS MINUS MULT DIV

%type <val> sexpr atom list list_items

%start sexprs

%%
sexprs: sexpr
      {
        printf(";> ");
        println_value(eval(global_env, $1));
      }
      | sexprs sexpr
      {
        printf(";> ");
        println_value(eval(global_env, $2));
      };

sexpr:  atom
     |  list
     |  LPAREN sexpr DOT sexpr RPAREN
     {
       $$ = cons($2, $4);
     };

list: LPAREN list_items RPAREN
    { $$ = $2; };

list_items: /* empty list */ { $$ = make_nil(); }
          | sexpr list_items { $$ = cons($1, $2); }
          | sexpr DOT sexpr { $$ = cons($1, $3); };

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
