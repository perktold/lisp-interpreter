LEX     = flex
YACC    = bison
CC      = gcc -g
CFLAGS  = -DYYDEBUG=1 -fPIC -rdynamic
LIBS    = -lreadline -lfl -lm
OBJS	= lisp.o lisp.tab.o lex.yy.o

MODULE_LIBS = -lpq
MODULE_SRC  = $(wildcard modules/*.c)
MODULE_SO   = $(MODULE_SRC:.c=.so)
STD_MODULES = modules/std_lib.so
.SUFFIXES:

main: main.c $(OBJS) $(STD_MODULES)
	$(CC) $(CFLAGS) -o $@ main.c $(OBJS) $(LIBS)

lisp.o: lisp.c lisp_api.h lisp.h lex.yy.o
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
	rm -f *.o main test lisp.tab.c lisp.tab.h lex.yy.c lex.yy.h modules/*.so

test: test.o $(OBJS) $(STD_MODULES)
	$(CC) $(CFLAGS) -o $@ test.o $(OBJS) $(LIBS)
	./test
	
test.o: test.c lisp.h lex.yy.o
	$(CC) $(CFLAGS) -c test.c

run: main
	./main

run_std: main
	./main lisp/standard_functions.lisp

modules: $(MODULE_SO)
	@echo "All modules built successfully."

modules/%.so: modules/%.c lisp.o
	gcc -fPIC -c $< -o $*.o
	gcc -fPIC -shared $*.o lisp.o -o $@ $(MODULE_LIBS)
	rm $*.o

.PHONY: clean test run
