ROOT = ..
BIN = $(ROOT)/bin

YACC = bison -y
LEX = flex -l

YACC_C = $(BIN)/monga.y.c
LEX_C = $(BIN)/monga.l.c

.PHONY: all clean

all: $(BIN)/monga

clean:
	rm $(YACC_C) $(LEX_C)

$(YACC_C): monga.y | $(BIN)
	$(YACC) --output=$@ -d $<

$(LEX_C): monga.l | $(BIN)
	$(LEX) --outfile=$@ $<

$(BIN)/monga: $(LEX_C) $(YACC_C) | $(BIN)
	$(CC) $^ -I. -o $@

$(BIN):
	mkdir -p $@
