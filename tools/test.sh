#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cmake -S tests -B build-host -DCMAKE_C_COMPILER=clang
cmake --build build-host -j8
ctest --test-dir build-host --output-on-failure
