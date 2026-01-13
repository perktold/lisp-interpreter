LEX     = flex
YACC    = bison
CC      = gcc -g
CFLAGS  = -DYYDEBUG=1
LIBS    = -lreadline -lfl -lm
OBJS	= lisp.o lisp.tab.o lex.yy.o
.SUFFIXES:

main: main.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ main.c $(OBJS) $(LIBS)

lisp.o: lisp.c lisp.h lex.yy.o
	$(CC) $(CFLAGS) -c lisp.c

lisp.tab.c lisp.tab.h: lisp.y
	$(YACC) -d lisp.y

lex.yy.c lex.yy.h: lisp.l lisp.tab.h
	$(LEX) --header-file=lex.yy.h lisp.l

lisp.tab.o: lisp.tab.c
	$(CC) $(CFLAGS) -c lisp.tab.c

lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) -c lex.yy.c

clean:
	rm -f *.o main test lisp.tab.c lisp.tab.h lex.yy.c lex.yy.h

test: test.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ test.o $(OBJS) $(LIBS)
	./test
	
test.o: test.c lisp.h lex.yy.o
	$(CC) $(CFLAGS) -c test.c

run: main
	./main

run_std: main
	./main lisp/standard_functions.lisp

.PHONY: clean test run
