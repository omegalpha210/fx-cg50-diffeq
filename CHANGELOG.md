# Changelog

## v0.12.0-beta.7 — Preparation screens and full audit

- Dedicated cancellable Table/Drawing screens; SF 0–50 with legacy normalization.
- Independent colors for up to ten first-order ICs using the existing save matrix.
- G-Solve Event endpoint, Table dx, Recall ownership and partial redraw corrections.
- Systematic mode/solver/consumer/UI/persistence coverage; 57 host/UBSan groups.
- [Complete evidence, review choices and limits](docs/FULL_AUDIT.md).

## v0.9.0-beta.2 — Graph exploration and visual documentation

- Ordinary SELECT EXE now runs NEXT/GRAPH/DONE from any row. EDIT EXE commits and
  selects the next visible field; EXIT commits/stays. FUNC is shown in EDIT,
  VAR only in relevant modes, and Main uses digits1–4 or arrow selection.
- First-order initial values use one common x0 and `y0={...}` for up to nine
  solutions. Higher-order/SYS input shows one complete initial-state vector.
- OUTPUT now has a single dependent-state ON/OFF setting shared by graph, TRACE,
  G-Solve, Table and CSV. x is always the first Table/CSV column. The existing six
  solution colors are retained; F3 opens the color chooser.
- Slope fields have Segment/Arrow styles and six pale colors, with Arrow/Pale Blue
  defaults. Graph Settings shows a real color swatch. SF density appears only in
  scalar first-order Parameters; unsupported modes and their INIT preserve it.
- TRACE adds NORMAL/FAST/FASTER, interpolated movement and viewport following,
  retaining cancellation and bounded-cache behavior during extension.
- Table combines both integration directions in ascending x. TOP/BTM/MID jump to
  reachable ends or the center, while horizontal scrolling keeps x fixed. Valid
  terminal samples remain visible and numerical ends use inline notices.
- Preflight checks reject excessive RK4 work before calculation, with guidance
  for h and Max steps. Numerical failures preserve valid computed regions.
- Shared EDIT hints, clearer IC/OUTPUT help and magenta Graph PREV complete the
  calculator UI polish. Graph PREV returns to Parameters; submenus keep their controls.
- New visual README in English and Korean, using original icons and eight
  reproducible captures from the app renderer.


Session format v6 reads same-device v3/v4/v5 records. Review migration notes before upgrading.

## v0.9.0-beta.1 — Initial public beta

- Native fx-CG50 ODE graphing: seven equation modes, 1–9 states/orders, bidirectional RK4.
- Multiple ICs, first-order slope fields, V-Window, pan/zoom, colors and phase portraits.
- TRACE and G-Solve ROOT/MAX/MIN/Y-ICPT/X-CAL/Y-CAL/ICPT.
- Table, STAT-compatible CSV and explicit SAVE/RCL with recoverable session slots.
- Unified EXE field completion, inline FUNC/VAR, Output color controls and persistent zoom.
- Retained valid regions across magnitude/domain failures, without invented post-gap IVPs.
- Existing native storage world-switch and TRACE repeat/cancellation fixes.
- Fresh-checkout packaging creates the ignored dist/ output directory automatically.
- MIT project license, dependency notices and public source/release packaging.

Core functionality has been tested on real fx-CG50 hardware; additional hardware
coverage is ongoing. The release remains beta. See README limitations and the
hardware retest checklist.
