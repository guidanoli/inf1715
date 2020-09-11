ROOT = ..
BIN = $(ROOT)/bin
SRC = $(ROOT)/src

.PHONY: tests all generate

all: tests

tests: $(BIN)/mongatest
	chmod +x run
		./run

generate: $(BIN)/mongatest
	chmod +x generate
	./generate

$(BIN)/mongatest: t.monga.c $(BIN)/lex.yy.c $(SRC)/monga.h
	cc -I$(SRC) t.monga.c $(BIN)/lex.yy.c -o $@


