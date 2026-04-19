.PHONY: all src examples tests example-tests test coverage clean

all: src examples

src:
	$(MAKE) -C src

examples: src
	$(MAKE) -C examples

tests: src
	$(MAKE) -C tests

example-tests: src
	$(MAKE) -C examples test

test:
	$(MAKE) -C tests clean
	$(MAKE) -C examples clean
	$(MAKE) -C src clean
	$(MAKE) -C src
	$(MAKE) -C tests
	$(MAKE) -C tests run
	$(MAKE) -C examples test


coverage:
	$(MAKE) -C tests clean
	$(MAKE) -C src clean
	$(MAKE) -C src COVERAGE=1
	$(MAKE) -C tests coverage COVERAGE=1

clean:
	$(MAKE) -C tests clean
	$(MAKE) -C examples clean
	$(MAKE) -C src clean
