# DIFFEQ v0.12.0-beta.4 — Graph interaction and BOX Zoom

- Clearer 3px Main divider; Graph Settings F1 INIT works on every row.
- TIME/PHASE labels only with SYS2 VIEW; EVT remains type-independent.
- Delayed neutral G-Solve feedback after about 156ms, with existing cancellation.
- Shared 9px black/white TRACE/G-Solve/BOX point cross; 2px selected curves retained.
- Graph F6 INIT restores the current view's graph-entry window; EXIT goes back.
  Compatible cache is reused; incomplete/replaced cache or Event report uses safe redraw.
  ZOOM ORIG remains the factory window.
- ZOOM F5 BOX: arrows move 4px, EXE confirms two corners, minimum 6px each dimension,
  pale stipple/blue outline, either-stage EXIT cancels without changing the viewport.
- Equation/IC F1 INIT resets only those inputs; Equation F2 FUNC/F3 VAR are EDIT-only.
- Incomplete Equation/IC drafts are allowed until NEXT; focus first error inline.
  IC values update only after all fields validate; unfinished IC drafts are not saved.
- Existing nonfatal red domain/numerical status and valid-region TRACE/G-Solve preserved.

44 host/UBSan groups, clean 27-unit SH build/link with zero warnings and 13 package
checks are required from candidate and exact tag. BSS 72304 (+352), data 752 (+48,
gint RTC driver), largest frame 2648 (+80) B. No second framebuffer or trajectory;
IC draft payload is bounded to 1920 heap bytes plus allocator overhead.
Numerical algorithms, Event counting, SAVE v10 and frozen v3–v9 readers are unchanged.

[Full interaction audit](../INTERACTION_AUDIT.md), [user guide](../USER_GUIDE.md),
[28 renderer frames](../ui-review/interaction-overview.png).
**HARDWARE RETEST REQUIRED:** all 46 cases in [HARDWARE_RETEST](../HARDWARE_RETEST.md)
are pending. Host frames are not calculator photographs or an SH emulator.
