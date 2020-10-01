ROOT = ..
BIN = $(ROOT)/bin

YACC = bison -y
LEX = flex -l

LEX_C = $(BIN)/monga.l.c
LEX_DB = $(BIN)/monga-ldb

YACC_C = $(BIN)/monga.y.c
YACC_H = $(YACC_C:.c=.h)
YACC_DB = $(BIN)/monga-ydb

.PHONY: all clean

all: $(LEX_DB) $(YACC_DB)

clean:
	rm $(YACC_C) $(LEX_C)

$(YACC_H) $(YACC_C): monga.y | $(BIN)
	$(YACC) --output=$(YACC_C) -d $<

$(LEX_C): monga.l $(YACC_H) | $(BIN)
	$(LEX) --outfile=$@ $<

$(YACC_DB): $(LEX_C) $(YACC_C) $(YACC_H) | $(BIN)
	$(CC) $^ -I. -o $@

$(LEX_DB): t.monga.c $(LEX_C) $(YACC_H) | $(BIN)
	$(CC) $^ -I. -I$(BIN) -o $@

$(BIN):
	mkdir -p $@
