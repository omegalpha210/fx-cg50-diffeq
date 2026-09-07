#!/usr/bin/env bash
# Reconstruct the pinned native macOS SDK. No sudo or shell rc edits.
set -euo pipefail
cd "$(dirname "$0")/.."
DIFFEQ_PROJECT_ROOT="$PWD"
mkdir -p .local/src .local/build docs/build-logs "$HOME/.local"
if [[ -L "$HOME/.local/diffeq-sdk" ]]; then
  [[ "$(readlink "$HOME/.local/diffeq-sdk")" == "$PWD/.local" ]] || {
    echo 'Existing diffeq-sdk alias points to another project; refusing to replace it.' >&2; exit 1;
  }
elif [[ -e "$HOME/.local/diffeq-sdk" ]]; then
  echo 'Existing diffeq-sdk path is not our symlink; refusing to replace it.' >&2; exit 1
else
  ln -s "$PWD/.local" "$HOME/.local/diffeq-sdk"
fi
missing=()
for dep in cmake pkgconf libpng libusb ncurses texinfo gmp mpfr libmpc bash gnu-getopt; do
  brew list --versions "$dep" >/dev/null 2>&1 || missing+=("$dep")
done
if (( ${#missing[@]} )); then
  HOMEBREW_NO_AUTO_UPDATE=1 HOMEBREW_NO_INSTALL_CLEANUP=1 brew install "${missing[@]}"
fi
[[ -d .local/venv ]] || python3 -m venv .local/venv
.local/venv/bin/pip install -r tools/requirements.txt
source tools/env.sh
python3 - <<'PY'
import json, subprocess
from pathlib import Path
for name, spec in json.loads(Path('tools/toolchain-lock.json').read_text()).items():
    dst=Path('.local/src')/name
    if not dst.exists():
        subprocess.run(['git','clone',spec['url'],str(dst)],check=True)
        subprocess.run(['git','-C',str(dst),'checkout',spec['commit']],check=True)
    head=subprocess.check_output(['git','-C',str(dst),'rev-parse','HEAD'],text=True).strip()
    if head != spec['commit']:
        raise SystemExit(f'{name}: version differs from lock; refusing automatic checkout')
for name, patch in [('fxsdk','fxsdk-macos-string.patch'),('sh-elf-binutils','binutils-macos-zlib.patch')]:
    p=str(Path('tools/patches',patch).resolve()); dst=str(Path('.local/src',name))
    if subprocess.run(['git','-C',dst,'apply','--reverse','--check',p],capture_output=True).returncode:
        subprocess.run(['git','-C',dst,'apply',p],check=True)
PY
cmake -S "$DIFFEQ_SDK_ROOT/src/fxsdk" -B "$DIFFEQ_SDK_ROOT/build/fxsdk" \
  -DCMAKE_INSTALL_PREFIX="$DIFFEQ_SDK_ROOT/prefix" \
  -DFXLINK_DISABLE_UDISKS2=1 -DFXLINK_DISABLE_SDL2=1
cmake --build "$DIFFEQ_SDK_ROOT/build/fxsdk" -j8
cmake --install "$DIFFEQ_SDK_ROOT/build/fxsdk"
for part in binutils gcc; do
  if [[ "$part" == binutils ]]; then
    archive=binutils-2.42.tar.xz; url="https://ftp.gnu.org/gnu/binutils/$archive"
  else
    archive=gcc-14.1.0.tar.xz; url="https://ftp.gnu.org/gnu/gcc/gcc-14.1.0/$archive"
  fi
  file="$DIFFEQ_SDK_ROOT/src/sh-elf-$part/$archive"
  [[ -f "$file" ]] || curl -fL --retry 3 "$url" -o "$file"
done
python3 - <<'PYHASH'
import hashlib
from pathlib import Path
for line in Path('tools/downloads.sha256').read_text().splitlines():
    expected, name = line.split()
    part = 'binutils' if name.startswith('binutils') else 'gcc'
    actual = hashlib.sha256((Path('.local/src')/f'sh-elf-{part}'/name).read_bytes()).hexdigest()
    if actual != expected:
        raise SystemExit(f'{name}: downloaded archive checksum mismatch')
PYHASH
./tools/build-toolchain.sh
cd examples/hello
fxsdk build-cg -j8
cd ../..
python3 tools/verify_g3a.py examples/hello/DIFFEQ-hello.g3a
