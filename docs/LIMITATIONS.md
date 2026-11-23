# Known Limitations

What GAK does not do, and the hard limits built into the code. The design
choices behind the project's scope are in the
[white paper](WHITE_PAPER.md#12-current-limitations); this page lists the
concrete, operational limits a user can run into.

## GA engine (`src/`)

### Fixed limits

| Limit | Value | Where | What happens |
|---|---|---|---|
| Chromosome length | 2048 genes | `chromosome.cc` | Longer chromosomes are rejected; variable-length crossover truncates children to this length |
| Mating attempts per generation | 50 per slot to fill | `population.cc` | With duplicates disallowed, breeding stops early when distinct children run out |
| Stalled generations | 50 in a row with no new member | `population.cc` | The run stops with `RunResult::stalled` set |
| Evaluation count | `int` | `Population::Settings::numberOfTrials` | Budgets above about 2.1 billion evaluations are not representable |

### Behaviour to be aware of

- **Fitness must suit the selection rule.** Roulette selection on raw fitness
  (`FitnessMode::Evaluation`) needs non-negative fitness when maximizing; it
  throws otherwise. Use `Windowed` or `LinearNormalized` fitness, or
  tournament selection, for fitness that can be negative.
- **Fitness must be finite.** A NaN or infinite fitness value stops the run
  with an error.
- **Settings are checked when the run starts**, in `execute()`, not when the
  `Population` is constructed.
- **A parent can be chosen twice** for one mating; the children are then
  copies (before mutation).
- **Mutation changes values, never length.** Variable-length chromosomes
  change length only through crossover.
- **Only the deletion modes keep good members.** `DeleteAll` replaces the
  whole population each generation (no elitism); `DeleteAllButBest`,
  `DeleteHalf`, `DeleteQuarter` and `DeleteLast` keep the best.
- **Fitness is not cached.** A chromosome that reappears is evaluated again.
- **One thread.** A `Population` is not safe to share between threads, and
  evaluation is sequential.

### Reproducibility

A fixed seed reproduces a run exactly, including across GCC/libstdc++ and
Clang/libc++, because random draws use the portable helpers in
`src/random.hh`, the population sort is stable, and the Makefiles build with
`-ffp-contract=off`. The guarantee does not extend to:

- fitness functions that call math-library routines (`std::sin`, `std::exp`,
  …), whose last bit can differ on another operating system's math library;
- builds with other floating-point flags, such as `-ffast-math`;
- problem code that draws random numbers through `<random>` distributions
  instead of `gak::` helpers.

## Reports and the viewer

- **Regression plots show at most 1,000 rows**, evenly spaced through the
  file; the fit and every R² use all rows. Formula text rounds coefficients
  to 4 significant digits; the exact values are in the JSON
  (`intercept`, `term_weights`).
- **Reports grow with the run.** Generation reports are kept in memory and
  written at the end. A symbolic regression report is about 1.8 MB for a
  one-input, 80-generation run and about 22 MB for a ten-input,
  390-generation run (`--samples 0` makes it smaller).
- **The viewer loads the whole report** into the browser; very large reports
  are slow to open.
- **The viewer has a light theme only**, fixed-size canvases, and no
  automated browser tests (it is checked by running its script against a stub
  DOM).
- **Firefox under SSH X forwarding** may not open a file dialog; see
  `widget.use-xdg-desktop-portal.file-picker` in
  [RUNNING_DEMOS.md](RUNNING_DEMOS.md#viewer-troubleshooting). Pasting the JSON into the viewer
  works everywhere.

## Symbolic regression (mode `R`)

### Data

- **CSV only**, with a header row, comma separators, no quoted fields and no
  missing values; every value must be a finite number. Column names must be
  unique. A UTF-8 byte-order mark is ignored; names that are not valid UTF-8
  are shown with a replacement character in reports.
- **Every column except the target is an input.** There is no way to exclude
  a column other than removing it from the file.
- **Inputs are used as given.** They are not standardized, so formulas such
  as `exp(x)` become undefined (and are penalized) when an input is large:
  `exp` is undefined above 700 and any intermediate value above 10¹² counts
  as overflow. A file with any input or target above 10¹² in magnitude is
  rejected with a request to rescale it.
- **Inputs of very different scales fit less neatly.** The least-squares fit
  uses a small ridge shared by all terms, so a term more than about 10⁴
  times smaller than another is shrunk. The search usually compensates with
  an extra constant (for example `5.9e5*flow` beside a pressure in pascals),
  which costs formula size. Rescaling such columns to similar ranges gives
  simpler formulas.
- **The train/test split is fixed** by a constant seed, so every run of the
  same file holds out the same rows. Use `--holdout` to change its size.

### Search

- **Squared error only.** Gross outliers pull the fit (see
  `linear_outliers.csv`); there is no robust loss.
- **A fixed operator set and constant table**
  (`+ − × ÷ sin cos exp log sqrt ^2 neg`; constants `0.1 0.5 1 2 3 5 10`).
  Other constants come from constant tuning, a short pattern search
  (40 evaluations per formula by default).
- **At most 8 additive terms get their own coefficient.** A formula with more
  is fitted with one shared coefficient.
- **Like terms are not merged.** `x + x` is two terms, each with its own
  coefficient, and costs the size of both; the search only removes such
  redundancy when a smaller formula fits as well.
- **The archive keeps one formula per size.** Cross-validation chooses among
  the best formula of each size found across all restarts, so a different
  formula of the same size that would generalize better is never compared.
  Formulas undefined on any cross-validation fold are dropped from the
  comparison.
- **Noisy data cannot always tell formulas apart.** At about 30% noise,
  `√L` and a straight line, or a line with and without a small wiggle,
  predict about equally well; the one-standard-error rule then prefers the
  simpler formula, which may not be the true law.
- **Stall detection is coarse.** A run stops early only after 50 generations
  in which no new member entered the population; a population that keeps
  admitting slightly different, no-better formulas runs to its budget.
- **Some structures remain hard**, for example `m1·m2/r²` (usually found only
  approximately) and the `sin(x1)` term in `x1·x2 + sin(x1)`.

### Scale

- **Tested on 1 to 10 inputs.** Hundreds of inputs would need screening
  before the search, which is not implemented.
- **Run time grows with inputs and rows.** The default budget is 15,000
  trials per restart for each pair of inputs; a ten-input, 250-row run takes
  5–12 seconds. Most of it is constant tuning, and the restarts run one
  after another on a single core.

Results, benchmarks and the reasoning behind the defaults are in
[SYMBOLIC_REGRESSION.md](SYMBOLIC_REGRESSION.md).

## Example problems

- **Traveling salesman** scores an open path: there is no edge back to the
  starting city.
- **Neither Sudoku mode reliably solves its puzzle.** The generic version
  (`U`) shows what the generic operators cannot do; the constraint-aware
  version (`C`) gets much closer, but in an 8-seed sweep it scored 496–504
  of the 513 a solution scores, without solving it.
- **The problem inputs are fixed in code** (the Sudoku puzzle, the graph, the
  timetable, the knapsack items); only mode `R` reads data from a file.

## Tooling

- **`make coverage`** needs LLVM's `llvm-profdata` and `llvm-cov`. Without
  them, use `make coverage-gcc`, which needs only GCC.
- **`lcov` misreads GCC 15 coverage data** (impossible percentages and
  "mismatch" warnings); `make coverage-gcc` uses `gcov` directly instead.
- **`check_labs.sh` is a smoke test**: it checks that each lab's command runs
  and prints a result, not that the result is good.
