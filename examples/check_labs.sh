#!/bin/sh
set -eu

# Run from the script's own directory so ./GA resolves wherever it is invoked.
cd "$(dirname "$0")"

MODE="${1:-all}"

# Every run must show the GA actually executed: a parameter block, at least
# one generation progress line, and a final candidate line ending in a
# fitness value.
check_ran() {
  printf '%s' "$1" | grep -q "GA parameters:"
  printf '%s' "$1" | grep -Eq "^Generation [0-9]+ Number of Evaluations [0-9]+"
  printf '%s\n' "$1" | tail -n 1 | grep -Eq "[0-9]+\.[0-9]+$"
}

run_and_check() {
  label="$1"
  shift
  echo "[lab] $label"
  output="$("$@")"
  printf '%s\n' "$output"
  check_ran "$output"
}

check_knapsack() {
  output="$(./GA K --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output"
  check_ran "$output"
  printf '%s' "$output" | grep -q "Problem input:"
  printf '%s' "$output" | grep -q "GA parameters:"
  printf '%s' "$output" | grep -q "Items:"
}

check_symbolic() {
  output_a="$(./GA A --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_a"
  check_ran "$output_a"
  printf '%s' "$output_a" | grep -q "Problem input:"
  printf '%s' "$output_a" | grep -q "sorted 26-letter alphabet"

  output_s="$(./GA S --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_s"
  check_ran "$output_s"
  printf '%s' "$output_s" | grep -q "Target word: egghead\|target word"
}

check_constraints() {
  output_q="$(./GA Q --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_q"
  check_ran "$output_q"
  printf '%s' "$output_q" | grep -q "8x8"

  output_l="$(./GA L --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_l"
  check_ran "$output_l"
  printf '%s' "$output_l" | grep -q "Latin square"

  output_g="$(./GA G --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_g"
  check_ran "$output_g"
  printf '%s' "$output_g" | grep -q "graph coloring\|3-color graph coloring"
}

check_scheduling() {
  output_m="$(./GA M --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_m"
  check_ran "$output_m"
  printf '%s' "$output_m" | grep -q "course timetabling\|Timetable:"
}

check_numeric() {
  output_d="$(./GA D --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_d"
  check_ran "$output_d"
  printf '%s' "$output_d" | grep -q "Problem input:"
  printf '%s' "$output_d" | grep -q "GA parameters:"
  printf '%s' "$output_d" | grep -q "chromosome length"

  output_f6="$(./GA 6 --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_f6"
  check_ran "$output_f6"
  printf '%s' "$output_f6" | grep -q "Problem input:"
  printf '%s' "$output_f6" | grep -q "GA parameters:"
}

check_sudoku() {
  output_u="$(./GA U --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_u"
  check_ran "$output_u"
  printf '%s' "$output_u" | grep -q "Sudoku givens:"

  output_c="$(./GA C --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_c"
  check_ran "$output_c"
  printf '%s' "$output_c" | grep -q "Sudoku givens:"
}

case "$MODE" in
  all)
    check_knapsack
    check_symbolic
    check_numeric
    check_constraints
    check_scheduling
    check_sudoku
    ;;
  knapsack)
    check_knapsack
    ;;
  symbolic)
    check_symbolic
    ;;
  constraints)
    check_constraints
    ;;
  scheduling)
    check_scheduling
    ;;
  numeric)
    check_numeric
    ;;
  sudoku)
    check_sudoku
    ;;
  *)
    echo "usage: ./check_labs.sh [all|knapsack|symbolic|numeric|constraints|scheduling|sudoku]" >&2
    exit 1
    ;;
esac

echo "All lab checks passed"
