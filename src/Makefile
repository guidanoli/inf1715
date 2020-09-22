ROOT = ..
BIN = $(ROOT)/bin
YACC = bison -y
LEX = flex -l

.PHONY: all

all: $(BIN)/lex.yy.c $(BIN)/y.tab.c

$(BIN)/y.tab.c: monga.y | $(BIN)
	$(YACC) $< && mv $(notdir $@) $(BIN)

$(BIN)/lex.yy.c: monga.l monga.h | $(BIN)
	$(LEX) $< && mv $(notdir $@) $(BIN)

$(BIN):
	mkdir -p $@
