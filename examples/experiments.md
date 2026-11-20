# Running Experiments

The `GA` runner now supports machine-readable output, and this folder includes
a small sweep tool for comparing multiple seeds.

## Single Run Exports

You can export one run as JSON and per-generation CSV:

```sh
cd examples   # from the repository root
./GA K --seed 1 \
  --report-json /tmp/gak-knapsack.json \
  --report-csv /tmp/gak-knapsack.csv
```

What you get:

- JSON with settings, final summary, and generation reports
- CSV with one row per generation:
  - `generation`
  - `evaluations`
  - `best_fitness`
  - `worst_fitness`

To browse a report visually, open `viewer.html` (in this `examples/`
directory) in a web browser and load the JSON file into it, or paste the
JSON into its text box. Custom views exist for traveling salesman, N-Queens,
Latin square, knapsack, graph coloring, timetable, both Sudoku modes and
symbolic regression; other problems use a generic gene view.

## Multi-Seed Sweeps

Use the sweep helper:

```sh
cd examples   # from the repository root
./sweep_runs.sh K /tmp/gak-knapsack 1 2 3
```

You can also forward normal `GA` options after `--`:

```sh
./sweep_runs.sh C /tmp/gak-sudoku 1 2 3 -- --population 64 --trials 500
```

For each seed, the tool writes:

- `run_<seed>.txt`
- `run_<seed>.json`
- `run_<seed>.csv`

It also writes:

- `summary.csv`

The summary contains one row per seed with:

- `generations_completed`
- `evaluations`
- `solution_found`
- `stopped_early`
- `best_fitness` (the best value for the run's direction, so the smallest when
  minimizing)
- `worst_fitness`

## Suggested Experiment Patterns

### Stability across seeds

```sh
./sweep_runs.sh K /tmp/gak-knapsack 1 2 3 4 5
```

Use this to see how noisy a problem is under the same settings.

### Population-size comparison

```sh
./sweep_runs.sh Q /tmp/gak-q-small 1 2 3 -- --population 32
./sweep_runs.sh Q /tmp/gak-q-large 1 2 3 -- --population 128
```

Compare the resulting `summary.csv` files.

### Mutation-rate comparison

```sh
./sweep_runs.sh S /tmp/gak-s-lowmut 1 2 3 -- --mutation 0.005
./sweep_runs.sh S /tmp/gak-s-highmut 1 2 3 -- --mutation 0.02
```

### Generic vs constraint-aware problem design

```sh
./sweep_runs.sh U /tmp/gak-u 1 2 3 -- --trials 500
./sweep_runs.sh C /tmp/gak-c 1 2 3 -- --trials 500
```

This is especially useful for teaching why operator design matters.

## What To Look For

- how much best fitness varies by seed
- whether exact-solution problems stop early consistently
- whether a settings change improves both average and worst-case behavior
- whether a representation/operator change matters more than tuning

## Good Next Step

Once you have a few sweep directories, pull the `summary.csv` files into a
spreadsheet or notebook and compare:

- fitness by seed
- evaluations to solution
- settings vs outcome
