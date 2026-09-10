# Graph overlay and operation navigation — v0.12.0-beta.6

Baseline: development212f3a4 / public57b9079, v0.12.0-beta.5. The actual baseline
rerun passed47 host/UBSan groups, clean28-unit strict SH compile/link, zero warnings
and13 G3A checks. text214208/data752/BSS72352, largest frame2664, G3A243640 B.
Existing toolchain, history/tags and local-only PDFs were preserved.

## Overlay inventory and ownership

Coordinates below are logical384x216 at physical(6,4) in the396x224 framebuffer.
All text uses the existing normal font; glyph data height11 includes descenders.

| Content / owner | Beta.5 location | Current anchor / backing / color | Lifetime and cancellation |
|---|---|---|---|
| END/Partial numerical/domain / GraphResult | (7,4) | GRAPH_MESSAGE(7,4), measured white rectangle(5,2), width+4,height15; red | Persists with canonical plot; UI navigation cannot create Cancelled |
| Event STOP / existing result | (7,4) | Same message channel, neutral UI_INK | Valid termination, not an error; no status reinterpretation |
| G-Solve graph selection / operation-local ordinal | (7,20), differing Select/Curve A/B strings | (7,4), exact common252px instruction; opaque256x15 backing; muted dark text, blue EXE once | Temporarily replaces warning; exact underlying top19 rows restored on EXIT/EXE |
| G-Solve result / local GsolveResults+index | Footer(7,184) | Same lower-left result panel, white(0,179,384,19), blue values/red operation errors | EXIT ends operation only; no solver/report mutation on result navigation |
| G-Solve input / inline edit | Footer X=/Y= | Same data/input panel, blue input and inline red Invalid number | Existing edit validation kept; local operation cleanup restores pixels |
| G-Solve CALCULATING / UiBusy | Footer, full-screen upload | Lower panel text(7,184), small white114x12 max patch, neutral blue | 20 RTC ticks delay; <=8Hz; scratch EXIT discards results, restores graph/menu |
| TRACE values / TracePoint | (8,184), white footer | Unchanged lower data panel | Existing cursor/curve anchor and X-fixed/Y-only follow; EXIT restores footer |
| TRACE unavailable/query failure | Lower data panel | Remains an operation-data result in that dedicated panel | No synthetic coordinate; numerical warning still belongs at top-left |
| TRACE preparation / UiBusy | No feedback | CALCULATING in lower data panel(7,184), small white patch | Existing ui_trace_cancel first; success/failure/cancel remove busy; no new TRACE extension |
| Table index/page preparation / UiBusy | Immediate full preparation frames | Preparing Table at(7,28), white143x12 max patch in preparation area | Delayed; index/page completion paints Table; existing cancel returns safely |
| Drawing / graph_render UiBusy | Immediate footer Drawing hint | Drawing at(7,202), white81x12 max patch in preparation strip | Delayed across actual render/preflight paths; genuine trajectory cancellation retains Partial |
| TIME/PHASE/EVT / read-only Document | Right(4px top) | Same compact top-right status | Capability/enable rules unchanged; never displaced by the left channel |
| Phase N1/N2/frozen-field reference | Compact second row y20 | Unchanged static legend row, measured small backing | Decorative analysis legend, not operation instruction |
| BOX instruction / transaction | Footer y184 | Common top-left channel, EXE blue | Temporarily replaces warning; same inactive overlay scratch; EXIT restores exact pixels |
| AUTO/preflight/Phase notices | Footer or ad hoc row | Common top-left message helper | Acknowledgement/transient key boundary restores underlying pixels |
| EQPT values/classification | Lower163..197 panel | Unchanged dedicated numerical-data panel | Existing analysis selection and phase numerics |
| Graph details/OPTN Auto bounds | Full ordinary UI frames | Outside the Graph-area overlay convention | Existing modal details/bounds workflow; no new numerical capability |

`graph_message` implements instruction/warning/passive-status styles. Active
instruction has priority over warning; it temporarily hides it rather than adding
an inconsistent second warning row. Finishing the operation restores the original
pixels immediately. Results/data remain lower-left; VIEW/EVT stays upper-right.
Neither viewport height nor full-width white headers are introduced.

## G-Solve EXIT root cause and fix

Previously choose_curve called graph_render on every selection/blink iteration,
then graph_highlight_curve performed another numerical stream. ui_cancel inside
these streams could consume a fresh physical EXIT and return ODE_CANCELLED before
the selector read it. That redraw could publish Partial: Cancelled into the plot/
canonical report and leave the selector waiting for another EXIT. Result cycling
and returning to the submenu also called graph_render unnecessarily. Separately,
the native timer branch of ui_blink_key bypassed ui_getkey's held-EXIT filtering.

Now selection and result overlays borrow the inactive TRACE mask/footer storage.
The selector saves the top19 rows; results/input save bottom19. Selected curves
use a reversible mask over existing matching base-color pixels. ICPT results can
highlight both curves with one combined mask. The mask never draws synthetic
segments onto white. Black/Blue swap; other selected colors invert reversibly.
Every close restores pixel-identical underlying graph data without a solve.

All seven ROOT/MAX/MIN/Y-ICPT/X-CAL/Y-CAL/ICPT routes share exactly:

`UP/DOWN: SELECT GRAPH, EXE: SELECT`

It measures252px; EXE is blue, normal, single-painted at physical(183,8). Single
graph operations skip selection. ICPT retains its two graph stages when >2 graphs
exist and automatically uses both when exactly2 exist. Internal IC/variable names
are not renamed. Curve selection ordinal stays local to that UI operation.

Selection EXIT or result EXIT returns to G-Solve submenu; a second fresh EXIT
returns Graph base. Native blink waits and normal UI input ignore held EXIT.
Deferred bounded display redraw keys remain queued for the next UI boundary.
G-Solve scratch cancellation removes busy and returns the submenu without touching
canonical SolverReport or its status/Event markers. Genuine main graph integration/
runtime-extension cancellation can still report Partial: Cancelled.

Ordinary graph renders now fill the already existing258-point cache during their
normal stream, without a second solve or extra storage. If that cache was replaced
or is unavailable, G-Solve can prepare a bounded display-only transaction over the
existing configured range before selection. It reuses existing TRACE staging and
sampling; cancellation leaves cache, canonical report and pixels intact. Partial
display prefixes are not promoted to the canonical SYS2 trajectory cache. No solver
algorithm/status or saved session format changes. Cached highlight/repaint retains
its established finite sample resolution, not a new tolerance-accurate trajectory.

## Graph INIT and result visibility

Graph base F1 stays TRACE; F6 stays yellow/black INIT. F6 Graph INIT, V-WIN F1 INIT
and ZOOM F4 ORIG all call `ui_vwindow_reset`. It is the former V-WIN reset code,
extracted unchanged into the shared geometry unit: factory active-view bounds/
scales, retaining grid/labels/projection/axes, setting Phase readiness when needed,
then existing model_sync_solver_window. TIME AUTO follows the factory X window;
MAN solver bounds remain manual. Equation/IC, method/h/h0/tolerances, Event,
Output/colors and saved state are retained. Both UI entry points remain present.

The156-byte graph-entry snapshot and its capture/restore helpers had no other
consumer and were removed. TRACE F1 retains its distinct entry cursor/curve reset.
BOX remains a reversible two-corner transaction. Graph reset uses matching cached
coverage when possible and the existing safe renderer otherwise; no no-solve promise.

`graph_result_visible_y` takes a window and a result coordinate, not TRACE state.
A finite in-domain result may translate Y only, keeping X bounds/Xscale, Y span/
Yscale, solver settings, result values/index, selected graphs and Event state.
An already safe marker does not move the window. Otherwise it lands at the nearest
vertical safe edge: center y7..171, protecting its9px cross plus a small margin from
the plot edge and fixed result panel beginning y179. Horizontal edge clipping is
retained because X may not move. NaN/Inf, either coordinate magnitude>1e100 and
unrepresentable translations are refused. LEFT/RIGHT result cycling applies the
same helper and repaints prepared display samples; it never reruns G-Solve.
TRACE's10%/30% Y-follow hysteresis and fixed horizontal metadata remain separate.

## Shared busy convention and native upload

UiBusy owns only label/area, the existing clock/frame flags and cancel callback.
All four paths poll cancellation before clock or pixels. Visibility begins after
20/128s =156.25ms; updates are separated by16/128s =125ms. Cycle is `/ - \\ |`.
There is no percentage, timer allocation, interrupt drawing or new timing source.
A fixed rectangle wide enough for the widest spinner avoids residual old glyphs.

Installed gint rtc.h confirms128Hz RTC; display.h exposes11px font data height.
Installed r61524.h and its matching source explicitly support small synchronous
CPU rectangle uploads, waiting for prior DMA first. Native busy uses
`r61524_display_rect`, not a whole LCD upload for each character update. Only this
small CPU-transferred region is changed. Other graph rendering uses normal gint
update paths; no OS/Fugue driver lifecycle is bypassed or replaced.

Each poll saves the small rectangle, draws/uploads it, then immediately restores
VRAM. End uploads the clean rectangle. This prevents scratch cancellation or ongoing
plot streaming from baking a white patch into the graph. During TRACE/graph staging,
the temporary pixels occupy only the unused tail after TraceSamples inside the
existing24096-byte scratch union (host2840 B, native2852 B). Table
uses the inactive existing footer capacity. No new framebuffer or large busy buffer.
Physical transfer speed/LCD sequencing still require hardware testing. Host captures
show renderer VRAM at update time; native partial uploads retain prior LCD pixels
outside that patch until the final ordinary full update.

## Domain policy, tests and review

No broad pale-red domain/singularity x-band is added. A per-curve/state numerical
termination does not establish that every y or other curve is invalid over the
whole remaining x interval. Existing red warnings, skipped invalid segments and
valid-side TRACE/G-Solve are retained. Reliable per-curve invalid intervals would
be a separate future feature.

All47 baseline groups remain. New graph_overlays and graph_overlay_ui cover message
width/anchor/EXE glyph geometry, exact overlay/dual-curve restoration, canonical
report preservation, scratch rollback, all selection/result EXIT paths and held
EXIT, shared defaults/AUTO/MAN, marker Y-only translation/no-op/invalid values,
multiple results without new solves, all four delayed/cancel/cleanup busy lifecycles
and genuine trajectory cancellation. Native-key adapter additionally checks the
actual rectangular-upload call and held-EXIT timer path. No ASan/emulator claim.

[20 current renderer frames](ui-review/overlay-overview.png), five additional3x
previews and updated existing galleries are generated by tools/capture_overlays.py.
Busy frames use deterministic host RTC/poll fixtures; normal key/render handlers
are production code. EN/KO README gallery size is preserved, replacing changed
images only. ACCEPTANCE/MEMORY_AUDIT and Release VALIDATION hold exact build totals.

**HARDWARE TEST REQUIRED / HARDWARE RETEST REQUIRED:** all34 new priority cases
remain pending, including partial LCD upload, slow RK45 cancellation, one-EXIT,
marker/panel visibility, factory reset, MENU/Fugue, SAVE/RCL and memory margin.
