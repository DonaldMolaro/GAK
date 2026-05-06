# GAK

`GAK` is a small C++ genetic algorithm library with a set of example
optimization and constraint-satisfaction problems.

The project started as an older GA codebase and has been modernized to build cleanly on a current macOS C++ toolchain, with tests, example tests, safer ownership, and a more usable public configuration API.

## What is here

- `src/`
  The GA library itself.
- `examples/`
  Runnable example problems, an example test harness, and course material.
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

## Toolchain And Portability

The project is intentionally lightweight and currently builds with a plain
`make`-based workflow and a modern C++ compiler.

Known good environments:

- macOS with the Apple Clang toolchain from Command Line Tools
- Fedora Linux on `aarch64`
  Verified on `Linux fedora 6.19.8-200.fc43.aarch64`

Current compiler expectations:

- C++14 support
- standard library support for `std::unique_ptr`, `std::mt19937`, and related
  modern C++ facilities

If a new platform builds cleanly, it is worth adding it here so the repo keeps
an explicit portability record.

## Test

Run the full supported test suite:

```sh
make test
```

Run only the example-target tests:

```sh
make example-tests
```

Generate coverage:

```sh
make coverage
```

The combined production-code coverage report is written to
[coverage/coverage.txt](/Users/donaldmolaro/src/GAK/coverage/coverage.txt).

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
- `G`
  Graph coloring with symbolic node assignments
- `M`
  Timetable scheduling with symbolic slot assignments
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

- `G`
  Graph coloring over symbolic node assignments. This is a compact example of
  soft-to-hard constraint scoring on an adjacency structure.

- `M`
  Timetabling over symbolic slot assignments. This is a scheduling-style
  example with hard conflict avoidance and soft preference rewards.

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

## What Problems Fit Well

`GAK` works best for problems where:

- candidates can be encoded as gene strings
- partial progress can be scored meaningfully
- mutation/crossover are useful search tools

Strong fits include:

- binary decision problems like knapsack
- symbolic search problems like spelling or ordering
- constraint problems like N-Queens, Latin squares, and Sudoku
- black-box numeric optimization benchmarks

Some problems are especially strong fits only when they use custom operators.
Sudoku is the clearest example in this repo.

For a fuller discussion of where the library fits well and where it does not,
see [docs/PROBLEM_FIT.md](/Users/donaldmolaro/src/GAK/docs/PROBLEM_FIT.md).

For a higher-level technical overview of the system, see
[docs/WHITE_PAPER.md](/Users/donaldmolaro/src/GAK/docs/WHITE_PAPER.md).

## Public Library Shape

The core design is now composition-based:

- `Population` is the GA engine
- `PopulationProblem` describes a problem
- `Population::Settings` configures a run

Important pieces:

- [src/population.hh](/Users/donaldmolaro/src/GAK/src/population.hh)
  Main GA engine, `PopulationProblem`, and public configuration types
- [src/chromosome.hh](/Users/donaldmolaro/src/GAK/src/chromosome.hh)
  Generic chromosome representation and built-in crossover/mutation
- [src/base.hh](/Users/donaldmolaro/src/GAK/src/base.hh)
  Packed symbolic/base-string storage
- [src/genetic.hh](/Users/donaldmolaro/src/GAK/src/genetic.hh)
  Umbrella include for the public library surface

### Configuration API

New code should use `Population::Settings`.

Example:

```cpp
Population::Settings settings;
settings.operation = Population::OperationMode::Maximize;
settings.numberOfIndividuals = 150;
settings.numberOfTrials = 12000;
settings.chromosomeLength = 12;
settings.baseStates = 2;
```

### Minimal usage

```cpp
class MyProblem : public PopulationProblem
{
public:
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
};

Population::Settings settings;
MyProblem problem;
Population population(settings, problem);
population.run();
```

## Constraint-Aware Operators

The library supports problem-specific operator hooks through
`PopulationProblem`:

- `initializeCandidate(...)`
- `mateCandidates(...)`
- `mutateCandidate(...)`

The default engine still uses the generic chromosome behavior, but structured
problems can provide problem-specific operators without rewriting the main GA
loop.

This is especially important for problems like Sudoku, where the generic operators waste most of their effort breaking invariants that could instead be preserved by construction.

The best example of this in the repo is [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc).

For a direct comparison between the generic and constraint-aware Sudoku designs,
see [docs/SUDOKU_DESIGN.md](/Users/donaldmolaro/src/GAK/docs/SUDOKU_DESIGN.md).

For the generic crossover strategies built into `Chromosome`, see
[docs/CROSSOVER_MODES.md](/Users/donaldmolaro/src/GAK/docs/CROSSOVER_MODES.md).

## Exact-Solution Early Stop

Some problems know when they have found an exact solution. Those problems can
implement `PopulationProblem::hasReachedSolution(...)`.

Current examples that do this include:

- `Spell`
- `Alpha`
- `NQueens`
- `GraphColoring`
- `Timetable`
- `Sudoku`
- `SudokuConstrained`

When a solution is found, the engine stops early and reports it in
`Population::RunResult`.

## Recommended examples to study

If you want to understand the project by reading a few files instead of everything:

1. [src/population.hh](/Users/donaldmolaro/src/GAK/src/population.hh)
2. [src/population.cc](/Users/donaldmolaro/src/GAK/src/population.cc)
3. [examples/GA.cc](/Users/donaldmolaro/src/GAK/examples/GA.cc)
4. [examples/nqueens.cc](/Users/donaldmolaro/src/GAK/examples/nqueens.cc)
5. [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc)

If you want to add a new example, start with
[docs/WRITING_EXAMPLES.md](/Users/donaldmolaro/src/GAK/docs/WRITING_EXAMPLES.md).

If you want practical guidance on running and tuning the demos, see
[docs/RUNNING_DEMOS.md](/Users/donaldmolaro/src/GAK/docs/RUNNING_DEMOS.md).

If you want a guided learning path through the examples, start with
[examples/README.md](/Users/donaldmolaro/src/GAK/examples/README.md).

## Current status

Supported:

- top-level build via `make`
- library tests via `make test`
- example tests via `make example-tests`
- modernized example suite in `examples/`

Removed:

- old CVS repository metadata
- the unsupported `remodel/` subtree

## Documentation Map

- [docs/WRITING_EXAMPLES.md](/Users/donaldmolaro/src/GAK/docs/WRITING_EXAMPLES.md)
  How to build a new problem and wire it into the repo
- [docs/PROBLEM_FIT.md](/Users/donaldmolaro/src/GAK/docs/PROBLEM_FIT.md)
  What kinds of problems fit GAK well
- [docs/RUNNING_DEMOS.md](/Users/donaldmolaro/src/GAK/docs/RUNNING_DEMOS.md)
  How to run and tune the example modes
- [docs/SUDOKU_DESIGN.md](/Users/donaldmolaro/src/GAK/docs/SUDOKU_DESIGN.md)
  Why generic and constraint-aware Sudoku behave differently
- [docs/CROSSOVER_MODES.md](/Users/donaldmolaro/src/GAK/docs/CROSSOVER_MODES.md)
  Built-in generic crossover behavior
- [docs/TESTING_AND_COVERAGE.md](/Users/donaldmolaro/src/GAK/docs/TESTING_AND_COVERAGE.md)
  Test and coverage workflow
