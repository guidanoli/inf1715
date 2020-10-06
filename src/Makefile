ROOT = ..
BIN = $(ROOT)/bin

YACC = bison -y
LEX = flex -l

MONGA_O = $(BIN)/monga.o

LEX_C = $(BIN)/monga.l.c
LEX_O = $(LEX_C:.c=.o)
LEX_DB_O = $(BIN)/monga-ldb.o
LEX_DB = $(BIN)/monga-ldb

YACC_C = $(BIN)/monga.y.c
YACC_O = $(YACC_C:.c=.o)
YACC_H = $(YACC_C:.c=.h)
YACC_DB_O = $(BIN)/monga-ydb.o
YACC_DB = $(BIN)/monga-ydb

GEN_C = $(LEX_C) $(YACC_C)
GEN_H = $(YACC_H)
GEN_O = $(GEN_C:.c=.o) $(MONGA_O) $(YACC_DB_O) $(LEX_DB_O)
GEN_BIN = $(LEX_DB) $(YACC_DB)

.PHONY: all clean

all: $(LEX_DB) $(YACC_DB)

clean:
	rm -f $(GEN_C) $(GEN_H) $(GEN_O) $(GEN_BIN)

$(MONGA_O): monga-utils.c monga.h | $(BIN)
	$(CC) -c $< -o $@ -I. -I$(BIN)

$(YACC_H) $(YACC_C): monga.y | $(BIN)
	$(YACC) --output=$(YACC_C) -d $<

$(LEX_C): monga.l $(YACC_H) | $(BIN)
	$(LEX) --outfile=$@ $<

$(LEX_O): $(LEX_C) monga.h $(YACC_H) | $(BIN)
	$(CC) -c $< -o $@ -I. -I$(BIN)

$(YACC_O): $(YACC_C) | $(BIN)
	$(CC) -c $< -o $@

$(YACC_DB_O): monga-ydb.c monga.h $(YACC_H) | $(BIN)
	$(CC) -c $< -o $@ -I. -I$(BIN)

$(YACC_DB): $(LEX_O) $(YACC_O) $(YACC_DB_O) $(MONGA_O) | $(BIN)
	$(CC) $^ -o $@

$(LEX_DB_O): monga-ldb.c monga.h $(YACC_H) | $(BIN)
	$(CC) -c $< -o $@ -I. -I$(BIN)

$(LEX_DB): $(LEX_DB_O) $(LEX_O) $(MONGA_O) | $(BIN)
	$(CC) $^ -o $@

$(BIN):
	mkdir -p $@
