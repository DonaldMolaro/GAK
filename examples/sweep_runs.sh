#!/bin/sh
set -eu

usage() {
  cat <<'EOF'
Usage:
  ./sweep_runs.sh MODE OUTDIR SEED [SEED ...] [-- GA_OPTIONS...]

Examples:
  ./sweep_runs.sh K /tmp/gak-knapsack 1 2 3
  ./sweep_runs.sh C /tmp/gak-sudoku 1 2 3 -- --population 64 --trials 500

For each seed, the script writes:
  - run_<seed>.txt   human-readable console output
  - run_<seed>.json  structured run report
  - run_<seed>.csv   per-generation CSV report

It also writes:
  - summary.csv      one row per seed with key result metrics
EOF
}

if [ "$#" -lt 3 ]; then
  usage >&2
  exit 1
fi

MODE="$1"
OUTDIR="$2"
shift 2

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
GA_BIN="$SCRIPT_DIR/GA"

EXTRA_ARGS=""
SEEDS=""
FOUND_SEPARATOR=0

while [ "$#" -gt 0 ]; do
  if [ "$1" = "--" ]; then
    FOUND_SEPARATOR=1
    shift
    break
  fi
  SEEDS="${SEEDS} $1"
  shift
done

if [ "$FOUND_SEPARATOR" -eq 1 ]; then
  EXTRA_ARGS="$*"
fi

if [ -z "${SEEDS# }" ]; then
  echo "No seeds provided." >&2
  usage >&2
  exit 1
fi

mkdir -p "$OUTDIR"

summary_path="$OUTDIR/summary.csv"
echo "seed,generations_completed,evaluations,solution_found,stopped_early,best_fitness,worst_fitness,json_report,csv_report,text_report" > "$summary_path"

json_number() {
  key="$1"
  file="$2"
  sed -n "s/^[[:space:]]*\"$key\": \\([^,]*\\),\$/\\1/p" "$file" | head -n 1 | tr -d ' '
}

json_bool() {
  key="$1"
  file="$2"
  sed -n "s/^[[:space:]]*\"$key\": \\(true\\|false\\),\$/\\1/p" "$file" | head -n 1
}

json_array_first() {
  key="$1"
  file="$2"
  sed -n "s/^[[:space:]]*\"$key\": \\[\\([^],]*\\).*/\\1/p" "$file" | head -n 1 | tr -d ' '
}

for seed in $SEEDS; do
  base="$OUTDIR/run_${seed}"
  text_path="${base}.txt"
  json_path="${base}.json"
  csv_path="${base}.csv"

  echo "[sweep] mode=$MODE seed=$seed"

  # shellcheck disable=SC2086
  "$GA_BIN" "$MODE" --seed "$seed" --report-json "$json_path" --report-csv "$csv_path" $EXTRA_ARGS > "$text_path"

  generations_completed="$(json_number generations_completed "$json_path")"
  evaluations="$(json_number evaluations "$json_path")"
  solution_found="$(json_bool solution_found "$json_path")"
  stopped_early="$(json_bool stopped_early "$json_path")"
  best_fitness="$(json_array_first most_fit "$json_path")"
  worst_fitness="$(json_array_first least_fit "$json_path")"

  echo "${seed},${generations_completed},${evaluations},${solution_found},${stopped_early},${best_fitness},${worst_fitness},$(basename "$json_path"),$(basename "$csv_path"),$(basename "$text_path")" >> "$summary_path"
done

echo "Wrote sweep results to $OUTDIR"
