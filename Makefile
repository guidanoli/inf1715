.PHONY: all src tests

all: src

src:
	$(MAKE) -C src

tests:
	$(MAKE) -C tests
