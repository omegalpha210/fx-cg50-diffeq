# fx-CG50 feature plan

Current milestone: v0.12.0-beta.5 Native tiles / fixed-X TRACE / unified warnings, complete in source/host/SH
validation. [Rules and evidence](TILES_TRACE_AUDIT.md). All16 new device retests are
pending. After hardware dogfooding, review v1.0.0-rc.1 readiness; do not add a new
numerical feature or automatically declare v1.0. The historical implementation
plan below contains superseded controls; USER_GUIDE is the current key map.

Reference analysis was completed after the verified minimal G3A milestone and was expanded into the screen-level `UI_REFERENCE_AUDIT.md`. The A/B baseline and C extensions are complete in code and host validation for 0.1.0. User hardware tests confirmed baseline 1st/2nd calculation and solution graphs; the UI reference build requires the focused retest in `HARDWARE_RETEST.md`.

## A. Reproduce original behavior

- [ORIGINAL] 1st: separable, linear, Bernoulli, general, each with its original input decomposition.
- [ORIGINAL] Linear second-order equation with three coefficient functions.
- [ORIGINAL] General N-th order 1-9; SYS size 1-9; correct mode-specific derivative/state notation.
- [ORIGINAL] N-th → SYS conversion with corresponding IC conversion.
- [ORIGINAL] Classical RK4, h, range, Step sampling; first graph draws every accepted fixed step, later redraw uses Step.
- [ORIGINAL] Multiple IC solution family; per-variable/per-family graph selection and numeric output.
- [ORIGINAL] SF 0-100 in first-order modes; field-only view with no IC and SF>0.
- [ORIGINAL] V-Window ranges/scales/defaults, editable Xdot with Xmax coupling, axes and tick marks.
- [ORIGINAL] Recall previous equation; overflow/nonfinite calculations can stop before full range.
- [ORIGINAL] Access to numeric solution values as in List/STAT. OS List binary compatibility is not asserted.

## B. Adapt UI for fx-CG50

- [EXTENSION] [UI ADAPTATION] 384x216 native gint UI with top title, clear selection highlight, compact forms and F1-F6 bar.
- [EXTENSION] [UI ADAPTATION] Home retains F1 1st / F2 2nd / F3 N-th / F4 SYS / F5 RCL and adds F6 SET.
- [EXTENSION] [UI ADAPTATION] Equation rows use in-place cursor editing; LEFT/RIGHT starts cursor movement and UP/DOWN validates before changing fields.
- [EXTENSION] [UI ADAPTATION] ICs use an independent vertical block editor. It preserves up to 9 complete vectors without ambiguous `y1` set labels or undocumented list broadcasting.
- [EXTENSION] [UI ADAPTATION] Output uses original-style variable rows and IC-set columns with graph/list state in each cell.
- [EXTENSION] [UI ADAPTATION] Table TOP/BTM pages are computed in fixed buffers. A bounded `DIFFSTATnn.csv` route uses the official STAT List Editor import because no public direct List API was found.
- [EXTENSION] [UI ADAPTATION] Private constants instead of reading/writing shared OS variable memory. Numeric expression inputs accept e.g. 1/4 and pi.
- [INFERRED] Recall restores equation-specific expressions, constants, outputs and ICs while current app-wide Solver/V-Window graph settings remain active.
- [EXTENSION] [UI ADAPTATION] No changes to OS G-Mem needed because rendering stays private to this add-in.

## C. Implemented after baseline core stabilized

- [EXTENSION] Bright magenta/cyan/light-green-first palette, ~2 px solution curves, factory Grid/Axis Label ON and a 384x198 graph viewport.
- [EXTENSION] Direct phase plot for any two selected state components; original obtained this through STAT.
- [EXTENSION] Trace, zoom, pan and automatic window estimates with explicit limits.
- [EXTENSION] Native paged Table with TOP/BTM/DIR and safe STAT-compatible storage export.
- [EXTENSION] Dirty checkpoint/session recovery across app runs, with versioned two-slot validation.
- [EXTENSION] Clear numerical stop reasons and calculation cancellation.
- [EXTENSION] fx-CG50-style ROOT/MAX/MIN/Y-ICPT/ICPT/Y-CAL/X-CAL over the current X domain, independent of Y clipping, with curve/result navigation.
- [EXTENSION] Graph Setup controls for Grid and Axis Label without reducing the viewport.

Completed in v0.10: SYS2 Phase field/nullclines/equilibria. Completed in v0.11: optional Dormand–Prince RK45 with embedded error control, runtime budgets and full consumer integration; RK4 remains default. See RK45_NUMERICS.md for verification. Completed in v0.12: one Event definition (direction/MARK/STOP), bounded marker/refinement and read-only RK4/RK45 diagnostics; see EVENTS.md. Deferred: solution integral, sweeps, bifurcation/Poincaré, implicit stiff solvers, CAS and FSAL optimization.

Completed in v0.12 beta.2: simplified Main/SAVE confirmation, INIT/ADV, SELECT cycles, three-stage progress and contextual key colors. No numerical or session-format changes.

## Delivery gates

1. Actual minimal G3A: complete, commit c6a4ba6, 54,172 bytes.
2. Local-PDF analysis and architecture before implementation: complete, ae9d3d3.
3. Host solver, parser, normalization tests and native relinks: complete, 0c0c0be / 6e669b1 / ed093a1.
4. Native UI/graph/table/storage and host framebuffer review: complete, 3198fdb / f40d11c / 6de8889 / c23989d.
5. Final G3A, icons, warnings-as-errors build, seven test groups, documentation: see [acceptance report](ACCEPTANCE.md) and release commit.
6. Baseline 1st/2nd calculation and graph received user hardware confirmation. The modified UI reference build remains **HARDWARE RETEST REQUIRED**; host tests and a G3A package cannot prove the changed target behavior.
