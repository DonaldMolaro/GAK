.PHONY: all src examples tests example-tests test coverage coverage-gcc coverage-lib clean

LLVM_PROFDATA ?= $(shell if command -v xcrun >/dev/null 2>&1; then xcrun --find llvm-profdata; else command -v llvm-profdata; fi)
LLVM_COV ?= $(shell if command -v xcrun >/dev/null 2>&1; then xcrun --find llvm-cov; else command -v llvm-cov; fi)
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
	# Rebuild the demo runner the clean step removed (so ./GA works
	# afterwards) and test it end to end.
	$(MAKE) -C examples
	$(MAKE) -C examples cli-check


# Line coverage with GCC's gcov: no LLVM tools needed. Runs the full
# `make test` flow (library tests, example tests and the CLI checks) on an
# unoptimized instrumented build, then prints per-file line coverage.
GCOV_FLAGS = -O0 -g -std=c++14 --coverage

coverage-gcc:
	$(MAKE) clean
	$(MAKE) test CXXFLAGS="$(GCOV_FLAGS)" LDFLAGS="--coverage"
	@echo "Line coverage (production .cc files):"
	@for dir in src examples; do (cd $$dir && gcov -n *.cc 2>/dev/null); done | \
	  awk "/^File '/ { file = \$$2; gsub(/'/, \"\", file) } \
	       /^Lines executed:/ && file !~ /\// && file ~ /\.cc\$$/ && file !~ /test_main/ { \
	         split(\$$2, parts, \":\"); printf \"  %-24s %s %s %s\\n\", file, parts[2], \$$3, \$$4 } \
	       /^Lines executed:/ { file = \"\" }"

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
