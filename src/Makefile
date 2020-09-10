ROOT = ..
BIN = $(ROOT)/bin

.PHONY: all

all: $(BIN)/lex.yy.c

$(BIN)/lex.yy.c: monga.l monga.h | $(BIN)
	lex monga.l && mv lex.yy.c $(BIN)

$(BIN):
	mkdir $@
