.PHONY: all src tests clean

all: src

src:
	$(MAKE) -C src

tests: src
	$(MAKE) -C tests

clean:
	$(MAKE) -C src clean
