# UI consistency acceptance — v0.12.0-beta.3

Baseline development9e64365 / public4dcb2d6. Full screen/F-key inventory, installed
gint font reproduction, implementation and current rules: [UI_CONVENTIONS](UI_CONVENTIONS.md).

- **42/42 host/UBSan groups PASS**,31.06s development final; all prior41 retained.
- Semantic F-key colors across all slots, normal single-painted EXE glyphs,
  removed redundant help, Main grouping, h0/AUTO/MAN and Output line previews.
- Field-preserving validation and consistent F5 NO/F6 YES confirmations.
- TIME/PHASE/EVT and shared cached/fresh labels; Event marker screen offset fixed;
  TRACE/G-Solve/Phase overlays remain readable with existing2px/blink policy.
- All numerical/parser/ODE, Event algorithms, storage and migration preserved.
  Graph changes are painting/layering only; no new state or trajectory allocation.
- Clean27-unit SH compile/link, zero warnings,13/13 package checks,236200-byte G3A.
- text206816,data704,BSS71952,max ui_graph2568; data/BSS0, maxframe−4 versus beta.2.
- 24-screen [visual audit](ui-review/consistency-overview.png), refreshed workflow
  and existing gallery; installed-font ink masks/paint counts/widths and handler tests.

**HARDWARE TEST REQUIRED:** all36 new items in [HARDWARE_RETEST](HARDWARE_RETEST.md).
Exact public commit/tag, independent candidate/exact-tag runs and downloaded asset
verification belong to Release VALIDATION.md; host frames do not prove device behavior.

Earlier milestone records below are historical.

---

# UI/UX polish acceptance — v0.12.0-beta.2

Baseline development12877bc, public9b4ac21 / v0.12.0-beta.1. The installed tools,
private references and all existing development/public history are preserved.

- **41/41 strict host/UBSan groups PASS**,21.95s local final run; baseline39/39
  in16.18s. Existing numerical/parser/Event/TRACE/G-Solve/Phase/Table/storage and
  native-key/lifecycle regressions remain enabled. UI scripts use the new keys.
- Main1–6, shared EXE/F6 OPEN handler, F1–F5 blank, internal selector retention.
- SAVE confirmation: NO/EXIT and pre-confirmation have zero file calls; YES/EXE
  executes the unchanged backend once. Opening-key repeat cannot approve. Failed
  save returns safely with the SAVE selector retained.
- INIT Yellow/Black; Parameters F1 preserves method, Event and V-Window. V-WIN
  preserves appearance/projection/manual solver settings; automatic Xrange still
  follows TIME geometry. Settings/Output reset only their own documented scope.
  Graph Settings Style is the requested exception: F1/F2 blank, arrows only;
  INIT is available on other Settings rows. ADV Black/White contains EVENT/INFO only.
- SELECT cycles include only visible rows. EDIT cursor, INFO scroll and all
  data/graph/palette navigation keep their existing policies. All seven ODE types
  show1/3→2/3→3/3 and return through EXIT; submenus have no progress. V-WIN only
  appears in Parameters/Graph. EXE help is Blue; only Main's first MENU is Red.
- Five requested hints measure281/243/247/112/266 pixels in the installed-font
  host atlas, within368px. Colored glyph masks retain exact original positions.
  All mode titles clear the reserved40px progress area.
- **27 target C units**, clean SH compile/link, zero warnings,13/13 G3A checks.
  Development235448 B; public binary is rebuilt from the exact tag and hashed.
- SH text206064/data704/BSS71952; BSS/data delta0; maxframe ui_graph2572 B unchanged.
- Main/Equation/IC/Parameters/Settings/Output/SAVE and existing gallery images
  regenerated with production handlers. No manual images or emulation claims.

No changes to src/ode, src/math, src/storage.c, src/app_state.c, src/graph or their
numerical/model/Event/cache headers relative to12877bc. Only UI presentation and
navigation change; no new session version, buffer, solver or persistence backend.
**HARDWARE TEST REQUIRED:** all36 new control/LCD/key/reentry checks at the top of
[HARDWARE_RETEST](HARDWARE_RETEST.md). Host PASS is not physical-device evidence.
Exact public IDs, hash, timing and download verification are in release VALIDATION.md.

The previous milestone records below are historical; the current control map is
[USER_GUIDE](USER_GUIDE.md).

---

# Event / Solver Diagnostics acceptance — v0.12.0-beta.1

Baseline development e885230; public v0.11.0-beta.1 / 3892707. Existing installed
fxSDK/gint/SH tools reused, RK4/RK45 arithmetic and all prior features preserved.

- **39/39 host/UBSan groups pass**, 24.52s local full run, including all previous36.
- One compiled Event expression, ANY/RISING/FALLING (always increasing x), MARK
  and per-IC/per-direction STOP; accepted numerical steps only, no rejected RK45 hit.
- Bounded48-iteration secant/bisection with selected-method scratch landing,
  cancellation/shared budgets, no canonical overshoot or invalid-gap root.
- Initial-zero and zero-plateau policy, repeated/32-marker cap, ten ICs, SYS9,
  N-th→SYS conversion, domain/pole/singularity cases and mixed-family status tested.
- TIME/Phase squares, STOP Table root state, TRACE no-extension bounds,
  transactional marker preservation and valid-prefix G-Solve are covered.
- Actual RHS instrumentation agrees with report counters. INFO/menu stress has no
  solver calls or file I/O. Graph cancellation and numerical-limit status persist.
- SAVE v10 current/recall Event roundtrip and independent frozen v3–v9 fixtures pass.
- **27 target C units**, strict clean SH compile/link, zero warnings; 13/13 G3A
  checks. Development package234320 B; release hash is from the exact public tag.
- Target text204936/data704/BSS71952. BSS+7056, data unchanged, maxframe2572 B.
- Existing Parameters captures refreshed; two new production-rendered Event/Info
  images. Host captures are not calculator photographs or an SH emulator.

[Event algorithms, benchmark and limits](EVENTS.md), [memory](MEMORY_AUDIT.md),
[29 physical retests](HARDWARE_RETEST.md). Exact public-source validation, artifact
hash and timing are recorded in the Release's VALIDATION.md/SHA256SUMS.txt.
**HARDWARE TEST REQUIRED:** every new Event/Diagnostics device path, target numeric
accuracy, key timing, stack high-water, LCD, MENU/Fugue and native SAVE migration.
Multiple Event definitions, FSAL, sweeps, Poincaré/bifurcation, new/stiff/implicit
solvers, CAS, Laplace and special functions are deliberately deferred.

---

The following is the retained v0.11 RK45 milestone record.

# RK45 acceptance — v0.11.0-beta.1

2026-09-08. Baseline development 6bc10c9, public v0.10.0-beta.1 / 713ddc2.
Classical RK4 remains default and its numerical arithmetic is unchanged.
Dormand–Prince 5(4), max component-scaled error norm, target landing and shared
runtime budgets are integrated across all equation types and consumers.

- 36/36 host/UBSan groups pass (see release VALIDATION.md for exact-tag timing); all previous 33 groups remain enabled.
- New core/order/tolerance/rejection/cancel/domain/budget tests, all modes/9 states,
  ten ICs, all G-Solve modes, Table/STAT, Phase, TRACE extension/cancel and UI stress.
- SAVE v9 current/recall plus independently frozen v3–v8 migration fixtures pass.
- 25 target C units compile/link with strict warnings-as-errors; zero warnings.
- 13/13 package checks; development G3A 224312 B. The public exact-tag asset has
  its own version/timestamp; use release VALIDATION.md and SHA256SUMS.txt.
- Target text194928/data704/BSS64896; data delta0, BSS delta+80.
- RK45 workspace652 B; integrate988 B / step100 B frames; overall max2572 B.
- Parameters screenshots regenerated from actual source UI, including RK45 and
  tolerance fields. Unaffected screenshots/gallery structure are retained.

Full coefficients, output policy, benchmark table, limitations and memory
accounting: [RK45_NUMERICS](RK45_NUMERICS.md). RK45 is not a stiff solver; local
error control cannot certify a mathematical pole location or global accuracy.
FSAL, events, parameter sweeps, bifurcation/Poincaré, implicit/CAS are deferred.
**HARDWARE TEST REQUIRED:** RK45 numerical behavior with the target math library,
9-state/10-IC load, cancellation/repeat, LCD, stack high-water, MENU/Fugue and
native v9/legacy SAVE/RCL. Host PASS is not physical-device validation.

---

The following is the retained v0.10 Phase milestone record.

# SYS 2D Phase Portrait and local analysis acceptance

2026-09-08. Baseline: `hardware-crash-fix / 0fbe0de`, public beta.3 `ad02d8c`.
The existing fxSDK/gint 2.11 and SH GCC 14.1 toolchain was reused. Earlier
history and local reference files are preserved. This milestone implements P0–P4.

## Behavior

- SYS2 F4 VIEW contains TIME / PHASE / TABLE. PHASE plots y2 against y1;
  TIME retains all existing numerical and G-Solve workflows. Table remains x-domain.
- Separate windows and phase preferences are saved in v8. Same-device v3–v7
  records have frozen readers; old active SYS2 phase geometry migrates without
  changing automatic or manual Solver settings. Analysis results are not saved.
- The original SYS2 RK4 stream simultaneously fills the existing bounded TRACE
  cache. Phase projection, TRACE and window fitting reuse it; there is no second
  phase integration, trajectory allocation or framebuffer. Original-stream
  extrema initialize Phase bounds with 12% margins. The 258-point display cache
  can lose detail over long intervals; invalid gaps remain disconnected.
- Independent FIELD/NULL toggles; 20×11 normalized arrows, 20×20 streaming
  numerical contours (N1 red, N2 blue). Invalid samples are skipped.
- Deterministic 12×12 equilibrium candidates, bounded damped Newton refinement,
  view-relative residual/dedup tolerances, maximum 16 sorted roots and diamond
  markers. EQPT LEFT/RIGHT and INFO expose local linear Jacobian classification.
- Bytecode x detection distinguishes autonomous systems. Non-autonomous
  trajectory is allowed; FIELD/NULL use IC x0 and label it. EQPT is autonomous only.
- Analysis preflight does not touch plot pixels. EQPT commits only on success;
  phase changes and TRACE follow roll back on cancellation/work-limit failures.
  Bounded atomic paint follows successful preflight and leaves arriving controls
  queued. LCD timing requires hardware measurement.

## Verification

**33/33 host/UBSan groups pass**, including all 29 previous groups plus phase
numerics, cache, renderer and actual UI. No tests are disabled. Coverage includes
known classifications, 3/9-root examples, scale/translation, poles/jumps/domain
failures, workload limits, cancel preservation, cache identity and full-stream
bounds, hidden outputs, independent windows, single-family TRACE repeats,
FIELD/NULL toggles, EQPT/INFO and 120 production TIME↔PHASE transitions.
Existing RK4, parser, 1–9-state models, G-Solve, Table/STAT, ten-IC, storage,
key lifecycle and navigation stress remain enabled.

Strict SH full compile/link and **13/13 package checks** pass. Flags include
`-Wall -Wextra -Werror -Wframe-larger-than=3072 -Os -g -fstack-usage`.
No warnings or errors. Public source is independently rebuilt and tested;
release `VALIDATION.md` records its exact commit, binary size and SHA256.

| Resource | Baseline | Phase milestone |
| --- | ---: | ---: |
| BSS | 62,992 | 64,816 |
| data | 704 | 704 |
| New permanent fixed-point result | 0 | 1,484 |
| Phase state per Document (SH) | 0 | 72 |
| TRACE total retained points | 258 | 258 |

BSS increases **1,824 bytes**. Existing sample/overlay storage is reused. Largest
new numerical frame is `phase_equilibria`, **2,144 bytes** in the linked build;
`phase_nullclines` is 1,192 bytes. Whole-program largest frame is `ui_graph`,
2,572 bytes (baseline 2,468). Individual frames exclude nested calls and OS stack.
Detailed numerical tolerances and bounds: [PHASE_NUMERICS.md](PHASE_NUMERICS.md).

Three [own-renderer Phase screens](images/PHASE.md) show SYS input, field/trajectory
and nullclines/EQPT/classification. They are reproducible and visually inspected;
no manual screenshot or hardware photograph is used.

**HARDWARE TEST REQUIRED:** [the retest list](HARDWARE_RETEST.md) includes physical
key repeat/cancel, long-expression latency, LCD colors, MENU/Fugue, SH stack
high-water, native SAVE/RCL migration and all existing workflows. Host PASS is
not calculator PASS. RK45/adaptive integration, bifurcations, Poincaré sections,
events, parameter sweeps, symbolic solving, 3D and general N-state stability are
intentionally deferred; no optional diagnostics feature was added.
