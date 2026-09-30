# Building and developing DIFFEQ

Start with [PROJECT_STRUCTURE](PROJECT_STRUCTURE.md) and the
[source/state/compatibility map](docs/development/CODE_MAP.md). Runtime and test
sources share the existing numerical implementation. Host adapters are isolated
in `tests/`; no host font, counters or fault wrappers enter the SH binary.

## Existing installation

The tested stack is fxSDK/gint 2.11.0, SH GCC 14.1.0, binutils 2.42,
fxlibc 1.5.1, the pinned OpenLibm SH port, CMake 4.4.3 and Apple Clang 17.
[Exact revisions](tools/toolchain-lock.json) and [archive hashes](tools/downloads.sha256)
are retained. Other platforms/toolchains are unverified.

```sh
./tools/test.sh                 # build/host; all 66 strict host/UBSan groups
./tools/build.sh                # build/target; strict SH and 13 package checks
./tools/test.sh --clean         # same path, clean compile; no cache relocation
./tools/build.sh --clean        # same path, clean compile/link
python3 tools/host_font.py --check
```

`build.sh` uses the actual installed fxSDK CMake modules rather than the fxSDK
CLI's hardcoded `build-cg`. It finds an existing SDK through `tools/env.sh` or
`DIFFEQ_SDK_ROOT`; an already configured shell installation also works. No SDK
reinstallation is needed. Application flags include `-Wall -Wextra -Werror
-Wframe-larger-than=3072 -Os -g -fstack-usage`; no fast-math. Host assertions and
UBSan stay enabled. Optional ASan is unavailable on the audited macOS host: an
empty-main control hangs in runtime initialization. No ASan PASS is claimed.

Final `.g3a` stays in `dist/`; ELF/map/stack files stay in `build/target`.
`sh-elf-size build/target/diffeq` and its `.su` files provide memory evidence.
`./tools/build.sh --hello` retains the minimal example source and builds its cache
in `build/hello-target`; it is not part of the production link.

For a genuinely new macOS/Homebrew environment only, inspect `tools/bootstrap.sh`.
It reconstructs the pinned SDK, verifies downloads, and refuses to overwrite an
existing SDK alias. SDK files and reconstruction logs live under `.local/`.
This is a recorded reconstruction route, not a tested hosted CI service.

## Generated assets and renderer evidence

```sh
python3 tools/capture_readme.py
python3 tools/capture_ui.py
# Review outputs in build/captures first. Promote only the allowlisted gallery:
python3 tools/capture_readme.py --update-docs
```

Pillow is needed only for optional rendering/generation. Capture scripts use the
actual app handlers and font, not a CPU/OS emulator. Their scratch directories are
managed beneath `build/tmp/captures`; default runs do not modify documentation.
[Gallery provenance](docs/captures/README.md) lists all suites. Archived contact
sheets are immutable milestone evidence. Contact-sheet captions explicitly use
the existing classic Pillow bitmap font: the installed9.3 and12.3 implementations
produce the same glyph/frame pixels. Compressed PNG bytes may differ across
Pillow/zlib versions; compare decoded dimensions/RGB pixels across versions, while
release SHA256 records the exact binary artifact. No new font asset is added.
`host_font.py --check` compares the
credited atlas conversion byte for byte, from any working directory. Package icon
PNGs and compact menu geometry remain canonical inputs; changing their generators
is separate from regenerating disposable build intermediates.

## Safe cache removal

```sh
python3 tools/clean_builds.py                # dry-run exact obsolete caches
python3 tools/clean_builds.py --apply        # only after review/fresh rebuild
```

The helper verifies repository root, expected CMake home, no tracked content,
no nested repository and no symlinks before deletion. SDK, reference manuals,
release evidence, user files and dist are excluded. A guard cannot determine
ownership of unique untracked files: inspect inventory and uniqueness first.
Do not use broad globs or `git clean -fdx`.

## Source snapshots and releases

`VERSION` names the beta; CMake/G3A numeric metadata use `0.12.0`/`00.12.0000`.
The public snapshot has its own existing main history. Do not push development
branches/history. The source-only helper accepts only an existing, clean main
checkout with the expected origin, selects explicit public content, scans it and
preserves license notices. It defaults to dry-run and never commits or pushes.

```sh
python3 tools/public_snapshot.py --destination /path/to/public-checkout
# After source validation and reviewing its exact change/removal list:
python3 tools/public_snapshot.py --destination /path/to/public-checkout --apply
```

Follow [release preflight](docs/release/PREFLIGHT.md): preserve ancestry/tags;
run all tests, clean strict target and package validation from the exact public
candidate and tagged source; check source equality, links, secrets/history and
notices. Use the next unused beta tag when the binary changes. Attach the verified
binary, SHA256SUMS, VALIDATION and assembled dependency notices; re-download and
compare bytes/GitHub digests. Never publish manuals, private paths/logs, SDKs,
build caches or unnecessary generated artifacts. Relinking can change the G3A
timestamp/hash, so a release checksum identifies that published artifact.

**HARDWARE TEST REQUIRED:** native OS power/lifecycle/storage behavior, physical
keys/LCD latency and cumulative stack/heap high-water. The [hardware checklist](docs/HARDWARE_RETEST.md)
separates these from host checks. Cleanup does not claim new device verification.
