# Lesson Plan

This lesson plan is meant for either self-study checkpoints or an instructor
walking a learner through the repo live.

## Module 1: Orientation

Goal:

- understand what `GAK` is
- identify the engine, problems, and runner

Read:

- [README.md](/Users/donaldmolaro/src/GAK/README.md)
- [examples/README.md](/Users/donaldmolaro/src/GAK/examples/README.md)

Checkpoint:

- learner can explain the difference between `Population` and `PopulationProblem`

## Module 2: Binary Chromosomes

Run:

```sh
cd /Users/donaldmolaro/src/GAK/examples
./GA K --seed 1
```

Checkpoint:

- learner can explain why one gene maps naturally to one knapsack item

## Module 3: Symbolic Chromosomes

Run:

```sh
./GA A --seed 1
./GA S --seed 1
```

Checkpoint:

- learner can explain the difference between binary and symbolic genes

## Module 4: Numeric Benchmarks

Run:

```sh
./GA D --seed 1
./GA 6 --seed 1
```

Checkpoint:

- learner can explain how a chromosome can still drive a numeric objective

## Module 5: Constraint Scoring

Run:

```sh
./GA Q --seed 1
./GA L --seed 1
```

Checkpoint:

- learner can explain what partial progress means in a constraint problem

## Module 6: Representation Matters

Run:

```sh
./GA U --seed 1
./GA C --seed 1
```

Checkpoint:

- learner can explain why `C` usually behaves better than `U`

## Module 7: Tuning

Run:

```sh
./GA K --seed 1 --population 250
./GA K --seed 1 --mutation 0.02
./GA C --seed 1 --trials 20000
```

Checkpoint:

- learner can explain when tuning helps and when the encoding is the real problem

## Module 8: Authoring

Read:

- [docs/WRITING_EXAMPLES.md](/Users/donaldmolaro/src/GAK/docs/WRITING_EXAMPLES.md)

Checkpoint:

- learner can sketch a new `PopulationProblem` with a plausible encoding
