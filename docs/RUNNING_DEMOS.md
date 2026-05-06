# Running And Tuning The Demos

The example runner in [examples/GA.cc](/Users/donaldmolaro/src/GAK/examples/GA.cc)
is both a demo program and a practical way to explore how different GA
settings behave on different problem types.

## Basic Usage

Build the examples:

```sh
make -C examples
```

Run a mode from `examples/`:

```sh
cd examples
./GA <mode>
```

Every demo now prints:

1. the problem input
2. the resolved GA parameters
3. the run output

## Common CLI Overrides

The most useful runtime options are:

```sh
./GA <mode> --seed 123
./GA <mode> --population 250
./GA <mode> --trials 20000
./GA <mode> --mutation 0.02
./GA <mode> --crossover 0.75
./GA T --grid 250
```

These options let you experiment without editing the compiled defaults.

The runner also supports machine-readable export:

```sh
./GA <mode> --report-json /tmp/run.json
./GA <mode> --report-csv /tmp/run.csv
```

The JSON report can be loaded into the local viewer at
[examples/viewer.html](/Users/donaldmolaro/src/GAK/examples/viewer.html) to
scrub through per-generation progress visually.

## What The Parameter Block Means

The runner prints a `GA parameters:` section before the run starts.

Important fields:

- `operation`
  Whether the engine is minimizing or maximizing fitness
- `population`
  Number of individuals maintained per generation
- `trials`
  Evaluation budget
- `chromosome length`
  Chromosome length in genes
- `base states`
  Number of values each gene can take
- `mutation`
  Per-gene mutation rate
- `crossover`
  Crossover probability
- `reproduction`
  Whether duplicates are allowed into the next generation
- `selection`
  Parent selection strategy
- `deletion`
  How much of the population is replaced each generation
- `fitness`
  How selection weights are derived from raw fitness
- `length mode`
  Fixed-length vs variable-length chromosome interpretation

## Practical Tuning Advice

### If the run converges too slowly

Try:

- increasing `--population`
- increasing `--trials`
- slightly increasing `--crossover`

This is often helpful for:

- Sudoku
- Latin square
- N-Queens
- knapsack with harder settings

### If the run stagnates early

Try:

- increasing `--mutation`
- allowing more trials
- checking whether the encoding is too destructive for the problem

If the problem is highly constrained, this may be a sign you need a
constraint-aware example rather than just a new mutation rate.

### If the run is too noisy

Try:

- lowering `--mutation`
- using a fixed `--seed`
- increasing population size slightly

### If you want reproducibility

Always use:

```sh
./GA <mode> --seed 123
```

This is especially helpful when comparing parameter changes.

## Reading Demo Output

For many runs, the raw fitness number is less useful than the final printed
candidate.

Examples:

- `Knapsack`: read the selected item list and total weight/value
- `NQueens`: read the printed board
- `LatinSquare`: read the final grid
- `Sudoku` / `Sudoku+`: read the final board, not just the score

## Suggested First Experiments

### Benchmark-style runs

```sh
./GA D --seed 1
./GA 6 --seed 1
```

Use these to get a feel for classic minimization runs.

### Symbolic search

```sh
./GA S --seed 1
./GA A --seed 1
```

These are good for understanding symbolic encodings and exact-solution early
stopping.

### Constraint problems

```sh
./GA Q --seed 1
./GA L --seed 1
./GA U --seed 1
./GA C --seed 1
```

Compare `U` and `C` directly if you want to see the effect of
constraint-aware operators.

### Sequencing/routing

```sh
./GA T --seed 1 --grid 200
```

This is a good reminder that generic operators can attack ordering problems,
but may not preserve structure as well as a custom route encoding would.

## When To Move Beyond CLI Tuning

If you have already tried population/trial/mutation/crossover changes and the
run still behaves poorly, the next likely issue is not tuning but
representation.

That is when you should revisit:

- the encoding
- the fitness signal
- exact-solution detection
- whether the problem needs custom operators

For that step, see
[docs/WRITING_EXAMPLES.md](/Users/donaldmolaro/src/GAK/docs/WRITING_EXAMPLES.md)
and
[docs/PROBLEM_FIT.md](/Users/donaldmolaro/src/GAK/docs/PROBLEM_FIT.md).

For repeatable multi-seed experiments, see
[examples/experiments.md](/Users/donaldmolaro/src/GAK/examples/experiments.md).
