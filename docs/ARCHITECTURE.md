# Architecture

The application is native C11. The numerical engine, expression compiler, equation model, graph geometry and UI are independent modules. The target links against installed gint headers and `Gint::Gint`; host tests link the same mathematical core against the host C library.

## UI consistency (v0.12 beta.3)

Shared helpers own semantic softkeys, disjoint normal-font help segments,
field-preserving errors and F5 NO/F6 YES confirmations. Existing solver_custom and
method control AUTO/MAN and h/h0 labels without new model state. graph_labels is
shared by fresh/cached frames; Event squares precede equilibrium markers, with
compact labels/status and active result footers above curves. No numerical loop
or storage protocol changes. Full inventory and rendering defect evidence:
[UI_CONVENTIONS](UI_CONVENTIONS.md).

## UI controls (v0.12 beta.2 baseline)

Main digit/EXE/F6 selection shares one handler. SAVE confirmation owns NO/YES and
calls the existing backend only after a fresh EXE/F6 key. No new app/session state.
Common UI helpers provide cyclic SELECT movement, right-aligned workflow progress,
semantic INIT/ADV softkeys and measured key-glyph recoloring. EDIT/data navigation
are excluded. Parameters retains its existing numerical INIT scope and ADV state;
V-WIN INIT preserves graph appearance/projection. The numerical/storage source is
unchanged. Current controls and row-specific Style exception: [USER_GUIDE](USER_GUIDE.md).

## Events and runtime report (v0.12)

Document appends EventConfig to its frozen v9 prefix. v10 saves current/recall
preferences; all v3–v9 readers default Event OFF. CompiledModel owns one compiled
Event program and shared work counters. events.c dispatches RK4/RK45 and installs
an accepted-state hook before canonical commit. Safeguarded scratch target landing
uses the selected solver without nested Event detection. A directed initial-zero
STOP probes before publishing samples. Runtime SolverReport contains the bounded
32 markers and aggregate counters; a second 32-marker list stages TRACE updates.
No new trajectory/framebuffer allocation or parser is introduced. Event STOP is
valid terminal state in Graph/TRACE/Table/G-Solve; Phase reuses the same marker
states. INFO is read-only and excludes Phase analysis. See [EVENTS](EVENTS.md).

## RK45 dispatch and output (v0.11 baseline)

Document appends OdeAdaptive without changing the old OdeSettings ABI. Frozen
v3–v8 readers set RK4 defaults; v9 stores the new preferences. CompiledModel
owns operation-local work counters. events.c now dispatches to the unchanged RK4
driver or rk45.c, a generic Dormand–Prince 5(4) implementation with a 652-byte
fixed workspace. Adaptive internal steps and the Xdot-based output grid are
separate. Streaming consumers retain their bounded allocations. Table/G-Solve
land at requested x; TRACE cached interpolation remains display-only.
See [RK45_NUMERICS](RK45_NUMERICS.md) for budgets, memory and quantitative tests.

## Modules and data flow

- `src/math/expression.c`: bounded recursive-descent compiler → compact RPN bytecode. Parse once when Parameters GRAPH validates the document, evaluate repeatedly at RK stages. No heap AST and no string parsing in the integration loop.
- `src/ode/rk4.c`: classical RK4 on 1-9 state components, accepting a generic RHS callback. Each step is transactional: a failed stage leaves the previous accepted point intact.
- `src/ode/ode.c`: range/direction/step budget validation, boundary clipping, streaming sample callback, cancellation and stop status.
- `src/ode/validity.c`: wraps trusted sample streams with validity status and NULL continuity delimiters; Graph/TRACE/G-Solve retain valid regions independently of guard failures.
- `src/ode/model.c`: normalizes seven equation kinds to first-order systems and validates mode-specific variables. N-th conversion renames derivative tokens safely.
- `src/app.c`, `src/app_state.c`, `include/app.h`: bounded persistent document, compiled working equation, last-calculation recall snapshot and an iterative top-level screen dispatcher with explicit parent states.
- `src/ui/`: common frames/softkeys, text/token input, forms, graph and paged table. TRACE uses its existing bounded cache/mask/footer with one timer; G-Solve selection streams the canonical curve. Black↔Blue highlighting shares one policy; no full framebuffer clone is added.
- `src/graph/`: coordinate transforms, clipped line segments, axes/ticks/grid/labels and first-order slope fields; the renderer consumes streaming solutions and can redraw one selected Cartesian or phase curve.
- `src/ode/gsolve.c`: enumerates Output-visible IC/component curves and computes roots, extrema, Y-ICPT, Y-CAL, X-CAL and two-curve ICPT from numerical solutions, independently of renderer clipping.
- `src/storage.c`: versioned app-private save/recall plus bounded STAT-compatible CSV. Every target filesystem transaction runs in the OS world through `gint_world_switch()` as required by installed gint.
- `tests/`: analytic solver tests, parser/domain/bounds checks and model/conversion checks; host display adapter for UI verification does not emulate SuperH hardware.

Flow: edit strings/settings → validate complete document → compile expressions → normalize RHS → RK4 point → streaming consumer → next point. Graph, table and CSV reuse this exact path. Errors propagate with an explicit status instead of silently replacing values with zero.

## Representations

- Equation kind: separable, linear, Bernoulli, general first-order, linear second-order, N-th, SYS. Dim is fixed at 1/2 or chosen 1-9.
- N-th state `s[0]=y`, `s[1]=y'`, ..., `s[n-1]=y^(n-1)`; RHS shifts the state and evaluates the highest derivative. In N-th strings `y1` means y', while in SYS `y1` means first state component. `y` aliases the first state in general/SYS.
- Linear second-order RHS: `s0'=s1`, `s1'=h(x)-f(x)*s1-g(x)*s0`. Bernoulli RHS: `g(x)*pow(y,n)-f(x)*y`.
- Compiled expression: fixed opcode/argument arrays plus literal pool; index-based variables. Explicit maximum instruction count, parser nesting and evaluation stack bounds.
- Initial condition: independent coordinate plus up to 9 state values. Up to 9 sets; empty set count is valid only for first-order slope-field-only mode.
- Graph config: finite ordered x/y windows, positive tick scales, derived/editable Xdot coupling, optional grid/axis labels, output mask for each family, phase component choices. Factory grid/labels are on.
- Solver config: ordered finite range, positive finite h, positive integer Step, SF 0-100, bounded maximum step count and magnitude guard. Before an explicit range override, X bounds follow `ceil(view.xmin)`/`floor(view.xmax)`.

## RAM and performance policy

Target application arrays are fixed-size. No trajectory-sized dynamic allocation. The solver keeps a few arrays of at most 9 doubles (72 bytes each) and discards past points after the consumer sees them. A graph consumer retains only the previous point for line segments. A slope field evaluates and draws one small segment at a time.

Measured on the final SH target (32-bit pointers, 8-byte double; target struct alignment differs from the macOS host):

| Object | SH bytes | Allocation |
|---|---:|---|
| Document | 2,920 | Two in static App |
| ExprProgram | 900 | Nine in CompiledModel |
| CompiledModel | 8,116 | Static App |
| App (documents, model/load union, flags) | 13,960 | Static BSS |
| Session Record scratch | 0 | Version-4 record is streamed; v3 read compatibility; load staging reuses the compiled-model union |
| TablePage (8 rows × 10 columns, step/status) | 768 | Table stack; a second instance is static export scratch |
| STAT page scratch | 2,048 | Static BSS; formatting happens after the RK callback returns |
| TRACE selected-IVP / mask / footer | 20,660 / 9,504 / 14,592 | Fixed static scratch |
| Whole linked BSS | 62,736 | Includes application and linked runtime globals |
| Whole linked initialized data | 704 | Additional to BSS |

The largest individual SH application frame is ui_constants 1,932 bytes; model
conversion is 1,752, gsolve_run 1,660, table 1,520 and app_run 1,332. ui_trace is
608, ui_graph 80, scalar gsolve_input 476 and model_path_branch 180. Frames exclude
callees; parser nesting remains capped at 32. Installed gint reserves a 16 KiB
stack, but physical high-water is unmeasured. Parsing never recurses inside RK4,
and export formatting/I/O runs only after the sample callback unwinds.

Reproducible compiler evidence is in `docs/build-logs/validity-release-memory.txt`. CMake emits `.su` files and `build-cg/diffeq.map`, enables `-Wframe-larger-than=3072`, and treats warnings as errors. No application VLA, alloca, trajectory heap buffer, or direct heap allocation is used. gint/fxlibc still allocate their own VRAM, driver and Fugue descriptor structures.

Installed `gint/display-cg.h` clarifies that the OS content rectangle is 384x216, while gint exposes **396x224**. The app uses a centered 384x216 content area. gint owns one 177,408-byte VRAM (+ alignment guards); no complete app framebuffer clone is allocated; TRACE saves only a 19-row footer strip. The installed platform header states that since gint 2.8 the default uses one VRAM (the general display.h contains older triple-buffering wording). The app explicitly calls `dsetvram(gint_vram,NULL)` and draws complete frames. Optional triple buffering would require a second VRAM but is not enabled here.

Line clipping occurs in floating-point window coordinates **before** conversion to integer pixels: gint's dline documentation says very distant off-screen endpoints can be expensive. Fully off-screen segments are rejected, but integration is not stopped merely for being off-screen since a curve can return. Finite magnitude and step-budget limits prevent runaway computation.

Graph redraw and Table use recomputation rather than storing all points. TRACE prepares a bounded selected-IVP cache plus a one-bit raster and footer strip, then moves/blinks without integrating; failed preparation cannot overwrite the complete plot. Table reads collect only the requested page. This trades bounded RAM for CPU; navigation and export remain cancelable. Sampling stride is independent of integration h.

## Error handling and testing boundaries

Invalid syntax, variables out of mode scope, excess bytecode/nesting, division by zero, nonfinite function results, invalid IC/range/h, precision-limited steps, divergent magnitude, and exhausted step budgets are distinct failures. User cancellation is distinguishable from numerical failure. Graph numerical invalidity uses ODE_HAS_INVALID with the cause retained; user cancellation/fatal errors remain separate. Invalid gaps reset line/bracket continuity; searches report only found features in computed valid regions. Table retains its existing incomplete-page behavior.

A fixed-step solver cannot reliably detect every singularity or stiffness-induced error. The documentation recommends smaller h/range and comparison after halving h. Hardware driver timing, keyboard behavior, OS return, physical display, and storage permissions require calculator tests.

G-Solve represents a curve as an IC-family/component pair. ROOT/X-CAL, extrema and ICPT share a bounded candidate/refinement pipeline: scan accepted RK4 points over `solver X ∩ current V-Window X`, detect a sign change in y−target, the selected derivative, or `curveA−curveB`, then bisect while reintegrating from the original IC to each trial x. Both IC-origin branches are scanned, results are sorted/deduplicated, and NULL gaps reset brackets and secondary-curve continuity. A failed secondary ICPT lookup caches its accepted boundary: an inward scan can re-query from that IC on reaching its trusted side, instead of globally rejecting the pair. Y-CAL and Y-ICPT directly integrate to the requested x. No Y viewport test appears in numerical analysis; Y clipping is applied only when a result pointer is drawn. No screen pixels are numerical coordinates, and no trajectory-sized buffer is allocated. A 32-result cap bounds RAM. Tangential roots without a sampled equality/sign change and oscillations faster than h can be missed; solution integrals remain unimplemented.


## Table and persistence details

`src/ode/sampling.c` centralizes direction/range traversal and exact IC-origin queries. `validity.c` adds the optional segmented consumer layer for Graph/TRACE/G-Solve; table keeps the raw stop-status path. `src/ode/table.c` collects only seven requested rows plus one lookahead, applying Step independently of integration. A separate counting callback locates the last page for BTM. Negative pages start at the IC and decrease x; each page is recomputed without a retained trajectory.

`src/storage.c` alternates `DIFFEQ0.dat` and `DIFFEQ1.dat`. Records contain current/recall Documents plus magic, version, target ABI size, generation and FNV-1a corruption checksum. Version 4 appends per-curve colors; the old version-3 prefix/ABI is still read and receives default colors. Invalid color IDs are sanitized. Save streams the header and two live Documents instead of constructing a duplicate Record (v4 target: 5,864 bytes). Load stages two Documents in a union with the compiled model before changing live state. Exact size, checksum, strings, finite fields, masks and state bounds must all validate. Save writes the slot opposite the latest valid record and separately verifies it; the other valid slot remains untouched on write failure. This is an interruption-recovery design, not a claim of physically atomic writes or cryptographic integrity. Version/ABI changes can invalidate a saved record.

Installed gint states that BFile-backed Unix/C99 calls cannot run inside the gint world. Each slot probe, record read/write/verify, CSV create/append and safe cleanup is therefore a synchronous OS-world worker invoked through `gint_world_switch()`. No UI, keyboard polling, integration or display operation runs inside those workers. Low-level calls replace buffered stdio, so the application no longer creates hidden `FILE` buffers or relies on an `fclose()` implementation that discards flush/close errors. Session writes occur only in the explicit Main SAVE dispatcher state; cold startup does not load. RCL explicitly loads after confirmation.

The public installed SDK provides storage-memory files but no supported Main Memory List API. Table → STAT therefore exclusively creates the first free `DIFFSTAT00.csv`…`DIFFSTAT99.csv` for the current IC/direction and selected List columns. Each seven-row page is computed first; only after integration returns does the exporter format it in bounded static RAM and append it in one OS-world open/write/close transaction. The apostrophe label row is ignored by the documented STAT List Editor importer. Data is capped at 998 rows so label plus data stays within its 999-row CSV bound. A partial file is removed only after its descriptor is known to have closed; a close failure is reported without an unsafe remove attempt. See [STAT_EXPORT_AUDIT](STAT_EXPORT_AUDIT.md).

The host display adapter is linked only by `tests/CMakeLists.txt` and supplies the documented subset of display/keyboard APIs using the upstream font atlas. Its framebuffers are development-only. The target directly links gint and contains none of the host framebuffer/command-script implementation.

## Stages and graph menu policy

Equation, IC and Parameters share the same Document. UiStageState retains small
selection/scroll/editor state; it never stores a Document copy. F3 V-WIN at all
stages and Parameters F4 OUTPUT/F5 SET use caller ancestry in AppNavigation. Graph returns to Parameters.
Main EXIT does not return from main; MENU uses the installed gint lifecycle.

Graph softkey menu changes preserve plot VRAM without integration or I/O. Actual
view/operation changes still stream results. TRACE alone has bounded scratch and
reversible overlays; no full framebuffer copy exists. AUTO fits Y within current X and preserves manual Solver bounds.
The 8-event transformed-input queue costs 32 bytes including its count on SH; it
prevents compute/blink polling from discarding pending UI input. MENU caught by
a computation poll is deferred to the next UI boundary. TRACE uses a separate
coalesced intent, EXIT/MENU priority and fixed 8/s repeat profile restored on exit.
See WORKFLOW_SPEC.md and TRACE_EVENT_AUDIT.md.


## Mathematical boundary of invalid segments

Magnitude >1e100, NaN/Inf and domain errors keep distinct causes. The guard and
RK4 step equations are unchanged. Each direction retains its accepted IC-connected
prefix; the opposite direction and other ICs remain independent. RK4 cannot create
an initial condition beyond failure. Consumers accept valid/gap/valid only when a
provider supplies trusted values there; the host analytic fixture exercises this
without claiming a numerical restart. TRACE jumps only among retained finite points,
and exact x=/Y-CAL reject unreachable targets. See [NUMERICAL_VALIDITY_AUDIT.md](NUMERICAL_VALIDITY_AUDIT.md).

## SYS 2D Phase analysis

`Document.view` retains TIME geometry; its phase flag selects the independent
`phase_view` only for a two-variable system. `model_view()` centralizes active
geometry. V8 storage appends phase preferences and freezes v7/v6 readers.
The first SYS2 graph stream also fills existing TRACE staging; successful
completion commits it. Phase projection, AUTO and TRACE consume that same
bounded 258-point cache. Original-stream min/max is retained before decimation.

`src/ode/phase.c` owns bounded vector/nullcline/equilibrium mathematics;
`src/graph/phase.c` owns streamed background layers and one 16-root result.
No numerical phase trajectory is integrated separately. Dry analysis precedes
paint; fixed-point results commit only on success. See [algorithms and limits](PHASE_NUMERICS.md).
