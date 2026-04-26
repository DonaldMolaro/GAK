# Lab: Generic Vs Constraint-Aware Sudoku

This is the most important conceptual lab in the repo.

## Goal

See why operator design matters more than parameter tuning for some problems.

## Run

```sh
cd /Users/donaldmolaro/src/GAK/examples
./GA U --seed 1
./GA C --seed 1
```

## Compare

Focus on:

- how the two modes describe the same puzzle
- how quickly they improve
- how stable promising structure looks
- how plausible the final boards are

## Questions

1. What does `U` allow that makes Sudoku harder?
2. What invariants does `C` preserve?
3. Why is a better representation often stronger than just increasing trials?

## Follow-Up Experiments

Try:

```sh
./GA U --seed 1 --trials 20000
./GA C --seed 1 --trials 20000
./GA U --seed 1 --mutation 0.02
./GA C --seed 1 --mutation 0.02
```

Ask:

1. Does tuning rescue the generic representation?
2. Which mode seems to waste more effort on obviously bad boards?
3. Which mode would you want as the starting point for a real Sudoku solver?

## Takeaway

This comparison teaches the central lesson of the repo:

- generic operators are useful
- but some problems need structure-preserving operators to search effectively
