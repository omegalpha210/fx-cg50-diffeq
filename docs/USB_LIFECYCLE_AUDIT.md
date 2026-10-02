# DIFF EQ USB lifecycle audit

## Evidence and limits

Audit date: 2026-10-02. **CASE B — HARDWARE TEST REQUIRED.**
No physical USB test has been performed. This is a guarded handoff candidate,
not a demonstrated fix for the reported white flash or failed MENU return.
RAM exhaustion, corrupt files and an OS defect remain unproven hypotheses.
Fake Select Connection Mode UI: **NO**.

The actual local CASIO fx-CG50 Software Guide v3.40 was read, including printed
13-2 (PDF page 446 of 667). The pictured OS labels are `USB Flash`,
`ScreenRecv`, `ScreenRecv(XP)`, and `Projector`; the F3 explanation abbreviates
the XP receiver as `ScreenR(XP)`. The manual says insertion presents Select
Connection Mode, but it may not appear while the busy status icon or a graph,
Geometry object or another figure is flashing. Stop that activity before
reconnecting. Section 13-3 describes the main-memory backup on mass-storage
entry. This warning does not establish the cause of this add-in's symptom.
[Official CASIO guide](https://www.casio.com/content/dam/casio/global/support/manuals/calculators/pdf/004-en/f/fx-CG50_Soft_v340_EN.pdf).

Installed sources inspected:

- gint 2.11, commit `badbd0fd2bd8ac796fd55d49b93691741bd8a139`:
  `include/gint/usb.h`, `mpu/usb.h`, `mpu/cpg.h`, `mpu/power.h`,
  `gint.h`, `fs.h`; `src/usb/usb.c`, `src/kernel/world.c`,
  `osmenu.c`, `src/fs/close.c` and `src/fs/fugue/fugue.c`.
  [Upstream](https://git.planet-casio.com/Lephenixnoir/gint).
- fxSDK 2.11, commit `09e2cf5fdab8d529217203a9a97a93facaa57311`:
  libfxlink's host device/hotplug implementation uses PC-side libusb.
  It is not an add-in plug detector.
  [Upstream](https://git.planet-casio.com/Lephenixnoir/fxsdk).
- libfxcg `Jonimoose/libfxcg` master at
  `daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18`, `include/fxcg/usb.h`:
  USB_Open/IsOpen/Close and transfer operations do not document a connection-mode
  dialog entry point.
  [Header](https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/include/fxcg/usb.h).

`usb_is_open()` reports gint's configured USB connection, set at DVSQ 3.
It does **not** report cable presence. Calling `usb_open()` just to sense a
cable would acquire/configure USB interfaces and interrupts. None of the four
baseline add-ins opens gint USB or links its USB driver. No new USB ownership,
interrupt acknowledgement, register write, driver power-on, syscall number,
signature scan or private OS address was added.

No supported direct Select Connection Mode API was verified in these sources.
`gint_osmenu()` is the documented main-menu world-switch helper; its own
driver save/restore and native implementation remain the installed SDK's
responsibility. Returning from main also finalizes the runtime, but these apps
retain their existing resumable MENU lifecycle. We call neither
`gint_osmenu_native()` from the wrong world nor any copied private syscall.

## Conservative foreground detector

`usb_native.h` reads the installed SDK's **named** `INTSTS0.VBSTS` VBUS field
only when `USBCLKCR.CLKSTP == 0`, `MSTPCR2.USB0 == 0`, and `SYSCFG.SCKE != 0`.
These clock checks follow gint's USB driver source. This is a small read-only
register adapter, **not a public gint cable-status helper**. If registers are
unpowered the result is unknown; the adapter does not enable them.

**Detection availability is unverified.** If the OS leaves the USB block off,
the app may see unknown for its whole run and miss an insertion. Host mocks
prove software handling of a supplied sample, not that a sample will be
available on a physical fx-CG50. An unknown sample never invents an unplug.
The first valid sample establishes a baseline, including launch with a cable
already attached; a known disconnected-to-connected edge requests one handoff.
A real observed unplug rearms detection.

The foreground-only four-boolean latch coalesces USB/MENU/OFF requests, absorbs
insertions during an existing transition and refuses reentry. Connected polling
does not retry a failed save or repeatedly switch worlds. Cable samples do not
increment keyboard activity or reset idle. Normal post-OS settings refresh
retains each app's existing power semantics.

## Four-app comparison

| App | Safe boundary | Persistence | Existing scheduler |
| --- | --- | --- | --- |
| DIFF EQ | Cooperative solver/drawing/table rollback | Explicit SAVE only (owner decision) | Key scanner / UI blink |
| SOKOBAN | Completed synchronous move | Dirty committed progress, A/B | Key scanner |
| NUM GAME | End of bounded CPU/generator/game step | Existing committed state/checkpoint | 250 ms / 16 Hz fallback |
| DIAMOND | AI cancellation or committed replay skip | One committed resume, A/B | 20 ms / 64 Hz fallback |

All four use their existing main-thread OS transition. They share a small source
pattern only, with no new shared binary or copied application framework.
Storage world switches are expected and separate from the **one** OS menu
transition. Host checks distinguish these counts.

## Hardware gate

Run and record each case on the actual calculator, including OS/software
version, whether VBUS detection became available, OS return success, actual
Select Connection Mode appearance, and white-flash recurrence:

| Case | Status |
| --- | --- |
| Idle insertion; modal insertion | HARDWARE TEST REQUIRED |
| Active game / solver / CPU / generator insertion | HARDWARE TEST REQUIRED |
| Committed replay or timed phase insertion | HARDWARE TEST REQUIRED |
| Dimmed insertion; insert plus MENU; insert plus SHIFT+AC/ON | HARDWARE TEST REQUIRED |
| Keep connected without repeated handoff; remove/reinsert | HARDWARE TEST REQUIRED |
| USB Flash with valid save and closed descriptors | HARDWARE TEST REQUIRED |
| ScreenRecv / ScreenRecv(XP) / Projector availability | HARDWARE TEST REQUIRED |
| Repeated original MENU → white flash symptom checks | HARDWARE TEST REQUIRED |

Record OS dialog appearance separately from successful Main Menu return.
A cable reconnection requirement is only a possibility until observed; do not
claim it is universally required. If the detector stays unknown, record the
missed detection rather than treating a manual MENU as automatic success.
No new USB binary release is authorized before this gate passes.

## Project integration

The key scanner callback records physical activity only. Foreground
`power_poll(false)` requests the existing solver/drawing/table cancellation;
the owner restores its transaction before `power_poll(true)` enters the OS.
**The owner explicitly chose manual SAVE only for USB.** Stable RAM is retained;
USB does not save or claim to save a cancelled computation. The existing UI blink
timer is restored through gint's world switch; no USB timer is installed.

Native storage operations remain complete OS-world transactions. gint 2.11
`src/fs/close.c` releases its descriptor even when the underlying close fails.
The local `file_close_guard.h` instead uses the documented `gint/fs.h`
descriptor interface to retain that descriptor until its close callback succeeds.
One bounded cleanup attempt gates MENU/OFF; an unresolved handle prevents handoff.
Failed close cleanup suppresses automatic retry until fresh physical activity or an explicit request, without inventing a USB idle reset. This does not change the save format or explicit SAVE policy.

## Validation record

Baseline: 66/66 UBSan; 250144-byte DIFFEQ.g3a; text 194348, rodata 25836, data 336, BSS 72128 bytes.

## Final local validation

Clean strict host/UBSan: **68/68** (baseline 66/66, none removed). Clean SH compile/link: **zero warnings**, package validation passed. No physical hardware tests were performed.

| ELF section (bytes) | Baseline | Candidate | Delta |
| --- | ---: | ---: | ---: |
| `.text` | 194348 | 195052 | +704 |
| `.rodata` | 25836 | 25836 | +0 |
| `.data` | 336 | 336 | +0 |
| `.bss` | 72128 | 72128 | +0 |

Largest own compiler frame: 2664 bytes (`app_run`); this is not measured total stack high water. Driver-specific sections are excluded from the four plain ELF rows above.

- `DIFFEQ.g3a`: 250144 → **250848 bytes** (+704); SHA256 `2da5decfeb1bc410254ce5b70ef72aa3acc06094666ecb98da1919304e91acd4`.
