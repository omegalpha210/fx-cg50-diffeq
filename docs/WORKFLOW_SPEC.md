# Input workflow and lifecycle — v0.9.0-beta.3

Equation → Initial Conditions → Solver Parameters → Graph is the common path.
Main digits1–4 or UP/DOWN+EXE choose the equation type; F1–F4 are blank/inert.

| Screen | F1 | F2 | F3 | F4 | F5 | F6 |
|---|---|---|---|---|---|---|
| Equation | VAR where relevant | FUNC in EDIT | V-WIN | | | NEXT |
| IC | PREV | | V-WIN | | | NEXT |
| Parameters | PREV | INIT | V-WIN | OUTPUT | SET | GRAPH |
| OUTPUT | | | COLOR | INIT | | DONE |
| Graph base | TRACE | ZOOM | V-WIN | TABLE | G-SLV | PREV |
| TRACE | x= | NORMAL | FAST | FASTER | LEFT | RIGHT |
| Table | TOP | BTM | MID | | STAT | GRAPH |
| Main | | | | | RCL | SAVE |

SELECT EXE invokes an ordinary form's primary action from any row. EDIT EXE
validates, commits and selects the next visible row; the edited last row stays.
EDIT EXIT commits/stays; a further EXIT returns to the parent. Invalid drafts
remain available to correct. LEFT/RIGHT starts editing at the beginning/end;
direct input replaces. The common EDIT hint follows logical state, not blink.

OUTPUT keeps its separate row completion: nonlast EXE selects next; a modified
last row commits/stays, then another EXE completes. LEFT/RIGHT toggles ON/OFF,
F3 opens the palette. Color selection uses its own arrows/EXE/EXIT contract.

SF appears only for the four scalar first-order kinds. Parameters maps visible
rows to actual fields; higher-order Step advances to Max steps. INIT preserves
hidden SF, while first-order INIT restores SF12. Graph Settings owns appearance.

FUNC/VAR replace the equation softkey bar without changing the draft; first EXIT
closes that bar. Destructive size-change confirmation keeps explicit YES/NO.
X/Y-CAL has a separate blank prompt: EXE/F6 RUN calculates after validation.

Navigation remains iterative; auxiliary views restore their caller's selector.
PREV is the existing magenta style and Graph PREV returns to Parameters. ZOOM,
G-Solve and TRACE retain their own submenus. Explicit SAVE/RCL and the Fugue
world switch remain; ordinary navigation does not write sessions. MENU/reentry
and held-key/timer behavior require hardware regression checks.

See [USER_GUIDE](USER_GUIDE.md), [HARDWARE_RETEST](HARDWARE_RETEST.md) and
[ACCEPTANCE](ACCEPTANCE.md) for full controls and validation scope.
