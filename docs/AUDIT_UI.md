Current beta.8 changes are authoritative in [VISIBILITY_PROMPT_AUDIT](VISIBILITY_PROMPT_AUDIT.md).
R1/UIR1 are resolved by owner decision, implemented and validated. Drawing now
keeps Graph with a bottom bar; Output has per-IC ON/OFF and color; SAVE is v11;
numeric EXIT cancels immediately to G-Solve page2. The beta.7 report below is
historical evidence, including its former review choices and original controls.

---

# UI and state audit — beta.7

The explicit Table/Drawing, SF and IC-color fixes passed 53 host/UBSan groups,
strict SH compilation/linking and package checks before this systematic audit.
This document records the subsequent source inventory and representative actual
app workflows. It does not claim every Cartesian combination or physical key/LCD
verification. **HARDWARE TEST REQUIRED** applies to all device behavior below.

## Shared rules checked against the implementation

- The logical content is 384×216 at physical offset (6,4) on a 396×224 framebuffer.
  Forms use a 21px blue header, 22px field spacing, at most seven ordinary rows,
  one contextual help line and the existing 18px softkey strip. Equation adds its
  general formula above six fields. The ninth Equation/IC/Output entries therefore
  use another page, never a row underneath the help.
- MENU is handled by installed gint `getkey()`/`GETKEY_DEFAULT`, which enables
  OS return/redraw and arrow repetition. Computation polling temporarily disables
  immediate MENU handling, retains the key and returns to the normal UI path.
  Native key tests cover the app's queue/transform policy; actual OS switching is
  not simulated by the ordinary host key source.
- Fresh EXIT leaves exactly the layer described below. `ui_getkey` and the native
  blink path discard held EXIT. Critical confirmations and BOX additionally reject
  held EXE/F6 acceptance; G-Solve selection/results reject their transition repeats.
  Native default getkey repeats arrows, not arbitrary F-keys. Synthetic held
  F-keys outside explicitly guarded states are not evidence of a device bug.
- SELECT UP/DOWN wraps only visible choices. Graph pan, text cursors, BOX corners,
  Table pages/columns and numerical TRACE traversal remain bounded movement.
  Curve/result selectors have their existing cyclic selection semantics.
- Plain forms: SELECT EXE invokes the primary F6 action. EDIT EXE commits and
  advances, except the last edited row stays until another EXE. EDIT EXIT commits
  and stays selected. Output retains its list-specific EXE-next-row policy.
- Equation/IC allow incomplete drafts during editing, row movement and EXIT;
  NEXT validates all required fields and focuses the first error. Parameters,
  V-Window and Event retain their existing validation-on-commit policy.
- Stage 1/3, 2/3 and 3/3 appear only on Equation, IC and ordinary Parameters.
  FUNC/VAR and ADV suppress stage text; auxiliary screens never acquire it.
- INIT yellow/black; ADV black/white; V-WIN orange/black; SET green/black;
  NEXT cyan/black; PREV magenta/black; GRAPH/RUN red/white. NORMAL/FAST/FASTER and
  the COLOR label keep their existing specialized styling. The retained EXE help
  token uses the ordinary font, blue, once; only the Main MENU token is red.
- Input errors remain with the field. Numerical limits remain red over the valid
  Graph/Table region; Event STOP remains neutral. G-Solve selection temporarily
  owns the top-left message channel and restores the prior warning.

## Screen and substate inventory

In the table, F-keys are always listed in F1…F6 order; `—` is blank/inert.
MENU follows the shared native policy above in every row. “Form” inherits the
SELECT/EDIT rules above; explicit differences are recorded. “None” means no
additional INIT, progress, error panel or busy animation is present.

| Screen/substate | F1 / F2 / F3 / F4 / F5 / F6 | EXE / EXIT | Arrows and wrapping | INIT, help, stage, errors and busy |
|---|---|---|---|---|
| Main tiles | — / — / — / — / — / OPEN | Open selected tile; EXIT stays Main | 2D row/column wrap; 1–6 shortcuts | No INIT/stage/busy; MENU hint; partial focus repaint |
| First-order subtype tiles | — / — / — / — / — / OPEN | Open subtype; EXIT Main | Same 2D wrap; 1–4 shortcuts | No INIT/stage/busy; existing MENU hint |
| N-th/SYS size SELECT/EDIT | — / — / — / — / — / OPEN | Form; EXIT SELECT Main | Single selected field; bounded EDIT cursor | 1–9 hint; invalid size inline; no stage/busy; replacement confirm when user data exists |
| Equation SELECT | INIT / — / — / — / — / NEXT | NEXT validates all; EXIT subtype/size/Main parent | Visible equation fields wrap | Equation-only INIT; LEFT/RIGHT edit hint; stage 1/3; first invalid required field inline |
| Equation EDIT | INIT / FUNC / VAR when supported / — / — / NEXT | Commit draft/next row; EXIT commits draft/stays | Bounded caret; UP/DOWN commits draft and moves without edge wrap | Same scoped INIT; EDIT hint; stage 1/3; incomplete drafts allowed until NEXT |
| Equation FUNC/VAR | Up to five tokens / page or BACK | Ordinary key returns to editor; EXIT closes token bar only | Editor cursor behavior resumes after closing | No separate INIT; token bar owns help/stage suppression; unsupported tokens inert |
| IC SELECT | INIT / — / — / — / — / NEXT | Validate all required values; EXIT Equation | Visible rows wrap; scalar list is one field | IC-only INIT; scalar/list or full-state hint; stage 2/3; first bad field inline |
| IC EDIT | INIT / — / — / — / — / NEXT | Commit draft/advance; EXIT commits draft/stays | Bounded caret and committed row movement | Incomplete draft retained; 10-item/191-character bounds; INIT usable during error |
| Parameters SELECT | INIT / ADV / V-WIN / OUTPUT / SET / GRAPH | GRAPH; EXIT IC | Method LEFT/RIGHT toggles; numeric rows enter EDIT; visible rows wrap | Method-preserving numerical INIT; context per row; stage 3/3; AUTO/MAN reads shared flag |
| Parameters EDIT | Same parent actions | Validate/commit before action; invalid value stays EDIT | Bounded caret and row movement | Positive h/tolerances, work and SF limits inline; no calculation or busy from merely editing |
| ADV | EVENT / INFO / — / — / — / — | EXE inert; EXIT Parameters | Arrows inert | No INIT/stage/busy; settings remain visible; no solver or I/O side effects |
| Event SELECT | — / — / — / — / — / DONE | DONE; EXIT ADV | Enabled/direction/action LEFT/RIGHT; four-row SELECT wrap | No INIT/stage; toggle or E scope hint; empty E is allowed while disabled |
| Event E EDIT/FUNC/VAR | VAR if supported / FUNC / — / — / — / DONE, or token bar | Validate E and commit/next; EXIT commits/stays; token EXIT closes only bar | Bounded caret; row movement after commit | EDIT hint; expression error inline; enabled event/IC validation before Graph |
| Solver INFO | — / — / — / — / — / — | EXE inert; EXIT ADV | Bounded UP/DOWN scrolling, no wrap | No INIT/stage/busy; last-trajectory snapshot or No solver run yet; dynamic method/Event rows |
| V-Window SELECT | INIT / — / — / — / — / DONE | DONE; EXIT caller Parameters/Graph | Seven-row wrap; LEFT/RIGHT EDIT | Factory active-window INIT; Xdot relation hint; no stage |
| V-Window EDIT/numeric FUNC | — / FUNC / — / CLEAR / DEL / OK; picker uses OPEN/page keys | Validate/commit; EXIT commits/stays; numeric picker EXIT returns editor | Bounded caret; picker SELECT wraps | Range/scale/Xdot errors inline; no broad window reset or numerical solve on editor visit |
| Output single IC or component rows | INIT / — / COLOR / — / — / DONE | Next row; last modified row stays once, then DONE; EXIT Parameters | UP/DOWN wrap; LEFT/RIGHT shared component ON/OFF | Output-only INIT; one-line hint and line previews; no stage/busy |
| Multiple-IC shared visibility row | INIT / — / — / — / — / DONE | Existing list-next policy; EXIT Parameters | LEFT/RIGHT toggles all ICs; visible rows wrap | `y (all ICs)`; explicit shared-visibility help, no ambiguous color preview |
| Multiple-IC color rows | INIT / — / COLOR / — / — / DONE | Existing list-next policy; EXIT Parameters | UP/DOWN scroll/wrap; LEFT/RIGHT inert | IC1 y…IC10 y; F3 COLOR hint; no independent ON/OFF value; exact stored preview |
| Graph Settings Grid/Label | INIT / — / — / — / — / DONE | DONE; EXIT caller | LEFT/RIGHT toggle; visible rows wrap | Grid/Label/field-appearance INIT; no stage/busy; values retain current view scope |
| Graph Settings Style/Color | INIT / — / COLOR only on Color / — / — / DONE | DONE; EXIT caller | Style LEFT/RIGHT; Color RIGHT/F3; rows wrap | Only scalar-first-order appearance rows; INIT on every row; concise context |
| Curve/Field palette | — / — / — / — / — / — | Apply chosen color; EXIT cancel to exact parent row | Bounded 2×3 arrows, no wrap | No INIT/stage/busy; EXE SELECT / EXIT cancel; six names/swatches |
| Graph TIME base | TRACE / ZOOM / V-WIN / TABLE or SYS2 VIEW / G-SLV / INIT | EXE inert; EXIT Parameters | Continuous pan, +/- zoom; no selection wrap | Factory active-window INIT; top-left status; TIME/EVT only when applicable; delayed Drawing for real work |
| Graph SYS2 PHASE base | TRACE / ZOOM / V-WIN / VIEW / ANLYS / INIT | EXE inert; EXIT Parameters | Phase-state pan/zoom | Same active-window INIT; PHASE/EVT; independent window and existing phase labels |
| ZOOM | IN / OUT / AUTO / ORIG / BOX / — | EXE inert; EXIT Graph base | Pan remains available | ORIG shares factory reset; AUTO failure uses compact graph notice; expensive operations retain cancellation |
| BOX point 1/point 2 | — / — / — / — / — / — | Fresh EXE sets first/commits second corner; EXIT cancels to ZOOM | Bounded 4px cursor, arrow repeat, no wrap | Top-left point guidance; too-small selection inline; no window mutation before valid second EXE |
| TRACE preparing | Previous graph controls until delayed lower indicator | EXIT cancels; no entered cursor committed | Pending arrows retain existing queue policy | CALCULATING... spinner after delay; no full-screen prep; canonical graph preserved |
| TRACE active | INIT / NORMAL / FAST / FASTER / LEFT / RIGHT | EXE inert; EXIT Graph base | UP/DOWN curve selector; LEFT/RIGHT fixed-horizontal visible connected trajectory; endpoint F5/F6 clamp | Cursor/curve-only INIT retains speed; coordinates/status in lower panel; Y-only follow |
| G-Solve menus | ROOT / MAX / MIN / Y-ICPT / ICPT / >; Y-CAL / X-CAL / — / — / — / < | EXE inert; EXIT Graph base | Pan stays available | No INIT/stage; exact existing operations; single-curve selection skipped |
| G-Solve curve selection | — / — / — / — / — / CANCEL | Fresh EXE selects; EXIT/F6 G-Solve submenu | UP/DOWN curve selector wraps; excluded ICPT curve skipped | Exact common top-left prompt; cached reversible highlight; warning restored; no selection solve |
| G-Solve numeric EDIT/SELECT | — / — / — / — / — / RUN | EXE/F6 validates/runs; EXIT EDIT commits/stays, next EXIT cancels | Numeric caret only | Lower x=/y= entry and Invalid number; no stage; existing EXIT-commit policy reviewed below |
| G-Solve calculating | Previous operation controls | Cancel first through numerical callback | Queued keys retain shared policy | Delayed CALCULATING... lower indicator; scratch cancellation does not replace canonical report |
| G-Solve result/notice | — / — / — / — / — / BACK | Fresh EXE/F6/EXIT returns submenu | LEFT/RIGHT result selection only | Fixed lower numeric panel; minimal Y-only marker follow; graph status retained; no repeated numerical search |
| SYS2 VIEW | TIME / PHASE / TABLE / — / — / — | EXE inert; EXIT Graph base | Arrows inert | No INIT/stage; projection actions return base; Table returns current projection |
| Phase ANLYS | FIELD / NULL / EQPT / INFO / — / — | EXE inert; EXIT Graph base | LEFT/RIGHT selected equilibrium; no trajectory pan in menu | Independent FIELD/NULL; EQPT result panel; non-autonomous rejection; no invented progress |
| Phase analysis/Equilibrium INFO | — / — / — / — / — / OK | EXE/F6/EXIT ANLYS | Inert | Read-only help or Jacobian/eigenvalue/classification; local-linear limitation explicit |
| Graph options/state selector | — / PG- if needed / PG+ if needed / — / — / OPEN | Open item; EXIT Graph/parent list | UP/DOWN selector wrap; digits | No INIT/stage; same-state-axis conflict message; legacy projection only where supported |
| Graph calculation/window details | — / — / — / — / — / OK | EXE/F6/EXIT Graph options caller | Inert | Read-only existing result/window details; no new numerical query |
| Table preparing | Entire strip hidden once delayed screen appears | EXIT cancels to prior stable Graph/Table | Input poll before paint | Blue Preparing Table... spinner header; EXIT cancels row; neutral body; no stage |
| Table ready | TOP / BTM / MID / — / STAT / GRAPH | EXE inert; EXIT/F6 Graph | UP/DOWN page, LEFT/RIGHT dependent columns; bounded, no wrap | No INIT; ascending x/fixed first column; terminal status inline; RK4 Step or actual RK45 dx footer |
| STAT preparing/result | No active strip on static prep; result — / — / — / — / — / OK | Cancellation during export; EXE/F6/EXIT dismiss result | Inert | Existing neutral export preparation and file/result message; no automatic retry or fake percentage |
| Drawing preparing | Entire strip hidden after delay | EXIT cancels to prior stable screen/result | Cancel poll first; transaction rollback | Blue Drawing... spinner header; EXIT cancels row; white body; no partial framebuffer commit |
| Recall choices | — / — / — / — / — / OPEN | RAM last calculation or saved-load choice; EXIT Main | UP/DOWN wrap; digits 1/2 | No INIT/stage; missing last calculation uses message; saved choice asks confirmation |
| SAVE/load/resize/convert confirmation | — / — / — / — / NO / YES | Fresh EXE/F6 accepts; EXIT/F5 declines | Inert; held acceptance ignored | No INIT/stage/busy; no replacement/I/O before consent |
| Storage/migration/status message | — / — / — / — / — / OK | EXE/F6/EXIT parent | Inert | Existing short wrapped explanation; no hidden retry, no numerical reset on dismissal |

`ui_row`, standalone `ui_edit` and standalone `ui_number` are retained helpers,
not additional native user-facing routes. The numeric function picker remains a
real route from shared numeric editing and is included above.

## INIT scope audit

| INIT | Resets | Preserves |
|---|---|---|
| Equation F1 | Current mode's equations/power and equation drafts | IC, Solver, V-Windows, Output/colors, Event, graph appearance |
| IC F1 | Current mode's initial values/list and IC drafts | Equation, Output including inactive IC colors, Solver/windows/Event |
| Parameters F1 | AUTO range, h/h0, Max steps, visible method defaults; first-order SF12 | Method, hidden method settings/unsupported SF, windows, colors, Event, field appearance |
| V-WIN F1 / Graph F6 / ORIG | Active factory bounds/scales/Xdot | View mode, Grid/Label/projection, MAN range, h/tolerance/Event/Output; AUTO follows factory TIME window |
| Output F1 | Shared dependent mask ON and all default curve colors | IC list/equations/numerics/field appearance/windows/Event |
| Graph Settings F1 | Active Grid/Axis Label and field appearance defaults | Solver SF density, equations/IC/Output/Event/window geometry |
| TRACE F1 | Entry cursor/curve | Speed, frozen X geometry and solver preferences; existing Y-follow policy |

Existing complete-Document C assertions (`ui_controls`, `graph_overlays`,
`graph_interaction`, `output_instances`) validate these scopes. Changing IC count
does not now reset Output preferences; new and restored indexed preferences are
documented in [MULTI_IC_COLOR_AUDIT](MULTI_IC_COLOR_AUDIT.md).

## Representative integration scenarios run

`full_workflow_ui` executes production handlers through the scripted host key
adapter, with no emulator or alternate app logic. It complements existing tests:

| Scenario | Actual workflow and checks |
|---|---|
| A | General/RK4/single IC → Graph → TRACE → Table → G-Solve → Output and return |
| B | General/RK45/5 IC → five curves, IC5 TRACE, final Table columns, G-Solve selection, Output |
| C | General/RK4/10 IC → IC10 TRACE and final columns; independent F3 colors/INIT/SAVE/pixels in `output_instances` and `output_instances_ui` |
| D | Second/RK45 → y/y' Table columns → direct two-curve ICPT without unnecessary selector |
| E | N-th9/RK4 → physically enter x0 plus all nine state ICs → Graph/last-derivative TRACE/last Table and Output columns |
| F | SYS9/RK45 → physically enter nine RHSs and all nine ICs → Graph/last-state TRACE/Table/Output; hidden SF absent |
| G | SYS2 → PHASE → FIELD/NULL → EQPT → equilibrium INFO → VIEW EXIT/HOLD; existing `phase_ui` covers non-autonomous policies |
| H | General/RK45 Event MARK → Graph EVT → Table endpoint → INFO |
| I | Same Event STOP → neutral END: Event in Graph/Table → INFO |
| J | Restricted sqrt RHS → nonfatal domain warning → valid TRACE/Y-ICPT → Table numerical endpoint |
| K | Explicit SAVE → new process defaults/no implicit I/O → confirmed saved RCL → five ICs, RK45, Output and reset runtime INFO |

The added workflow also checks all first-order subtypes and minimum/maximum
N-th/SYS Equation blank drafts, first-offender validation, last-page IC missing
values and INIT recovery. FUNC/VAR/ADV/Event/INFO/ZOOM/BOX/VIEW/G-Solve/palette and
critical confirms receive EXIT, held EXIT and a second fresh EXIT. No repeated
navigation key triggers a hidden calculation or write in existing metric tests.

Targeted post-fix host/UBSan run: eight groups (`full_workflow_ui`,
`explicit_session_policy`, `ui_workflows`, `event_ui`, `rk45_ui`,
`graph_interaction_ui`, `ui_consistency`, `output_instances_ui`) passed in 6.27s.
Final milestone-wide gates and artifact measurements are recorded separately.

## Findings, severity and disposition

### UID1 — P2 / AUTO-FIX: RK45 Table showed inactive RK4 Step

Reproduction: Second → Parameters → set RK4 Step7 → switch RK45 → Graph → Table.
The rows were spaced 0.033333333 but the footer claimed `Step 7`. The actual RK45
grid is `max(TIME Xdot, solver span/1024)` and ignores the retained RK4 Step.
The regression failed before the fix. The footer now reads `dx 0.0333333` from
the already-built Table index. RK4 retains its Step label; termination status,
grid construction and every numerical value are unchanged.

### UID2 — P2 / AUTO-FIX: cancelled Drawing replaced Last calculation

Reproduction: successfully Graph RHS1; edit RHS2; cancel the next Drawing before
its first trajectory; return Main → Recall → Last calculation. Previously RHS2
was returned even though no graph/result had been accepted. A first-ever cancelled
attempt similarly created a Recall entry. The snapshot was assigned before
calling the graph renderer.

The regression failed before the fix. `screen_calculate` now requests Drawing
without changing Recall. The first accepted Graph result commits into the existing
Recall document; valid partial prefixes/Event STOP remain eligible. Cancelled
Drawing and auxiliary redraws do not replace it. This adds one local boolean,
not a Document backup or heap allocation. Tests cover old snapshot retention,
no snapshot after first cancel and replacement after a later accepted graph.

### UID3 — P2 / AUTO-FIX: shared visibility caption exceeded its column

Renderer review of the new five-IC Output screen found that the initial
`y output (all ICs)` caption reached the fixed colon/ON value column. A production
font width assertion reproduced the failure before the repair. The caption is now
`y (all ICs)`, with its shared visibility meaning and help unchanged. The width
must remain at most104px, leaving at least4px before the colon. Actual app tests
verify that the shortened caption is the one displayed and that curve-only rows
still carry no ON/OFF values.

### UIR1 — P2 / closed in beta.8 (historical reproduction): escaping invalid G-Solve numeric EDIT

The existing Y-CAL/X-CAL numeric prompt begins empty. EXIT in EDIT attempts to
validate/commit and stays in the prompt on invalid input; a valid commit followed
by another EXIT leaves it. Repeated EXIT on an empty entry therefore requires
correction first. This follows the established numeric EDIT contract, while a
dedicated cancel policy is also reasonable. No automatic change was made to this
workflow. Palette/curve/result cancellation and their HOLD behavior are separate
and remain correct.

### UII1 — P3 / INFO: physical timing and long-operation feedback

Graph Drawing, TRACE, Table and G-Solve have the shared delayed cancel-first
feedback appropriate to their surface. STAT uses an existing static preparation
screen; legacy OPTN Auto has its existing calculating-bounds screen. Phase EQPT
and ZOOM AUTO retain their bounded cancel-polling implementations. No spinner was
added solely because a path exists. Actual calculator latency, key repeat and LCD
transfers require the milestone's hardware retest; host timing is not device
timing. No new timer, solver, Phase feature or percentage estimate was introduced.

No P0/P1 UI or index-corruption finding was reproduced in this audit. This is a
bounded evidence statement, not proof that every device interaction is defect-free.
