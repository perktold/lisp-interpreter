%{
#include <stdlib.h>
#include "lisp.h"
#include "lisp.tab.h"

int tcnt = 0;
%}

%%
[+-]?([0-9]+|([0-9]*\.[0-9]+))([eE][-+]?[0-9]+)? {
                yylval.dval = atof(yytext);
                printf("%d: NUMBER: %f\n", ++tcnt, yylval.dval);
                return NUMBER;
                }

\"(\\.|[^"\\])*\" {
                // remove starting and ending quotes
                int len = yyleng - 2;
                char *str = (char*)malloc(len + 1);
                if (!str) { fprintf(stderr, "Out of memory\n"); exit(1); }
                strncpy(str, yytext + 1, len);
                str[len] = '\0';

                yylval.strval = str;
                printf("%d: STRING: \"%s\"\n", ++tcnt, yylval.strval);
                return STRING;
                }

[ \t\n]+        { printf("%d: WHITESPACE\n", tcnt); return WHITESPACE; }
"("             { printf("%d: LPAREN\n", ++tcnt); return LPAREN; }
")"             { printf("%d: RPAREN\n", ++tcnt); return RPAREN; }
"\."            { printf("%d: DOT\n", ++tcnt); return DOT; }
"+"             { printf("%d: PLUS\n", ++tcnt); return PLUS; }
"-"             { printf("%d: MINUS\n", ++tcnt); return MINUS; }
"*"             { printf("%d: MULT\n", ++tcnt); return MULT; }
"/"             { printf("%d: DIV\n", ++tcnt); return DIV; }
.               ; // do nothing 
%%
