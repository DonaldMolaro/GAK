# GAK

`GAK` is a small C++ genetic algorithm library with a set of example optimization and constraint-satisfaction problems.

The project started as an older GA codebase and has been modernized to build cleanly on a current macOS C++ toolchain, with tests, example tests, safer ownership, and a more usable public configuration API.

## What is here

- `src/`
  The GA library itself.
- `examples/`
  Runnable example problems and an example test harness.
- `tests/`
  Library-focused unit tests and coverage support.
- `Makefile`
  Top-level build/test entry points.

## Build

Build the library and the example runner:

```sh
make
```

Build just the examples:

```sh
make examples
```

Clean build artifacts:

```sh
make clean
```

## Test

Run the full supported test suite:

```sh
make test
```

Run only the example-target tests:

```sh
make example-tests
```

Generate library coverage:

```sh
make coverage
```

The coverage report is written to [tests/coverage.txt](/Users/donaldmolaro/src/GAK/tests/coverage.txt).

For a more detailed developer workflow, see
[docs/TESTING_AND_COVERAGE.md](/Users/donaldmolaro/src/GAK/docs/TESTING_AND_COVERAGE.md).

## Example runner

Build the example runner:

```sh
make examples
```

Run it from `examples/`:

```sh
cd examples
./GA <mode>
```

Available modes:

- `D`
  Dome function minimization with binary fixed-length chromosomes
- `F6`
  F6 optimization benchmark with binary fixed-length chromosomes
- `A`
  Alphabet ordering with symbolic fixed-length chromosomes
- `S`
  Word spelling with symbolic variable-length chromosomes
- `T`
  Traveling salesman with symbolic fixed-length chromosomes
- `Q`
  8 queens constraint satisfaction
- `K`
  Knapsack combinatorial optimization
- `L`
  Latin square symbolic constraint satisfaction
- `U`
  Sudoku using generic symbolic operators plus givens/row/column/box scoring
- `C`
  Constraint-aware Sudoku with row-valid initialization, locked givens, row-wise crossover, and row-swap mutation

### Example mode reference

The example runner is meant to be both a demo and a set of working reference
problems. These are the most useful things to know before you run a mode.

- `D` and `F6`
  Continuous-function minimization examples encoded as binary chromosomes.
  These are good first runs if you want to see the classic GA flow on a small,
  generic search space. Expect fitness values to improve toward smaller numbers.

- `A`
  Alphabet ordering over symbolic fixed-length chromosomes. This is a classic
  symbolic example and a good reference for problems where each gene can take
  more than two values.

- `S`
  Variable-length symbolic spelling example. This is the best example to read
  if you want to understand how the legacy engine handles variable-length
  chromosomes.

- `T`
  Traveling salesman over symbolic chromosomes. This is a sequencing-style
  example and a useful reminder that generic operators can work for routing-like
  problems, but may not preserve as much structure as a specialized encoding.

- `Q`
  Eight queens constraint satisfaction. This is a compact example of mapping a
  board problem onto a symbolic chromosome with a purely fitness-driven
  constraint score.

- `K`
  Knapsack optimization with a binary decision vector. This is the simplest
  combinatorial optimization example in the tree.

- `L`
  Latin square construction over symbolic cell values. A successful run should
  converge toward a grid with unique values across each row and column.

- `U`
  Generic Sudoku. This uses the normal symbolic operators plus a scoring
  function for givens, rows, columns, and boxes. It demonstrates how far the
  library can go with generic operators alone, but it still spends a lot of
  effort repairing broken structure.

- `C`
  Constraint-aware Sudoku. This is the strongest example of the modernized
  extension hooks. Rows start valid, givens stay locked, mutation only swaps
  mutable values within a row, and crossover operates on whole rows.

Most modes print generation progress and then a best candidate summary at the
end. Constraint problems usually become easiest to judge by reading the final
printed board rather than by the raw score alone.

## Public library shape

The core type is `Population`, which you subclass to define a problem-specific fitness function and output formatter.

Important pieces:

- [src/population.hh](/Users/donaldmolaro/src/GAK/src/population.hh)
  Main GA engine and public configuration types
- [src/chromosome.hh](/Users/donaldmolaro/src/GAK/src/chromosome.hh)
  Generic chromosome representation and built-in crossover/mutation
- [src/base.hh](/Users/donaldmolaro/src/GAK/src/base.hh)
  Packed symbolic/base-string storage
- [src/genetic.hh](/Users/donaldmolaro/src/GAK/src/genetic.hh)
  Umbrella include for the public library surface

### Modern configuration API

New code should prefer `Population::Options` and the `enum class` configuration values rather than the old positional constructors.

Example:

```cpp
Population::Options options;
options.operation = Population::OperationMode::Maximize;
options.numberOfIndividuals = 150;
options.numberOfTrials = 12000;
options.geneticDiversity = 12;
options.baseStates = 2;
```

## Constraint-aware operators

The library now supports subclass-specific operator overrides in `Population`:

- `createInitialChromosome()`
- `mateChromosomes(...)`
- `mutateChromosome(...)`

The default engine still uses the classic generic chromosome behavior, but constrained examples can now provide problem-specific operators without rewriting the whole GA loop.

This is especially important for problems like Sudoku, where the generic operators waste most of their effort breaking invariants that could instead be preserved by construction.

The best example of this in the repo is [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc).

For a direct comparison between the generic and constraint-aware Sudoku designs,
see [docs/SUDOKU_DESIGN.md](/Users/donaldmolaro/src/GAK/docs/SUDOKU_DESIGN.md).

For the generic crossover strategies built into `Chromosome`, see
[docs/CROSSOVER_MODES.md](/Users/donaldmolaro/src/GAK/docs/CROSSOVER_MODES.md).

## Recommended examples to study

If you want to understand the project by reading a few files instead of everything:

1. [src/population.hh](/Users/donaldmolaro/src/GAK/src/population.hh)
2. [src/population.cc](/Users/donaldmolaro/src/GAK/src/population.cc)
3. [examples/GA.cc](/Users/donaldmolaro/src/GAK/examples/GA.cc)
4. [examples/nqueens.cc](/Users/donaldmolaro/src/GAK/examples/nqueens.cc)
5. [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc)

If you want to add a new example, start with
[docs/WRITING_EXAMPLES.md](/Users/donaldmolaro/src/GAK/docs/WRITING_EXAMPLES.md).

## Current status

Supported:

- top-level build via `make`
- library tests via `make test`
- example tests via `make example-tests`
- modernized example suite in `examples/`

Removed:

- old CVS repository metadata
- the unsupported `remodel/` subtree

## Next documentation targets

Good next steps after this README:

- add richer API comments for the constraint-aware override hooks
- add a short architecture note for `Population::execute()` and `RunResult`
- add a small contribution guide for adding tests and examples together
