#!/usr/bin/env python3
"""Generate synthetic CSV datasets for testing symbolic regression.

Each dataset comes from a known formula plus measurement-style noise, so a
regression run can be checked against the ground truth listed in README.md.
Only the Python standard library is used, and every dataset has its own fixed
seed, so re-running this script reproduces the files byte for byte.

Noise is set to roughly 30% of the standard deviation of each true signal,
so even the true formula only reaches R^2 ~= 0.91: realistic noisy
measurements rather than near-perfect curves.

Usage (from the repository root):
    python3 examples/data/generate_test_data.py
    python3 examples/data/generate_test_data.py --noise-scale 0.5 --out /tmp/data

--noise-scale multiplies every noise level (1 = the committed files, 0 = no
noise); --out writes somewhere other than this directory.
"""

import argparse
import csv
import math
import os
import random

OUT_DIR = os.path.dirname(os.path.abspath(__file__))
NOISE_SCALE = 1.0  # set from --noise-scale


def uniform_xs(rng, count, low, high):
    """Unevenly spaced sample points, sorted, as real measurements often are."""
    return sorted(rng.uniform(low, high) for _ in range(count))


def noise(rng, sigma):
    return rng.gauss(0.0, sigma * NOISE_SCALE)


def write_csv(name, header, rows, digits=4):
    path = os.path.join(OUT_DIR, name)
    with open(path, "w", newline="") as handle:
        writer = csv.writer(handle, lineterminator="\n")
        writer.writerow(header)
        for row in rows:
            writer.writerow([f"{value:.{digits}f}" for value in row])
    print(f"wrote {name} ({len(rows)} rows)")


def linear(rng):
    # y = 3x - 2, additive noise sigma 2.5
    xs = uniform_xs(rng, 60, -5.0, 5.0)
    return [(x, 3.0 * x - 2.0 + noise(rng, 2.5)) for x in xs]


def quadratic(rng):
    # y = 0.5x^2 - x + 1, additive noise sigma 0.9
    xs = uniform_xs(rng, 60, -4.0, 6.0)
    return [(x, 0.5 * x * x - x + 1.0 + noise(rng, 0.9)) for x in xs]


def sine_trend(rng):
    # y = 2 sin(x) + 0.5x, additive noise sigma 0.5
    xs = uniform_xs(rng, 80, 0.0, 4.0 * math.pi)
    return [(x, 2.0 * math.sin(x) + 0.5 * x + noise(rng, 0.5)) for x in xs]


def exponential_decay(rng):
    # Radioactive-style decay: counts = 500 e^(-0.3 t), Poisson-like noise
    # (sigma = 4 sqrt(counts): larger where the counts are larger).
    ts = uniform_xs(rng, 50, 0.0, 15.0)
    rows = []
    for t in ts:
        mean = 500.0 * math.exp(-0.3 * t)
        rows.append((t, max(0.0, mean + noise(rng, 4.0 * math.sqrt(mean)))))
    return rows


def pendulum(rng):
    # Pendulum period: T = 2*pi*sqrt(L / 9.81) ~= 2.006 sqrt(L),
    # 7% relative timing noise.
    lengths = uniform_xs(rng, 40, 0.1, 3.0)
    rows = []
    for length in lengths:
        period = 2.0 * math.pi * math.sqrt(length / 9.81)
        rows.append((length, period * (1.0 + noise(rng, 0.07))))
    return rows


def rational(rng):
    # y = x / (1 + x^2), noise sigma 0.1
    xs = uniform_xs(rng, 70, -6.0, 6.0)
    return [(x, x / (1.0 + x * x) + noise(rng, 0.1)) for x in xs]


def linear_outliers(rng):
    # y = 1.5x + 4 with sigma 1.2 noise, plus 5 gross outliers
    xs = uniform_xs(rng, 50, 0.0, 10.0)
    rows = [(x, 1.5 * x + 4.0 + noise(rng, 1.2)) for x in xs]
    for index in rng.sample(range(len(rows)), 5):
        x, y = rows[index]
        rows[index] = (x, y + rng.choice([-1.0, 1.0]) * rng.uniform(8.0, 15.0))
    return rows


def two_input(rng):
    # For later multi-input work: y = x1*x2 + sin(x1), noise sigma 0.9
    rows = []
    for _ in range(120):
        x1 = rng.uniform(-3.0, 3.0)
        x2 = rng.uniform(-3.0, 3.0)
        rows.append((x1, x2, x1 * x2 + math.sin(x1) + noise(rng, 0.9)))
    return rows


DATASETS = [
    ("linear_noisy.csv", ["x", "y"], linear, 101),
    ("quadratic.csv", ["x", "y"], quadratic, 102),
    ("sine_trend.csv", ["x", "y"], sine_trend, 103),
    ("exponential_decay.csv", ["time_s", "counts"], exponential_decay, 104),
    ("pendulum.csv", ["length_m", "period_s"], pendulum, 105),
    ("rational.csv", ["x", "y"], rational, 106),
    ("linear_outliers.csv", ["x", "y"], linear_outliers, 107),
    ("two_input.csv", ["x1", "x2", "y"], two_input, 108),
]


def main():
    global NOISE_SCALE, OUT_DIR
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--noise-scale", type=float, default=1.0,
                        help="multiply every noise level (default 1)")
    parser.add_argument("--out", default=OUT_DIR, help="output directory (default: this directory)")
    args = parser.parse_args()
    if args.noise_scale < 0:
        parser.error("--noise-scale must be non-negative")
    NOISE_SCALE = args.noise_scale
    OUT_DIR = args.out
    os.makedirs(OUT_DIR, exist_ok=True)
    for name, header, generator, seed in DATASETS:
        write_csv(name, header, generator(random.Random(seed)))


if __name__ == "__main__":
    main()
