.PHONY: all src tests

all: src

src:
	$(MAKE) -C src

tests: src
	$(MAKE) -C tests
