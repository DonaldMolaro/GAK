# Lab: Symbolic Chromosomes

This lab uses `A` and `S` to show how `GAK` handles non-binary genes.

## Goal

Understand the difference between:

- fixed-length symbolic chromosomes
- variable-length symbolic chromosomes
- exact-solution early stop

## Run

```sh
cd /Users/donaldmolaro/src/GAK/examples
./GA A --seed 1
./GA S --seed 1
```

## Observe

Look at:

- `chromosome length`
- `base states`
- `length mode`
- whether the run stops as soon as it finds the exact target

## Questions

1. Why does `A` use fixed length while `S` uses variable length?
2. Why are both examples symbolic rather than binary?
3. What does one gene mean in each problem?
4. Why can these problems stop early?

## Experiments

Try:

```sh
./GA A --seed 1 --mutation 0.02
./GA S --seed 1 --mutation 0.02
./GA S --seed 1 --population 250
```

Ask:

1. Which problem is more sensitive to mutation rate?
2. Does a larger population change how quickly the exact solution appears?
3. When does extra search stop helping much?

## Takeaway

These examples show that:

- genes can represent symbols, not just bits
- chromosome structure matters
- exact-solution detection is a useful GA optimization when the target is known
