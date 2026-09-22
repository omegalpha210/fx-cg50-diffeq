# Current closure — v0.12.0-beta.9

SYSTEM power settings are integrated. A new fault-injection audit reproduced and
fixed SAVE overwriting a good slot after a transient preflight read error.
Long lists gain current-position headers.63/63 host/UBSan, strict29-unit SH and
13 package checks pass; no new reboot was reproduced in host execution.
[Current detailed audit and error matrix](POWER_SAFETY_AUDIT.md) separates native
policy checks, unchanged numerical coverage, optional ASan environment limitation
and pending physical power/LCD/storage tests. Earlier R1/UIR1 decisions stay closed.

---

# Current closure — v0.12.0-beta.8

**R1 and UIR1: Resolved by user decision / Implemented / Validated.**
Independent scalar IC visibility and immediate numeric EXIT cancellation are now
implemented with v11 migration and consumer/key regressions. Drawing uses the
requested Graph-preserving bottom bar. Current open review decisions: **0**.
Final59/59 host/UBSan, strict28-unit SH/zero warnings, package13/13; all previous57
groups remain. [Current complete evidence](VISIBILITY_PROMPT_AUDIT.md),
[34 pending device checks](HARDWARE_RETEST.md). The audit below preserves beta.7
scope, findings and numerical limitations; current beta.8 policy supersedes its
old shared visibility and preparation-page descriptions.

---

# Full functionality and consistency audit — beta.7

## Scope and sequence

The baseline was development `befd016`, public `490b624`, release beta.6, with
49 host/UBSan groups. The four explicit owner-reported fixes were implemented
first: Table preparation, Drawing preparation, SF density and independent
first-order IC colors. They passed 53 host/UBSan groups and strict 28-unit SH
compilation/linking plus 13 package checks before the systematic audit began.
Table same-page reuse then passed its five relevant groups and another SH
link/package check. Existing history, installed toolchain and local references
were retained.

This is an equivalence-class audit, not the full Cartesian product of every mode,
setting and key sequence. Only reproduced defects, incorrect mathematical
results, unsafe state/index handling and explicit convention/consumer propagation
violations are automatically changed. Ambiguous workflows remain review items;
bounded numerical methods and device-only uncertainties are documented limits.
No solver replacement, new Phase feature, stiff method or runtime dependency was
introduced. Negative tests while refining one correction are evidence for that
correction, not additional findings.

Detailed evidence:

- [Mathematical and structural checks](AUDIT_MATH.md)
- [Complete UI/substate inventory and navigation](AUDIT_UI.md)
- [Graph, G-Solve, TRACE, Table, Event and Phase consumers](AUDIT_CONSUMERS.md)
- [Persistence, migration and resource ownership](AUDIT_PERSISTENCE.md)
- [Explicit multiple-IC color architecture](MULTI_IC_COLOR_AUDIT.md)

## Capability matrix

This matrix was defined before systematic fixes. Both methods means RK4/RK45
representatives were executed, not every feature at every dimension and IC count.

| Mode class | Required structure | Methods / IC representatives | Feature classes checked |
|---|---|---|---|
| Separable | 1 state; f(x)g(y) | Both; shared first-order IC path, 1/2/5/10 | Equation/IC/Parameters/SF/Output/Graph/TRACE/ZOOM/BOX/Table/G-Solve/Event/INFO/SAVE/CSV |
| Linear first order | 1 state; g(x)-f(x)y | Both; scalar and shared multiple-IC path | Scalar consumers; coefficient sign and variable scope |
| Bernoulli | 1 state; g(x)y^n-f(x)y | Both; scalar and shared multiple-IC path | Power, subtype conversion, domain limits and scalar consumers |
| General first order | 1 state; f(x,y) | Both; 1/2/5/10 IC | Full scalar workflow and independent trajectory colors |
| Second order | [y,y']; one complete two-value IC vector | Both | Coefficient mapping, y/y' Output, TIME consumers, ICPT and Event terminals |
| N-th | Orders 1–9; n states and n IC values | Both at every order | Derivative indexing, SYS conversion, order 1/9 UI and TIME consumers |
| SYS | Sizes 1–9; m states and m IC values | Both at every size | Variable indexing, sparse Output, SYS 1/9 UI and TIME consumers |
| SYS2 PHASE | Horizontal y1, vertical y2; integration x is separate | Both | TIME/PHASE, TRACE, FIELD, NULL, EQPT, INFO and non-autonomous restrictions |

Direction/domain classes include forward, backward and both branches; off-grid
endpoint landing; regular log/sqrt/tan domains; poles and restricted domains;
trusted disconnected display segments; Event truncation; magnitude, step and work
limits; and cancellation. Boundaries include dimensions 1/9, IC counts 1/2/5/10,
SF 0/1/12/50/51, legacy SF 100, first/last Output rows, Event/G-Solve/Phase capacities
and terminal Table rows.

## Mathematical checks

The new `full_math_matrix` makes 682 analytic/mapping comparisons. Both methods
and directions cover exponential growth/decay, quadratic forcing, harmonic and
hyperbolic second-order equations, a forced second-order equation, scalar subtype
signs, N-th exponential cases at every order, a ninth-order polynomial, unequal
SYS rates at every dimension and coupled harmonic SYS2. Nonzero common x0 and
distinct IC amplitudes exercise all ten families.

Every N-th dimension also evaluates a weighted expression with distinct derivative
values, verifies shift rows, rejects the first out-of-scope derivative, converts
to SYS and compares RHS/results while preserving ICs. This checks indexing
independently of equal-component examples.

For the selected finite intervals, RK4 h=.005 had maximum observed absolute error
1.36156508e-9; RK45 h0=.4, RelTol 1e-9 and AbsTol 1e-12 had maximum 3.50168605e-9.
The general asserted bound was 3e-8, with tighter polynomial/mapping assertions.
These are host observations, not global error guarantees.

Existing tests retain convergence, endpoint equality, adaptive rejection,
tolerance floors, step underflow, magnitude/work limits, trial-stage cancellation,
expression precedence/scopes and malformed input. Event-free accounting checks
RK45 attempts=accepted+rejected and seven RHS calls per attempt; successful RK4
steps use four RHS calls. Event refinement has its own counted work. Table x
order, harmonic radius and conversion mappings are checked independently.

Scalar/second/higher/system definitions were rechecked against the local DIFF EQ
manual; artwork and PDFs remain private. The pinned primary Dormand–Prince
reference and implementation choices are documented in [AUDIT_MATH](AUDIT_MATH.md).
No solver/parser arithmetic defect was established. The separate Event-terminal
G-Solve consumer defect below was reproduced and fixed.

## UI/state consistency and actual workflows

[AUDIT_UI](AUDIT_UI.md) inventories every screen/substate's F1–F6, EXE, EXIT, MENU,
arrows, wrap, EDIT/SELECT, INIT, contextual help, progress, errors and busy behavior.
It covers all eight INIT scopes; Equation/IC deferred validation and first-offender
focus; RK4/RK45 dynamic rows; scalar-only SF; ten-IC Output scrolling; semantic
softkey colors; and single-painted blue EXE.

FUNC, VAR, ADV, Event, INFO, ZOOM, BOX, VIEW, G-Solve, palettes and critical
confirmations receive one EXIT, held EXIT and a second fresh EXIT. Numeric
cursors, Graph pan, Table navigation and fixed-horizontal TRACE remain distinct
from wrapping SELECT lists. Installed getkey repeats arrows by default; ordinary
host scripts are not an OS or physical keyboard emulator.

| Required scenario | Executed end-to-end behavior and evidence |
|---|---|
| A — General / RK4 / single IC | Equation/IC → Graph → TRACE → Table → G-Solve → Output and return; `full_workflow_ui` |
| B — General / RK45 / 5 IC | Five families, IC5 TRACE, final Table columns, G-Solve selection and Output; `full_workflow_ui` |
| C — General / 10 IC + colors | IC10/last-column navigation; every F3 color, INIT, exact Graph/TRACE/G-Solve pixels and SAVE/RCL; `full_workflow_ui`, `output_instances`, `output_instances_ui` |
| D — Second / RK45 / y,y' / ICPT | Both columns and direct two-curve ICPT; Event-terminal intersections under both methods in `full_consumers` |
| E — N-th9 / RK4 | Physically enter x0 plus nine ICs → Graph, last-derivative TRACE and final Table/Output columns; `full_workflow_ui` |
| F — SYS9 / RK45 | Physically enter nine RHSs/nine ICs → Graph, last-state TRACE/Table/Output; no hidden SF; `full_workflow_ui` |
| G — SYS2 / PHASE / FIELD / NULL / EQPT | Projection, independent layers, equilibrium/classification, INFO and VIEW return; `full_workflow_ui`, Phase groups |
| H — Event MARK | General/RK45 configuration → Graph EVT → Table endpoint → INFO; dedicated Event tests also cover both directions/methods |
| I — Event STOP | Neutral Graph/Table terminal, valid TRACE and G-Solve endpoint domain; `full_workflow_ui`, `full_consumers`, Event groups |
| J — Restricted numerical domain | Sqrt warning with valid TRACE/Y-ICPT and Table terminal; log/pole/tan/large-exponential classes in numerical/consumer tests |
| K — SAVE → cold state → RCL | Explicit save, fresh process without implicit load, confirmed five-IC/RK45/color recall and no runtime diagnostics; `full_workflow_ui`, `full_persistence_matrix` |

Renderer review includes both full busy screens, SF 50 and invalid SF, single/
five/ten-IC Output, independently colored curves, G-Solve selection and maximum-row
screens. It caught the caption overlap UID3. The final review checked 16 native
captures and four integer enlargements independently, with no remaining visual
blocker. [Native captures](ui-review/) show software-rendered frames and intended
LCD transfers, not physical contrast or refresh speed.

## Persistence checks

`full_persistence_matrix` covers 46 mode/dimension/method cases. Each saves
different current/recall documents, initializes a cold App, explicitly loads,
compares every Document byte and recompiles. Two truncated slots fail without
replacing live documents. Existing tests retain alternate-slot fallback, malformed
headers, checksum/length/version validation and short-I/O/cancellation cleanup.

The inventory includes equations/power, ten IC slots/nine states, RK4/RK45
settings, hidden Step, SF, AUTO/MAN range, complete TIME/PHASE windows, projection/
axes, Grid/Label, Output mask/all 90 color bytes, field appearance and Event.
Active and inactive preferences persist. No missing stored preference was
reproduced. TRACE cursor, temporary results, compiled expressions and diagnostics
remain intentionally unsaved.

Version 10 is retained. Frozen v3–v9 readers remain compatible: SF above 50 clamps
on every v3–v10 load path; v4–v6 preserve nine stored color rows and default the
absent tenth; v3 defaults all colors. SAVE/load reuse the existing CompiledModel
staging union; UI routes recompile before calculation. Last-calculation publication
after Drawing is corrected by UID2.

## Edge cases, invariants and memory ownership

- Window bounds/scales remain valid; G-Solve Y-follow leaves X unchanged. TRACE
  retains entry X bounds/scale/Xdot and follows only connected visible samples.
  Recorded invalid gaps are never connected by a line/root.
- G-Solve validates ordinals/common domains and sorted finite bounded results.
  Event endpoints are accepted while beyond-STOP queries are rejected. Constant
  1e-90, unrelated events, large offsets, huge viewports and curved near-endpoint
  counterexamples guard against broad false-root allowances.
- Event direction means increasing x in both integration directions. Rejected
  RK45 trials cannot trigger. Initial/plateau roots, invalid Event gaps, refinement
  cancellation and the 32-marker bound remain covered.
- Table x is strictly ascending, terminals are trusted, missing cells explicit.
  Sparse SYS9 and ten-IC columns share CSV/STAT mapping. All outputs OFF keeps x;
  legacy nic=0 safely has no Table trajectory.
- Capacities remain nine states, ten ICs, 192-byte expressions, 258 display points,
  32 G-Solve results, 32 Event markers, 16 equilibria, seven Table rows, eight queued
  events and 998 STAT data rows.
- IC colors reuse 90 existing bytes. Busy painting borrows the unused staging
  tail, uploads at most three full-width rows at a time and restores VRAM. There
  is no additional framebuffer/trajectory buffer. Event-terminal search state
  is local to its existing G-Solve operation.
- Application heap use remains retained IC drafts: at most ten 192-byte strings
  plus allocator overhead. Allocation failure retains the editor; NEXT, INIT and
  replacement release drafts. Navigation is iterative.

| Measured firmware resource | beta.6 | Final beta.7 development | Change |
|---|---:|---:|---:|
| text | 215,952 B | 219,056 B | +3,104 B |
| data | 752 B | 768 B | +16 B |
| BSS | 72,192 B | 72,192 B | 0 B |
| Largest single stack frame | 2,664 B | 2,664 B (`app_run`) | 0 B |
| `.g3a` | 245,384 B | 248,504 B | +3,120 B |
| New application heap allocations | — | None | 0 |

Final frames also include `ui_graph` at 1,616 B and G-Solve search at 1,052 B.
The initialized 16-byte increase is gint display-window state; no BSS framebuffer
was added. Compiler frame size is not cumulative stack high-water. Details and
ownership are in [MEMORY_AUDIT](MEMORY_AUDIT.md).

## Explicit fixes and auto-fixed audit issues

Four explicit requests are counted separately from systematic findings:

| Explicit request | Completed behavior |
|---|---|
| A — Table preparation | Delayed native blue Preparing Table... spinner header, white EXIT cancels row/body, hidden softkeys, cancel-first polling and stable return. Prepared same-page data is reused; quick work does not flash the screen. |
| B — Drawing preparation | Same native layout with Drawing..., delayed four-frame spinner and hidden softkeys. Temporary transfer pixels are restored; cancellation does not commit a corrupt plot/report. |
| C — SF density | Default 12; valid 0–50; UI rejects 51; model/renderer defend the bound; old 51/100 load as 50 without a format bump. |
| D — Independent IC colors | Existing matrix exposed as IC1 y…IC10 y; separate shared visibility; per-row F3, unchanged default cycle, INIT and deterministic shrink/grow/SAVE/RCL. |

Systematic AUTO-FIX findings:

| ID | Severity | Reproduction/root cause | Correction and evidence |
|---|---|---|---|
| CONS-1 | P1 | Event STOP's tiny endpoint residual lacked a later sign change; same-family ICPT could reject or lose the trusted stopped state during refinement | Accepted same-family state and local Event x precision for ROOT/X-CAL/ICPT terminals, with STOP boundary and false-positive guards; `full_consumers` and retained domain/Event tests. |
| UID1 | P2 | RK45 Table advertised inactive Step7 although spacing was 0.033333333 | Read dx from the existing Table index; RK4 Step/numerical grid unchanged; failing-before/passing-after `full_workflow_ui`. |
| UID2 | P2 | Cancelled RHS2 Drawing replaced Last calculation after only RHS1 completed; first cancel also created recall | Commit existing Recall only after accepted first Graph, preserve valid partial/Event results and old/no-recall state on cancellation; `full_workflow_ui`. |
| UID3 | P2 | New shared visibility caption overlapped the fixed colon/ON column | Shorten to y (all ICs), assert real-font width at most 104px, retain help/semantics; Output tests and renderer review. |
| GRAPH-1 | P1 | A completed non-cancelled partial pan committed a new raster/report/cache, then restored old geometry and showed a Phase-prefixed error in TIME; dry preflight could also replace the canonical report without painting | Keep completed valid-prefix geometry with its result; handle cancellation first; dry preflight returns without report mutation; name notices for the actual view. Five relevant UI/Phase/overlay/busy groups pass. |

GRAPH-1 was found during final independent review. Second/RK45 with Max steps 10,
then Graph RIGHT, now retains the valid Step-limit prefix and its new window
(-3.78 to 8.82), report and cache together. Reopening V-Window does not require a
new solve or change its report. Conversely, RK4 Max steps 10 with X window
-1.5 to 1.5 rejects an over-budget Graph INIT before painting and preserves the
old window/report/cache; the notice says Graph rather than Phase. A direct C
fixture checks dry plan/phase preflight pixels and report. Completed OK,
HAS_INVALID, EVENT_STOP or nonzero-step prefixes are accepted, while cancellation
is handled before that decision. The five targeted groups passed in 2.49s before
the final whole-project rebuild.

Legacy tenth-slot color initialization belongs to explicit D; same-page Table
reuse belongs to explicit A. Neither is counted again. Intermediate negative cases
while refining CONS-1 are not separate defects.

### Finding totals and release disposition

| Actionable systematic findings | P0 | P1 | P2 | P3 | Total |
|---|---:|---:|---:|---:|---:|
| AUTO-FIX defects, all fixed | 0 | 2 | 3 | 0 | 5 |
| Former review decisions (both closed in beta.8) | 0 | 0 | 2 | 0 | 2 |
| Total defects/review decisions | 0 | 2 | 5 | 0 | 7 |

The four explicit requests are separate work. Five INFO themes below are
intentional limitations, not extra defects assigned an artificial severity.
Every reproduced P0/P1 defect passed its regression and final development gates;
none remains unresolved in the executed matrix. Device success is not implied
by these counts.

## Former review decisions — closed in beta.8

| Finding | Decision and implementation | Status / evidence |
|---|---|---|
| R1, P2, scalar IC visibility | Owner approved independent ON/OFF. Appended v11 ten-bit preference; common visible-curve predicate; output/export filtering; hidden numerical/Event data retained. | Resolved by user decision / Implemented / Validated: trajectory_visibility, output_instances, field_storage, full_persistence_matrix, visibility_prompt_ui. |
| UIR1, P2, G-Solve numeric EXIT | Owner approved unconditional temporary-operation cancel. Separate EXIT branch before validation; retain page2 and HOLD boundary; EXE alone commits. | Resolved by user decision / Implemented / Validated: visibility_prompt_ui and retained G-Solve/UI/key lifecycle groups. |

Root causes, exact migrations, controls and regression cases are in
[beta.8 audit](VISIBILITY_PROMPT_AUDIT.md). No open review remains for R1/UIR1.
The preceding beta.7 matrix and severity counts are historical baseline evidence.

## Known intentional limitations / INFO

Five themes consolidate overlapping detailed-report notes:

1. **Discrete sampling cannot certify a complete domain** (`MATH-I1`). RK4 can
   miss an unsampled pole: y'=1/(x-.5), x0=1, h=.005 may finish numerical steps
   although no continuous IVP solution crosses x=.5. An exact-hit step stops;
   audited RK45 shrinks/stops. Neither guarantees finding every singularity.
   COMPLETE means finished numerical work, not certified domain validity. No
   symbolic pole detector, restart across failure or domain shading was added.
2. **Local tolerance and bounded display samples differ** (`MATH-I2`, `CONS-I2`).
   RK45 controls a local estimate, not a global guarantee. TRACE/rollback may
   interpolate connected retained samples; numerical Table/G-Solve lands on
   requested x. One cancelled-prefix repaint differed from the original raster
   by two pixels while matching its committed cache and preserving state/report.
3. **Bounded stores are not exhaustive discovery proofs** (`CONS-I1`, color
   capacity note). Result/Event/Phase capacities are 32/32/16; coincident curves
   can have infinitely many intersections and only bounded representatives.
   Six palette colors repeat across ten independently editable preferences.
4. **Other wait paths need device timing before further UX changes** (`CONS-I3`,
   `UII1`). STAT/legacy OPTN Auto have static preparation; ZOOM AUTO/Phase EQPT
   retain cancellation. Host Phase benchmarks were sub-millisecond for the chosen
   cases, which says nothing definitive about SH latency. No speculative spinner
   or false percentage was added.
5. **Existing mathematical feature scope remains** (`CONS-I4`). Phase equilibrium
   and classification are bounded/local, not exhaustive proofs. Non-autonomous
   trajectories are allowed; FIELD/NULL use reference x and EQPT is disabled.
   No stiff solver, multiple events, failure continuation, Poincare or bifurcation
   feature was introduced.

Persistence power-loss guarantees remain limited to the alternating-record
protocol; simulated I/O cannot prove physical Fugue behavior. Device-only gaps are
grouped below rather than counted as new bugs.

## Validation

After GRAPH-1 and every other source correction, the complete development suite
passed **57/57 host/UBSan groups in 32.21s**. The final clean native build passed
**28 strict C units with zero warnings**, completed SH linking and passed **13/13
package checks**. The resource table above records this post-fix build. All prior
numerical/parser/UI/storage/cancellation regression groups remain enabled; the
new groups cover the explicit fixes and matrix gaps rather than duplicating the
same implementation assertions to increase the test count.

The public process must rebuild/test the clean candidate and exact tag source,
then redownload assets and compare bytes/SHA256/GitHub digest. Exact public commit,
tag, checksum and independent release results belong in release VALIDATION.md;
development gates alone are not completed public verification.

## Hardware-only checks

**HARDWARE TEST REQUIRED:** actual key repeat/release, MENU/Fugue/filesystem
behavior, RTC cadence, LCD uploads/contrast, long-expression cancellation latency,
SH numerical execution, cumulative stack high-water and heap fragmentation.
Compiler single-frame sizes and host timing do not replace these measurements.

[HARDWARE_RETEST](HARDWARE_RETEST.md) contains 27 pending current cases, beginning
with the requested Table/Drawing
layouts, all spinner frames, hidden softkeys/cancellation, SF 50/51 and legacy
clamping, five/ten independent IC colors and their Graph/TRACE/G-Solve/SAVE
consumers, full workflows, Event-terminal G-Solve, cancelled-Drawing Recall,
partial-pan/preflight consistency and MENU/Fugue. Record exact release SHA256,
device/OS, inputs, keys and observations.
