# Current memory — v0.12.0-beta.8

| SH bytes | beta.7 | beta.8 | Delta |
|---|---:|---:|---:|
| text |219056|220020|+964|
| data |768|768|+0|
| BSS |72192|72192|+0|
| largest single application frame |2664|2664 (app_run)|0|
| G3A |248504|249468|+964|

No new heap allocation or framebuffer. SH Document grows3040→3044B; App symbol
grows15160→15168B for current/recall, absorbed by linker alignment in total BSS.
The ten-bit v11 mask is appended after the frozen v10 prefix. Drawing borrows
2304B of existing2852B staging tail for three384-pixel rows; six uploads/bar.
Table retains2376B three396-pixel rows,75 initial/seven header uploads.
ui_graph1616B/search1052B remain; IC draft payload cap1920B remains. Single .su
frames do not prove cumulative device stack/heap margin. HARDWARE TEST REQUIRED.
[Complete architecture and validation](VISIBILITY_PROMPT_AUDIT.md).

Earlier milestone records below are historical where superseded above.

---

# Full audit and explicit fixes memory — v0.12.0-beta.7

| SH measurement (B) | beta.6 | beta.7 | Delta |
|---|---:|---:|---:|
| text | 215952 | 219056 | +3104 |
| data | 752 | 768 | +16 |
| BSS | 72192 | 72192 | 0 |
| largest application frame | 2664 (app_run) | 2664 (app_run) | 0 |
| G3A | 245384 | 248504 | +3120 |

The 16 initialized bytes are the installed gint `dwindow` clipping structure,
used by the dedicated Table/Drawing screen's strip renderer. There is no new
production heap allocation, framebuffer, trajectory copy or large BSS object.
The existing `Document.color[10][9]` matrix already stores all per-IC preferences;
independent Output controls and retained colors need no new document/save bytes.
SAVE remains v10. SF normalization does not change storage layout.

Dedicated preparation screens borrow the existing TRACE scratch tail (2852 B
on SH) for three 396-pixel rows, or 2376 B. Each synchronous upload is followed
by byte-for-byte source restoration. The first delayed canvas uses 75 bounded
strips; later frames update only the 21-row header in seven strips. G-Solve and
TRACE retain the existing small lower-left patch. The host-only LCD shadow and
test snapshots are not linked into firmware.

Drawing report/Event staging and display-prefix rollback reuse existing storage.
G-Solve's Event endpoint check adds a small local state vector to its current
search; no search heap or persistent result cache is introduced. Table reuses
its existing local page when only the column or an unchanged row position moves.

Measured single application frames: `app_run`2664, `phase_equilibria`2144,
`model_convert_system`1944, `ui_initial_conditions`1860, `ui_table`1796,
`ui_graph`1616 and G-Solve `search`1052 B. These are individual compiler `.su`
frames, not cumulative runtime high-water. Existing IC draft heap policy is
unchanged. **HARDWARE TEST REQUIRED:** nested stack/library/interrupt/OS use,
heap margin, LCD transfer timing and MENU/Fugue behavior.

Earlier milestone measurements below are historical; beta.7 supersedes the
beta.6 Table/Drawing patch layout and cancellation policy.

---

# Overlay/navigation memory — v0.12.0-beta.6

| SH measurement (B) | beta.5 | beta.6 | Delta |
|---|---:|---:|---:|
| text | 214208 | 215952 | +1744 |
| data | 752 | 752 | 0 |
| BSS | 72352 | 72192 | -160 |
| largest application frame | 2664 (app_run) | 2664 (app_run) | 0 |
| G3A | 243640 | 245384 | +1744 |

Graph-entry snapshot156 B is removed; remaining BSS/data changes are layout and
alignment around tiny overlay flags/row metadata. UiBusy is bounded local metadata,
reusing the existing RTC driver. No new framebuffer, trajectory/result copy, heap
image or per-operation large UI buffer. Inactive scratch remains24096 B; native
TraceSamples21244 B leaves2852 B for transient busy pixels during staging. Host
ABI leaves2840 B. Largest CALCULATING patch114x12 uses2736 B; Drawing81x12 uses1944 B.
Table143x12 uses3432 B in the inactive existing14592 B footer capacity. Buffers are
not simultaneously owned by an active overlay and a busy transaction.

Source pixels are restored immediately after synchronous partial LCD upload.
Selected curve masks and top/bottom19-row restoration reuse existing scratch.
G-Solve uses local results; Y-follow does not copy or recompute those results.
Next largest frames: phase_equilibria2144, model_convert_system1944,
ui_initial_conditions1860, ui_table1796 (+28), ui_graph1588 (+64) B.
The unchanged menu assets remain832 B geometry +8 B palette; PNG exports3378 B.
Single .su frames are not combined stack high-water; runtime heap/stack, interrupts,
OS/Fugue and physical LCD transfer margin remain HARDWARE TEST REQUIRED.

Earlier milestone records below are historical.

---

# Tiles / fixed TRACE memory — v0.12.0-beta.5

| SH measurement (B) | beta.4 | beta.5 | Delta |
|---|---:|---:|---:|
| text | 212548 | 214208 | +1660 |
| data | 752 | 752 | 0 |
| BSS | 72304 | 72352 | +48 |
| largest application frame | 2648 (app_run) | 2664 (app_run) | +16 |
| G3A | 241980 | 243640 | +1660 |

The frozen TraceViewport is 36 B; remaining BSS delta is alignment/layout padding.
Menu data is const/read-only: 756 B coordinate pairs, 60 B stroke records and
16 B ranges = 832 B geometry, plus 8 B pastel palette. Eight 108x34 PNG previews
sum to 3378 B and are not linked into firmware. Menu movement uses no image heap
cache, ODE solve or full redraw; only old/new tiles are repainted.

TRACE's origin cursor/selection uses bounded local metadata. No new framebuffer,
trajectory array or heap allocation. Existing IC draft heap limit stays 1920 B
payload plus allocator overhead. Next frames: phase_equilibria 2144,
model_convert_system 1944, ui_initial_conditions 1860, ui_table 1768, ui_graph 1524 B.
Single .su frames exclude nested calls, libraries, interrupts and OS stack use.
**HARDWARE RETEST REQUIRED:** total memory margin and native refresh/key timing.

Earlier milestone records below are historical; current controls above take priority.

---

# Graph interaction memory — v0.12.0-beta.4

| SH target | beta.3 | beta.4 | Delta |
|---|---:|---:|---:|
| text |206816|212548|+5732|
| data |704|752|+48|
| BSS |71952|72304|+352|
| largest application frame |2568 (ui_graph)|2648 (app_run)|+80|
| G3A bytes |236200|241980|+5780|

Symbol comparison attributes data+48 to the installed gint RTC driver (_drv_rtc),
used read-only for delayed busy feedback. BSS adds the156-byte graph-entry snapshot,
20-byte BOX bounds,8-byte plot identity/ready flag, and replaces the old pointer
coordinates/flag with a176-byte81-pixel local marker patch (plus alignment).
No second framebuffer, trajectory, large static draft array or numeric workspace.
BOX reuses9504 bytes of the inactive TRACE mask for at most4512 saved uint16 pixels,
and its existing14592-byte footer backup. The scratch union size is unchanged.

IC draft strings allocate only edited fields: at most10×192=1920 payload bytes
plus allocator overhead. They are freed on NEXT success, INIT or document replacement;
back navigation intentionally retains them. Allocation failure preserves the current
editor. No session format change. More frames: phase_equilibria2144,
model_convert_system1944,ui_initial_conditions1860,ui_table1768,ui_graph1524 B.
Individual .su frames do not measure nested library/gint/OS/interrupt high-water.
**HARDWARE RETEST REQUIRED:** heap/stack margin, RTC/Fugue, LCD and repeat latency.

Earlier milestones below are historical.

---

# UI consistency memory update — v0.12.0-beta.3

| SH target | beta.2 | beta.3 | Delta |
| --- | ---: | ---: | ---: |
| text | 206064 | 206816 | +752 |
| data | 704 | 704 | 0 |
| BSS | 71952 | 71952 | 0 |
| largest application frame (ui_graph) | 2572 | 2568 | -4 |
| G3A bytes | 235448 | 236200 | +752 |

No persistent allocation, numerical workspace or trajectory/framebuffer copy added.
Help uses bounded normal-font segments; legends/status use small local text and
backplates. No3px selected-curve pass. Remaining measured frames: phase_equilibria2144,
model_convert_system1944,ui_table1768,app_run1336 B. Per-function .su measurements
exclude nested library/gint/OS/interrupt high-water. Physical stack margin and
key-repeat/LCD performance remain **HARDWARE TEST REQUIRED**.

Earlier milestone records below are historical.

---

# UI/UX polish memory update — v0.12.0-beta.2

| SH target | beta.1 baseline | beta.2 | Delta |
| --- | ---: | ---: | ---: |
| text | 204936 | 206064 | +1128 |
| data | 704 | 704 | 0 |
| BSS | 71952 | 71952 | 0 |
| largest application frame (ui_graph) | 2572 | 2572 | 0 |
| G3A bytes | 234320 | 235448 | +1128 |

No new persistent UI state, trajectory/framebuffer buffer or numerical workspace.
The progress label uses4 local bytes; colored help reuses installed dnsize/dtext
metrics and paints token glyphs over the same text positions. Existing stage state
holds ADV context. SAVE confirms before calling the unchanged storage backend.
Individual frames do not measure combined stack/library/interrupt/OS high-water;
physical key latency, stack margin and LCD behavior remain HARDWARE TEST REQUIRED.

Earlier milestone measurements follow as historical records.

---

# Event / Diagnostics memory update — v0.12.0-beta.1

Strict clean SH GCC 14.1 build compared with development e885230 (v0.11):

| Target measurement | Before | After | Delta |
| --- | ---: | ---: | ---: |
| text | 194928 | 204936 | +10008 |
| initialized data | 704 | 704 | 0 |
| BSS | 64896 | 71952 | +7056 |
| Document | 2844 | 3040 | +196 |
| CompiledModel | 8148 | 9072 | +924 |
| largest application frame (ui_graph) | 2572 | 2572 | 0 |

Target sizeof: EventConfig195, EventMarker88, EventMarkers2820, SolverReport2928
bytes. The report includes its 2820-byte marker list; its other metadata is108 B.
A second 2820-byte static list stages TRACE markers for transactional cancellation.
Current/recall Event configuration and one compiled bytecode program account for
the remaining application growth, with linker layout/alignment in aggregate BSS.
No extra trajectory or framebuffer is allocated. Total BSS growth is below10 KiB.

Event refinement keeps one 88-byte marker and a solver result/state in bounded
local workspace, with the bracket/reference metadata. The measured accepted_event
frame (including inlined refinement) is396 B; model_integrate244 B,
integrate_plain96 B, initial_probe48 B, event_value32 B. A scratch call uses the
selected solver workspace (RK45 OdeDopri652 B within its988 B control frame).
There is no recursive Event hook in refinement. The directed initial-zero probe
runs before the canonical integration; it does not nest a second probe.

Other frames: model_convert_system1944 (transactional ODE+Event text),
ui_event1264, ui_solver_info268, app_run1340, phase_equilibria2144, ui_table1768.
The maximum remains2572 B with -Wframe-larger-than=3072 and zero warnings.
Individual frames exclude nested library/interrupt/gint/OS usage and cannot prove
combined high-water headroom. Physical stack margin and responsiveness remain
**HARDWARE TEST REQUIRED**. Full limits and benchmarks: [EVENTS](EVENTS.md).

Earlier milestone measurements follow as historical records.

---

# RK45 memory update — v0.11.0-beta.1

SH GCC 14.1 full build: text194928/data704/BSS64896 bytes. Phase baseline
text187848/data704/BSS64816; BSS **+80**, data **unchanged**.
Target sizeof checks: OdeDopri652 (504 B stages +144 B candidate/error +4 B index),
OdeAdaptive20, OdeWork32, Document2844, CompiledModel8148 bytes.
Workspace is local to the integration call, not a new mutable static allocation.
Coefficient tables occupy448 B read-only. No additional trajectory/cache/framebuffer.
The added BSS is preferences/diagnostics plus linker alignment.

Measured frames: ode_rk45_integrate988, ode_dopri_step100, ui_parameters872;
largest overall ui_graph2572, phase_equilibria2144, ui_table1768 bytes.
A frame excludes its callers/callees; G-Solve can evaluate nested queries from a
sample callback. Bounded frames alone do not establish available stack headroom.
Physical high-water and interrupt/OS stack overlap: **HARDWARE TEST REQUIRED**.
Detailed algorithms and benchmark: [RK45_NUMERICS](RK45_NUMERICS.md).

The earlier audits below describe their respective fixed-RK4 milestones.

# Phase milestone memory update (2026-09-08)

The current P0–P4 build adds **1,824 B BSS** (62,992 → 64,816), with data unchanged
at **704 B**. One static `PhaseResults` holds at most 16 roots (**1,484 B SH**).
Phase window/preferences occupy **72 B per Document**, and current/recall add
144 B. TRACE capture bookkeeping and streamed bounds add approximately 204 B;
linker alignment accounts for the difference in aggregate BSS.

The existing 258-point TRACE storage and overlay/staging union are reused; there
is no extra trajectory, framebuffer, vector grid or contour list. Contours keep
two small sampled rows on the stack. Candidate search stages one result locally.
In the full target build the largest new numerical frame is `phase_equilibria`
**2,144 B**, nullclines **1,192 B**; `ui_graph` is **2,572 B** and is the largest
individual application frame. These exclude nested calls and gint/OS frames;
physical stack high-water remains HARDWARE TEST REQUIRED.

The earlier audit below documents prior milestones. For current trajectory,
storage and cancellation behavior use [Phase acceptance](ACCEPTANCE.md) and
[Phase algorithms](PHASE_NUMERICS.md).

---

# Memory and resource audit

## Current feedback milestone (33bcd2a)

The new validity stream and UI changes reuse the existing TRACE buffers. No new
static allocation is 1 KB or larger, and no trajectory or full framebuffer copy,
Document field, session-record buffer or heap allocation has been added.

| Measurement | 6addf4d baseline | Current SH | Delta |
|---|---:|---:|---:|
| text | 151,384 | 154,712 | +3,328 |
| initialized data | 704 | 704 | 0 |
| BSS | 62,720 | 62,736 | +16 |
| ELF sum | 214,808 | 218,152 | +3,344 |
| Document | 2,920 | 2,920 | 0 |
| App | 13,960 | 13,960 | 0 |
| TRACE cache/mask/footer | 44,756 | 44,756 | 0 |
| app_run frame | 1,328 | 1,332 | +4 |
| ui_trace frame | 1,336 | 608 | -728 |
| ui_graph frame | 72 | 80 | +8 |
| gsolve_input frame | 1,196 | 476 | -720 |
| largest application frame (constants) | 1,928 | 1,932 | +4 |

The cache is 20,660 bytes, mask 9,504 and footer 14,592. TraceSamples' invalid flag
uses existing structure padding. The new highlight XOR value is two bytes; net
BSS rises 16 bytes including placement/alignment. The validity visitor is bounded
stack state (model_path_branch frame 180 bytes). G-Solve Search adds small status/
continuity fields and still retains at most 32 results; its compiler frame is
856 bytes. ICPT remembers a failed secondary branch boundary without copying it.
Document/App layout and v4/v3 saved-record handling remain unchanged.

Current _euram is 0x08110a50, leaving **439,728 bytes** to the installed stack
boundary 0x0817c000, 16 bytes below the prior static address-space margin. This is
not measured free heap or a combined call-chain high-water. Gint's single
177,408-byte VRAM and 16 KiB stack reservation are unchanged. No frame exceeds the
existing 3,072-byte warning limit. Physical stack/library allocation margins are
**HARDWARE TEST REQUIRED**.

TRACE still prepares only one IVP, then moves/blinks without integration. Its
priority/cancellation and timer/repeat cleanup remain. Scalar X/Y-CAL cursor blink
uses one stopped-on-return timer and footer redraw, with no extra integration.
Graph/ZOOM, chooser, IC, navigation and TRACE stress tests remain bounded; host
counters cannot establish OS/library allocation high-water or LCD/key timing.

Evidence: [full SH memory/symbols/frames](build-logs/validity-release-memory.txt),
[target layout probe](build-logs/validity-release-layout.txt),
[full strict build](build-logs/validity-release-native.txt),
[17 host/UBSan groups](build-logs/validity-release-tests.txt).

## Historical measurements below

All following uses of “current” belong to the explicitly named older milestones.
Their field semantics and numerical-invalidity descriptions are superseded by
[WORKFLOW_SPEC.md](WORKFLOW_SPEC.md) and [NUMERICAL_VALIDITY_AUDIT.md](NUMERICAL_VALIDITY_AUDIT.md).

## Historical 6addf4d / 7cd5027 measurements

The confirmed TRACE defect is a repeat-queue saturation path into numerical
cancellation and destructive redraw; see [TRACE_EVENT_AUDIT.md](TRACE_EVENT_AUDIT.md).
No CPU-overload or new RAM-exhaustion cause is claimed. The earlier Fugue reboot
fix is preserved. Physical stack/allocator high-water remains **HARDWARE TEST REQUIRED**.

| Measurement | 06a491e | Current SH target | Delta |
|---|---:|---:|---:|
| text | 146,924 | 151,384 | +4,460 |
| initialized data | 704 | 704 | 0 |
| BSS | 17,776 | 62,720 | +44,944 |
| ELF sum | 165,404 | 214,808 | +49,404 |
| Document | 2,836 | 2,920 | +84 |
| App | 13,792 | 13,960 | +168 |
| app_run frame | 1,332 | 1,328 | -4 |
| largest application frame (constants) | 1,928 | 1,928 | 0 |

New fixed TRACE scratch is **44,756 bytes**: 20,660-byte selected-IVP cache
(129 exact RK4 points per direction, 258 including duplicated IC), 9,504-byte
one-bit curve raster, and 14,592-byte 384x19 footer strip. Tiny event/overlay flags
and alignment account for the remaining net BSS change beyond the App growth.
There is no full framebuffer clone or unbounded trajectory storage. Long traces
retain a coarser Step-aligned sample grid; all curve segments can still be drawn.
Only one selected IVP is cached, so 9 ICs x 9 states do not multiply this buffer.
Scratch preparation changes neither the saved plot nor any complete/partial state;
failed work is not displayed. Overlays are reversed on exit.

Colors add 81 bytes plus target padding to each Document. App's current/recall
increase by 168 bytes; its two-Document load staging still fits the existing
8,116-byte CompiledModel union. v3 target record is 5,696 bytes, v4 is 5,864; both
are streamed. No additional permanent session record or heap allocation is used.

Current ui_trace frame is 1,336 bytes, ui_graph 72, gsolve_input 1,196, V-Window 716,
Parameters 628 and IC 368. These exclude callees. Navigation remains iterative;
TRACE is a bounded Graph mode. No frame exceeds the existing 3,072-byte warning
threshold. The largest reported application frame remains constants at 1,928.

Current _euram is 0x08110a40. Margin to the 0x0817c000 stack boundary is 439,744
bytes, down 44,944 from 06a491e. This is a static address-space margin, not measured
free heap. Gint's single 177,408-byte VRAM and 16 KiB stack reservation are unchanged.
There are no app malloc/calloc/realloc/free/alloca calls. TRACE allocates no heap;
its one blink timer is stopped on exit and its key repeat transform is restored.
Library allocations and physical key/timer/OS timing are not inferred from host.

Evidence: [SH memory/frames](build-logs/feedback-memory.txt),
[target layout probe](build-logs/feedback-layout.txt),
[full strict build](build-logs/feedback-full-native.txt),
[15 UBSan test groups](build-logs/feedback-tests.txt). Host stresses 100 TRACE
entry/exit, blink and curve-switch cycles, 100 chooser and IC cycles, 100 ZOOM/
G-Solve menu cycles, and 1,000 SET/navigation cycles. Native-branch adapters also
check repeat flood/control priority, released-HOLD discard, transform restoration
and balanced timer lifetimes. None of this establishes device runtime high-water.

## Historical measurements below

The remaining sections describe the **06a491e/84aefe0 history**. Their uses of
“current” and no-cache/v3-only descriptions apply to those old milestones and are
superseded by the table and ownership description above.

## Historical 06a491e comparison against 84aefe0 (2026-09-07)

| Measurement | 84aefe0 | Current | Delta |
|---|---:|---:|---:|
| text | 145,048 | 146,924 | +1,876 |
| initialized data | 704 | 704 | 0 |
| BSS | 17,744 | 17,776 | +32 |
| ELF sum | 163,496 | 165,404 | +1,908 |
| App object | 13,792 | 13,792 | 0 |
| app_run frame | 652 | 1,332 | +680 |
| ui_graph frame | 440 | 72 | -368 |
| largest frame (constants) | 1,924 | 1,928 | +4 |

The BSS delta is exactly the eight transformed key events and their count. The
larger dispatcher frame contains bounded per-stage UI state and compiler-inlined
validation/dimension input; it does not grow with navigation. ui_parameters is
628 bytes, ui_initial_conditions 352 and graph_auto_window 260. No full Document,
Session, framebuffer or trajectory copy was added. G-Solve operations remain
bounded synchronous helpers beneath the local softkey menu.

The current _euram is 0x08105ab0, leaving 484,688 bytes to the 0x0817c000 stack
boundary before runtime allocation (32 fewer than 84aefe0). This is not physical
heap/stack high-water. The 16 KiB stack reservation and gint's single 177,408-byte
VRAM are unchanged. The source has no direct heap allocation; no application
allocation accumulates in menu loops. Library allocation high-water is unmeasured.

Final compiler evidence is in build-logs/release-memory.txt. All application
sources passed the strict full SH build. The following sections preserve the
**84aefe0 crash-fix measurements/history**; their then-current sizes and automatic
checkpoint policy are historical, superseded by this table and WORKFLOW_SPEC.md.

## Reproduction method

The release target is configured with `-Os -g -Wall -Wextra -Werror -Wframe-larger-than=3072 -fstack-usage` and a linker map at `build-cg/diffeq.map`. The measurements use the installed SH GCC/binutils rather than host object sizes:

```sh
source tools/env.sh
fxsdk build-cg -c
fxsdk build-cg -j8
sh-elf-size build-cg/diffeq
sh-elf-nm -S --size-sort build-cg/diffeq
find build-cg -name '*.su' -type f
```

`sizeof` values were compiled for `-mb -m4-nofpu`; the macOS host has a different pointer size and alignment and is not used for target accounting.

## Linked image and static RAM

| Measurement | `df7bdc8` baseline | Current target | Change |
|---|---:|---:|---:|
| `.text` | 142,904 | 145,048 | +2,144 |
| `.data` | 704 | 704 | 0 |
| `.bss` | 21,904 | 17,744 | -4,160 |
| ELF `text+data+bss` | 165,512 | 163,496 | -2,016 |

The code increase comes from the iterative dispatcher, checked streaming storage workers and blink handling. Static RAM fell because the 5,696-byte session `Record` scratch and 512-byte CSV line scratch were replaced by a 2,048-byte page-format buffer, a net reduction of 4,160 bytes.

The current principal application objects are:

| Object | SH bytes | Lifetime and ownership |
|---|---:|---|
| `App` (`_app`) | 13,792 | Static for the add-in lifetime |
| `Document` | 2,836 | Current and Recall are members of `App` |
| `CompiledModel` | 8,116 | Unioned with two-Document load staging |
| `ExprProgram` | 900 | Up to nine programs inside `CompiledModel` |
| export `TablePage` (`_csv_page`) | 768 | Static, reused one page at a time |
| export format buffer (`_csv_buffer`) | 2,048 | Static, reused after integration returns |
| session `Record` scratch | 0 | Version-3 bytes are streamed from live Documents |

The installed fx-CG50 linker script maps application static RAM from `0x08101400` and reserves the first `0x1400` bytes of user RAM for gint's VBR. The current `_euram` is `0x08105a90`. Gint places the user stack below `0x0817c000`, reserving 16 KiB at the top of the 512 KiB user-RAM region. This leaves 484,720 bytes between `_euram` and the stack boundary for gint's static-RAM allocation arena before runtime allocations. This is an address-space margin, not a measured free-heap guarantee.

Gint also creates a 350 KiB arena in the OS stack area and then allocates world-save state, driver flags and display memory. Its default color VRAM is 177,408 bytes. These library allocations are outside the application's static object table and are one reason the ELF figures must not be presented as runtime high-water.

## Stack frames and call depth

Largest current application frames reported by SH GCC are:

| Function | Bytes |
|---|---:|
| `ui_constants` | 1,924 |
| `model_convert_system` | 1,752 |
| `gsolve_run` | 1,648 |
| `ui_table` | 1,520 |
| `ui_number` | 1,160 |
| equation acceptance/validation | 1,100 |
| G-Solve search | 992 |
| graph numeric edit helper | 948 |
| form numeric helper | 944 |
| equation inline numeric helper | 940 |
| `storage_save` | 912 |
| `ui_vwindow` | 700 |
| `app_run` | 652 |
| `ui_trace` | 632 |
| `ui_parameters` | 616 |
| `expr_eval` | 584 |
| `ode_rk4` | 516 |
| `ui_graph` | 440 |
| `graph_render` | 380 |
| `storage_load` | 300 |
| native slot probe | 288 |

These numbers exclude callees. The baseline `ui_graph` frame was 2,268 bytes; returning graph sub-screen transitions to the dispatcher reduced it to 440 bytes. The old UI was bounded and non-recursive, but it represented navigation with up to roughly seven simultaneous screen frames. The current fixed-state dispatcher represents parents in `AppNavigation`; repeating a screen path does not add C frames. Host stress checks 1,000 parent-graph cycles and 1,000 actual SET, 100 Graph/checkpoint and 100 equation-family dispatcher cycles.

Parser nesting remains explicitly capped at 32. Expression parsing is complete before RK4 begins, so parser recursion and solver frames do not overlap. Table and CSV formatting occur only after the streaming integration callback has returned. G-Solve retains at most 32 result points.

## Heap, buffers and resource ownership

The application source has no direct `malloc`, `calloc`, `realloc`, `free` or `alloca` call. It stores no full trajectory and makes no framebuffer copy:

- graph rendering retains only the previous streamed point for a clipped segment;
- TRACE and G-Solve redraw the selected curve from the canonical model instead of cloning trajectory data;
- Table computes one bounded page on demand;
- STAT/CSV computes one page, returns from RK4, formats it into the static buffer, and then appends it;
- session save streams the existing v3 header, Documents, padding and checksum without building a duplicate record;
- session load stages two Documents in the union that otherwise holds the compiled model. Save/Load occur only after compiled graph work has ended, and every CALC recompiles the model.

Gint and fxlibc can allocate internal state. Those resources are controlled as follows:

- every session probe/read/write and CSV create/append/remove is a synchronous OS-world transaction;
- each worker closes its descriptor on every path after a successful open;
- no descriptor is retained across a key wait or screen transition;
- a close failure is surfaced, and cleanup does not attempt to remove a path whose native handle might still be locked;
- buffered `FILE` objects are no longer used by application persistence, avoiding their hidden stream buffer and lost `fclose()` errors;
- TRACE and multi-curve G-Solve own at most one timer. The callback only sets a volatile flag; EXIT stops and frees it before return, and G-Solve EXE stops it before the selected numerical operation;
- keyboard and drawing objects are not stored beyond their owning loop, and graph highlighting uses the live `Document` and `CompiledModel` only while both remain valid.

## Remaining hardware measurements

The linker map and host stress tests cannot measure the deepest combined gint/Fugue stack, allocator fragmentation, a failed OS close, physical timer cadence or native display-driver state. The P0 scenarios in [HARDWARE_RETEST.md](HARDWARE_RETEST.md) therefore require repeated calculator runs. A hardware stack-canary or allocator telemetry build would be appropriate only if those tests still reproduce a failure after the filesystem fix.
