#!/usr/bin/env bash
set -euo pipefail

formatter=""
for candidate in clang-format clang-format-18 clang-format-17 clang-format-16; do
  if command -v "$candidate" >/dev/null 2>&1; then
    formatter="$candidate"
    break
  fi
done

if [[ -z "$formatter" ]]; then
  echo "clang-format is required." >&2
  echo "Install it with: sudo apt-get install clang-format" >&2
  exit 2
fi

mapfile -t files < <(
  find src tests tools -type f \( -name '*.cpp' -o -name '*.h' \) -print 2>/dev/null |
    sort
)

if ((${#files[@]} == 0)); then
  echo "No C++ files found."
  exit 0
fi

"$formatter" --dry-run --Werror "${files[@]}"
