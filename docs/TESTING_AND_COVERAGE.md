# Testing And Coverage

This project has two supported test layers:

- library-focused tests in [tests/test_main.cc](/Users/donaldmolaro/src/GAK/tests/test_main.cc)
- example-focused tests in [examples/test_main.cc](/Users/donaldmolaro/src/GAK/examples/test_main.cc)

The top-level [Makefile](/Users/donaldmolaro/src/GAK/Makefile) provides the
main entry points.

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

Run only the library tests:

```sh
make tests
make -C tests run
```

Run only the example tests:

```sh
make example-tests
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
  [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc)

These tests are the right place to verify:

- a new demo problem
- a problem-specific encoding
- a custom operator override

## Coverage Workflow

Generate coverage with:

```sh
make coverage
```

This rebuilds the library and the library test harness with LLVM coverage
instrumentation enabled, runs the library tests, and writes reports under
`tests/`.

Generated coverage artifacts:

- [tests/coverage.profraw](/Users/donaldmolaro/src/GAK/tests/coverage.profraw)
- [tests/coverage.profdata](/Users/donaldmolaro/src/GAK/tests/coverage.profdata)
- [tests/coverage.txt](/Users/donaldmolaro/src/GAK/tests/coverage.txt)

The textual report in `coverage.txt` is the easiest place to inspect uncovered
branches after a change.

## Important Limitation

The current `make coverage` target measures the library test binary in `tests/`.
It does not currently combine in the example test harness from `examples/`.

That means:

- core library coverage is measured directly
- example coverage should still be maintained with `make example-tests`
- example-only code paths may be well tested without showing up in the coverage
  percentage

If you extend the examples significantly, it is still worth adding focused
example tests even when those branches are not part of the coverage summary.

## Good Testing Habits In This Repo

- Run `make test` after changes that touch public behavior.
- Run `make coverage` after changes in `src/` when you want to check for missed
  engine branches.
- Prefer deterministic assertions on invariants over trying to assert exact GA
  end states.
- For constrained problems, test what operators preserve after initialization,
  crossover, and mutation.
- When adding a new example, update [examples/test_main.cc](/Users/donaldmolaro/src/GAK/examples/test_main.cc)
  alongside the example itself.
