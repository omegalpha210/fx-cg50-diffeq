#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
source tools/env.sh
if [[ "${1:-}" == --hello || ! -f CMakeLists.txt ]]; then
  cd examples/hello
  fxsdk build-cg -j8
  python3 ../../tools/verify_g3a.py DIFFEQ-hello.g3a
else
  fxsdk build-cg -j8
  python3 tools/verify_g3a.py dist/DIFFEQ.g3a
fi
