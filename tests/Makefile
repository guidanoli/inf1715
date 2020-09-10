ROOT = ..
BIN = $(ROOT)/bin
SRC = $(ROOT)/src

.PHONY: tests all generate_outputs

all: tests

tests: $(BIN)/mongatest
	chmod +x run_tests.sh
	./run_tests.sh $^

generate_output: $(BIN)/mongatest
	chmod +x generate_output.sh
	./generate_output.sh $^

$(BIN)/mongatest: t.monga.c $(BIN)/lex.yy.c $(SRC)/monga.h
	cc -I$(SRC) t.monga.c $(BIN)/lex.yy.c -o $@


