.PHONY: all src tests clean

all: src

src:
	$(MAKE) -C src

tests: src
	./tests/run

clean:
	$(MAKE) -C src clean
