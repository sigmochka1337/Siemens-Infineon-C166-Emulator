#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
status=0

for suite in executor memory; do
  source_dir="$root_dir/src/$suite/tests"
  build_dir="$root_dir/build-tests/$suite"
  printf '\n=== %s tests ===\n' "$suite"

  if ! cmake -S "$source_dir" -B "$build_dir" ||
     ! cmake --build "$build_dir" --config Debug ||
     ! ctest --test-dir "$build_dir" -C Debug --output-on-failure --no-tests=error; then
    status=1
  fi
done

exit "$status"
