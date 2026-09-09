# Building and developing DIFFEQ

The runtime/numerical/UI source and all existing tests match the validated local
milestone `83de313`. This beta updates the existing public snapshot history; private
development branches and manuals are not imported. Public-only changes cover
release metadata, documentation and the host-only README image helper.

## Verified stack

| Component | Tested version |
|---|---|
| Host | macOS arm64, Apple Clang 17 |
| CMake | 4.4.3 (project minimum 3.15) |
| fxSDK / gint | 2.11.0 / 2.11.0 (CMake requires gint ≥2.11) |
| SH GCC / binutils | 14.1.0 / 2.42 |
| fxlibc | 1.5.1 |
| OpenLibm | SH port based on 0.7.0 |
| Optional image tools | Python 3, Pillow |

Exact upstream revisions: [toolchain lock](tools/toolchain-lock.json).
GNU archive checksums: [downloads.sha256](tools/downloads.sha256).
Other operating systems/toolchain versions have not been verified by this release.

## Existing fxSDK installation

Use a shell in which `fxsdk`, `sh-elf-gcc` and the installed CMake modules are available:

```sh
fxsdk build-cg -j8
python3 tools/verify_g3a.py dist/DIFFEQ.g3a
```

The root project links `Gint::Gint`; its installed configuration resolves fxlibc,
OpenLibm and libgcc. fxSDK selects the big-endian SH4 no-FPU target and uses fxgxa
with the project's original icons. Application warnings are errors:
`-Wall -Wextra -Werror -Wframe-larger-than=3072 -Os -g -fstack-usage`.
No fast-math is enabled. ELF, linker map and `.su` files stay in ignored `build-cg/`.

After initial configuration, a clean target compile/link is:

```sh
cmake --build build-cg --clean-first -j8
python3 tools/verify_g3a.py dist/DIFFEQ.g3a
sh-elf-size build-cg/diffeq
```

## Project-local macOS SDK

The existing setup uses a whitespace-free `~/.local/diffeq-sdk` alias pointing at
one workspace's `.local/`. `source tools/env.sh` activates that SDK/venv and the
Homebrew tools in the current shell. `./tools/build.sh` then builds the add-in.
Do not replace an existing alias or reinstall a working toolchain to build another
source checkout; it can use the same installed SDK.

For a fresh macOS/Homebrew environment only, `./tools/bootstrap.sh` reconstructs
pinned tools, downloads verified GNU archives and runs the retained minimal example.
It creates the alias, installs missing Homebrew/Python dependencies and refuses to
overwrite an alias pointing elsewhere. Inspect the script before running it.
It is a recorded reconstruction route, not a tested CI service.

The retained patches handle a missing fxSDK string header and binutils system-zlib
configuration on macOS. The GCC installer carries its upstream soft-float patch.
C++ can be installed by that toolchain route, but this add-in uses C and does not
link libstdc++. fxlink UDisks2/SDL2 options are disabled in the macOS bootstrap.
No reference PDF is required to compile or test the app.

## Host tests and UI captures

```sh
./tools/test.sh
# Optional, after Pillow is available:
python3 tools/capture_ui.py
python3 tools/capture_readme.py
```

The script configures `tests/`, builds and runs all 44 CTest groups with strict
warnings, assertions and UBSan by default. The drawing/key adapter executes the
actual application sources, with deterministic counters and temporary test files.
It is not a SuperH/OS emulator. Physical timing, Fugue behavior and stack/allocator
high-water require a calculator. ASan coverage is not claimed.

The font atlas is already checked in with its upstream notice. `tools/host_font.py`
regenerates its derived header; `tools/make_icons.py` regenerates original icons.
No manual screenshots are included. Development-only PyMuPDF in the optional
bootstrap requirements was used to inspect local manuals; it is not linked or
redistributed with the add-in. Normal builds/tests do not need it.

## Releases

`VERSION` is the public prerelease string. CMake's project version and numeric G3A
metadata use its numeric base (`0.12.0`, `00.12.0000`); the container cannot express
`-beta.4`. The Git tag and Release make the beta designation explicit.

Release from a clean tagged commit: clean target build, host tests, package check,
then calculate SHA256. Attach `DIFFEQ.g3a`, `SHA256SUMS.txt` and the assembled
`THIRD_PARTY_NOTICES.txt` to the prerelease. Relinking embeds a build timestamp, so
hashes can differ across builds even with unchanged source. Do not commit binaries,
manuals, local toolchains, private paths or raw diagnostic logs.

Phase renderer screenshots: `python3 tools/capture_phase.py`. Bounded analysis benchmark: `build-host/benchmark_phase`. Algorithm limits are in [PHASE_NUMERICS.md](docs/PHASE_NUMERICS.md).

Event/Diagnostics screenshots: `python3 tools/capture_events.py`. Event benchmark: `build-host/test_events`. SAVE format v10; frozen v3–v9 readers retained. See [EVENTS](docs/EVENTS.md).

The v0.12.0-beta.4 graph interaction pass adds transactional BOX, entry-view INIT,
local point markers, delayed busy feedback and deferred Equation/IC validation.
Numerical algorithms/storage are unchanged; model.c only extracts existing defaults.
`tools/capture_interaction.py` reproduces 28 review frames; busy uses deterministic
host RTC advancement. Native feedback reads the installed gint RTC driver.
