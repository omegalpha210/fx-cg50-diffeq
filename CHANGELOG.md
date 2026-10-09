# Changelog

## Unreleased — UI/UX refresh (visual only)

- Full-width (396px) header with accent rule and 3-step stage indicator; F-keys are
  full-width dark tabs with a 3px semantic band (GRAPH/RUN filled), empty slots blank.
- Key-cap hints (`EXE:`, `LEFT/RIGHT:` …) and precise edit hints: a non-last field says
  "commit + next field", the last says "EXE/EXIT: commit" (existing EXE rule unchanged).
- Pale selected rows with accent bar, white bordered edit box, `=` for equations/ICs/Event E,
  `:` for settings, LEFT/RIGHT arrows on option rows, AUTO/MAN chips, scrollbars on long lists.
- Error banner; the edited culprit field turns red. Confirmations are cards over the dimmed screen.
- Equation template card and per-field example chips; IC solution chips in curve colors.
- Table: right-aligned numbers, curve-colored column headers, scrollbar. TRACE swatch, readout hairlines.
- Crisp 1px geometry only: no rounded corners, no synthetic bold, no new fonts/bitmaps.
- G-Solve results name the curve (`IC1 y' MAX 1/2`); x and y sit in the F1-F5 info panel.
- Graph selection and Y-CAL/X-CAL prompts name the highlighted curve (blinking swatch) in that panel.
- CALCULATING… (G-Solve and TRACE) uses the full-width bottom bar with EXIT cancels, like Drawing.
- Fix: hardware TLB-miss crash on every G-Solve result (fxlibc printf has no `%.*g`; host libc
  hid it). Literal precisions only; new `firmware_printf_formats` test scans all formats.
- Solver, parser, storage, power, TRACE/plot geometry (384x198 plot) untouched: numerical golden and
  all 18 UI-golden REPORT/FRAME sequences identical; UI golden pixels re-recorded once.

## v0.12.0-beta.10 — Source and workspace cleanup, preserved behavior

- Remove four proven unused functions; separate Graph rendering and Zoom input.
- Fixed build/host, build/target and reviewed capture promotion; guarded cache/snapshot tools.
- Organized current docs, meaningful archives and 29 retained PNGs; 211 covered intermediate captures removed.
- Original solver/parser/storage algorithms, SAVE v11/v3–v10 compatibility and UI controls retained.
- 66 host/UBSan groups (all 63 retained), exact 273-record numerical and 18-workflow UI comparisons.
- Strict 29-unit SH, zero warnings, 13 package checks; text/G3A −1,224 B, RAM/maximum frame unchanged.
- [Detailed actions and validation](docs/audits/CODE_CLEANUP_AUDIT.md).

## v0.12.0-beta.9 — SYSTEM power settings and failure safety

- Respect SYSTEM APO/backlight duration/brightness; main-thread suspend, rollback and wake-key draining.
- SAVE preflight I/O failure aborts without overwriting good slots; v11 bytes unchanged.
- Long selector/output headers show position. 63 host/UBSan groups; hardware timing remains pending.
- [Power/error audit](docs/audits/POWER_SAFETY_AUDIT.md).

## v0.12.0-beta.8 — Drawing bar and approved output/navigation decisions

- Graph-preserving bottom Drawing bar; Table preparation unchanged.
- Independent IC ON/OFF/color, consistent visible consumers, safe all-OFF.
- SAVE v11 mask persistence and explicit all-ON v3–v10 migration.
- EXE-only numeric validation; unconditional EXIT to G-Solve page2.
- Two closed review decisions,59 regression groups and34 pending hardware cases.
- [Implementation and evidence](docs/audits/VISIBILITY_PROMPT_AUDIT.md).

## v0.12.0-beta.7 — Preparation screens and full audit

- Dedicated cancellable Table/Drawing screens; SF 0–50 with legacy normalization.
- Independent colors for up to ten first-order ICs using the existing save matrix.
- G-Solve Event endpoint, Table dx, Recall ownership and partial redraw corrections.
- Systematic mode/solver/consumer/UI/persistence coverage; 57 host/UBSan groups.
- [Complete evidence, review choices and limits](docs/audits/FULL_AUDIT.md).

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
