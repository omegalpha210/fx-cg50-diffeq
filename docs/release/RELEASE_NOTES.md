# DIFFEQ v0.12.0-beta.3 — UI consistency

- Unified semantic softkeys, including GRAPH/RUN red in every key position.
- Simplified contextual help and Main grouping; kept OPEN/NEXT/GRAPH meanings.
- Fixed thick leading EXE: empty-prefix font width caused a one-pixel duplicate
  overpaint. All retained EXE hints now use normal glyphs painted once, in blue.
- Added RK45 h0 and existing solver-range AUTO/MAN labels without changing solvers.
- Added small TIME/PHASE and enabled-only EVT labels; improved graph overlay order,
  corrected Event marker screen offset, protected G-Solve result text from pointers.
- Preserved selected curves' existing 2px blink and Black/Blue contrast policy.
- Added Output horizontal line previews; OFF preserves the selected color.
- Kept input errors in their fields and numerical END on Graph/Table. Critical
  SAVE/load/resize/convert confirmations share F5 NO/F6 YES, EXIT/EXE and HOLD guard.
- Refreshed EN/KO guidance and renderer screenshots; documented the full UI audit.

42/42 host/UBSan groups, clean27-unit SH build/link with zero warnings,13/13 G3A
checks are required from candidate and exact tagged source. BSS71952/data704 B
unchanged; maximum single application frame2568 B (4 B lower). Release VALIDATION
records exact source, timings and hash. No numerical solver, Event algorithm,
parser, storage protocol or migration change; SAVE v10 and v3–v9 readers retained.

**HARDWARE TEST REQUIRED:** all36 new checks in [HARDWARE_RETEST](../HARDWARE_RETEST.md),
including LCD/key-repeat, native SAVE/RCL, MENU/Fugue and stack high-water. Host
frames are not device photos. [UI audit/rules](../UI_CONVENTIONS.md), [guide](../USER_GUIDE.md).
