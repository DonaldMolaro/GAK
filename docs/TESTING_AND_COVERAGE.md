# Testing And Coverage

This project has two supported test layers:

- library-focused tests in [tests/test_main.cc](../tests/test_main.cc)
- example-focused tests in [examples/test_main.cc](../examples/test_main.cc)

The top-level [Makefile](../Makefile) provides the
main entry points.

## Known Build Environments

The current codebase has been validated in at least these environments:

- macOS with Apple Clang / Command Line Tools
- Fedora Linux on `aarch64`
  Reported working on `Linux fedora 6.19.8-200.fc43.aarch64`

If you validate another compiler or platform, it is useful to capture it in
the top-level README and here.

## Common Commands

Run the full supported test flow:

```sh
make test
```

This does a clean rebuild of:

- `src/`
- `tests/`
- `examples/`

Then it runs:

- the library unit-test binary
- the example test harness
- the command-line checks in [examples/check_cli.sh](../examples/check_cli.sh)

Run only the library tests:

```sh
make tests
make -C tests run
```

Run only the example tests:

```sh
make example-tests
```

Run only the command-line checks (builds `GA` first):

```sh
make -C examples cli-check
```

Rebuild just the library:

```sh
make src
```

## What The Test Layers Cover

### Library tests

The library tests cover core GA mechanics such as:

- `BaseString` storage and symbolic encoding behavior
- `Chromosome` mutation and crossover behavior
- `Population` selection, replacement, execution, and error paths
- coverage-oriented regression cases for old bug fixes

These tests are the right place to verify:

- a core engine bug fix
- a library API contract
- a deterministic operator behavior

### Example tests

The example tests cover the supported example problems as runnable reference
implementations.

These tests verify things like:

- known fitness behavior on example problems
- example-specific invariants
- short successful runs
- constraint-aware operator preservation in examples like
  [examples/sudoku_constrained.cc](../examples/sudoku_constrained.cc)

These tests are the right place to verify:

- a new demo problem
- a problem-specific encoding
- a custom operator override

### Command-line checks

[examples/check_cli.sh](../examples/check_cli.sh) runs the real `GA` binary
and checks exit codes and messages: usage and `--help`, strict rejection of
malformed option values, configuration errors reported as `GA: <reason>`,
every mode running, each override actually being applied, mode `R`'s data
loading and reports, and fixed-seed reproducibility. It is the right place to
verify a new runner option or a change to what the runner prints. A check
reads:

```sh
expect STATUS "text the output must contain" "description" -- $GA args...
```

## Coverage Workflow

### With GCC only (gcov)

```sh
make coverage-gcc
```

This needs nothing beyond GCC. It rebuilds everything unoptimized with
`--coverage`, runs the full `make test` flow, including the command-line
checks, so the `GA` runner is measured too, and prints per-file line coverage
for the production `.cc` files. For line-by-line detail, run `gcov` in `src/`
or `examples/` afterwards; each `<file>.gcov` marks never-executed lines with
`#####`. `make clean` removes the coverage data files.

Most of the remaining uncovered lines are defensive `throw`s for states the
public API cannot produce, such as out-of-range gene symbols, which `setValue`
already rejects.

### With LLVM (llvm-cov)

Generate coverage with:

```sh
make coverage
```

This rebuilds the library, the library test harness, and the example test
harness with LLVM coverage instrumentation enabled. It then merges both test
runs into one combined report focused on production `.cc` files.

Generated coverage artifacts:

- [coverage/combined.profdata](../coverage/combined.profdata)
- [coverage/coverage.txt](../coverage/coverage.txt)
- [tests/coverage.profraw](../tests/coverage.profraw)
- [tests/coverage.profdata](../tests/coverage.profdata)
- [tests/coverage.txt](../tests/coverage.txt)
- [examples/coverage.profraw](../examples/coverage.profraw)
- [examples/coverage.profdata](../examples/coverage.profdata)
- [examples/coverage.txt](../examples/coverage.txt)

The combined annotated report in [coverage/coverage.txt](../coverage/coverage.txt)
is the easiest place to inspect uncovered branches after a change. The
top-level combined report intentionally excludes test harness files and header
files so the percentage reflects exercised production code.

If you want the original library-only flow, use:

```sh
make coverage-lib
```

That keeps the narrower `tests/`-only report when you only want to measure the
core engine.

## Good Testing Habits In This Repo

- Run `make test` after changes that touch public behavior.
- Run `make coverage` after changes in `src/` when you want to check for missed
  engine branches.
- Prefer deterministic assertions on invariants over trying to assert exact GA
  end states.
- For constrained problems, test what operators preserve after initialization,
  crossover, and mutation.
- When adding a new example, update [examples/test_main.cc](../examples/test_main.cc)
  alongside the example itself.
