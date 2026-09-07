# DIFFEQ v0.9.0-beta.3

This fx-CG50 beta improves TRACE navigation and first-order initial-value input.

- TRACE follows valid solutions vertically and horizontally while preserving
  window spans and configured Solver Xrange. Both axes pan in one redraw.
- NORMAL/FAST/FASTER use yellow/bright-green/cyan backgrounds, black text and a
  selected border. F5 LEFT/F6 RIGHT jump to configured solver endpoints while
  retaining curve and speed. EXIT returns to Graph.
- Reaching a calculated endpoint does not prefetch. Only a requested move beyond
  the runtime range starts controlled extension, including a FAST/FASTER crossing
  in the same input. Cancelled work preserves the completed graph.
- Graph Settings uses LEFT/RIGHT for Grid/Axis Label; redundant ON/OFF softkeys
  and the fixed density hint are removed. Style/Color controls are retained.
- First-order y0 lists accept ten values, including ten Table solution columns
  with frozen x. The editor stays at 191 characters, with separate count/length
  errors. Higher-order/SYS still takes one complete vector with at most nine states.
- Main Menu help and renderer-based English/Korean README images are updated.

## Saved sessions

New saves use **v7**, with ten initial-condition slots. Same-device v3/v4/v5/v6
records are read through frozen layouts and the existing two-slot recovery path.
Back up existing sessions before upgrading; older add-ins may reject v7 saves.
LOAD preserves the old files. Later explicit SAVE rotates the two slots.
Earlier constants/mask/vector adaptations are described in the
[migration audit](../OUTPUT_LIST_AUDIT.md).

## Download and verification

Copy **DIFFEQ.g3a** to the calculator USB drive root, safely disconnect, and launch
DIFF EQ. **SHA256SUMS.txt** identifies the download. **THIRD_PARTY_NOTICES.txt**
retains the dependency licenses; **VALIDATION.md** records the exact public source,
tests, link/package checks and binary hash.

**HARDWARE TEST REQUIRED.** Host/UBSan and renderer checks do not verify LCD
contrast, real repeat/cancellation timing, MENU/Fugue reentry or on-device
SAVE/RCL/STAT. The [32 priority cases](../HARDWARE_RETEST.md) remain pending.
RK4/parser, G-Solve/ICPT, segmented validity, work caps and the existing lifecycle
and navigation protections are retained. TRACE shares a fixed 258-point budget;
many curves or long domains can reduce interpolation detail. Out-of-cache
movement may need a calculation per input.

Project code uses MIT; dependencies retain their original notices. This is an
unofficial community project, not affiliated with or endorsed by CASIO.
