# Function-preserving source and workspace cleanup

The actual starting point was development `2fb4f9f`, public `4fb786c`, beta.9:
63 host/UBSan groups and a 251,368-byte G3A. The request's beta.8 numbers were
historical, not a rollback target. No SDK reinstall, architecture rewrite, feature
addition, UI redesign or save-version change was performed.

## Inventory and classification

| Area | Classification | Decision |
|---|---|---|
| src / include | A runtime source | Keep algorithms; remove only proven unused APIs; separate Graph responsibilities |
| tests and host font/adapters/goldens | B / F | Keep all existing tests and frozen save readers; add before/after contracts |
| tools / lock / patches | C | Fixed output paths, guarded deletion/snapshot, CWD-independent font check |
| current docs / gallery | D / H | Preserve current product information, reorganize and verify links |
| assets/icons/menu geometry | E | Preserve canonical source/generator and attributed font pipeline |
| frozen v3–v10 SAVE layouts | F | Keep every migration; current format remains v11 |
| superseded design/measurement/contact sheets | G | Archive meaningful evidence with explicit supersession |
| covered individual/3x captures | H / J | Remove only after exact retained-sheet pixel comparison |
| obsolete CMake caches | I | Reconfigure fresh; inspect exact paths; preserve unique probes; delete guarded allowlist |
| reference manuals | KEPT — REFERENCE | Both original PDFs unchanged and excluded publicly |
| user scratch file / uncertain old probe ownership | K | Preserve; no guess-based deletion |
| installed SDK/venv and release history | G / required development environment | Preserve; never blanket-clean .local |
| dist / retained hello package | Final outputs / reference | Preserve original bytes separately; rebuild production package |

The private preflight recorded Git status, all top-level counts/logical bytes,
tracked/untracked status, protected hashes and source hashes. The public-safe
[per-file action list](CLEANUP_FILE_ACTIONS.tsv) records all 52 moves, imports,
211 capture removals and 72 tracked-log decisions. Source names alone were never
used as removal evidence.

## Code decisions and coverage

| Item | Old role / references checked | Classification / action | Reason / regression |
|---|---|---|---|
| ui_row | Selection-row wrapper; no caller | REMOVED | Source/header/tests/tools, callback/table/generator references and object U/relocations checked; production UI goldens |
| ui_number | Number wrapper calling ui_edit; no caller | REMOVED | Only orphan helper chain; 220-byte symbol; editor/parser/form tests |
| ui_edit | Legacy editor called only by orphan ui_number | REMOVED | No runtime/test/tool caller; 604-byte symbol; actual editors remain |
| trace_direction_invalid | Old direction helper; no caller | REMOVED | Full references, nm/map and tests checked; 40-byte symbol; TRACE/domain tests |
| ui_graph | Mixed render/menu/Zoom state responsibilities | SPLIT | render_graph_menu, render_equilibrium_selection, handle_zoom_input; branch order, draw order, cancellation/rollback and FP factors retained |
| GraphMenu constants | File-local state IDs | RENAMED / clarified | Typed local enum; no serialized or external value change |
| show_gsolve_notice argument | Unused App pointer | REMOVED | All four calls updated; result/selection/cancel contracts retained |
| editor math include | No remaining math use after orphan deletion | REMOVED | Strict host/SH compilation |
| ode_integrate / ode_rk45_integrate; trace_step/follow; graph_highlight/follow | Tests/reference entry points | KEPT — CURRENT | Deliberately exercised by host/reference tests, even where native entry paths differ |
| clamp / formatting / hash variants | Different numerical, screen or storage semantics | KEPT — CURRENT | No forced generic merge; existing shared text centering/dialog/busy helpers already centralize equal behavior |
| v3–v10 readers and document ABI structs | Frozen compatibility | KEPT — LEGACY | Record layouts/fixtures and reason in CODE_MAP; full persistence and fault injection |
| runtime source icon data / host font header | Verified generated inputs | KEPT — CURRENT | Canonical generator + attributed atlas; regeneration/check byte-identical |

Four dead symbols totaled 1,080 bytes before removal; the final ELF/map contain
none. Overall SH text savings also include helper/control-flow/link alignment.
There is no large mechanical rename or macro compression. Authored C LOC
6,699→6,656; headers 680→676; total 7,379→7,332. `ui_graph` 157→125 lines,
lexical branch tokens 102→80, maximum nesting remains six. These are transparent
readability proxies, not claims of measured cyclomatic complexity or faster runtime.
New helpers are 7/13/24 lines with distinct responsibilities.

The [CODE_MAP](../development/CODE_MAP.md) documents every responsibility,
Graph state/key/HOLD behavior, header dependencies, mutable static/global owners,
feature/parser contract and native v3–v11 SAVE layouts. No header cycle was found;
large BSS state stays static. Unnecessary header proliferation was avoided.

## Behavior and asset preservation

RK4, model/fixed integration, parser, Event, G-Solve, Phase/nullcline/equilibrium,
storage/migrations, geometry and production renderer are byte-identical to the
baseline. RK45 and power source changes only update documentation-path comments;
FP expression order and lifecycle behavior are unchanged. SAVE magic 0x44455131,
version 11, namespace and all v3–v10 layouts/readers remain unchanged.

The frozen numerical contract contains 273 records: complete scalar samples,
order-9/system-9 states, accepted/rejected/work counters, every Event direction and
MARK/STOP, G-Solve modes/intersections, SYS2 vectors/nullclines/equilibria, domain
termination and multi-IC visibility/color. Before/after output is byte-identical
on this host. Portable checks use 1e-11 relative / 1e-12 absolute float tolerance;
integer counters/status/order remain exact. The 18 actual-app UI contracts match
FRAME/TEXT/PLOT/REPORT hashes exactly. All original 63 test groups remain enabled.

Both PDFs and the pre-existing untracked user scratch file retain their hashes.
Package icons, menu const geometry, attributed font atlas/header and generators
are preserved; no new font or dependency binary was added. The existing public
README gallery/helper was made canonical locally with unchanged image bytes.
211 duplicate/intermediate PNGs were compared against retained sheets, including
native and scaled pixels. Final docs keep 16 representative images, five current
sheets and eight frozen historical sheets (29 PNGs). One stale SYS9 header in a
sheet was regenerated from the existing beta.9 handler (`1 of 9`); no UI behavior
was changed. All 11 capture suites run; promotion only copies allowlisted current
images and never overwrites archived sheets.

Caption regeneration also exposed an environment difference: Pillow 9.3 uses a
classic bitmap default, while installed Pillow 12.3 uses a FreeType default.
A shared caption_font helper now selects the existing classic bitmap explicitly.
No font binary or production font changes. Both versions reproduce native/scale
and contact-sheet pixels; PNG compression bytes may differ by Pillow/zlib version.
The original gallery stays unchanged. This fixes tool reproducibility without a
UI redesign or a new dependency installation.

## Build and file hygiene

Fresh configurations use build/host and build/target, not moved CMake caches.
Optional hello uses build/hello-target; normal captures use build/captures/suite,
managed scratch build/tmp/captures. SDK reconstruction logs use .local/toolchain-logs.
Two clean target builds repeat 29 C units, zero warnings and all 13 container
checks. Host fresh/clean builds pass all 66 groups with UBSan/assertions enabled.
No new random root directory or one-off root log/binary is produced.

Ten exact obsolete CMake caches were inspected for expected CMake home, references,
Git-tracked files and symlinks. 29 unique probe/test-output candidates were copied
and hash-checked before deletion. Generated SAVE/CSV/image outputs are test products;
two handwritten cache measurement probes are retained privately for ownership review.
The guarded helper defaults to dry-run and never selects SDK, manuals, dist,
release evidence or arbitrary paths. It has synthetic rejection/deletion tests.

426 raw logs were copied and SHA-checked into private history first; 12 referenced
historical measurements remain in archive/build-logs. The other 60 tracked outputs
and 354 ignored logs leave the source tree, while exact private evidence and Git
history remain. They are not published. Archive is reserved for meaningful design,
compatibility and visual evidence, rather than every reproducible intermediate.

## Before/after workspace structure

| Metric | Before | After canonical local validation |
|---|---:|---:|
| Tracked files | 544 | 311 |
| Root files (including user scratch/OS metadata before) | 8 | 14 |
| Root directories (excluding .git) | 12 | 11 |
| Tracked docs | 350 | 98 |
| Docs PNGs | 231 | 29 |
| Obsolete audited CMake caches | 10 | 0 |
| Managed capture scratch entries | — | 0 |
| Workspace logical bytes excluding .git | 2,649,216,738 | 2,528,833,968 |

Root directories are now assets, build, dist, docs, examples, include, src, tests,
tools, .github and the preserved private .local. New root product/license/contributor
entry points explain the larger root-file count. Generated previews are in build,
not mixed with source docs. Ten old caches remove 5,383 files / 221,566,842 B;
new canonical builds and preserved evidence explain the smaller net disk decrease.
Logical usage includes the installed SDK and private history, not allocated disk
blocks. Public verification later adds intentional ignored builds in its checkout;
its binaries/cache are not public source. Two untracked OS metadata files were
removed by exact path; the existing user scratch file remains REVIEW REQUIRED.

## Validation and limits

Required host/UBSan **66/66**, strict **29-unit SH / zero warnings**, package
**13/13**. Tests cover solver/parser/domain, every mode/dimension, Events,
G-Solve/Phase, TRACE/Table/export, SAVE v3–v11/corruption/I/O faults, allocation
failure, MENU/OFF policy, HOLD/cancellation and UI regressions. Golden/test/tool
coverage increased; no test was removed or disabled.

ASan remains unavailable on this host: an empty-main control also hangs before
main in runtime initialization. No ASan PASS is claimed. Hardware-only native
power/suspend, MENU/Fugue, media/LCD/key behavior and cumulative stack/heap high-water
remain **HARDWARE TEST REQUIRED**. Existing hardware retest items are not closed
by cleanup or host simulation.

| Measurement | Beta.9 before | Cleanup after | Difference |
|---|---:|---:|---:|
| SH size text (includes rodata/runtime code sections) | 221,920 | 220,696 | −1,224 B |
| .text section | 195,404 | 194,348 | −1,056 B |
| .rodata | 26,004 | 25,836 | −168 B |
| data | 768 | 768 | unchanged |
| BSS | 72,240 | 72,240 | unchanged |
| Largest single application frame | 2,664 | 2,664 | unchanged |
| G3A | 251,368 | 250,144 | −1,224 B |

Original development dist SHA256:
`dc89c52224a90b2873b1f996e58eb098f2824e373983f026618a799bdfb7f37d`.
Baseline public beta.9 SHA256:
`273fd5a4d1cc59b11b0980706924ff98aa37c7edef62941e12c57eea4778a998`.
The final immutable published binary hash is recorded in the release SHA256SUMS
and VALIDATION; relinking timestamps can change SHA despite unchanged code.
Single-frame data does not prove cumulative device-stack safety.

## Publication and remaining review

The clean public snapshot retains public main ancestry and immutable older tags;
internal history/policy, PDFs, environment/manual-analysis records, raw logs,
SDK/build trees and unnecessary binaries are excluded. MIT and all third-party
notices retain their original bytes. Exact public candidate/tag test/build/package,
source/link/privacy/history checks and release re-download gates precede publication.

REVIEW REQUIRED is limited to the preserved user scratch file and historical
measurement-probe ownership; neither was deleted or affects the current build.
Installed SDK/build/release history remains intentional private evidence. No new
feature decision, numerical discrepancy or unresolved required gate is hidden by
cleanup. Current contributor entry points are PROJECT_STRUCTURE and DEVELOPMENT.
