.PHONY: all src examples tests test coverage clean

all: src examples

src:
	$(MAKE) -C src

examples: src
	$(MAKE) -C examples

tests: src
	$(MAKE) -C tests

test:
	$(MAKE) -C tests clean
	$(MAKE) -C src clean
	$(MAKE) -C src
	$(MAKE) -C tests
	$(MAKE) -C tests run

coverage:
	$(MAKE) -C tests clean
	$(MAKE) -C src clean
	$(MAKE) -C tests coverage COVERAGE=1

clean:
	$(MAKE) -C tests clean
	$(MAKE) -C examples clean
	$(MAKE) -C src clean
