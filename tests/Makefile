ROOT = ..
BIN = $(ROOT)/bin
SRC = $(ROOT)/src
SCRIPTS = edit func list add compare count eval generate run var

.PHONY: all chmodx tests

all: tests

chmodx: $(SCRIPTS)
	chmod +x $^

tests: $(BIN)/mongatest chmodx
	./run

$(BIN)/mongatest: t.monga.c $(BIN)/lex.yy.c $(SRC)/monga.h
	cc -I$(SRC) t.monga.c $(BIN)/lex.yy.c -o $@


