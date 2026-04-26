# Writing New GAK Examples

This project is now built around composition:

- `Population` is the GA engine
- `PopulationProblem` describes a problem
- the engine runs a problem with a chosen `Population::Settings`

That means new examples should normally implement a problem object, not
subclass `Population`.

Good files to study before adding a new example:

1. [examples/nqueens.hh](/Users/donaldmolaro/src/GAK/examples/nqueens.hh) and [examples/nqueens.cc](/Users/donaldmolaro/src/GAK/examples/nqueens.cc)
2. [examples/knapsack.hh](/Users/donaldmolaro/src/GAK/examples/knapsack.hh) and [examples/knapsack.cc](/Users/donaldmolaro/src/GAK/examples/knapsack.cc)
3. [examples/sudoku_constrained.hh](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.hh) and [examples/sudoku_constrained.cc](/Users/donaldmolaro/src/GAK/examples/sudoku_constrained.cc)
4. [examples/GA.cc](/Users/donaldmolaro/src/GAK/examples/GA.cc)

## Choose The Right Example Style

There are two main patterns in this repo.

### Generic-operator problems

Use the built-in initialization, crossover, and mutation when arbitrary
gene-level edits are acceptable.

Good fits:

- binary optimization
- symbolic search with soft constraints
- small toy and benchmark problems

Examples:

- `Dome`
- `F6`
- `NQueens`
- `Knapsack`
- `LatinSquare`

For this style, you usually implement:

- `evaluateFitness(...)`
- `printCandidate(...)`
- optionally `hasReachedSolution(...)`

### Constraint-aware problems

Use custom operators when the default GA spends most of its time breaking
structure that should really be preserved.

Good fits:

- Sudoku
- timetable construction
- route/order problems with validity rules
- permutation-like problems with locked cells or fixed positions

Examples:

- `SudokuConstrained`

For this style, you usually implement:

- `evaluateFitness(...)`
- `printCandidate(...)`
- `initializeCandidate(...)`
- `mateCandidates(...)`
- `mutateCandidate(...)`
- optionally `hasReachedSolution(...)`

## Choose An Encoding Before You Write Fitness

Most GA quality comes from the representation, not the scoring function.

Questions to answer first:

- What does one gene mean?
- How many genes are there?
- How many values can each gene take?
- Which constraints can be preserved by construction?
- Can the problem recognize an exact solution?

Examples:

- `Knapsack`: one binary gene per item, `baseStates = 2`
- `NQueens`: one symbolic gene per column, value = row
- `LatinSquare`: one symbolic gene per cell, `baseStates = board width`
- `SudokuConstrained`: one symbolic gene per cell, but operators preserve row
  validity and locked givens

Remember:

- `chromosomeLength` is chromosome length
- `baseStates` is the number of representable values per gene

## Configure The Engine

New examples should use `Population::Settings`.

```cpp
Population::Settings settings;
settings.operation = Population::OperationMode::Maximize;
settings.numberOfIndividuals = 150;
settings.numberOfTrials = 12000;
settings.chromosomeLength = 64;
settings.baseStates = 2;
settings.bitMutationRate = 0.01;
settings.crossOverRate = 0.70;
```

The most important fields to choose intentionally are:

- `operation`
- `numberOfIndividuals`
- `numberOfTrials`
- `chromosomeLength`
- `baseStates`
- `bitMutationRate`
- `crossOverRate`
- `variableLength`

## Minimal Example Skeleton

```cpp
class MyProblem : public PopulationProblem
{
public:
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;
};
```

Run it with:

```cpp
Population::Settings settings;
// ... fill settings ...

MyProblem problem;
Population population(settings, problem);
population.run();
```

## Exact-Solution Detection

Some problems know when they are solved exactly.

Examples in this repo:

- `Spell`
- `Alpha`
- `NQueens`
- `Sudoku`
- `SudokuConstrained`

Those problems implement:

```cpp
bool hasReachedSolution(const Population& population,
                        const BaseString& genes,
                        double fitness) const override;
```

When this returns `true`, the engine stops early and reports:

- `RunResult::solutionFound`
- `RunResult::stoppedEarly`

Use this hook when:

- there is a known exact optimum
- the fitness function can cheaply detect it
- continuing the search after success has no value

## Constraint-Aware Operator Skeleton

```cpp
class MyStructuredProblem : public PopulationProblem
{
public:
   double evaluateFitness(const BaseString& genes) override;
   void printCandidate(const BaseString& genes, std::ostream& out) const override;

   std::unique_ptr<Chromosome> initializeCandidate(Population& population) override;
   std::pair<std::unique_ptr<Chromosome>, std::unique_ptr<Chromosome> >
      mateCandidates(Population& population, Chromosome& mother, Chromosome& father) override;
   void mutateCandidate(Population& population, Chromosome& chromosome) override;
};
```

Use this when the problem needs to preserve invariants that the generic
operators do not understand.

## Hook A New Example Into The Repo

To make a new example part of the supported tree:

1. Add the `.hh` and `.cc` files under `examples/`
2. Update [examples/Makefile](/Users/donaldmolaro/src/GAK/examples/Makefile)
3. Add a mode in [examples/GA.cc](/Users/donaldmolaro/src/GAK/examples/GA.cc)
4. Add focused checks in [examples/test_main.cc](/Users/donaldmolaro/src/GAK/examples/test_main.cc)

## Testing Guidance

A good example test should verify at least:

- the problem can be constructed
- known fitness behavior on a few hand-built candidates
- exact-solution detection if the problem supports it
- operator invariants for structured problems
- participation in a short successful run

For constraint-aware problems, invariants matter more than exact final search
paths.

## Practical Advice

- Start with the cleanest encoding you can find.
- Use fitness to rank candidates, not to endlessly repair obviously broken
  ones.
- Implement `hasReachedSolution(...)` whenever the problem truly knows it has
  won.
- Keep `printCandidate(...)` readable; it is part of the demo experience.
- If your example needs a lot of structure-preserving logic, that is usually a
  sign the override hooks are the right tool.
