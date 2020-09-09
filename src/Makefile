ROOT = ..
BIN = $(ROOT)/bin

all: $(BIN)/lex.yy.c

$(BIN):
	mkdir -p $@

$(BIN)/lex.yy.c: monga.l monga.h $(BIN)
	lex $<
	mv lex.yy.c $(BIN)
