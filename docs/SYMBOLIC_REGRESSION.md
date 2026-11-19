# Symbolic Regression

Mode `R` discovers a formula that fits measured data: given a CSV of points,
it searches for an expression `y = f(inputs)` rather than fitting the
coefficients of a model you chose in advance.

```sh
make examples
cd examples
./GA R --data data/pendulum.csv --seed 1
```

```
period_s ~= 0.01133 + 1.997*(sqrt(length_m)) | nodes=2 train R2=0.9981 RMSE=0.0282 | test R2=0.9983 RMSE=0.0243
```

The true law is `T = 2π√(L/9.81) ≈ 2.006 √L`; the GA recovered the square-root
law and a coefficient within 0.5% from 30 noisy training points.

## Options

| Option | Meaning |
|---|---|
| `--data PATH` | CSV file (required). Header row, numeric values. |
| `--target NAME` | Output column. Defaults to the last column; every other column is an input. |
| `--holdout R` | Share of rows held out to test generalization (default `0.25`, may be `0`). |
| `--restarts N` | Independent populations; the one with the best *training* fitness is reported (default `4`). |
| `--tuning N` | Constant-tuning evaluations per formula (default `40`; `0` turns tuning off). |
| `--trials N` | Evaluation budget **per restart** (default `15000`). |
| `--seed N`, `--population N`, `--mutation R`, `--crossover R`, `--selection S`, `--tournament N` | As for every mode. |
| `--report-json P`, `--report-csv P` | Reports; the JSON includes a `regression` visualization for `viewer.html`. |

The CSV loader is strict: a non-numeric value, a ragged row, an unknown
`--target`, or fewer than 3 data rows stops the run with the file and line.
Test datasets with known answers are in [examples/data](../examples/data/README.md).

## Reading the output

Each result line shows:

- the formula, as `a + b*(f)`: `f` is what the GA evolved, and `a` and `b`
  are fitted afterwards (see linear scaling below);
- `nodes`: the size of `f`, which is what the length penalty charges for;
- `train R2` / `RMSE`: fit on the rows the GA saw;
- `test R2` / `RMSE`: fit on the held-out rows. A test score much worse than
  the training score means the formula is fitting the noise.

The number after `::` is the fitness being minimized (lower is better).

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

### Linear scaling

For each candidate `f`, the intercept `a` and slope `b` minimizing the squared
error of `a + b*f` are solved in closed form (least squares). The GA therefore
only has to find the **shape** of the relationship; scale and offset come for
free. This is the standard technique (Keijzer, 2003) that makes symbolic
regression practical on real data, and it is why the evolved constants can
stay small.

### Fitness

```
fitness = MSE_train / variance(y_train) + 0.002 * nodes
```

The first term is the normalized error: 0 is a perfect fit and 1 is no better
than predicting the mean, independent of the units of `y`. The second is a
parsimony penalty that stops formulas growing to fit noise.

### Train/test split

A fixed share of rows is held out. The split uses its own fixed seed, so
different `--seed` values explore different searches over the *same* split
and are directly comparable.

### GA settings

4 restarts of 400 individuals and 15,000 evaluations each, rank-based
roulette selection (`LinearNormalized`), half the population replaced each
generation, duplicates rejected, crossover 0.8, mutation 0.04 per gene,
constant tuning with 40 evaluations. A run takes about 0.8 seconds on the
bundled datasets.

Tournament selection (`--tournament N`) is available but was not better here:
with constant tuning it lowered the recovery rate.

## Results on the bundled datasets

A run counts as a recovery when the reported formula's RMSE on the full
dataset is within 5% of the true formula's RMSE (the noise level listed in
[examples/data](../examples/data/README.md)). The defaults were chosen using
seeds 1–10; the table reports seeds **11–20**, which played no part in that
choice:

| Dataset | Before (no tuning, 1 run) | Tuning only | Default (tuning + 4 restarts) |
|---|---|---|---|
| `linear_noisy` | 10 / 10 | 10 / 10 | 10 / 10 |
| `quadratic` | 9 / 10 | 10 / 10 | 10 / 10 |
| `sine_trend` | 3 / 10 | 10 / 10 | 10 / 10 |
| `exponential_decay` | 2 / 10 | 6 / 10 | 10 / 10 |
| `pendulum` | 10 / 10 | 10 / 10 | 10 / 10 |
| `rational` | 1 / 10 | 6 / 10 | 7 / 10 |
| `two_input` | 1 / 10 | 2 / 10 | 2 / 10 |
| **Total** | **36 / 70** | **54 / 70** | **59 / 70** |

The "before" and "tuning only" columns use a single run of 60,000
evaluations, the same total budget as the default's 4 × 15,000.

Typical recovered formulas:

- `counts ≈ 500·exp(−0.3015·t)` (true `500 e^(−0.3t)`)
- `y ≈ 0.9956·x / (1 + x²)` (true `x / (1 + x²)`)
- `period_s ≈ 1.997·√length_m` (true `2.006 √L`)

`linear_outliers` is left out of the count: squared error is pulled by the
outliers, so the best-scoring formula is not the true line.

## Known limitations and next steps

- **Two-input interactions.** `two_input` (`x1·x2 + sin(x1)`) is still found
  on only about 2–7 seeds in 10. Most misses stop at `x1·(x2 + c)`, a good
  approximation that the search rarely leaves. More restarts help (6/10 with
  4 × 60,000 evaluations on seeds 1–10); island populations with migration
  would be the next step.
- **Outliers.** The loss is squared error, so a few gross outliers pull the
  fit. A robust loss (absolute or Huber error) would be an option to add.
- **Cost.** Constant tuning makes each evaluation several times more
  expensive. `--tuning 0` restores the old speed; caching fitness for
  repeated formulas would recover some of the cost.
- **Many inputs.** The encoding already handles any number of input columns.
  Cost grows with rows × formula size per evaluation, and the search space
  grows with the alphabet, so higher-dimensional data will need cheaper
  evaluation and feature-selection-aware reporting.
