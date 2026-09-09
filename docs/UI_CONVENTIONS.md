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
| Output | Output selection | INIT / — / COLOR / — / — / DONE | ON/OFF and persistent colors; EXE follows rows; replace box with curve line preview |
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
color. The chooser and persisted palette/index policy are unchanged.

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
