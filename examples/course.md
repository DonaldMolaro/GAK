# Genetic Algorithms For The Classical Engineer

This course uses the actual `GAK` example problems to teach genetic algorithms
from a practical engineering perspective.

Estimated effort: 6 to 10 hours of self-study, depending on how much time you
spend running experiments between modules.

## Course Goals

By the end of the course, you should be able to:

- explain what a genetic algorithm is doing in plain language
- choose a chromosome representation for a problem
- recognize when generic mutation/crossover are good enough
- recognize when a problem needs structure-preserving operators
- read and modify a `PopulationProblem` in this codebase
- use the examples as templates for your own optimization problems

## Module 1: What A Genetic Algorithm Is

Read:

- [README.md](/Users/donaldmolaro/src/GAK/README.md)
- [glossary.md](/Users/donaldmolaro/src/GAK/examples/glossary.md)

Run:

```sh
cd /Users/donaldmolaro/src/GAK/examples
./GA K --seed 1
```

Focus on:

- population
- chromosome length
- fitness
- mutation
- crossover

Self-check:

1. What is being varied from candidate to candidate?
2. What signal tells the GA that one candidate is better than another?
3. Why is this called a search method rather than a direct solver?

## Module 2: Binary Decision Vectors

Read:

- [lab_knapsack.md](/Users/donaldmolaro/src/GAK/examples/lab_knapsack.md)

Run:

```sh
./GA K --seed 1
./GA K --seed 1 --mutation 0.02
./GA K --seed 1 --population 250
```

Key idea:

Knapsack is the cleanest example of a binary chromosome: one gene per item,
selected or not selected.

Self-check:

1. What does one gene mean in this example?
2. Why is knapsack a strong fit for a basic GA?
3. What does the penalty do when the bag is overweight?

## Module 3: Symbolic Chromosomes

Read:

- [docs/WRITING_EXAMPLES.md](/Users/donaldmolaro/src/GAK/docs/WRITING_EXAMPLES.md)

Run:

```sh
./GA A --seed 1
./GA S --seed 1
```

Key idea:

Not every problem is binary. `GAK` supports symbolic genes directly through
`BaseString`, so each gene can represent one of several states.

Self-check:

1. How is `A` different from `S`?
2. Why is `S` variable-length while `A` is fixed-length?
3. Why can these problems stop early when solved exactly?

## Module 4: Numeric Black-Box Optimization

Read:

- [lab_numeric.md](/Users/donaldmolaro/src/GAK/examples/lab_numeric.md)

Run:

```sh
./GA D --seed 1
./GA 6 --seed 1
```

Key idea:

The GA does not need gradients or symbolic math. It only needs a way to score
candidates.

Self-check:

1. Where do the numeric coordinates actually come from?
2. Why are these examples still using chromosomes even though the target
   problem looks numeric?
3. What are the tradeoffs of binary encoding for numeric search?

## Module 5: Constraint Satisfaction

Run:

```sh
./GA Q --seed 1
./GA L --seed 1
```

Key idea:

A good GA constraint problem rewards partial progress instead of treating every
invalid candidate as equally bad.

Self-check:

1. What does partial progress mean in N-Queens?
2. What does partial progress mean in Latin squares?
3. Why is a graded score important for these problems?

## Module 6: Generic Operators Vs Structured Problems

Read:

- [docs/SUDOKU_DESIGN.md](/Users/donaldmolaro/src/GAK/docs/SUDOKU_DESIGN.md)
- [docs/CROSSOVER_MODES.md](/Users/donaldmolaro/src/GAK/docs/CROSSOVER_MODES.md)

Run:

```sh
./GA U --seed 1
./GA C --seed 1
```

Key idea:

This is the most important design lesson in the repo: a representation and
operator choice can matter more than fine-tuning mutation rates.

Self-check:

1. What kinds of invalid structure does generic Sudoku keep generating?
2. What invariants does constraint-aware Sudoku preserve?
3. Why is `C` a better template for hard constrained problems?

## Module 7: Reading The API

Read:

- [src/population.hh](/Users/donaldmolaro/src/GAK/src/population.hh)
- [src/chromosome.hh](/Users/donaldmolaro/src/GAK/src/chromosome.hh)
- [src/base.hh](/Users/donaldmolaro/src/GAK/src/base.hh)

Focus on:

- `PopulationProblem`
- `Population::Settings`
- `evaluateFitness(...)`
- `printCandidate(...)`
- `initializeCandidate(...)`
- `mateCandidates(...)`
- `mutateCandidate(...)`
- `hasReachedSolution(...)`

Self-check:

1. Which parts are engine responsibilities?
2. Which parts are problem responsibilities?
3. Which hook would you override first for a highly constrained problem?

## Module 8: Building Your Own Problem

Read:

- [docs/PROBLEM_FIT.md](/Users/donaldmolaro/src/GAK/docs/PROBLEM_FIT.md)
- [docs/WRITING_EXAMPLES.md](/Users/donaldmolaro/src/GAK/docs/WRITING_EXAMPLES.md)
- [lab_first_problem.md](/Users/donaldmolaro/src/GAK/examples/lab_first_problem.md)

Exercise:

Pick a small optimization problem from your own work and answer:

1. What does one gene mean?
2. What should `chromosomeLength` be?
3. How many `baseStates` do you need?
4. Can you score partial progress?
5. Can you detect an exact solution?
6. Do you need generic operators or custom ones?

## Recommended Capstone

Build a new example in one of these categories:

- binary selection with penalties
- symbolic arrangement with soft constraints
- constrained board problem with custom operators

Best templates:

- `Knapsack` for binary search
- `NQueens` for symbolic constraint scoring
- `SudokuConstrained` for structured operators

## Next References

- [lesson_plan.md](/Users/donaldmolaro/src/GAK/examples/lesson_plan.md)
- [prerequisites.md](/Users/donaldmolaro/src/GAK/examples/prerequisites.md)
- [lab_knapsack.md](/Users/donaldmolaro/src/GAK/examples/lab_knapsack.md)
- [lab_symbolic.md](/Users/donaldmolaro/src/GAK/examples/lab_symbolic.md)
- [lab_numeric.md](/Users/donaldmolaro/src/GAK/examples/lab_numeric.md)
- [lab_constraints.md](/Users/donaldmolaro/src/GAK/examples/lab_constraints.md)
- [lab_sudoku.md](/Users/donaldmolaro/src/GAK/examples/lab_sudoku.md)
- [lab_first_problem.md](/Users/donaldmolaro/src/GAK/examples/lab_first_problem.md)
