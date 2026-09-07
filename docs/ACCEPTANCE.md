# Public beta.2 acceptance and evidence

Public runtime source and existing tests match validated development milestone
`0de88c7e0382bc2717db4b2fc9b2036582e38ca3`. The previous public snapshot is
`699dbb5` / `v0.9.0-beta.1`; this update preserves that clean public history.

## Verified source baseline

The development milestone passed **28/28 host/UBSan groups**, a clean SH compile
of **21 target C units** and linker, and **13/13 G3A container checks**. The exported
public candidate independently passed all **28/28 host/UBSan groups** before tagging.
No test group is disabled. Existing numerical/parser, G-Solve/ICPT, TRACE,
preflight, navigation, storage migration/recovery and Table regressions are included.

The source baseline's SH section sizes are text **164,960**, data **704** and BSS
**62,720 bytes**. Largest reported static function frame: **2,468 bytes**, ui_graph.
These are linker/compiler measurements, not device allocator/interrupt-stack peaks.
No large buffer or calculator code was added for README rendering.

The 17 form hint/context widths fit 368 pixels: EDIT243, OUTPUT189, comma128,
longest V-WIN336. Host tests check logical EDIT during blink, contextual restoration,
PREV pixels/navigation, pale swatches, visible Parameters row mapping, hidden SF
INIT/mode/SAVE retention, and clear seven-row layouts.

## Exact-tag release validation

The downloadable binary is built from **v0.9.0-beta.2**, not copied from a development
dist directory. Publication requires a clean tagged tree, full host/UBSan run,
strict SH compile/link with `-Wall -Wextra -Werror -Wframe-larger-than=3072`,
13 container checks and an artifact hash. The release's **VALIDATION.md** records
that run's exact public commit, counts, section sizes, binary size and SHA256.
**SHA256SUMS.txt** identifies the downloadable bytes. No raw local log is public.

[Release assets and validation](https://github.com/omegalpha210/fx-cg50-diffeq/releases/tag/v0.9.0-beta.2)
contain the final release evidence. Numeric G3A version is 00.09.0000, with beta.2
identified by VERSION, Git tag and Release. Header timestamps can change on relink.

## Visual and publication checks

Eight README views are reproduced from the production renderer at one consistent
example, plus the original icon. All nine assets were visually checked; no manual
screenshots are present. The hero graph and icon use nearest-neighbor enlargement;
PNG optimization is lossless. See [image provenance](images/README.md).

The public source comparison and bounded private-path/secret audit are described
in [PREFLIGHT.md](release/PREFLIGHT.md). MIT and dependency notices are retained.
English/Korean README content and relative image/document links are checked together.

**HARDWARE TEST REQUIRED.** Host rendering is not a physical LCD or SH/OS emulator.
The latest display/held-key/blink behavior, MENU/Fugue, native SAVE/RCL/STAT and SH
numerical behavior need fx-CG50 retesting. The [hardware checklist](HARDWARE_RETEST.md)
retains 28 UI polish, 37 simplification and 49 solver/navigation cases.
