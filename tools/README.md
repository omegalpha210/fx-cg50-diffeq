# Project tools

Run helpers from any working directory; build/test scripts locate the project
root themselves. Existing SDK installations are reused. Bootstrap is a separate,
explicit installation operation and is not part of normal build or cleanup.

| Purpose | Canonical command | Output |
| --- | --- | --- |
| Host regression and UBSan | `./tools/test.sh` | `build/host/` |
| Repeat all host compilation | `./tools/test.sh --clean` | Same cache, clean-first build |
| Native strict SH build and package check | `./tools/build.sh` | `build/target/`, final `dist/DIFFEQ.g3a` |
| Repeat all SH compilation | `./tools/build.sh --clean` | Same cache, clean-first build |
| Retained minimal add-in | `./tools/build.sh --hello` | `build/hello-target/`, final `examples/hello/DIFFEQ-hello.g3a` |
| Independent container verification | `python3 tools/verify_g3a.py dist/DIFFEQ.g3a` | 13 checks; no package mutation |
| Verify native menu geometry | `python3 tools/generate_menu_icons.py --check` | No source mutation |
| Verify host-only font conversion | `python3 tools/host_font.py --check` | No source mutation |
| Inspect obsolete CMake caches | `python3 tools/clean_builds.py` | Dry run only |

Both build scripts accept `-j JOBS`; `DIFFEQ_BUILD_JOBS` defaults to 8. Host tests
use Clang unless `CC` selects another installed compiler. The normal host helper
explicitly enables UBSan and disables the separate, opt-in ASan diagnostic so a
previous diagnostic cache cannot silently change the required regression run.
The installed fxSDK
CLI fixes its build directory to `build-cg`, so `build.sh` passes the same actual
toolchain/module arguments directly to CMake at the canonical target location.
Do not move an existing CMake cache into this layout: configure a new cache and
verify its build first. Build objects, ELF/map and test binaries are generated;
`dist` and the retained example package are final user/milestone outputs.

`source tools/env.sh` retains the existing whitespace-free `~/.local/diffeq-sdk`
alias. Set `DIFFEQ_SDK_ROOT` before sourcing to select another existing SDK tree.
`DIFFEQ_FXSDK_CMAKE_DIR` can select an installed fxSDK module directory directly;
otherwise `build.sh` checks the SDK prefix, then the installed `fxsdk` executable
prefix. No normal helper installs or changes the toolchain.

Before applying cache cleanup, inspect Git status/tracked paths, protect reference
and final output hashes, identify unique/user-owned files, and demonstrate a fresh
rebuild. `clean_builds.py --dry-run` lists exact known legacy caches. Only after
that review use `clean_builds.py --apply` with the reviewed exact path arguments.
Current caches (`build/host`, `build/target`, `build/hello-target`) are accepted
only when named explicitly. The helper rejects unexpected CMake source roots,
tracked files, nested Git repositories, symlinks and paths outside its allowlist.
It never accepts SDK paths, reference directories, dist, generic tmp directories
or a wildcard. SDK caches inside `.local/src` and `.local/build/fxsdk` stay intact.

Temporary independent build/package work belongs under `build/tmp` or a managed
OS temporary directory. Normal helpers reuse the fixed current caches. Keep only
one current package staging area when needed; release artifacts remain in dist.

The independent verifier remains separate from the upstream G3A generator so
they can expose different faults. Capture suites exercise actual production C UI
handlers through the host adapter; they do not emulate calculator hardware.
Capture workflows are `capture_ui`, `capture_phase`, `capture_events`,
`capture_consistency`, `capture_interaction`, `capture_tiles`, `capture_overlays`,
`capture_audit`, `capture_visibility`, `capture_power`, and `capture_readme`
(`.py` under tools). Each writes only to `build/captures/<suite>` by default,
with managed scratch space under `build/tmp/captures`. `--update-docs` explicitly
copies the fixed current gallery allowlist to `docs/captures`; tiles also update
the eight canonical `assets/menu` previews. Archived sheets stay frozen.
See the [gallery](../docs/captures/README.md) for exact reproduction commands.

Asset generators have separate purposes: `make_icons.py` authors the two package
menu PNGs; `generate_menu_icons.py` authors native compact polyline geometry;
`host_font.py` converts the already attributed test atlas. Firmware uses the
installed gint default font. Preserve these canonical inputs and their notices.

The cache and public-snapshot guard regressions can be run with
`python3 tests/test_cleanup_tools.py`; all destructive cases use disposable fixtures.

## Toolchain reconstruction and retained evidence

- `bootstrap.sh`: reconstruct the pinned SDK on macOS with Homebrew. Installs host dependencies, creates one whitespace-free symlink at `~/.local/diffeq-sdk`, and keeps SDK sources/builds/install inside workspace `.local/`. No sudo and no shell startup edits.
- `env.sh`: source into bash/zsh to activate tools for this shell only.
- `build-toolchain.sh`: manual upstream SH binutils/GCC/OpenLibm/fxlibc/gint build, after host fxSDK installation. New reconstruction logs stay under `.local/toolchain-logs/`; detailed binutils logs remain in `.local/src/sh-elf-binutils/build/`. Current audit evidence links retained historical logs under `docs/archive/build-logs/`.
- `build.sh --hello`: build the retained minimal gint add-in without altering the SDK.
- `build.sh`: configure/build the application at `build/target` and independently check its package.
- `verify_g3a.py FILE`: independently validate container fields and both checksums; based on fxSDK `fxgxa/{g3a.h,edit.c,util.c}`. This does not emulate code or certify hardware operation.
- `toolchain-lock.json`: exact upstream repository revisions. Downloaded GNU releases are binutils 2.42 and GCC 14.1.0, as selected by those upstream scripts.
- `patches/`: minimal local macOS fixes. GCC additionally receives the upstream soft-float patch already shipped in its installer repository.

The GCC build includes C and C++ compilers, but only libgcc is required here. This C application does not need a libstdc++ build. fxlink's Linux-only UDisks2 integration and optional SDL2 viewer are disabled. USB driver support remains compiled in.

Event/Info screenshots: `python3 tools/capture_events.py`. Event benchmark:
`build/host/test_events`; see [EVENTS](../docs/EVENTS.md).
