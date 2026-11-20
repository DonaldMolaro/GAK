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
    # Two inputs: y = x1*x2 + sin(x1), noise sigma 0.9
    rows = []
    for _ in range(120):
        x1 = rng.uniform(-3.0, 3.0)
        x2 = rng.uniform(-3.0, 3.0)
        rows.append((x1, x2, x1 * x2 + math.sin(x1) + noise(rng, 0.9)))
    return rows


def gravitation(rng):
    # Newton's law of gravitation with G = 1: F = m1*m2 / r^2, 8% relative
    # noise. Three inputs, two multiplied and one an inverse square.
    rows = []
    for _ in range(100):
        m1 = rng.uniform(1.0, 10.0)
        m2 = rng.uniform(1.0, 10.0)
        r = rng.uniform(1.0, 5.0)
        rows.append((m1, m2, r, m1 * m2 / (r * r) * (1.0 + noise(rng, 0.08))))
    return rows


def ideal_gas(rng):
    # Ideal gas law in litres, bar and kelvin: P = n R T / V with
    # R = 0.08314 L bar / (mol K), 5% relative noise.
    rows = []
    for _ in range(100):
        n = rng.uniform(0.5, 5.0)
        t = rng.uniform(250.0, 400.0)
        v = rng.uniform(10.0, 50.0)
        rows.append((n, t, v, 0.08314 * n * t / v * (1.0 + noise(rng, 0.05))))
    return rows


def mechanical_energy(rng):
    # Kinetic plus potential energy: E = m v^2 / 2 + m g h with g = 9.81,
    # 5% relative noise. A sum of two products sharing the mass.
    rows = []
    for _ in range(100):
        m = rng.uniform(1.0, 10.0)
        v = rng.uniform(0.0, 10.0)
        h = rng.uniform(0.0, 20.0)
        rows.append((m, v, h, (0.5 * m * v * v + m * 9.81 * h) * (1.0 + noise(rng, 0.05))))
    return rows


def five_inputs(rng):
    # Five inputs, two of them irrelevant: y = 2*x1*x3 - x4^2, noise sigma
    # 0.95 (about 30% of the signal). x2 and x5 do not affect y.
    rows = []
    for _ in range(150):
        x = [rng.uniform(-2.0, 2.0) for _ in range(5)]
        rows.append(tuple(x) + (2.0 * x[0] * x[2] - x[3] * x[3] + noise(rng, 0.95),))
    return rows


def friedman1(rng):
    # The Friedman #1 benchmark (Friedman 1991): ten inputs uniform on
    # [0, 1], only the first five used:
    # y = 10 sin(pi x1 x2) + 20 (x3 - 0.5)^2 + 10 x4 + 5 x5, noise sigma 1.
    rows = []
    for _ in range(250):
        x = [rng.uniform(0.0, 1.0) for _ in range(10)]
        y = (10.0 * math.sin(math.pi * x[0] * x[1]) + 20.0 * (x[2] - 0.5) ** 2
             + 10.0 * x[3] + 5.0 * x[4])
        rows.append(tuple(x) + (y + noise(rng, 1.0),))
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
    ("gravitation.csv", ["m1", "m2", "r", "F"], gravitation, 109),
    ("ideal_gas.csv", ["n_mol", "T_K", "V_L", "P_bar"], ideal_gas, 110),
    ("mechanical_energy.csv", ["m_kg", "v_mps", "h_m", "E_J"], mechanical_energy, 111),
    ("five_inputs.csv", ["x1", "x2", "x3", "x4", "x5", "y"], five_inputs, 112),
    ("friedman1.csv", [f"x{i}" for i in range(1, 11)] + ["y"], friedman1, 113),
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
