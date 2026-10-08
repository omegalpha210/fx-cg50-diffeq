# MENU input ownership compatibility — 2026-10-07

The normal DIFF EQ target now defers MENU until the opening input has left both
gint keyboard states and the event queue. This is a **COMPATIBILITY_FIX** for
the input boundary. CASIO's private long-OFF return branch remains unknown;
physical long-OFF success is **NOT_INDEPENDENTLY_VERIFIED**. The existing
HARDWARE TEST REQUIRED label describes that limitation, not another requested
user experiment.

Baseline: development `95a6bc0b8f1e0dca663e2b437b16e51b179693d2`. Its tracked
source/index were clean; the pre-existing untracked `tempCodeRunnerFile.python`
was preserved. No public sync, push, shared SDK modification or UI redesign is
part of this change. Rollback artifact and exact final receipts are retained
in the primary task's `build/power-menu-final/` directory.

## Changed execution contract

Previously idle MENU, deferred computation MENU and TRACE MENU all eventually
called `power_osmenu()` and immediately executed `gint_osmenu()`. No release
precondition existed. The installed SDK's direct path calls the resolved ROM
helper; returning from that helper does not prove stable Main Menu display.

The original CHESS failure recorded a consumed MENU bit at helper entry. In
gint 2.11, `keydown()` and `keydev_idle()` read `state_queue`, which changes when
events are consumed. They do not directly read current physical state. Merely
sleeping until `keydown()` changes can therefore wait forever on an unconsumed
UP event. `state_now` is the scanner's latest successfully queued state; queue
overflow can delay its agreement with the physical keyboard too.

The new path is:

1. Accept one foreground MENU request and preserve the live session.
2. Return to normal foreground input processing. The real reader consumes UP
   events even when `getkey_opt()` filters them from its returned events.
3. Require all twelve rows of public `state_now` and `state_queue` to be zero,
   an empty valid native queue, and quiet observations across a fresh scan.
4. Restore brightness and complete the existing bounded descriptor cleanup.
   Require another quiet/fresh scan boundary after those OS-world workers.
5. Consume the request and call the unchanged SDK MENU helper once.

This excludes direct entry with the opening MENU still present in either gint
state or queued input. All-key release also excludes held EXE/CR selection of
the current app in the reverse-described Main Menu keyboard path. It does not
prove or clear an undocumented forced-selection flag in the current firmware.

Each readiness check returns to the caller. There is no sleep-only release
loop, new timer, interrupt masking, fixed handoff delay, elapsed-OFF selector,
private RAM access or forced retry. Two seconds or 512 unsuccessful foreground
checks cancel a request that has not become ready; the watchdog never calls the
OS as a fallback. EXIT cancels pending MENU and remains an ordinary UI key.
Malformed queue indexes also cancel. A completed quiet boundary is checked
before timeout, so a legitimate completed checkpoint is not rejected merely
because it took time.

## Requests and state preservation

Computation and drawing polls still request rollback before any OS action.
MENU wins a POWER already pending when MENU is accepted. A fresh manual POWER
accepted afterward survives the MENU settings refresh and is serviced at the
next foreground boundary. USB edge ownership prevents reentry or repeated
requests from the same stable cable. A fresh post-return MENU/USB edge remains
eligible for its own request.

The UI's deferred queue is no longer broadly cleared on MENU completion. Only
an actual POWER wake changes its reset epoch. This retains fresh deferred EXIT
and subsequent MENU while preserving removal of pre-suspend/wake input.

The runtime remains in the same process. `main`, `app_run`, `app_initialize`,
document storage and SAVE/RCL formats are unchanged. MENU does not automatically
save or reload. Current equations, ICs, preferences, incomplete inline drafts,
accepted calculation/recall state and UI stack retain their existing RAM owners.
The v11 SAVE format and v3–v10 migrations remain unchanged.

A forced normal exit/restart was rejected for DIFF EQ: a fresh
`app_initialize()` clears the working App and starts defaults, while RCL is an
explicit user action. Existing SAVE does not serialize every inline draft or
UI/calculation owner. Automatically writing that state would change the manual
SAVE contract. No exit/restart path was added or claimed as validated here.
If CASIO itself starts a new process, the existing defaults/explicit-RCL policy
still applies; this patch does not reconstruct RAM already lost by the OS.

## Evidence and limits

The regression adapters distinguish scanned state, consumed state and queued
UP. The previous actual `power.c` fails because it calls the helper while MENU
is held in those states. The previous actual `common.c` separately fails when
MENU completion discards fresh deferred EXIT. Both assertions pass with this
patch. Additional native cases cover held MENU/EXE, a new press during close,
post-close recheck, frozen clock/scanner, invalid queue, timeout/EXIT cancellation,
new MENU/OFF/USB, no busy wait, unchanged explicit SAVE and existing POWER policy.

The full existing 68-test UBSan suite and strict normal SH compile/package
checks are run on the final source. The linked `_power_osmenu` now arms the
foreground request; the guarded `_power_poll` path owns the one SDK call.
No diagnostic macro is needed for the behavior change. Exact source patch,
linked disassembly, test logs, final artifact hash and rollback hash are in the
primary task's DIFF EQ receipt.

These tests prove the changed software precondition, cancellation and state
contracts. They do not emulate CASIO retention, turn a mocked void helper into
proof of screen success, or establish the current ROM's private early-return
branch. The observed DIRECT-return failure motivates removing this unsafe
input entry condition; that private branch and device outcome remain uncertain.
