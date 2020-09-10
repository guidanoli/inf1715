.PHONY: all src tests

all: src tests

src:
	$(MAKE) -C src

tests:
	$(MAKE) -C tests
