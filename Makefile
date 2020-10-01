.PHONY: all src tests clean

all: src

src:
	$(MAKE) -C src

tests: src
	./tests/run monga-ldb

clean:
	$(MAKE) -C src clean
