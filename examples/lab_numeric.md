# Lab: Numeric Black-Box Optimization

This lab uses the `D` and `6` demo modes to show how `GAK` handles numeric
objectives even though the engine itself works with chromosomes and genes.

Estimated time: 35 to 50 minutes

## Goals

By the end of this lab, you should be able to:

- explain how a bit-string chromosome can still represent numeric search
- compare two benchmark-style objectives under the same GA settings
- recognize when a parameter change is helping exploration vs. simply making
  the run noisier
- connect the demo output back to encoding, fitness, and search behavior

## Before You Start

Read:

- [course.md](/Users/donaldmolaro/src/GAK/examples/course.md), Module 4
- [docs/RUNNING_DEMOS.md](/Users/donaldmolaro/src/GAK/docs/RUNNING_DEMOS.md)

Build the examples if needed:

```sh
cd /Users/donaldmolaro/src/GAK/examples
make
```

## Step 1: Run The Baseline Demos

Run:

```sh
./GA D --seed 1
./GA 6 --seed 1
```

Observe:

- the problem input block
- the GA parameter block
- the best and worst candidates shown in the summaries

Questions:

1. Which parts of the output describe the optimization problem itself?
2. Which parts describe the GA configuration?
3. Which part shows whether the run is improving over time?

## Step 2: Compare The Encodings Conceptually

Read:

- [examples/dome.cc](/Users/donaldmolaro/src/GAK/examples/dome.cc)
- [examples/f6.cc](/Users/donaldmolaro/src/GAK/examples/f6.cc)

Focus on:

- how the chromosome is decoded into numeric coordinates
- how those coordinates are fed into the objective

Questions:

1. Where does the gene string become a numeric point?
2. Why is the GA engine itself still agnostic about the math?
3. What is lost when a continuous problem is encoded with a finite chromosome?

## Step 3: Change One Knob At A Time

Run:

```sh
./GA D --seed 1 --population 250
./GA D --seed 1 --mutation 0.02
./GA D --seed 1 --trials 20000
```

Then repeat the same pattern for `6`:

```sh
./GA 6 --seed 1 --population 250
./GA 6 --seed 1 --mutation 0.02
./GA 6 --seed 1 --trials 20000
```

Interpretation guide:

- larger `population` usually buys broader exploration
- larger `trials` usually buys more search time
- larger `mutation` can help escape local traps, but can also destroy useful
  structure

Questions:

1. Which change helped more on `D`?
2. Which change helped more on `6`?
3. Did the same setting change help both problems equally?

## Step 4: Export A Structured Report

Run:

```sh
./GA D --seed 1 --population 120 --trials 4000 \
  --report-json /tmp/gak-d.json \
  --report-csv /tmp/gak-d.csv
```

Then inspect:

```sh
sed -n '1,40p' /tmp/gak-d.json
sed -n '1,12p' /tmp/gak-d.csv
```

Look for:

- the run seed
- the exact settings used
- per-generation best and worst fitness

Questions:

1. Why is this more useful than copying terminal output into notes?
2. What experiment question could you answer more easily with CSV?

## Step 5: Run A Small Sweep

Run:

```sh
./sweep_runs.sh D /tmp/gak-d-sweep 1 2 3 -- --population 80 --trials 2000
sed -n '1,10p' /tmp/gak-d-sweep/summary.csv
```

Compare the final best fitness across seeds.

Questions:

1. How much variation do you see from seed to seed?
2. Does one seed make the algorithm look much better or worse than the others?
3. Why is multi-seed testing important before concluding that a parameter
   change is good?

## Reflection

Write down short answers to these:

1. Why are `D` and `6` still good GA examples even though they are "numeric"?
2. What is one downside of a bit-string encoding for numeric search?
3. If you had to tune one setting first for a hard numeric problem, which
   would you try and why?

## Optional Extension

Design a follow-up experiment:

- keep the seed fixed and compare three mutation rates
- keep mutation fixed and compare three population sizes
- compare `D` and `6` under exactly the same budget

Record your hypothesis before running it.
