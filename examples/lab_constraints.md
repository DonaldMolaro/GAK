# Lab: Constraint Satisfaction

This lab uses `Q` and `L` to show how a GA can improve partial solutions even
before a problem is fully solved.

## Goal

Understand how to score partial progress on constrained problems.

## Run

```sh
cd /Users/donaldmolaro/src/GAK/examples
./GA Q --seed 1
./GA L --seed 1
```

## Observe

Look at:

- the problem input section
- how the printed candidates become more structured
- how the final board tells you more than the raw score alone

## Questions

1. What does a gene mean in N-Queens?
2. What does a gene mean in Latin square?
3. What counts as partial progress in each problem?
4. Why is a graded score better than a strict pass/fail score here?

## Experiments

Try:

```sh
./GA Q --seed 1 --trials 20000
./GA L --seed 1 --population 250
./GA L --seed 1 --mutation 0.02
```

Ask:

1. Which parameter changes seem to help most?
2. Does more search always produce a clearly better board?
3. Which problem feels closer to a hard-constraint puzzle?

## Takeaway

Constraint problems work well with a GA when:

- the encoding is clear
- invalid candidates are not all scored the same
- the fitness function can distinguish “closer” from “farther”
