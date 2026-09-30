#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
source tools/env.sh

hello=0
clean=0
jobs="${DIFFEQ_BUILD_JOBS:-8}"
while (( $# )); do
  case "$1" in
    --hello) hello=1 ;;
    --clean) clean=1 ;;
    -j|--jobs)
      [[ $# -ge 2 ]] || { echo "$1 requires a positive job count" >&2; exit 2; }
      jobs="$2"
      shift
      ;;
    --help)
      echo 'Usage: tools/build.sh [--hello] [--clean] [-j JOBS]'
      exit 0
      ;;
    *) echo "Unknown option: $1" >&2; exit 2 ;;
  esac
  shift
done
[[ "$jobs" =~ ^[1-9][0-9]*$ ]] || { echo 'Job count must be positive' >&2; exit 2; }

# fxSDK 2.11 configures build-cg unconditionally. Use its installed CMake
# toolchain and identical module arguments to keep this project's fixed layout.
sdk_cmake="${DIFFEQ_FXSDK_CMAKE_DIR:-$DIFFEQ_SDK_ROOT/prefix/lib/cmake/fxsdk}"
if [[ ! -f "$sdk_cmake/FXCG50.cmake" && -z "${DIFFEQ_FXSDK_CMAKE_DIR:-}" ]]; then
  fxsdk_executable="$(command -v fxsdk)"
  sdk_cmake="$(dirname "$(dirname "$fxsdk_executable")")/lib/cmake/fxsdk"
fi
[[ -f "$sdk_cmake/FXCG50.cmake" ]] || {
  echo 'Installed FXCG50.cmake not found; set DIFFEQ_FXSDK_CMAKE_DIR.' >&2
  exit 1
}

source_dir=.
build_dir=build/target
package=dist/DIFFEQ.g3a
if (( hello )); then
  source_dir=examples/hello
  build_dir=build/hello-target
  package=examples/hello/DIFFEQ-hello.g3a
fi
cmake -S "$source_dir" -B "$build_dir" \
  -DCMAKE_MODULE_PATH="$sdk_cmake" \
  -DCMAKE_TOOLCHAIN_FILE="$sdk_cmake/FXCG50.cmake" \
  -DFXSDK_CMAKE_MODULE_PATH="$sdk_cmake"
build_options=(--parallel "$jobs")
if (( clean )); then
  build_options+=(--clean-first)
fi
cmake --build "$build_dir" "${build_options[@]}"
python3 tools/verify_g3a.py "$package"
