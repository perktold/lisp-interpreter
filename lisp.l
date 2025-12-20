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
[ \t\n]+        { printf("%d: WHITESPACE\n", tcnt); return WHITESPACE; }
"("             { printf("%d: LPAREN\n", ++tcnt); return LPAREN; }
")"             { printf("%d: RPAREN\n", ++tcnt); return RPAREN; }
"\."            { printf("%d: DOT\n", ++tcnt); return DOT; }
"+"             { printf("%d: PLUS\n", ++tcnt); return PLUS; }
"-"             { printf("%d: MINUS\n", ++tcnt); return MINUS; }
"*"             { printf("%d: MULT\n", ++tcnt); return MULT; }
"/"             { printf("%d: DIV\n", ++tcnt); return DIV; }
\".*\"          { printf("%d: STRING\n", ++tcnt); return STRING; }
.               ; // do nothing 
%%
