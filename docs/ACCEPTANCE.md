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
