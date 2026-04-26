# Lab: Knapsack

This is the best first hands-on lab in the repo.

## Goal

Learn how a simple binary chromosome maps onto a concrete optimization problem.

## Run

```sh
cd /Users/donaldmolaro/src/GAK/examples
./GA K --seed 1
```

## Observe

Look at:

- the problem input
- the GA parameter block
- the final selected item list
- the reported weight and value

## Questions

1. What does a `1` gene mean in this example?
2. Why is `base states` equal to `2`?
3. Why is the fitness still useful even for overweight selections?

## Experiments

Try:

```sh
./GA K --seed 1 --mutation 0.02
./GA K --seed 1 --population 250
./GA K --seed 1 --trials 20000
```

Ask:

1. Which change helps more: more trials or more mutation?
2. When do you start seeing diminishing returns?
3. Does a larger population produce more stable behavior?

## Takeaway

Knapsack is a strong fit for a GA because:

- the encoding is natural
- mutation is easy to interpret
- crossover can preserve useful building blocks
- partial progress is easy to score
