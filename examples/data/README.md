# Symbolic Regression Test Data

Synthetic "measured" datasets for developing and checking symbolic regression.
Each file is generated from a known formula plus noise, so a run can be judged
against the ground truth below. The points are unevenly spaced, as real
measurements usually are.

The noise is deliberately heavy: about **30% of the spread of each true
signal**, so even the true formula only reaches R² ≈ 0.91–0.93. That is close
to real noisy measurements and makes overfitting visible: a formula that
scores better than the true one on the training rows is fitting noise.

Regenerate the files (they come out byte-for-byte identical) with:

```sh
python3 examples/data/generate_test_data.py
```

`--noise-scale K` multiplies every noise level (`0` gives noise-free data,
`0.5` half the noise) and `--out DIR` writes the files somewhere else, so you
can compare noise levels without touching the committed files:

```sh
python3 examples/data/generate_test_data.py --noise-scale 0.3 --out /tmp/gak-low-noise
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
| `linear_noisy.csv` | `x`, `y` | y = 3x − 2 | Gaussian, σ = 2.5 | 60 | Baseline; overfitting shows up as extra wiggles |
| `quadratic.csv` | `x`, `y` | y = 0.5x² − x + 1 | Gaussian, σ = 0.9 | 60 | Polynomial structure |
| `sine_trend.csv` | `x`, `y` | y = 2 sin(x) + 0.5x | Gaussian, σ = 0.5 | 80 | Mixing a periodic term with a trend |
| `exponential_decay.csv` | `time_s`, `counts` | counts = 500 e^(−0.3 t) | Poisson-like, σ = 4√counts, clipped at 0 | 50 | Exponentials; noise that shrinks as the value falls |
| `pendulum.csv` | `length_m`, `period_s` | T = 2π√(L / 9.81) ≈ 2.006 √L | 7% relative | 40 | Power law with physical column names |
| `rational.csv` | `x`, `y` | y = x / (1 + x²) | Gaussian, σ = 0.1 | 70 | Division; a shape with no simple polynomial fit |
| `linear_outliers.csv` | `x`, `y` | y = 1.5x + 4 | Gaussian, σ = 1.2, plus 5 outliers of ±8–15 | 50 | Robustness: squared error gets pulled by the outliers |
| `two_input.csv` | `x1`, `x2`, `y` | y = x1·x2 + sin(x1) | Gaussian, σ = 0.9 | 120 | Multiple inputs (random 2-D scatter) |

## Checking a result

How well the true formula itself fits each file. A discovered formula should
come close to these numbers, not beat them: beating them on the training
rows means it is fitting the noise.

| File | Residual RMS of the true formula | R² of the true formula | Noise / signal spread |
|---|---|---|---|
| `linear_noisy.csv` | 2.554 | 0.914 | 31% |
| `quadratic.csv` | 0.896 | 0.906 | 31% |
| `sine_trend.csv` | 0.522 | 0.923 | 30% |
| `exponential_decay.csv` | 31.10 | 0.934 | 26% |
| `pendulum.csv` | 0.191 | 0.917 | 30% |
| `rational.csv` | 0.084 | 0.933 | 26% |
| `linear_outliers.csv` | 3.568 (median absolute residual 1.179) | 0.637 | — |
| `two_input.csv` | 0.920 | 0.931 | 29% |

With this much noise, comparing errors is only part of the check. Because
the true formula is known, the sharper test is how far the fitted curve is
from the noise-free truth.

A discovered formula may look different and still be right. For example
`0.5x² − x + 1` and `0.5(x − 1)² + 0.5` are the same curve, so compare errors
and curves rather than the printed formula.
