# DIFFEQ v0.12.0-beta.6 — Graph overlay and navigation polish

- Graph instructions/warnings share a top-left, text-sized white backplate.
  Active selection temporarily owns the channel; warnings return on exit.
  Numerical result/data remains lower-left and TIME/PHASE/EVT top-right.
- Every applicable G-Solve selector uses `UP/DOWN: SELECT GRAPH, EXE: SELECT`.
  It fits252px; EXE is blue, normal and painted once. Single graph skips selection;
  ICPT preserves both stages when needed.
- One EXIT from selection/results returns G-Solve submenu; another fresh EXIT
  returns Graph. Held EXIT cannot skip layers. Selection/blink/result navigation
  no longer re-integrates the plot or publishes Partial: Cancelled. G-Solve scratch
  cancel discards temporary results and preserves canonical trajectory diagnostics.
- Graph F6 INIT, V-WIN F1 INIT and ZOOM ORIG use one factory active-window reset.
  Appearance, selected view, solver preferences, MAN bounds, Event/Output and saves
  remain. AUTO uses existing synchronization. Graph-entry snapshot is removed;
  TRACE F1 remains cursor/curve reset. Matching display caches are reused when possible.
- Hidden G-Solve9px result markers translate Y minimally above the fixed lower
  panel, preserving X/scales/Y span/result/index. Visible/invalid points do not pan.
  Result cycling uses existing results/display samples, not another G-Solve query.
- Shared delayed neutral `/ - \ |` busy feedback: CALCULATING in TRACE/G-Solve's
  lower panel, Preparing Table in its preparation area, Drawing in its strip.
  Cancellation first,156.25ms delay,<=8Hz. Small synchronous native LCD rectangle
  uploads and immediate VRAM restoration avoid full-screen spinner redraws/residue.
- Fixed-X TRACE, ordinary Graph pan/extension, tiles, BOX, numerical/parser/Event/
  Phase/Table algorithms, SAVE v10 and v3-v9 migrations are preserved.
- No broad red invalid-domain x-region shading: current status does not certify
  that every curve/state is invalid throughout such an interval.

49 host/UBSan groups (all previous47 retained), clean28-unit SH compile/link,
zero warnings and13 package checks, repeated from the exact public tag.
Text215952 (+1744), data752 (unchanged), BSS72192 (-160), max application frame2664
(unchanged), G3A245384 (+1744) B versus beta.5. No new framebuffer, trajectory,
large result cache or heap UI buffer. Busy borrows existing scratch; frame sizes
are not total device stack high-water.

Back up sessions when upgrading; older add-ins may reject newer saves. MIT,
dependency notices, public ancestry and existing tags remain unchanged.
[Full overlay audit](../OVERLAY_AUDIT.md), [20 production frames](../ui-review/overlay-overview.png),
[user guide](../USER_GUIDE.md), [hardware checklist](../HARDWARE_RETEST.md).
**HARDWARE TEST REQUIRED / HARDWARE RETEST REQUIRED:** all34 new priority cases
remain pending. Host tests/VRAM frames are not device PASS or an SH emulator.
