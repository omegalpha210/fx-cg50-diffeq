# fx-CG50 reboot crash audit

Historical audit for **84aefe0**. Its evidence and world-switch fix are preserved.
The newer staged-workflow milestone intentionally removes automatic saves and
automatic startup restoration as a **[USER REQUESTED ADAPTATION]**; references
to the then-release AUTOSAVE ON/OFF below describe 84aefe0, not current policy.
The later EXIT/re-selection-to-menu report is distinct from the fatal reboot: no
new RAM/corruption diagnosis is claimed. Main EXIT now stays in the app; installed
getkey handles MENU suspension. See WORKFLOW_SPEC.md and HARDWARE_RETEST.md.


## Reported failures

Two independent hardware paths ended in an OS busy indicator followed by a calculator reboot and initial setup screen:

1. Calculate a valid graph, EXIT to Equation, then continue navigating.
2. Main → SET → Solver Parameters → EXIT → SET → EXIT, without drawing a graph.

The second path excludes the numerical solver and graph renderer as necessary causes. Source tracing showed that both paths reached the automatic session save added at the `df7bdc8` milestone.

## CONFIRMED ROOT CAUSE

The baseline performed Fugue filesystem operations from the gint world in violation of the installed gint 2.11 contract.

The installed `.local/src/gint/include/gint/bfile.h` states that BFile cannot be used from within gint, requires `gint_world_switch()`, and otherwise the add-in is likely to crash. The same header explicitly says that Unix and C99 calls on Fugue (`open`, `read`, `write`, `fopen`, and related functions) are BFile-backed and still require the world switch. `.local/src/gint/include/gint/gint.h` documents `gint_world_switch()` as the boundary that restores the OS hardware state so BFile can run.

Before this fix, `src/storage.c` directly called:

- `fopen/fread/fwrite/fclose` for session slot load/save;
- `open/write/close/remove` for CSV and STAT export.

No call was inside an OS-world boundary. This is a confirmed API-contract violation on every target session and export path. Both reported EXIT paths deterministically invoked the violating session path at the point of failure. The violation is the confirmed code-level root cause addressed by this milestone; confirmation that it was the only physical reboot cause and that the reboot is eliminated is **HARDWARE TEST REQUIRED**.

## CONTRIBUTING FACTORS

- A clean visit to Solver Parameters was unconditionally marked dirty, so the exact short SET path performed an unnecessary write.
- Automatic saves were attached to nested workflow return paths instead of a single top-level lifecycle boundary.
- The baseline session path used buffered fxlibc stdio. Installed fxlibc allocates a `FILE` object and 512-byte buffer, while its `fclose()` returns success without propagating final flush or low-level close errors. The baseline checks therefore could not establish durable close success.
- A full 5,696-byte target `Record` duplicated current and Recall Documents in static BSS during persistence.
- Application navigation used a bounded but nested screen call hierarchy. It was not recursive and did not grow with transition count, but it obscured where screen resources had unwound.
- A Fugue close failure can leave a native handle locked. Removing the same path after an unconfirmed close can compound the failure.

## POSSIBLE BUT NOT CONFIRMED

- Overall RAM exhaustion is not supported by the linker map; substantial user RAM remains beyond application `.bss`.
- Application heap exhaustion, leak, double free and use-after-free were not found. The application contains no direct allocation calls.
- A 16 KiB stack overflow inside an uninstrumented OS/BFile call chain was possible on the old path, but was not measured. The short SET path's application frames alone do not support this explanation.
- Session corruption could explain a reset-looking launch after a reboot, but the observed initial setup screen does not prove that the DIFF EQ slot itself was damaged.
- Keyboard state or a stale callback was considered. The baseline owned no timer/callback; no evidence tied keyboard state to the reboot.

## FIX

### Filesystem execution context

All target filesystem calls now occur inside small synchronous OS-world workers invoked by `gint_world_switch(GINT_CALL(...))`. A worker owns the complete native transaction: open, exact/short read or write loop, close, and safe cleanup where applicable. UI, display, keyboard polling, cancellation and RK4 integration remain in the gint world. There is no nested world switch and no file descriptor survives a screen transition.

Session probe, record read, record write/close, verification, CSV create, page append/close and removal all reuse this mechanism. A partial file is removed only when close is known to have succeeded. A close failure is reported and the path is left alone because its native handle may still be locked.

### Session integrity and memory

The two-slot, generation and FNV-1a version-3 format is retained. Save streams a small header, live current Document, live Recall Document and checksum in the existing ABI layout. It writes only the slot opposite the newest valid record, closes it, then probes and validates the result. The prior valid slot is never invalidated first. Load validates exact size and checksum, stages both Documents, validates all model fields, and only then replaces live state. A corrupted newest slot falls back to the older valid slot; a failed manual load leaves the current Document unchanged.

The 5,696-byte full Record scratch was removed. Load staging shares an explicit union with `CompiledModel`; Save/Load run only where compiled graph state is no longer used, and every later CALC recompiles before graphing.

### Navigation and checkpoints

Application screens return a transition to one iterative `app_run()` dispatcher. A fixed `AppNavigation` parent table implements EXIT hierarchy without carrying it in C call ancestry. Calculate uses a replace transition so Graph's parent is Equation. Session persistence runs only after the Equation, Settings or Graph handler has returned, or on Quit. Explicit Save/Load are top-level dispatcher states.

Clean forms no longer mark the session dirty. Actual equation/form/graph mutations do. The final artifact keeps automatic persistence enabled; `DIFFEQ_AUTOSAVE=OFF` exists only for isolation builds and does not disable explicit Save.

### Blink and resource cleanup

TRACE and multi-curve G-Solve selection now reserve at most one gint timer per selection screen. Its interrupt callback only sets a volatile flag. Rendering and any TRACE integration stay in the main thread, use the canonical streamed trajectory, and allocate no framebuffer or curve copy. EXIT always stops/frees the timer before the owning screen returns; G-Solve EXE also stops it before the selected numerical operation starts. No timer survives to a checkpoint.

## REGRESSION TEST

- SH target builds with actual gint 2.11 headers and links with `-Wall -Wextra -Werror`, stack-usage output and a linker map.
- Both `DIFFEQ_AUTOSAVE=ON` and the temporary OFF diagnostic configuration compile and link for SH. The release is restored to ON.
- Host tests exercise numerical core, parser, all equation models, graph geometry, G-Solve, Table/STAT and two-slot storage corruption fallback.
- A failed session load is checked to leave the current Document intact.
- A 1,000-cycle state-machine stress test verifies explicit parents without recursive transition calls.
- Host A/B tests verify clean forms do not save, dirty boundaries save only with autosave ON, and explicit Save works with autosave OFF.
- PPM frame hashes verify TRACE's selected curve alternates and G-Solve transfers selection while preserving existing numerical semantics.
- Package validation checks the generated G3A header, payload, icons and both checksums.

The calculator scenarios in [HARDWARE_RETEST.md](../HARDWARE_RETEST.md) remain the release gate for physical Fugue behavior, timer cadence, key transforms, LCD redraw and reboot elimination.
