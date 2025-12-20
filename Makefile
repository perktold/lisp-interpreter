LEX     = flex
YACC    = bison
CC      = gcc -g
CFLAGS  = -DYYDEBUG=1
LIBS    = -lreadline -lfl -lm
.SUFFIXES:

lisp: lisp.c lisp.tab.o lex.yy.o
	$(CC) $(CFLAGS) -o $@ lisp.c lisp.tab.o lex.yy.o $(LIBS)

lisp.tab.c lisp.tab.h: lisp.y
	$(YACC) -d lisp.y

lex.yy.c: lisp.l lisp.tab.h
	$(LEX) --header-file=lex.yy.h lisp.l

lisp.tab.o: lisp.tab.c
	$(CC) $(CFLAGS) -c lisp.tab.c

lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) -c lex.yy.c

clean:
	rm -f *.o lisp lisp.tab.c lisp.tab.h lex.yy.c lex.yy.h

.PHONY: clean
