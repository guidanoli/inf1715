ROOT = ..
BIN = $(ROOT)/bin
SRC = $(ROOT)/src
SCRIPTS = edit func list add compare count eval generate run var

.PHONY: tests all generate chmodx

all: tests

chmodx: $(SCRIPTS)
	chmod +x $^

tests: $(BIN)/mongatest chmodx
	./run

generate: $(BIN)/mongatest chmodx
	./generate

$(BIN)/mongatest: t.monga.c $(BIN)/lex.yy.c $(SRC)/monga.h
	cc -I$(SRC) t.monga.c $(BIN)/lex.yy.c -o $@


