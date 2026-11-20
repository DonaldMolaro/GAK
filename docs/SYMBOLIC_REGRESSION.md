# Symbolic Regression

Mode `R` discovers a formula that fits measured data: given a CSV of points,
it searches for an expression `y = f(inputs)` rather than fitting the
coefficients of a model you chose in advance.

```sh
make examples
cd examples
./GA R --data data/exponential_decay.csv --seed 1
```

```
Selected by 5-fold cross-validation (from the best formula of each of 35 sizes; cv R2=0.9398 RMSE=31.8478):
counts ~= -3.863 + 497.7*(exp(time_s*(-0.3079))) | nodes=4 train R2=0.9526 RMSE=28.2623 | test R2=0.7880 RMSE=34.8576 ::0.055440
```

The true law is `counts = 500 e^(−0.3t)`; the GA recovered the exponential,
its amplitude within 0.5% and its rate within 3%, from 38 heavily noisy
training points. The lower test R² is not overfitting: the noise is
largest where the counts are high, and a few of those early points landed in
the held-out rows.

## Options

| Option | Meaning |
|---|---|
| `--data PATH` | CSV file (required). Header row, numeric values. |
| `--target NAME` | Output column. Defaults to the last column; every other column is an input. |
| `--holdout R` | Share of rows held out to test generalization (default `0.25`, may be `0`). |
| `--restarts N` | Independent populations; the one with the best *training* fitness is reported (default `4`). |
| `--tuning N` | Constant-tuning evaluations per formula (default `40`; `0` turns tuning off). |
| `--folds N` | Cross-validation folds for choosing the final formula (default `5`; `0` reports the GA's best training fit instead). |
| `--cv-se R` | Choose the smallest formula within R standard errors of the best cross-validated error (default `1`; lower favours richer formulas). |
| `--trials N` | Evaluation budget **per restart** (default 15,000 for each pair of inputs: 15,000 for one or two inputs, 45,000 for five, 75,000 for ten). |
| `--seed N`, `--population N`, `--mutation R`, `--crossover R`, `--selection S`, `--tournament N` | As for every mode. |
| `--report-json P`, `--report-csv P` | Reports; the JSON includes a `regression` visualization for `viewer.html`. |
| `--samples N` | Population members recorded per generation in reports, at evenly spaced fitness ranks (default `6`; `0` for none). |

The CSV loader is strict: a non-numeric value, a ragged row, an unknown
`--target`, or fewer than 3 data rows stops the run with the file and line.
Test datasets with known answers are in [examples/data](../examples/data/README.md).

## Reading the output

The run ends with the **selected** formula (see "Choosing the final formula"
below), printed after the GA's own best formulas. Each result line shows:

- the formula, as `a + b*(f)`: `f` is what the GA evolved, and `a` and `b`
  are fitted afterwards (see linear scaling below);
- `nodes`: the size of `f`, which is what the length penalty charges for;
- `train R2` / `RMSE`: fit on the rows the GA saw;
- `test R2` / `RMSE`: fit on the held-out rows. A test score much worse than
  the training score means the formula is fitting the noise.

The number after `::` is the fitness being minimized (lower is better).

## Several inputs

Every numeric column except the target is an input, and nothing in the
search is limited to one or two. With several inputs:

- **Which inputs a formula uses** is reported with it (`uses 3 of 5 inputs:
  x1 x3 x4`, and `inputs_used` in the JSON). An input a good formula leaves
  out is one the search found irrelevant.
- **Each additive term gets its own coefficient** (see "Fitting a
  coefficient to each term"), so sums like `m·v²/2 + 9.81·m·h` need only the
  right terms.
- **The default budget grows** with the number of inputs (`--trials`).
- **The viewer** plots predicted against measured values; points on the
  diagonal are predicted exactly.

## Watching a run

Load a `--report-json` report into [examples/viewer.html](../examples/viewer.html):

- **Generation 0 is the random starting population.** Expect chaos there:
  in a typical run the median formula is no better than the mean (fitness
  ≈ 1), most formulas never use the input, and the worst are undefined. The
  *best* of 400 random formulas still often looks reasonable, because linear
  scaling fits `a + b·f` to any shape and a plain `x` is common.
- **The fitness chart** shows the best formula, the population median, and a
  band for the middle 50%. Progress comes in jumps; the band collapsing onto
  the best line shows the population converging.
- **Sampled members** (best, 20%, 40%, …, worst by rank) are drawn as faint
  curves behind the best formula and listed under the plot.
- **The last frame** shows the formula chosen by cross-validation.

## How it works

### Encoding: postfix programs

A chromosome is a variable-length string of instruction genes, run left to
right by a stack machine ([expression.hh](../examples/expression.hh)):

- **variables** push an input value (each input appears 3 times in the
  alphabet, so formulas are not dominated by constants);
- **constants** push one of `0.1 0.5 1 2 3 5 10`;
- **unary operators** `sin cos exp log sqrt ^2 neg` replace the top value;
- **binary operators** `+ − × ÷` combine the top two.

An instruction that would underflow the stack is skipped, and only the
instructions that feed the final top-of-stack value form the formula. So every
gene string is a valid formula, and the library's generic variable-length
crossover and point mutation can be used unchanged. Offspring are truncated
to 64 genes to bound growth.

### Undefined values

Division by (nearly) zero, `log` or `sqrt` outside their domain, and overflow
make a formula *undefined* on the data. Undefined formulas get a large finite
penalty fitness, never NaN, which the engine would reject.

### Constant tuning

The constant table (`0.1 … 10`) cannot express a value like the `0.3` in
`e^(−0.3t)` directly. So before a formula is scored, its constants are
refined by a short deterministic pattern search: each constant is moved up or
down by a relative step, any move that lowers the training error is kept, and
the step halves when no move helps, for up to 40 evaluations. Fitness and the
printed formula use the tuned values; the genes keep the table values, so the
GA learns which *structures* tune well (a "Baldwinian" scheme). The search
uses no randomness, so results stay reproducible.

### Restarts

The population can converge on a local optimum, such as `x1·(x2 + 0.33)`
standing in for `x1·x2 + sin(x1)`, and a larger budget does not dislodge it.
The runner therefore runs several independent populations, with seeds
derived from `--seed`, and reports the one with the lowest training fitness.
The held-out rows never influence that choice, so the reported test score
stays an honest estimate. Restart 1 always uses the `--seed` value itself.

### Choosing the final formula

The GA minimizes *training* error, and on noisy data training error always
rewards extra terms that follow the noise: on `linear_noisy` most runs end on
a line decorated with wiggles such as `0.58·cos(x²)`. So the GA's best
formula is not the answer. Instead:

1. **Archive.** Throughout the search (all restarts), the formula with the
   lowest training error is kept for each formula size.
2. **Cross-validate.** Each archived structure is scored by 5-fold
   cross-validation on the training rows: its constants and scaling are refit
   on four folds and it predicts the fifth, so every training row is predicted
   by a fit that never saw it.
3. **One-standard-error rule.** Among the formulas whose cross-validated
   error is within one standard error of the best, the smallest wins. With a
   few dozen noisy points, smaller differences in error are not evidence that
   a larger formula is better.

The test rows play no part, so the reported test score stays honest.
`--folds 0` turns selection off; `--cv-se` makes the rule stricter or looser.

A single validation split was tried first and rejected: on small files it
leaves too few rows (9 for `pendulum`), and its noisy error estimate chose
badly in both directions.

### Fitting a coefficient to each term

A candidate formula is split at its top-level `+` and `−` into additive
terms (for `x1·x2 + sin(x1) − 3`, the terms `x1·x2` and `sin(x1)`; the
constant only moves the intercept). Then

```
y ≈ a + b1·term1 + b2·term2 + …
```

is solved by least squares, giving each term its own coefficient (up to 8
terms; a formula with more is fitted as one term). With a single term this is
classic linear scaling, `a + b·f` (Keijzer, 2003). The GA therefore only has
to find the right **terms**; their weights come for free. This matters most
with several inputs, where laws are often sums of terms with different
weights (`2·x1·x3 − x4²`, or Friedman #1's `10·sin(π·x1·x2) + 20(x3 − 0.5)² +
10·x4 + 5·x5`): with one shared coefficient the search would have to build
each weight out of table constants. Constant tuning, cross-validation and the
reported test score all use the same per-term fit.

### Fitness

```
fitness = MSE_train / variance(y_train) + 0.002 * (nodes + 6 * (terms − 1))
```

The first part is the normalized error: 0 is a perfect fit and 1 is no better
than predicting the mean, independent of the units of `y`. The second is a
parsimony penalty that stops formulas growing to fit noise. Each term beyond
the first is charged as 6 extra nodes, because its fitted coefficient can
fit noise just as extra nodes can; without that charge, noisy straight lines
picked up separately weighted wiggle terms.

### Train/test split

A fixed share of rows is held out. The split uses its own fixed seed, so
different `--seed` values explore different searches over the *same* split
and are directly comparable.

### GA settings

4 restarts of 400 individuals and 15,000 evaluations each (more with more
inputs; see `--trials`), rank-based
roulette selection (`LinearNormalized`), half the population replaced each
generation, duplicates rejected, crossover 0.8, mutation 0.04 per gene,
constant tuning with 40 evaluations. A run takes about 0.8 seconds on the
bundled datasets.

Tournament selection (`--tournament N`) is available but was not better here:
with constant tuning it lowered the recovery rate.

## Results on the bundled datasets

The datasets carry heavy noise (about 30% of each signal's spread; see
[examples/data](../examples/data/README.md)). Two measures, seeds 11–20:

- **Test R²**: how well the formula predicts the held-out rows. This is what
  you can measure on real data.
- **Structure recovered**: how often the fitted curve matches the noise-free
  true curve as closely as the true formula fitted to the same noisy rows
  could (within 0.15 of that "oracle" curve error, in units of the noise).
  This needs the known truth, so it is only available for test data.

### One or two inputs

| Dataset | Structure recovered | Median test R² |
|---|---|---|
| `linear_noisy` | 7 / 10 | 0.960 |
| `quadratic` | 10 / 10 | 0.863 |
| `sine_trend` | 10 / 10 | 0.902 |
| `exponential_decay` | 10 / 10 | 0.788 |
| `pendulum` | 5 / 10 | 0.933 |
| `rational` | 8 / 10 | 0.882 |
| `two_input` | 8 / 10 | 0.906 |
| **Total** | **58 / 70** | |

Fitting a coefficient to each additive term raised this from 49 / 70 (one
shared coefficient, everything else the same): `rational` went from 6 to 8,
`two_input` from 7 to 8 and `pendulum` from 2 to 5. Without the term penalty
it reached 52 / 70 but `linear_noisy` fell to 1 / 10, which is why each extra
term is charged.

### Three to ten inputs

| Dataset | Inputs | Structure recovered | Median test R² (true formula) | Time per run |
|---|---|---|---|---|
| `gravitation` | 3 | 2 / 10 | 0.979 (0.986) | 1.7 s |
| `ideal_gas` | 3 | 10 / 10 | 0.983 (0.991) | 1.0 s |
| `mechanical_energy` | 3 | 10 / 10 | 0.992 (0.990) | 1.9 s |
| `five_inputs` | 5 | 5 / 10 | 0.865 (0.907) | 1.5 s |
| `friedman1` | 10 | 4 / 10 | 0.967 (0.964) | 21.9 s |
| **Total** | | **31 / 50** | | |

With one shared coefficient and the one-and-two-input budget, the same sets
scored 20 / 50 (Friedman #1: 0 / 10, test R² 0.63). Per-term coefficients and
the larger budget each contributed; on Friedman #1 the search typically finds
`9.9·sin(3.13·x1·x2)` (π rediscovered), `9.9·x4` and `4.7·x5`, and
approximates `20(x3 − 0.5)²`, reaching the true formula's test score. Giving
each input fewer copies in the gene alphabet was also tried and did not help
(19–23 / 50).

### Selection rules compared

The rest of this section compares selection rules on the one- and two-input
sets (measured before per-term coefficients were added).

Selection never lowers the median test score, and raises it on the two
datasets where it changes the answer. The two measures disagree on
`pendulum`: at this noise level, over 0.1–3 m, a straight line in `L`
predicts the held-out rows as well as `√L` (better, in fact), so the
one-standard-error rule picks the simpler line. It is the better predictor
but not the true law. `--cv-se 0.5` keeps `√L` on 8 of 10 seeds at the cost
of keeping noise wiggles on `linear_noisy` (4 / 10 there).

| Selection rule | Structure recovered (of 70) |
|---|---|
| None (GA's best) | 53 |
| 5-fold CV, lowest error (0 SE) | 42 |
| 5-fold CV, 0.5 SE | 54 |
| 5-fold CV, 1 SE (default) | 49 |
| 10-fold CV, 0.5 / 1 SE | 52 / 49 |

Picking the lowest cross-validated error is worse than no selection: choosing
the best of 20 or more formulas by a noisy estimate overfits that estimate.

The same structure criterion on the earlier low-noise versions of these files
(noise 3–12% of the signal) gave 59 / 70 with the GA's best.

`linear_outliers` is left out of the counts: squared error is pulled by the
outliers, so the best-scoring formula is not the true line.

## Known limitations and next steps

- **What noisy data cannot tell apart.** With a few dozen points at 30%
  noise, `√L` and a straight line, or a line with and without a small
  wiggle, predict about equally well. Cross-validated selection then prefers
  the simpler formula, which predicts at least as well but may not be the
  true law. More data, or lower noise, is the only real cure; `--cv-se`
  chooses which way to lean.
- **Two-input interactions.** `x1·(x2 + c)` is a strong local optimum for
  `x1·x2 + sin(x1)`; restarts help, and island populations with migration
  would be the next step.
- **Outliers.** The loss is squared error, so a few gross outliers pull the
  fit. A robust loss (absolute or Huber error) would be an option to add.
- **Cost.** Constant tuning makes each evaluation several times more
  expensive. `--tuning 0` restores the old speed; caching fitness for
  repeated formulas would recover some of the cost.
- **Many inputs.** Tested on 3 to 10 inputs. Hundreds of inputs would need
  the inputs screened before the search (the alphabet and the search space
  grow with every input), which this does not do.
