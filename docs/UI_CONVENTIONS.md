# Current UI conventions — v0.12.0-beta.7

[Full audit](FULL_AUDIT.md), [busy-screen ownership](BUSY_SCREEN_AUDIT.md) and
[IC color consumers](MULTI_IC_COLOR_AUDIT.md) supplement retained beta.6 rules.

- Table/Drawing long preparation uses the native blue title row, white
  `EXIT cancels` row, neutral body and no visible F-key strip. Shared156ms delay
  and <=8Hz spinner; one canvas transfer, then header strips only. TRACE/G-Solve
  keep their lower-left busy panels. Fast/cached same-page actions do not flash.
- Drawing cancellation rolls back report/display/window state; initial cancel
  returns Parameters. It never publishes a cancelled partial construction or
  replaces Last calculation. Finished nonfatal prefixes remain valid results.
- SF is0..50, default12; input51 is an inline range error. All old save readers
  clamp larger SF to50; renderer has an independent defensive bound.
- Multiple first-order ICs expose shared y visibility separately from IC1 y..IC10 y
  color-only rows. F3 changes only the selected curve. INIT restores the existing
  palette; inactive indexed preferences persist through shrink/grow and SAVE/RCL.
- RK45 Table reports its actual display dx; the RK4 Step label is RK4-only.
- Existing Graph top-left instruction/warning, lower data panels, semantic colors,
  one-EXIT selection/results, fixed-X TRACE, factory Graph INIT and BOX remain.

Earlier milestone rules below are historical where superseded here.

---

# Current UI conventions — v0.12.0-beta.6

[Overlay inventory and implementation](OVERLAY_AUDIT.md) defines the current rules.
- Graph warning/instruction uses common logical(7,4), normal text and measured
  opaque white padding. Active instruction temporarily owns the channel, restoring
  the persistent warning afterward. Result/data panels remain lower-left;
  TIME/PHASE/EVT remain top-right. Phase legends stay on their compact second row.
- All G-Solve graph-selection operations use exactly
  `UP/DOWN: SELECT GRAPH, EXE: SELECT` (252px). EXE is blue, normal, painted once.
  Single graph skips selection; ICPT retains its necessary two selections.
- Selection/result EXIT returns to G-Solve submenu; the next fresh EXIT returns
  Graph. Held EXIT crosses no extra layer, including native blink waits. Scratch
  G-Solve cancellation never publishes trajectory Partial: Cancelled.
- Graph F6 INIT == V-WIN F1 INIT == ZOOM ORIG: shared factory active-window reset,
  preserving appearance, numerical preferences, Event/Output and MAN range. AUTO
  retains existing window synchronization. The Graph entry snapshot is removed.
  TRACE F1 INIT remains entry cursor/curve reset with speed retained.
- G-Solve results keep a fixed lower panel and translate Y minimally only if the
  full9px marker needs room. Keep X/scales/Y span/result/index/solver unchanged;
  visible markers cause no movement, invalid results cause no pan. No new query
  during result cycling; use existing bounded display samples.
- Busy is neutral `<label> <spinner>` with `/ - \ |`, delayed156.25ms and <=8Hz.
  TRACE uses its lower data panel; G-Solve retains that area; Table has its
  preparation area; Drawing uses the preparation strip. Cancellation polling first.
  Native uploads only the small rectangle with the installed synchronous driver;
  source pixels are restored immediately, and finish clears the LCD patch.
- BOX instruction joins the top-left channel; its transaction/hold rules stay.
  Beta.5 native tiles and fixed-X/Y-follow TRACE are retained. No broad red domain shading.

Earlier milestone records below are historical.

---

# Current UI conventions — v0.12.0-beta.5

[Full tiles/TRACE audit](TILES_TRACE_AUDIT.md) supplements the retained beta.4
INIT/BOX/draft/semantic-color rules. Current changes:

- Main is 2x3: four shared 184x58 graph tiles and two 184x25 text-only tiles.
  Subtype is 2x2 using identical first-four geometry; its third-row area stays blank.
  Digits appear once in badges. LEFT/RIGHT wraps rows; UP/DOWN wraps columns.
  Header/help/softkeys keep native geometry; Main title DIFF EQ; F1-F5 blank,
  F6 OPEN. No fake battery/F-key placeholders. Focus repaints only old/new tiles.
- Graph warnings share logical (7,4), text-sized opaque white padding at (5,2),
  normal red text; Event STOP stays neutral. Fresh/cached/active overlays share
  graph_status. Right VIEW/EVT corner stays clear; Phase legends and G-Solve choice
  help use the second compact row. Restore old text via existing base redraw.
- TRACE freezes entry horizontal min/max/scale and actual Xdot. Traverse only the
  selected visible connected valid sample component; no TRACE X pan, prefetch or
  extension. Y translation retains span/scale and rejects invalid/unrepresentable
  windows. Ordinary Graph/ZOOM/G-Solve-menu pan and AUTO extension stay unchanged.
- PHASE freezes state-axis horizontal bounds, follows the vertical state, and
  traverses integration-time order; stop at offscreen/gap boundaries, never project
  an outside state onto the screen. A slab without retained states stays unavailable.
- TRACE bar: INIT / NORMAL / FAST / FASTER / LEFT / RIGHT. INIT yellow/black;
  NORMAL orange/black; FAST bright green/black; FASTER cyan/black. Speed outline
  retained. F1 restores entry cursor/curve, preserving speed and all document
  settings. F5/F6 target configured endpoints clamped to reachable visible bounds.
  Arbitrary TRACE x= entry is removed; G-Solve Y-CAL still provides numerical queries.

Earlier milestone records below are historical; current controls above take priority.

---

# Current UI conventions — v0.12.0-beta.4

The complete before/after interaction inventory is [INTERACTION_AUDIT](INTERACTION_AUDIT.md).
[USER_GUIDE](USER_GUIDE.md) is the current full F-key table. Historical beta.3 audit
below is retained as evidence; these current rules supersede its changed controls.

- Semantic color follows action, not slot: INIT yellow/black, ADV black/white,
  V-WIN orange, SET green, NEXT cyan, PREV magenta, GRAPH/RUN red. Graph no longer
  uses PREV; its base F6 is INIT, and EXIT goes back. Established speed/COLOR styles stay.
- Graph Settings INIT is visible and works on every row, including Style. Style
  changes with LEFT/RIGHT, Color via RIGHT/F3. Equation/IC F1 INIT is screen-scoped.
- Generic EXE OPEN/NEXT/GRAPH help stays hidden. Retained EDIT/palette/curve/BOX
  confirmation EXE is blue, normal weight and single-painted. Main MENU alone is red.
- F2 FUNC/F3 VAR on Equation are EDIT-only; VAR only in its supported modes. Token
  palettes own their token bar and EXIT preserves the editor. Event has its own bar.
- Incomplete Equation/IC drafts are allowed until NEXT (or SELECT EXE). Focus the
  first error inline and keep INIT usable. Parent EXIT never forces validation.
  Parameters/Event/GRAPH safety checks retain their prior policy.
- SELECT lists alone wrap; cursor/graph/data movement retains its semantics.
  Stage1/3–3/3 only Equation/IC/Parameters; V-WIN only Parameters/Graph.
- AUTO/MAN reads existing range state; RK4 h/RK45 h0 retain the same stored value.
  TIME/PHASE only when SYS2 VIEW is available; enabled EVT is type-independent.
- Graph INIT restores the current view's entry geometry, retaining selected view
  and manual numerical preferences. AUTO follows geometry. Reuse compatible cache;
  safe redraw is allowed when cache/Event-report coverage has been replaced.
  ORIG keeps factory semantics. BOX commits only on valid second-point EXE;
  cancel restores exact pixels and leaves both windows/settings unchanged.
- Point markers are9px black crosses with white3px center, clipped to plot.
  Event squares/EQPT diamonds remain distinct. Selected2px curve/blink remains.
- Active results/cursors, persistent nonfatal red status and small static labels
  must remain readable. Input errors stay field-inline; numerical END/Partial stays
  over valid Graph/Table. Do not disable valid-domain TRACE/G-Solve or join gaps.
- Long G-Solve uses delayed neutral busy feedback with cancel-first polling and
  complete teardown; never fake percentage progress or draw from interrupts.
- Critical confirmations stay F5 NO/EXIT and F6 YES/EXE, ignoring opening HOLD.
  BOX point confirmation also requires a fresh EXE; arrows support repeat.

---

# Historical beta.3 audit (before beta.4)

# UI consistency audit — v0.12.0-beta.3

Baseline: development 9e64365 / public 4dcb2d6 (beta.2). Source inventory below
was recorded before this pass. The local original DIFF EQ manual was rechecked
at PDF p.13 (dense Parameter/V-Window rows and full graph); the fx-CG50 manual
at PDF p.43/printed1-35 (selected setup row/current status) and PDF p.197/printed
5-56 (G-Solve). These support the existing native layout; no reference image is
copied into the public project. Explicit current owner instructions take priority
over historical key maps. Host baseline:41/41,19.00s; SH warning0/package13/13.

## Screen and interaction inventory

All form titles use the blue21px header at x6/y4, normal font, baseline y9.
Ordinary SELECT is the same blue row/white text; EDIT keeps it and adds a caret
within the value area. Unless listed, vertical SELECT wraps over visible rows,
EXE follows F6, EXIT leaves one layer, and unused softkeys are blank/inert.
Graph occupies the complete198px region above the18px softkey bar. Table/INFO
scrolling, graph/TRACE/G-Solve motion and the2D palette retain their own policies.

| Screen/state | Title/header | F1 / F2 / F3 / F4 / F5 / F6 at beta.2 | Help and interaction audit |
|---|---|---|---|
| Main | Differential Equation | — / — / — / — / — / OPEN | 1–6 direct; EXE/F6 current item; MENU OS; remove redundant EXE hint, separate SYS/RCL visually |
| 1st subtype | First-order equation | — / — / — / — / — / OPEN | Four numbered choices; remove generic EXE: open |
| N-th/SYS size | Order/Variables (1-9) | — / — / — / — / — / OPEN | Numeric EDIT/SELECT, EXE commits before OPEN; invalid size stays in field |
| Equation SELECT | DIFF EQ / mode + general form,1/3 | VAR if relevant / — / — / — / — / NEXT | Remove EXE: NEXT; retain LEFT/RIGHT edit context |
| Equation EDIT | Same | VAR if relevant / FUNC / — / — / — / NEXT | Caret, commit/next hint; EXIT commits; invalid expression must retain field |
| FUNC/VAR | Parent title, no progress | Up to5 tokens / page or BACK | EXIT closes only token bar; no progress/generic EXE help |
| IC | Initial Conditions,2/3 | — / — / — / — / — / NEXT | Scalar/list syntax help; EXIT commits EDIT or returns to Equation; no generic EXE hint |
| Parameters | Parameter,3/3 | INIT / ADV / V-WIN / OUTPUT / SET / GRAPH | Dynamic Method/Step/tolerance/SF rows; range uses one solver_custom flag; expose AUTO/MAN, RK45 h0 |
| ADV | Parameter, no progress | EVENT / INFO / — / — / — / — | EXIT only closes submenu; no calculation/reset/I/O |
| Event | Event Settings | VAR in EDIT / FUNC in EDIT / — / — / — / DONE | Enabled/direction/action toggle; E editor; remove generic EXE: done |
| Solver INFO | Solver Info | — / — / — / — / — / — | Read-only scroll, EXIT; requested initial RK45 step should also read h0 |
| V-WIN SELECT | View Window / Phase View Window | INIT / — / — / — / — / DONE |7 rows, Xdot relation help; call/return only Parameters/Graph |
| V-WIN EDIT | Same | — / FUNC / — / CLEAR / DEL / OK | Numeric expressions/caret, shared commit hint; keep invalid field visible |
| Output | Output selection | INIT / — / COLOR / — / — / DONE | Shared dependent ON/OFF; multiple first-order ICs have color-only IC rows and a separate all-IC visibility row with F3 blank; EXE follows rows; curve line preview |
| Graph Settings Grid/Label | Graph settings | INIT / — / — / — / — / DONE | LEFT/RIGHT toggles |
| Graph Settings Style | Same | — / — / — / — / — / DONE | Arrows only, exact style hint; INIT on other rows |
| Graph Settings Color | Same | INIT / — / COLOR / — / — / DONE | RIGHT/F3 palette; six-color swatch |
| Palette | Small overlay: Curve/Field color | — / — / — / — / — / — |2D bounded arrows; retained EXE: SELECT / EXIT: cancel |
| Graph base TIME | No header | TRACE / ZOOM / V-WIN / TABLE or VIEW / G-SLV / PREV | Pan, +/-zoom, EXIT/F6 Parameters; add tiny view/EVT label |
| Graph base PHASE SYS2 | No header | TRACE / ZOOM / V-WIN / VIEW / ANLYS / PREV | Independent Phase window; same pan/back; optional legacy higher-state Phase remains documented |
| VIEW SYS2 | Graph | TIME / PHASE / TABLE / — / — / — | EXIT closes VIEW; switches reuse trajectory cache |
| ZOOM | Graph | IN / OUT / AUTO / ORIG / — / — | Arrows pan; EXIT closes only ZOOM; failures remain over plot |
| TRACE | Graph + coordinate footer | x= / NORMAL / FAST / FASTER / LEFT / RIGHT | Blink selected curve; arrows/numeric query retain numerical semantics; full cross cursor |
| TRACE x EDIT | Graph + x= caret | x= or Invalid x / — / — / — / — / — | EXE/EXIT evaluates entered x under existing policy |
| G-Solve menus | Graph | ROOT/MAX/MIN/Y-ICPT/ICPT/> or Y-CAL/X-CAL/—/—/—/< | Arrows pan; EXIT closes submenu |
| G-Solve curve selection | Graph + top prompt | — / — / — / — / — / CANCEL | UP/DOWN blink choice, EXE confirms; retain non-obvious EXE hint |
| G-Solve input | Graph + x/y EDIT footer | — / — / — / — / — / RUN | RUN is execute semantic; existing EXIT commit/back behavior |
| G-Solve result | Graph + pointer/result footer | — / — / — / — / — / BACK | LEFT/RIGHT results; EXIT/EXE/F6 back; pointer must not overwrite result text |
| Phase ANLYS | Graph | FIELD / NULL / EQPT / INFO / — / — | LEFT/RIGHT found equilibria; EXIT base; selected marker/detail overlay wins |
| Phase INFO | Phase analysis / Equilibrium INFO | — / — / — / — / — / OK | Read-only result/help panel; no solve/save |
| Table | Table / initial solutions | TOP / BTM / MID / — / STAT / GRAPH | Fixed x; rows/columns scroll without selector wrap; END/partial status retained |
| SAVE | Save session | — / — / — / — / NO / YES | EXE YES, EXIT NO, opening HOLD ignored; no pre-confirmation I/O |
| RCL | Recall | — / — / — / — / — / OPEN | Last calculation RAM vs saved session; preserve backend and migration |
| Load/convert confirm | Load saved session / Convert to system | YES / — / — / — / — / NO | Legacy confirmation key layout differs from SAVE/resize; audit before changing |
| Resize confirm | Change Equation Size | — / — / — / — / NO / YES | Critical replacement; nonselectable text, preserved no-write behavior |
| Graph options | Graph options / Select phase state | — / — / — / — / — / OPEN | Existing projection/settings/details extension; remove generic open help |
| Storage result/migration | Save/Load/STAT/Legacy status title | — / — / — / — / — / OK | Relevant file status, no automatic retry; long information panel remains justified |
| Input error | Previously full-screen message | — / — / — / — / — / OK | Prefer field-preserving inline error and return to editing |

Legacy ui_row/ui_edit/ui_number have no production caller from the native screen
workflow. They are not additional exposed screens; retain their helpers without
redesigning them. Their numeric editor primitives still use the common renderer.

## Stable conventions

- Semantic actions, not F-number, choose color: INIT yellow/black, ADV black/white,
  V-WIN orange/black, SET bright green/black, NEXT cyan/black, PREV magenta/black,
  GRAPH/RUN red/white; ordinary actions use base blue/white. Existing TRACE speed
  and COLOR palette treatments are purposeful exceptions, not new categories.
- Do not repeat generic EXE OPEN/NEXT/GRAPH/Enter hints. Retain EDIT commit/next,
  palette SELECT and graph interactions whose EXE action is not evident.
- Main MENU alone is red. Retained EXE help is normal-weight blue, drawn with
  exactly the same font primitive and baseline as adjacent text, once per glyph.
- Keep one help line where possible. Detailed usage belongs in USER_GUIDE.
- SELECT lists wrap over visible rows; EDIT/data/graph navigation do not acquire
  selector wrapping. EXIT replaces redundant PREV on forms.
- 1/3,2/3,3/3 appear only on Equation/IC/Parameters. V-WIN only Parameters/Graph.
- UI status reads existing state: a single AUTO/MAN flag for the whole solver
  range, shared stored h displayed as RK4 h/RK45 h0, TIME/PHASE with optional EVT.
- Preserve numerical/storage behavior. Input errors retain the field; numerical
  termination stays with the plot/table; critical replacement uses confirmation.

## EXE rendering defect

Installed gint render/topti.c dnsize returns minus char_spacing for an empty
prefix (-1 with the default font). Beta.2 ui_help drew the complete muted string
then overpainted EXE. A leading EXE was overpainted at x-1; Main's nonleading EXE
used a nonempty measured prefix and aligned correctly. The host adapter returned
0 for empty text and masked this device discrepancy. Fix the adapter to match gint,
test an isolated leading EXE against the same primitive, and render disjoint text
segments once. Do not add bold/outline/shadow or assume empty-prefix width is0.

## Applied beta.3 decisions and current key map

The inventory above is the before-state. All existing F-key positions are retained
except legacy Load/Convert confirmation: all critical confirmations now share
F1–F4 blank/inert, F5 NO/EXIT, F6 YES/EXE, and ignore held opening keys. Resize,
SAVE, load and conversion use that same primitive; storage/resize logic is unchanged.
GRAPH and RUN receive red/white in any F-number, verified across all six positions.
Other screen key maps remain as inventoried. No generic EXE hint remains on Main,
Equation SELECT, ordinary choose lists or Event SELECT. EDIT, palette SELECT and
G-Solve curve SELECT retain the normal blue EXE token. The palette suppresses its
parent's help while active. Main's four-pixel SYS/RCL gap creates no selectable row.

The whole solver X range uses the existing solver_custom flag: AUTO follows the
TIME window, MAN preserves a direct commit (even an unchanged value). Both rows
show that state except the actively edited row; INIT restores AUTO. RK45 uses h0
in Parameters and solver details/INFO; RK4 h and stored values are unchanged.
Output preview is a 28×2-pixel line on a small white backplate; OFF retains the same
color. Multiple first-order ICs show separate IC1 y through IC10 y color-only
rows; their F3 changes exactly that stored curve color. A separate
`y (all ICs)` row retains shared dependent visibility, has F3 blank and no
ambiguous preview. Color rows never display independent ON/OFF. Seven-row SELECT
paging/wrap reaches all ten ICs above the help. Single IC keeps the ordinary y row.
Shrinking/growing an IC list retains indexed color preferences; unused slots have
the original palette defaults. Output INIT resets all colors; IC INIT preserves
them. See [multiple-IC color audit](MULTI_IC_COLOR_AUDIT.md).

Graph draw order is background/grid/axes, field/nullclines, solution curves,
Event squares, equilibrium diamonds, compact view/legend/status backplates, then
active overlays. Selected curve emphasis retains existing 2px reversible rendering
and blink, including Black↔Blue. No optional 3px pass was added: it would obscure
more detail and complicate the existing restoration mask without a demonstrated
readability benefit. TRACE cursor/selected curve precede readable labels; the
coordinate/result footer clears its own region after pointers. Fresh and cached
frames share Event-before-equilibrium order and graph_labels. G-Solve's choice
prompt reserves the right corner for TIME/PHASE; status uses a second compact row.
TIME/PHASE labels optionally append EVT only when Event is configured enabled.
No large HUD, trajectory copy or framebuffer was added.

Marker audit also found physical Event pixels missing the UI_X/UI_Y offset.
The square now centers at the same screen coordinate as its trajectory point;
Event root/refinement data and algorithms are unchanged. Tests assert this offset.

Ordinary equation/IC/parameter/window/Event validation retains title, selected
field and draft with one red footer, acknowledged by EXE/EXIT/F6 EDIT. Numerical
termination uses END/partial text in Graph/Table, preserving valid prefixes and
Event, underflow, limits and cancel reasons. Read-only requested INFO, storage or
migration results still use an information panel when more than one line is needed;
critical actions alone ask YES/NO. Legacy graph-option/preflight information panels
remain intentional where no plotted result/field is available.

## Evidence and remaining device checks

- 42/42 host/UBSan groups PASS (31.06s development final), including all prior41.
- UI pixel tests match installed gint empty-prefix metrics and normal glyph masks,
  count glyph paints to forbid overdraw, bound hint widths and title clearance,
  verify semantic colors in every slot and all INIT Document scopes.
- Actual handler tests cover keys, SAVE/RCL/resize, AUTO/MAN/h0, inline errors,
  18 complete F-key bars, marker coordinates and overlay collisions. Existing
  numerical/parser/RK4/RK45/Event/TRACE/G-Solve/Phase/Table/storage tests stay enabled.
- Clean27-unit strict SH build/link: zero warnings;13/13 package checks.
- SH text206816 (+752), data704 (0), BSS71952 (0), max ui_graph frame2568 (-4).
  Development G3A236200 B; exact public artifact identifiers are in Release VALIDATION.
- [24 representative production-rendered screens](ui-review/consistency-overview.png)
  and [workflow screens](ui-review/workflow-overview.png) were regenerated and
  visually reviewed. capture_consistency.py reproduces the new set. They are
  host framebuffer captures, not device photos or an SH emulator.
- [36 new hardware checks](HARDWARE_RETEST.md) remain HARDWARE TEST REQUIRED,
  including LCD glyph contrast, marker/curve visibility and repeat latency.
