# DIFFEQ v0.12.0-beta.2 — UI/UX polish

- Simplified Main controls: digits 1–6 open every item; EXE/F6 OPEN use the current selection.
- Added SAVE confirmation: F5 NO/EXIT cancels; F6 YES/EXE saves once through the existing backend.
- Unified yellow/black F1 INIT with separate solver, window, graph-setting and output scopes. Method and Event are retained by Parameters INIT.
- Renamed solver utilities to black/white ADV: F1 EVENT, F2 INFO; opening utilities does not calculate or save.
- Added cyclic vertical SELECT navigation and Equation/IC/Parameters 1/3–3/3 indicators. EXIT goes back; IC/Parameters PREV removed.
- V-Window is available from Parameters and Graph. Equation/IC no longer expose it.
- Style uses LEFT/RIGHT with F1/F2 blank on that row; use another Graph Settings row for F1 INIT. Output INIT moved to F1; F3 COLOR retained.
- Improved one-line contextual hints: blue EXE, and red MENU only on Main. EN/KO controls and production-rendered screenshots are refreshed.

All **41 host/UBSan groups**, clean strict SH compile/link with **zero warnings**,
and **13 package checks** are required from candidate and exact tagged source.
Initialized data remains **704 B**, BSS **71952 B**, largest single application
frame **2572 B**. Exact source, binary hash and results are in the Release assets.

RK4/RK45, Event/Diagnostics, parser, TRACE/G-Solve/Phase/Table numerical logic,
SAVE/RCL backend and session migration are preserved. SAVE still writes v10 and
reads same-device v3–v9; older versions may reject new saves.

**HARDWARE TEST REQUIRED:** 36 new UI cases, LCD/key timing, MENU/Fugue reentry,
native SAVE/RCL, Event/RK45 smoke and stack high-water. See [hardware checklist](../HARDWARE_RETEST.md)
and [user guide](../USER_GUIDE.md). Host-rendered screenshots are not device photos.
