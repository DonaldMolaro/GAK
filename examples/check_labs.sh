#!/bin/sh
set -eu

MODE="${1:-all}"

run_and_check() {
  label="$1"
  shift
  echo "[lab] $label"
  output="$("$@")"
  printf '%s\n' "$output"
}

check_knapsack() {
  output="$(./GA K --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output"
  printf '%s' "$output" | grep -q "Problem input:"
  printf '%s' "$output" | grep -q "GA parameters:"
  printf '%s' "$output" | grep -q "Items:"
}

check_symbolic() {
  output_a="$(./GA A --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_a"
  printf '%s' "$output_a" | grep -q "Problem input:"
  printf '%s' "$output_a" | grep -q "sorted 26-letter alphabet"

  output_s="$(./GA S --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_s"
  printf '%s' "$output_s" | grep -q "Target word: egghead\|target word"
}

check_constraints() {
  output_q="$(./GA Q --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_q"
  printf '%s' "$output_q" | grep -q "8x8"

  output_l="$(./GA L --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_l"
  printf '%s' "$output_l" | grep -q "Latin square"

  output_g="$(./GA G --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_g"
  printf '%s' "$output_g" | grep -q "graph coloring\|3-color graph coloring"
}

check_scheduling() {
  output_m="$(./GA M --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_m"
  printf '%s' "$output_m" | grep -q "course timetabling\|Timetable:"
}

check_numeric() {
  output_d="$(./GA D --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_d"
  printf '%s' "$output_d" | grep -q "Problem input:"
  printf '%s' "$output_d" | grep -q "GA parameters:"
  printf '%s' "$output_d" | grep -q "chromosome length"

  output_f6="$(./GA 6 --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_f6"
  printf '%s' "$output_f6" | grep -q "Problem input:"
  printf '%s' "$output_f6" | grep -q "GA parameters:"
}

check_sudoku() {
  output_u="$(./GA U --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_u"
  printf '%s' "$output_u" | grep -q "Sudoku givens:"

  output_c="$(./GA C --seed 1 --population 24 --trials 120)"
  printf '%s\n' "$output_c"
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
