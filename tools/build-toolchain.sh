#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
source tools/env.sh
DIFFEQ_LOGS="$PWD/docs/build-logs"
DIFFEQ_SYSROOT="$(fxsdk path sysroot)"
# Build via the whitespace-free alias; all files physically remain in workspace.
if [[ "${1:-}" != --libraries ]]; then
cd "$DIFFEQ_SDK_ROOT/src/sh-elf-binutils"
make -f giteapc.make configure build install PREFIX="$DIFFEQ_SDK_ROOT/prefix" > "$DIFFEQ_LOGS/binutils.log" 2>&1
cd "$DIFFEQ_SDK_ROOT/src/sh-elf-gcc"
if [[ ! -d gcc-14.1.0 ]]; then
  tar -xf gcc-14.1.0.tar.xz
  patch -p0 < patches/gcc-14.1.0-libgcc-use-soft-fp.patch
fi
mkdir -p "$DIFFEQ_SYSROOT/sh3eb-elf/bin"
for tool in as ld ar ranlib; do
  ln -sf "$(command -v sh-elf-$tool)" "$DIFFEQ_SYSROOT/sh3eb-elf/bin/$tool"
done
mkdir -p build
cd build
../gcc-14.1.0/configure \
  --prefix="$DIFFEQ_SYSROOT" --target=sh3eb-elf \
  --with-multilib-list=m3,m4-nofpu --enable-languages=c,c++ \
  --with-system-zlib --without-headers --program-prefix=sh-elf- --enable-libssp --enable-lto \
  --enable-clocale=generic --enable-libstdcxx-allocator --disable-threads \
  --disable-libstdcxx-verbose --enable-cxx-flags=-fno-exceptions \
  --with-gmp="$(brew --prefix gmp)" --with-mpfr="$(brew --prefix mpfr)" \
  --with-mpc="$(brew --prefix libmpc)" > "$DIFFEQ_LOGS/gcc-configure.log" 2>&1
make -j8 all-gcc > "$DIFFEQ_LOGS/gcc-build.log" 2>&1
# Separate libgcc build prevents libgcc_tm.h generation races on macOS.
make -j1 all-target-libgcc >> "$DIFFEQ_LOGS/gcc-build.log" 2>&1
make install-strip-gcc install-strip-target-libgcc > "$DIFFEQ_LOGS/gcc-install.log" 2>&1
for file in "$DIFFEQ_SYSROOT/bin"/*; do
  ln -sf "$file" "$DIFFEQ_SDK_ROOT/prefix/bin/$(basename "$file")"
done
sh-elf-gcc --version
fi
cd "$DIFFEQ_SDK_ROOT/src/OpenLibm"
make -j8 OPENLIBM_HOME="$DIFFEQ_SDK_ROOT/src/OpenLibm" USEGCC=1 TOOLPREFIX=sh-elf- AR=sh-elf-ar CC=sh-elf-gcc \
  libdir="$(fxsdk path lib)" includedir="$(fxsdk path include)" \
  install-static-superh install-headers-superh > "$DIFFEQ_LOGS/openlibm.log" 2>&1
cd "$DIFFEQ_SDK_ROOT/src/fxlibc"
cmake -B build-gint -DFXLIBC_TARGET=gint -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-sh.cmake > "$DIFFEQ_LOGS/fxlibc.log" 2>&1
cmake --build build-gint -j8 >> "$DIFFEQ_LOGS/fxlibc.log" 2>&1
cmake --install build-gint >> "$DIFFEQ_LOGS/fxlibc.log" 2>&1
cd "$DIFFEQ_SDK_ROOT/src/gint"
fxsdk build-cg -j8 > "$DIFFEQ_LOGS/gint.log" 2>&1
fxsdk build-cg install >> "$DIFFEQ_LOGS/gint.log" 2>&1
printf 'Toolchain libraries installed; a minimal .g3a must still be built.\n'
