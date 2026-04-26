.PHONY: all src examples tests example-tests test coverage coverage-lib clean

LLVM_PROFDATA ?= $(shell xcrun --find llvm-profdata)
LLVM_COV ?= $(shell xcrun --find llvm-cov)
COVERAGE_DIR = coverage
COMBINED_PROFDATA = $(COVERAGE_DIR)/combined.profdata
COMBINED_REPORT = $(COVERAGE_DIR)/coverage.txt
COVERAGE_IGNORE_REGEX = (.*/test_main\\.cc|.*\\.hh$$)

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
	$(MAKE) -C tests gak_tests
	$(MAKE) -C examples example_tests
	cd tests && ./gak_tests
	cd examples && ./example_tests


coverage-lib:
	$(MAKE) -C tests clean
	$(MAKE) -C src clean
	$(MAKE) -C src COVERAGE=1
	$(MAKE) -C tests coverage COVERAGE=1

coverage:
	mkdir -p $(COVERAGE_DIR)
	$(MAKE) -C tests clean
	$(MAKE) -C examples clean
	$(MAKE) -C src clean
	$(MAKE) -C src COVERAGE=1
	$(MAKE) -C tests coverage COVERAGE=1
	$(MAKE) -C examples coverage COVERAGE=1
	$(LLVM_PROFDATA) merge -sparse tests/coverage.profraw examples/coverage.profraw -o $(COMBINED_PROFDATA)
	$(LLVM_COV) report tests/gak_tests -object examples/example_tests -instr-profile=$(COMBINED_PROFDATA) --ignore-filename-regex="$(COVERAGE_IGNORE_REGEX)" 2>/dev/null
	$(LLVM_COV) show tests/gak_tests -object examples/example_tests -instr-profile=$(COMBINED_PROFDATA) --ignore-filename-regex="$(COVERAGE_IGNORE_REGEX)" > $(COMBINED_REPORT) 2>/dev/null

clean:
	$(MAKE) -C tests clean
	$(MAKE) -C examples clean
	$(MAKE) -C src clean
	rm -rf $(COVERAGE_DIR)
