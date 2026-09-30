#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

clean=0
jobs="${DIFFEQ_BUILD_JOBS:-8}"
while (( $# )); do
  case "$1" in
    --clean) clean=1 ;;
    -j|--jobs)
      [[ $# -ge 2 ]] || { echo "$1 requires a positive job count" >&2; exit 2; }
      jobs="$2"
      shift
      ;;
    --help)
      echo 'Usage: tools/test.sh [--clean] [-j JOBS]'
      exit 0
      ;;
    *) echo "Unknown option: $1" >&2; exit 2 ;;
  esac
  shift
done
[[ "$jobs" =~ ^[1-9][0-9]*$ ]] || { echo 'Job count must be positive' >&2; exit 2; }

cmake -S tests -B build/host -DCMAKE_C_COMPILER="${CC:-clang}" \
  -DSANITIZE=ON -DADDRESS_SANITIZE=OFF
build_options=(--parallel "$jobs")
if (( clean )); then
  build_options+=(--clean-first)
fi
cmake --build build/host "${build_options[@]}"
ctest --test-dir build/host --output-on-failure
