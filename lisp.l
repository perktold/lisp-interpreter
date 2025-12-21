%{
#include <stdlib.h>
#include "lisp.h"
#include "lisp.tab.h"

int tcnt = 0;
%}

%%
[[:space:]]+    { /* ignore whitespace */ }
";"[^\n]*       { /* ignore comment */   }
"("             { return LPAREN; }
")"             { return RPAREN; }
"\."            { return DOT; }
"'"             { return QUOTE; }

[+-]?([0-9]+|([0-9]*\.[0-9]+))([eE][-+]?[0-9]+)? {
                yylval.dval = atof(yytext);
                
                return NUMBER;
                }

\"(\\.|[^"\\])*\" {
                // remove starting and ending quotes
                int len = yyleng - 2;
                char *str = (char*)malloc(len + 1);
                if (!str) { fprintf(stderr, "out of memory!\n"); exit(1); }
                strncpy(str, yytext + 1, len);
                str[len] = '\0';

                yylval.strval = str;
                
                return STRING;
                }

[^[:space:]\"()'.]+ {
                yylval.strval = strdup(yytext);
                
                return SYMBOL;
                }
.               ; // do nothing 
%%
