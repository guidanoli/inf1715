.PHONY: all src tests clean

all: src

src:
	$(MAKE) -C src

tests: src
	./tests/run monga_ldb
	./tests/run monga_ydb

clean:
	$(MAKE) -C src clean
