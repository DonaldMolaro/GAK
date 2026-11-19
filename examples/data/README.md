# Symbolic Regression Test Data

Synthetic "measured" datasets for developing and checking symbolic regression.
Each file is generated from a known formula plus noise, so a run can be judged
against the ground truth below. The points are unevenly spaced, as real
measurements usually are.

Regenerate the files (they come out byte-for-byte identical) with:

```sh
python3 examples/data/generate_test_data.py
```

## Running them

```sh
cd examples
./GA R --data data/quadratic.csv --seed 1
./GA R --data data/pendulum.csv --target period_s --seed 1
```

See [docs/SYMBOLIC_REGRESSION.md](../../docs/SYMBOLIC_REGRESSION.md) for how the
search works and how each dataset fares.

## Format

- Plain CSV with a header row; every value is numeric with 4 decimal places.
- The **last column is the output** (target); every other column is an input.
- Rows are sorted by the input for single-input files.

## Datasets

Listed roughly from easiest to hardest. "Noise" is the injected measurement
error; a good fit should have a residual RMS close to it, not lower (lower
means it is fitting the noise).

| File | Columns | Ground truth | Noise | Rows | What it tests |
|---|---|---|---|---|---|
| `linear_noisy.csv` | `x`, `y` | y = 3x − 2 | Gaussian, σ = 0.5 | 60 | Baseline: the GA should find a straight line |
| `quadratic.csv` | `x`, `y` | y = 0.5x² − x + 1 | Gaussian, σ = 0.3 | 60 | Polynomial structure |
| `sine_trend.csv` | `x`, `y` | y = 2 sin(x) + 0.5x | Gaussian, σ = 0.2 | 80 | Mixing a periodic term with a trend |
| `exponential_decay.csv` | `time_s`, `counts` | counts = 500 e^(−0.3 t) | Poisson-like, σ = √counts | 50 | Exponentials; noise that shrinks as the value falls |
| `pendulum.csv` | `length_m`, `period_s` | T = 2π√(L / 9.81) ≈ 2.006 √L | 1% relative | 40 | Power law with physical column names |
| `rational.csv` | `x`, `y` | y = x / (1 + x²) | Gaussian, σ = 0.02 | 70 | Division; a shape with no simple polynomial fit |
| `linear_outliers.csv` | `x`, `y` | y = 1.5x + 4 | Gaussian, σ = 0.4, plus 5 outliers of ±8–15 | 50 | Robustness: squared error gets pulled by the outliers |
| `two_input.csv` | `x1`, `x2`, `y` | y = x1·x2 + sin(x1) | Gaussian, σ = 0.1 | 120 | Multiple inputs (for later; random 2-D scatter) |

## Checking a result

The measured residual RMS against the true formula, which is the best a
correct fit can expect on these exact files:

| File | Residual RMS of the true formula |
|---|---|
| `linear_noisy.csv` | 0.511 |
| `quadratic.csv` | 0.299 |
| `sine_trend.csv` | 0.209 |
| `exponential_decay.csv` | 7.947 |
| `pendulum.csv` | 0.027 |
| `rational.csv` | 0.017 |
| `linear_outliers.csv` | 3.319 (median absolute residual 0.393) |
| `two_input.csv` | 0.102 |

A discovered formula may look different and still be right. For example
`0.5x² − x + 1` and `0.5(x − 1)² + 0.5` are the same curve, so compare errors
and curves rather than the printed formula.
