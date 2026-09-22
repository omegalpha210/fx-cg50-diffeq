# System power, failure paths and UI audit — v0.12.0-beta.9

## Result and limits

Baseline: development `1a16b1b`, public `8e6f8f6`, beta.8. Existing local changes,
history, manuals and installed SDK/compiler were retained. The previous59 groups
passed before implementation. Final **63/63 host/UBSan PASS (32.23s)**, clean
**29 C-unit strict SH compile/link, zero warnings/errors**, and **13/13 package
checks** passed. No new parser, RK4/RK45, G-Solve or Event arithmetic was introduced.
One persistence defect was reproduced with a failing assertion and fixed.
No new reboot/crash was reproduced by the executable host tests; this is not a
proof against all expressions, memory states or physical calculator failures.

**HARDWARE TEST REQUIRED:** settings syscall behavior, exact dim brightness,
10/60-minute suspend/resume, wake input, LCD/DMA behavior and physical memory
high-water remain unverified on the device. Host adapters exercise application
policy, not the SH CPU or OS syscall implementation. Previous owner-confirmed
first/second-order device behavior is not a hardware PASS for this change.

## SYSTEM settings and safe suspend

- Read SYSTEM Auto Power Off (10 or60 minutes), Backlight Duration (30s,1min,3min)
  and saved brightness (1–5) at entry, after MENU returns, and after power-on.
  There are no duplicate add-in preferences or new saved fields.
- Every physical press/release, modifiers included, restarts inactivity. A held
  key prevents an idle timeout. Blink redraws, calculations and spinner updates
  are not user activity. New input takes priority over automatic expiry.
- On the duration deadline, dim to the CG50 low-power level below user level1.
  On input, restore the selected brightness. Restore before MENU, power-off or
  normal app shutdown. Temporary brightness changes do not write the saved
  SYSTEM setting. Unknown duration/APO values disable only that action; unknown
  brightness prevents brightness writes rather than guessing.
- Idle waits poll through the installed keyboard API and sleep until interrupts;
  no additional timer or spin loop is installed. RTC ticks wrap at midnight.
- During Graph/Drawing/TRACE/G-Solve/Table/CSV work, expiry or SHIFT+AC requests
  cancellation. The owner restores its stable screen/accepted state before an
  input wait executes `gint_poweroff(true)`. No OS power call is made from the
  asynchronous keyboard filter, interrupt, half-drawn frame or active file worker.
  Non-cancellable OS calls finish before another poll. The timeout therefore
  applies at the next safe cancellation/input boundary, not as a hard interrupt.
- On resume, allow at least one keyboard scan and drain input until all keys are
  released. ON must not become AC-clear in an editor. Clear the UI's pre-suspend
  deferred queue, retain the current draft and redraw the stable screen. This
  preserves RAM for ordinary suspend/resume; SAVE is still explicit, not automatic.

The new native fixture executes the actual `src/power.c` with checked OS-world,
RTC, keyboard/filter, display and sleep adapters. It covers all30 APO/duration/
brightness combinations, exact deadlines, midnight, held keys, input priority,
modifier activity, chained filters, duplicate init/shutdown, deferred and manual
off, wake release, stale events, MENU/settings refresh, unsupported values and
blink timeout. The existing native common.c fixture additionally checks manual
off cancellation and queued-key removal in normal and TRACE input.

## Installed API / primary evidence

The installed gint2.11 (`badbd0fd2bd8ac796fd55d49b93691741bd8a139`) headers and
implementations were checked: `gint.h`, `rtc.h`, `keyboard.h`, `drivers/keydev.h`,
`drivers/r61524.h`, getkey/keyscan, world switching, RTC and R61524 driver. Its
getkey handles manual SHIFT+AC and MENU but does not implement CG50 inactivity
settings. OS workers use `gint_world_switch`; the keyboard filter only increments
an activity serial and chains its predecessor. The poweroff helper performs the
supported OS transition. The four small SH stubs were disassembled after build:
r0 syscall number, r1 `0x80020070`, preserved argument r4, tail jump.

| ABI | Index / units | Primary reference |
|---|---|---|
| GetAutoPowerOffTime |0x1E91 / minutes|[libfxcg header](https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/include/fxcg/system.h), [stub](https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/libfxcg/syscalls/GetAutoPowerOffTime.S)|
| GetBacklightDuration |0x12D9 /30s units|[libfxcg stub](https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/libfxcg/syscalls/GetBacklightDuration.S)|
| GetLightLevel |0x1E8F / saved1–5|[TeamFX original research](https://www.cemetech.net/forum/viewtopic.php?t=11908)|
| Bdisp_SetBacklightLevel |0x0199 / temporary1–5|[syscall reference](https://prizm.cemetech.net/Syscalls/Bdisp_SetBacklightLevel/), [TeamFX](https://www.cemetech.net/forum/viewtopic.php?t=11908)|

Settings choices come from [CASIO's SYSTEM documentation](https://www.casio.com/content/dam/casio/global/support/manuals/calculators/pdf/004-en/f/fx-CG50_Soft_v300_EN.pdf).
The [hardware guide](https://education.casio.co.uk/app/uploads/2022/08/fx-CG50-Hardware-User-Guide.pdf)
specifies a level below brightness1 after duration expiry. Our low-power PWM
choice is an **implementation inference** from the installed driver's level0
value0x14: set OS level1 first to select the corresponding port/register range,
then use public `r61524_set(0x5a1,0x14)` for its checked16-bit/SYNCO write. We do
not call the private brightness helper or toggle a guessed GPIO. Exact visual
matching to SYSTEM is **HARDWARE TEST REQUIRED**. No external library source was
copied or linked; MIT and existing binary dependency notices remain unchanged.

## Reproduced defect: SAVE after a transient read error

**Trigger:** existing valid slot0, changed live document, injected read failure
while SAVE scans the previous generation. Previously the boolean scan treated
I/O failure as no valid slot, chose slot0 and truncated the only known-good copy.
SAVE then reported failure at verification, although the previous save was lost.
This reproduced as a failed byte-for-byte document preservation assertion.

**Fix:** probe/read workers return valid, invalid/missing, or I/O failure. Native
read/seek/close failures propagate separately; open ENOENT remains a missing slot.
SAVE aborts before writing if any preflight probe/read had an I/O failure. RCL can
still recover another fully verified slot, and never replaces the live document
with a failed load. The v11 format, generation ordering and legacy migration stay
the same. The installed Fugue errno mapping was checked.

**Verification:** 40 read failure positions across preflight and post-write
verification; interrupted writes at three positions; open/read/seek/close errors;
short3-byte I/O; old-slot recovery and unchanged live documents; CSV write failure
removes the incomplete export. A post-write verification error can leave the new
complete generation too, but the prior good slot must survive. A close failure is
reported; a possibly locked native handle is not followed by unsafe removal.
Physical media/OS error codes and sudden battery removal remain device tests.

## Error and exit matrix

| Trigger | User-visible behavior | Evidence / recovery |
|---|---|---|
| Empty/partial expression, mismatched parentheses | Syntax error with field/position on NEXT | Expression/model and UI groups; draft retained, EXIT/INIT available |
| Variable outside mode/dimension; unsupported name | Variable not allowed | All mode/dimension mapping tests, parser stress; repair offending field |
| ≥192 input bytes, >32 nested parser depth, code/literal/stack cap | Input limit / Expression too complex | Boundary tests and bounded stress; never recursively parse without a cap |
| Invalid scalar/list IC, >10 entries, nonfinite or >1e100 IC | Inline IC error with retained field | IC/list, full workflow and allocation tests; earlier fields are not partially committed |
| IC draft allocation failure | Not enough memory; draft retained. | Injected first malloc failure; retry/EXIT/re-entry/NEXT retains and then commits text |
| Invalid bounds/scales, SF outside0–50, noninteger setting | Inline range/setting error | V-Window/solver/UI controls; old accepted values preserved |
| RK4 step cannot advance x; oversized step/work plan | h/step/work-limit message, calculation not launched | solver_safety and UI groups; shorten range/increase permitted step/budget |
| RK45 underflow, tolerance floor or work exhaustion | Red numerical status; existing valid region remains usable | RK45/consumer/validity groups; inspect INFO and adjust parameters |
| sqrt/log/division/power domain; nonfinite/magnitude overflow | Domain/singularity, NaN/infinity or magnitude status | Regular/pole/disconnected-domain tests; Graph/Table termination inline; no fabricated continuation |
| Event STOP | Neutral Event status, valid terminal sample | Both methods/directions/Event consumers; not a numerical error |
| G-Solve absent root/extremum/intersection or hidden outputs | Not found / Not available / No visible graph | G-Solve, visibility and full consumers; EXIT returns origin submenu once |
| Invalid X/Y-CAL numeric draft | Invalid number only on EXE | Empty/partial/valid/error EXIT always cancels without validation or extra solve |
| Manual EXIT/AC/MENU or new power deadline during calculation | Cancellation/rollback; no partial accepted graph/report | Native input, TRACE cache, Graph/Drawing/Table transactions; actual suspend at safe wait |
| SAVE I/O/settings/generation error | Save failed. Check settings and storage space. | Fault injection/new preflight fix; current RAM document and previous good generation retained |
| Both save slots missing/truncated/corrupt | No valid saved session found. | Storage/migration/full persistence; live document remains unchanged |
| CSV disk I/O or STAT row capacity | Storage I/O error / step limit, incomplete file removed | Export/cancel/bounded row/fault tests; explicit output operation only |
| No previous calculation, unsupported phase operation | Contextual explanatory notice | Full UI/phase/navigation suites; EXIT returns caller, no recursive app restart |

The40,000 deterministic text cases include high bytes/control characters, random
lengths and mutations of valid expressions:888 compile,39,112 reject. They also
exercise IC parsing and guarded conversion buffers.20,000 corrupt bytecode
programs exercise evaluator bounds. Successful evaluation must be finite. These
are bounded stress tests, not exhaustive fuzzing or SH stack measurements.
All prior59 groups remain enabled, including682 analytic/mapping comparisons,
all dimensions1–9, RK4/RK45, persistence migration, navigation, Event/Phase,
visibility, busy rectangles, transactional cancellation and input lifecycle.

The optional ASan build compiled, but the installed macOS/AppleClang runtime
hangs before main in recursive ASan shadow-memory initialization through dyld.
An independent empty-main ASan control also timed out; process sampling contains
no app frame. The run was stopped and is classified **environment unavailable**,
not a passing memory check or a reproduced app crash. `ADDRESS_SANITIZE=ON` is
available for compatible hosts. Required UBSan and strict target gates passed.
No toolchain was reinstalled or global runtime setting changed.

## UI, memory and release

Long SELECT lists and Output lists show a right-aligned `1 of 10` position in the
existing blue header; short lists omit it. UP/DOWN wrap, PG-/PG+, INIT, colors,
visibility and EXE/EXIT behavior remain. Equation/IC/Parameters keep their1/3–3/3
stage header. No extra generic help line or form PREV was added. Four production
renderer frames were visually checked: [overview](ui-review/power-overview.png).
The previous visibility captures were regenerated for the changed header.

| SH bytes | beta.8 | beta.9 | Delta |
|---|---:|---:|---:|
| text |220020|221920|+1900|
| data |768|768|0|
| BSS |72192|72240|+48|
| largest single application frame |2664|2664|0|
| G3A |249468|251368|+1900|

Power uses a small static state/filter pointer and no heap, framebuffer, extra
trajectory copy or extra timer. Storage adds one serialized worker error flag.
The largest single frame remains app_run2664B; this is not cumulative stack
high-water or proof of free heap under OS activity. IC draft cap1920B is unchanged.
The clean public candidate and exact annotated tag repeat all required gates;
Release VALIDATION.md records exact commit, SHA256 and measured results. Existing
public history/tags and license notices are preserved; private development history,
manuals, paths, logs, toolchains and build files are not in public source.
