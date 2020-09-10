ROOT = ..
BIN = $(ROOT)/bin
SRC = $(ROOT)/src

.PHONY: tests all

all: tests

tests: $(BIN)/mongatest
	chmod +x runtests.sh
	./runtests.sh $(BIN)/mongatest

$(BIN)/mongatest: t.monga.c $(BIN)/lex.yy.c $(SRC)/monga.h
	cc -I$(SRC) t.monga.c $(BIN)/lex.yy.c -o $@


