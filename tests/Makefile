ROOT = ..
BIN = $(ROOT)/bin
SRC = $(ROOT)/src

.PHONY: tests all gen

all: tests

tests: $(BIN)/mongatest
	chmod +x run
	./run

gen: $(BIN)/mongatest
	chmod +x gen
	./gen

$(BIN)/mongatest: t.monga.c $(BIN)/lex.yy.c $(SRC)/monga.h
	cc -I$(SRC) t.monga.c $(BIN)/lex.yy.c -o $@


