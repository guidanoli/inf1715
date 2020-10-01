ROOT = ..
BIN = $(ROOT)/bin
SRC = $(ROOT)/src

.PHONY: tests

tests: $(BIN)/mongatest
	./run

$(BIN)/mongatest: t.monga.c $(BIN)/monga.l.c $(BIN)/monga.y.h $(SRC)/monga.h
	cc -I$(SRC) -I$(BIN) t.monga.c $(BIN)/monga.l.c -o $@ $(CFLAGS)
