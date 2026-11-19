#!/bin/sh
# Black-box tests for the GA command-line runner: exit codes, error
# messages, every mode, and the reports mode R writes. Run from anywhere:
#   examples/check_cli.sh
set -u

cd "$(dirname "$0")"
GA=./GA
WORK="$(mktemp -d "${TMPDIR:-/tmp}/gak-cli.XXXXXX")"
trap 'rm -rf "$WORK"' EXIT INT TERM

failures=0
checks=0

fail() {
  echo "FAIL: $1" >&2
  failures=$((failures + 1))
}

# expect STATUS NEEDLE DESCRIPTION -- COMMAND...
# Runs COMMAND, then checks its exit status and that the combined
# stdout+stderr contains NEEDLE (an empty needle skips the text check).
expect() {
  status="$1"
  needle="$2"
  description="$3"
  shift 4
  checks=$((checks + 1))
  "$@" > "$WORK/out.txt" 2>&1
  actual=$?
  if [ "$actual" -ne "$status" ]; then
    fail "$description: exit status $actual, expected $status"
    return
  fi
  if [ -n "$needle" ] && ! grep -qF -- "$needle" "$WORK/out.txt"; then
    fail "$description: output does not contain '$needle'"
  fi
}

# --- usage and help -----------------------------------------------------
expect 1 "Usage:" "no arguments prints usage and fails" -- $GA
expect 0 "Usage:" "--help prints usage and succeeds" -- $GA --help
expect 0 "--restarts N" "help lists the regression options" -- $GA D --help
expect 1 "Usage:" "an unknown mode fails" -- $GA Z --seed 1
expect 1 "Usage:" "an unknown option fails" -- $GA D --bogus 1
expect 1 "Usage:" "an option without a value fails" -- $GA D --seed

# --- strict value parsing -----------------------------------------------
expect 1 "Invalid value for --population: '12abc'" "trailing junk is rejected" -- $GA D --population 12abc
expect 1 "Invalid value for --population: ''" "an empty number is rejected" -- $GA D --population ""
expect 1 "Invalid value for --population: '0'" "population must be positive" -- $GA D --population 0
expect 1 "Invalid value for --trials: '-5'" "trials must be positive" -- $GA D --trials -5
expect 1 "Invalid value for --seed: '-1'" "a negative seed is rejected" -- $GA D --seed -1
expect 1 "Invalid value for --seed: '99999999999'" "an out-of-range seed is rejected" -- $GA D --seed 99999999999
expect 1 "Invalid value for --mutation: '2'" "mutation must be a probability" -- $GA D --mutation 2
expect 1 "Invalid value for --crossover: 'abc'" "crossover must be numeric" -- $GA D --crossover abc
expect 1 "Invalid value for --grid: '0'" "grid must be positive" -- $GA T --grid 0
expect 1 "Invalid value for --selection: 'best'" "an unknown selection is rejected" -- $GA D --selection best
expect 1 "Invalid value for --tournament: '0'" "tournament size must be positive" -- $GA D --tournament 0
expect 1 "Invalid value for --holdout: '1'" "holdout must be below 1" -- $GA R --data data/quadratic.csv --holdout 1
expect 1 "Invalid value for --restarts: '0'" "restarts must be positive" -- $GA R --data data/quadratic.csv --restarts 0
expect 1 "Invalid value for --tuning: '-1'" "tuning must be non-negative" -- $GA R --data data/quadratic.csv --tuning -1

# --- configuration errors are reported, not crashes ---------------------
expect 1 "GA: Deletion mode replaces no individuals" "a population too small for its deletion mode" -- $GA D --population 1
expect 1 "GA: TravelingSalesman grid is too small" "a TSP grid too small for its cities" -- $GA T --grid 3

# --- every mode runs ------------------------------------------------------
for mode in D 6 A S T Q K L G M U C; do
  expect 0 "GA parameters:" "mode $mode runs" -- $GA "$mode" --seed 1 --population 24 --trials 120
done
expect 0 "Operation             :: " "--show-settings prints the settings report" -- \
  $GA K --seed 1 --population 12 --trials 24 --show-settings
expect 0 "tournament size   : 3" "--tournament switches selection and prints its size" -- \
  $GA K --seed 1 --population 12 --trials 24 --tournament 3
expect 0 "selection         : random" "--selection random is applied" -- \
  $GA K --seed 1 --population 12 --trials 24 --selection random
expect 0 "mutation          : 0.123" "--mutation is applied" -- \
  $GA K --seed 1 --population 12 --trials 24 --mutation 0.123
expect 0 "crossover         : 0.456" "--crossover is applied" -- \
  $GA K --seed 1 --population 12 --trials 24 --crossover 0.456
expect 0 "population        : 12" "--population is applied" -- \
  $GA K --seed 1 --population 12 --trials 24
expect 0 "trials            : 24" "--trials is applied" -- \
  $GA K --seed 1 --population 12 --trials 24

# --- mode R ------------------------------------------------------------------
QUICK="--seed 1 --population 60 --trials 600"
expect 1 "Mode R needs --data" "mode R without data fails" -- $GA R
expect 1 "GA: data/missing.csv: cannot open file" "a missing CSV is reported" -- $GA R --data data/missing.csv
expect 1 "target column 'nope' is not in the header" "an unknown target is reported" -- \
  $GA R --data data/pendulum.csv --target nope
printf 'x,y\n1,2\n3,oops\n4,5\n' > "$WORK/bad.csv"
expect 1 "bad.csv:3: column 'y' has a non-numeric value 'oops'" "a malformed CSV row is reported with its line" -- \
  $GA R --data "$WORK/bad.csv"
expect 0 "period_s ~= " "mode R prints a fitted formula" -- $GA R --data data/pendulum.csv $QUICK
expect 0 "Restart 2 (seed " "mode R reports each restart" -- $GA R --data data/pendulum.csv $QUICK --restarts 2
expect 0 "Best restart seed:" "mode R reports the chosen restart" -- $GA R --data data/pendulum.csv $QUICK --restarts 2
expect 0 "restarts          : 1" "--restarts is applied" -- $GA R --data data/pendulum.csv $QUICK --restarts 1
expect 0 "test rows         : 0" "--holdout 0 keeps every row for training" -- \
  $GA R --data data/pendulum.csv $QUICK --restarts 1 --holdout 0
expect 0 "inputs            : x1 x2" "multi-input CSVs are supported" -- \
  $GA R --data data/two_input.csv $QUICK --restarts 1
expect 0 "target            : length_m" "--target selects the output column" -- \
  $GA R --data data/pendulum.csv --target length_m $QUICK --restarts 1 --tuning 0

expect 0 "" "mode R writes reports" -- \
  $GA R --data data/quadratic.csv $QUICK --restarts 1 --report-json "$WORK/r.json" --report-csv "$WORK/r.csv"
checks=$((checks + 1))
grep -qF '"type": "regression"' "$WORK/r.json" || fail "the JSON report should contain a regression visualization"
checks=$((checks + 1))
head -n 1 "$WORK/r.csv" | grep -qx "generation,evaluations,best_fitness,worst_fitness" ||
  fail "the CSV report should start with its header"

expect 1 "Failed to open JSON report path" "an unwritable report path is reported" -- \
  $GA K --seed 1 --population 12 --trials 24 --report-json "$WORK/no/such/dir/r.json"
expect 1 "Failed to open CSV report path" "an unwritable CSV path is reported" -- \
  $GA K --seed 1 --population 12 --trials 24 --report-csv "$WORK/no/such/dir/r.csv"

# Fixed seeds reproduce mode R exactly, restarts included.
$GA R --data data/sine_trend.csv $QUICK --restarts 2 > "$WORK/first.txt" 2>&1
$GA R --data data/sine_trend.csv $QUICK --restarts 2 > "$WORK/second.txt" 2>&1
checks=$((checks + 1))
cmp -s "$WORK/first.txt" "$WORK/second.txt" || fail "a fixed seed should reproduce a mode R run exactly"

if [ "$failures" -ne 0 ]; then
  echo "$failures of $checks CLI checks failed" >&2
  exit 1
fi
echo "All $checks CLI checks passed"
