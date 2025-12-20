%{
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lisp.h"

int yylex(void);
int yyerror(const char *s);

static int is_integer(double x) {
    return floor(x) == x && isfinite(x);
}
%}

%union {
        int ival;
        double dval;
        const char *strval;
        value *val;
}
%token <dval> NUMBER
%token <strval> STRING
%token <ival> LPAREN RPAREN DOT WHITESPACE
%token <ival> PLUS MINUS MULT DIV

%type <val> sexpr atom sexprs

%start sexprs

%%
opt_ws: /* matches no whitespace */
      | WHITESPACE;

sexprs: sexpr
      | sexprs opt_ws sexpr;

sexpr:  atom
     |  LPAREN opt_ws sexpr opt_ws DOT opt_ws sexpr opt_ws RPAREN
     {
       $$ = cons($3, $7);
       print_val($$);
       printf("\n");
     };

atom:   NUMBER
    {
      if(is_integer($1)) {
        $$ = make_int($1);
      } else {
        $$ = make_double($1);
      }
    }
    |   LPAREN opt_ws RPAREN { $$ = make_nil(); }
    |   STRING { $$ = make_string($1); };
%%

int yyerror(const char *s) { 
  printf("%s\n",s); 
}
