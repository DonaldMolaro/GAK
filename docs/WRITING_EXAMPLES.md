# Writing New GAK Examples

This project is built around subclassing `Population` with a problem-specific
fitness function and, when needed, problem-specific genetic operators.

The best files to study before adding a new example are:

1. [examples/nqueens.hh](/Users/donaldmolaro/src/GAK/examples/nqueens.hh) and [examples/nqueens.cc](/Users/donaldmolaro/src/GAK/examples/nqueens.cc)
2. [examples/knapsack.hh](/Users/donaldmolaro/src/GAK/examples/knapsack.hh) and [examples/knapsack.cc](/Users/donaldmolaro/src/GAK/examples/knapsack.cc)
3. [examples/sudoku_constrained.hh](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.hh) and [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc)

## Decide Which Style Of Example You Need

There are two main patterns in this repo.

### Generic operator examples

Use the built-in chromosome mutation and crossover when the problem tolerates
arbitrary gene-level edits well.

Good fits:

- binary optimization problems
- symbolic search problems with soft constraints
- simple demos where the representation does not need to preserve invariants

Examples:

- `NQueens`
- `Knapsack`
- `LatinSquare`

For this style, you usually only need to override:

- `FitnessFunction(BaseString *b)`
- `FitnessPrint(BaseString *b)`

### Constraint-aware examples

Use custom operators when generic mutation and crossover spend most of their
time breaking structure that the problem really wants to preserve.

Good fits:

- Sudoku
- timetable construction
- routing with hard feasibility rules
- permutation problems where validity should hold by construction

Examples:

- `SudokuConstrained`

For this style, override:

- `createInitialChromosome()`
- `mateChromosomes(...)`
- `mutateChromosome(...)`
- plus the normal `FitnessFunction(...)` and `FitnessPrint(...)`

## Choose A Representation Carefully

Most of the quality of a GA example comes from the encoding, not just the
fitness function.

Questions to answer first:

- What does one gene represent?
- How many gene positions are there?
- How many states can each gene take?
- Which constraints can be guaranteed by construction instead of punished later?

Examples:

- `Knapsack`: one binary gene per item, `baseStates = 2`
- `LatinSquare`: one symbolic gene per cell, `baseStates = N`
- `SudokuConstrained`: one symbolic gene per cell, but initialization and
  mutation preserve row validity and locked givens

Remember that `geneticDiversity` is the chromosome length in genes, not the
number of symbols available. The symbol count is controlled separately by
`baseStates`.

## Configure The Population

New examples should prefer `Population::Options`.

```cpp
Population::Options options;
options.operation = Population::OperationMode::Maximize;
options.numberOfIndividuals = 150;
options.numberOfTrials = 12000;
options.geneticDiversity = 64;
options.baseStates = 2;
```

The most important fields to set intentionally are:

- `operation`
- `numberOfIndividuals`
- `numberOfTrials`
- `geneticDiversity`
- `bitMutationRate`
- `crossOverRate`
- `baseStates`

## Minimal Example Skeleton

```cpp
class MyExample : public Population
{
public:
   explicit MyExample(const Population::Options& options)
      : Population(options) {}

   double FitnessFunction(BaseString *b) override;
   void FitnessPrint(BaseString *b) override;
};
```

If the problem needs structure-preserving operators, add:

```cpp
protected:
   std::unique_ptr<Chromosome> createInitialChromosome() override;
   std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
      mateChromosomes(Chromosome *mother, Chromosome *father) override;
   void mutateChromosome(Chromosome *chromosome) override;
```

## Hook It Into The Example Runner

To make a new example part of the supported tree:

1. Add the new `.cc` and `.hh` files under `examples/`.
2. Update [examples/Makefile](/Users/donaldmolaro/src/GAK/examples/Makefile).
3. Add a mode to [examples/GA.cc](/Users/donaldmolaro/src/GAK/examples/GA.cc).
4. Add focused checks to [examples/test_main.cc](/Users/donaldmolaro/src/GAK/examples/test_main.cc).

## Testing Guidance

A good example test suite should verify at least:

- the example can be constructed with a sensible `Population::Options`
- `FitnessFunction(...)` behaves correctly on one or two known cases
- custom operators preserve their intended invariants
- the example participates in a short successful run

For constraint-aware examples, invariants matter more than exact final scores.
Test what must remain true after initialization, mutation, and crossover.

## Practical Advice

- Start with a representation that makes invalid states hard to create.
- Use fitness to rank good candidates, not to repair obviously bad ones forever.
- Keep example output readable in `FitnessPrint(...)`; this is part of the demo.
- If an example needs a lot of domain-specific operator logic, that is usually a
  sign you chose the right problem for the override hooks.
