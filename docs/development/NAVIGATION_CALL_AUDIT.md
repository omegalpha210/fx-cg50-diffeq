# Navigation and lifetime audit

## Preserved history

The pre-84aefe0 application used bounded nested screen calls (not recursion).
Reported clean SET/Parameters and dirty Graph EXIT paths both reached automatic
Fugue writes from the wrong world. This was the confirmed API-contract defect,
not evidence of transition-count stack growth or whole-RAM exhaustion. The
84aefe0 fix introduced checked OS-world workers and a fixed-parent dispatcher.
See CRASH_AUDIT.md for the historical traces and evidence.

## Current iterative dispatcher

`app_run()` owns AppNavigation and small editor/selection state. Each screen
returns STAY, OPEN, BACK or REPLACE to that single loop. No screen calls its
ancestor; the fixed parent array never grows with transition count.

```text
Main -> First-order type -> Equation
Main -> numeric Order/Variables -> Equation
Main -> Second-order Equation
Main -> RCL choice -> Load ->(replace)-> Equation
Equation -> IC -> Parameters -> Calculate ->(replace)-> Graph
Equation / IC / Parameters -> V-Window / Output / SET
SET -> Graph Settings / Private Constants
Graph -> TRACE / V-Window / Table / Graph Settings
```

Calculate is replaced by Graph under Parameters. Thus EXIT traverses Graph →
Parameters → IC → Equation → its originating selector (or Main) → Main. Main
EXIT stays there; no ordinary key selects a Quit state or returns main(). Each
auxiliary form uses its real caller. Stage selection and IC scroll are retained.
Stage changes validate drafts and keep one shared Document, never per-stage
Document copies. EXE cannot fall through into NEXT/GRAPH.

ZOOM and G-Solve softkeys are local, bounded menus within Graph, not dispatcher
screens. Opening/paging/closing them does not redraw plot pixels, integrate or
access files. Actual G-Solve selection/results are bounded modal helpers and
return to that menu. Other helpers (message, confirmation, token insertion, SETS)
are also bounded and never encode ancestor navigation.

## Files and startup policy

[USER REQUESTED ADAPTATION] Main SAVE is the only session-write entry. All former
edit/EXIT checkpoints and the AUTOSAVE build switch are removed; no automatic
startup restore occurs. Main RCL offers the original last-calculation recall and
explicit saved-session load as separate choices. Explicit Save/Load execute only
after UI handlers unwind to the dispatcher. Streaming v3/two-slot storage, world
switches and descriptor cleanup are unchanged from 84aefe0. This policy change
is independent of the old crash diagnosis.

## MENU, keys and timers

Installed gint 2.11 getkey.c handles unmodified MENU with gint_osmenu() and
GETKEY_MENU_DUPDATE. The common application wrapper does not handle it twice.
Numerical polls use getkey_opt with MENU disabled; they preserve up to eight
transformed keys and cancel for MENU, which the next UI read handles once.
File workers contain no UI/poll/integration callbacks. Delayed modifier transforms
stay enabled across timer timeouts, following keyboard.h's documented rule.

The fx-CG50 native osmenu.c path clears launcher events after return; its fallback
requires physical verification. main() is never recursively called or jumped to.
True startup initializes defaults; returning from MENU does not initialize again.
TRACE and curve selection allocate at most one timer, whose callback only sets a
flag. EXIT/EXE stops it before leaving the selection/starting an operation.

## Evidence and limits

Eleven host groups pass. Navigation stress runs 1,000 parent cycles, 1,000 actual SET
round trips, 100 all-stage/Graph cycles and 100 family cycles with no writes.
UI regression checks all seven families, all stage auxiliary returns by pixel
hash, draft errors and manual-range retention. 100 ZOOM/G-Solve cycles preserve
solver/search/read/write/open/close counts and every plot hash. Source inspection
finds no direct application heap allocation or added framebuffer/trajectory copy.
The target common.c branch also passes deterministic queue saturation, one-MENU
and 100 timer-lifetime tests using host adapters. Host tests cannot measure OS suspend/resume, timer speed, native Fugue resources
or stack high-water: **HARDWARE RETEST REQUIRED**, per HARDWARE_RETEST.md.
