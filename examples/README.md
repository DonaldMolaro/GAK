# Examples Index

This folder contains the runnable `GA` demo program, the example problem
implementations, and an instructional course for learning genetic algorithms
through the examples in this repo.

## Start Here: Self-Study Course

If you are new to genetic algorithms, start with:

- [course.md](/Users/donaldmolaro/src/GAK/examples/course.md):
  **Genetic Algorithms for the Classical Engineer**. A guided self-study course
  built around the actual `GAK` examples, from first principles through
  constrained operators.

## Teaching Material

- [lesson_plan.md](/Users/donaldmolaro/src/GAK/examples/lesson_plan.md):
  module-by-module teaching sequence with commands and checkpoints
- [prerequisites.md](/Users/donaldmolaro/src/GAK/examples/prerequisites.md):
  what a learner should understand before each stage
- [glossary.md](/Users/donaldmolaro/src/GAK/examples/glossary.md):
  key GA terms mapped to plain-language intuition
- [lab_knapsack.md](/Users/donaldmolaro/src/GAK/examples/lab_knapsack.md):
  guided first lab on binary decision vectors and penalties
- [lab_sudoku.md](/Users/donaldmolaro/src/GAK/examples/lab_sudoku.md):
  guided comparison between generic and constraint-aware operators

## Demo Modes

Run the example driver from this directory:

```sh
./GA <mode>
```

Supported modes:

- `D`: Dome benchmark minimization
- `F6`: F6 benchmark minimization
- `A`: alphabet ordering
- `S`: spelling a target word
- `T`: traveling salesman
- `Q`: N-Queens
- `K`: knapsack
- `L`: Latin square
- `U`: generic Sudoku
- `C`: constraint-aware Sudoku

## Suggested Learning Order

1. `K` for binary decision vectors
2. `A` and `S` for symbolic chromosomes
3. `D` and `F6` for numeric black-box optimization
4. `Q` and `L` for constraint scoring
5. `U` and `C` for representation and operator design

## Build And Test

Build the examples:

```sh
make -C examples
```

Run the example-target tests:

```sh
make example-tests
```

For more tuning guidance, see
[docs/RUNNING_DEMOS.md](/Users/donaldmolaro/src/GAK/docs/RUNNING_DEMOS.md).
