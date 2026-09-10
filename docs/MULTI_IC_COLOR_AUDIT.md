# Multiple-IC output color audit

This audit covers the explicit hardware feedback about first-order trajectory
colors. It precedes the broader functionality audit. Native screen behavior on
the physical calculator remains **HARDWARE TEST REQUIRED**.

## Existing architecture and reproduced cause

`Document.color[10][9]` already stores a palette index for every IC and dependent
component. It is a fixed 90-byte array, already included in SAVE/RCL. The former
Output screen displayed only `dim` rows and `model_output_color` re-seeded the
whole IC palette from the selected component's first color. In addition,
`initial_values_apply` called that re-seeding function on every accepted scalar
list edit. Therefore editing any initial values could erase independent colors
even if another path assigned them.

`Document.enabled` is different: it is one dependent-component mask shared by all
ICs. Changing that mask selects components for Graph, TRACE, G-Solve, Table and
CSV/STAT. It does not select individual first-order trajectories.

| Consumer | Existing source of state | Result after this fix |
|---|---|---|
| Graph streamed and cached curves | `model_color(d, family, variable)` | Draws the edited IC's actual stored color |
| TRACE selected curve | Same accessor in the selected-pixel mask | Highlights/restores that color; numerical cache identity stays unchanged |
| G-Solve selector/result overlay | Same accessor in the graph overlay mask | Uses the same curve color; selection highlighting remains reversible |
| Output preview | Formerly only family 0 | Each IC row uses that family's accessor and the existing 28×2 line |
| Table | Shared component mask; one column per first-order IC | Numerical columns and monochrome cell styling unchanged |
| CSV/STAT | Existing Table mapping and shared mask | Column names/order/content unchanged; colors are not data fields |
| SAVE/RCL | Existing `Document.color` matrix in current and recall records | All active and inactive color preferences round-trip |

## Implemented policy

- One first-order IC keeps the existing `y` row with visibility and F3 COLOR.
  Its F3 action now changes only that curve's preference.
- Multiple first-order ICs show `y (all ICs)` as a single ON/OFF control,
  followed by `IC1 y` through `IC10 y` color-only rows. F3 is blank on the shared
  visibility row; the color rows have no ON/OFF value and ignore LEFT/RIGHT.
- The existing seven-row SELECT paging and UP/DOWN wrap make all ten curves
  reachable. The last page's IC10 row stays above the contextual help.
- F3 changes exactly the selected stored color. F1 INIT restores the existing
  palette cycle and shared output defaults. Higher-order/system component rows
  retain their existing behavior.
- Color preferences belong to IC positions. Editing values or shrinking the list
  retains every position's color; growing restores those retained colors.
  Positions never edited already contain the default palette. Initial Conditions
  INIT remains scoped to initial conditions and does not reset Output colors.
- The established default palette order remains Magenta, Cyan, Bright Green,
  Red, Blue, Black, repeating by component/IC position. Output INIT uses that
  same order. Changing IC1's color does not re-seed unedited future ICs.

No new document fields, heap allocations or save-format version were necessary.
Current v10 records already contain all ten IC color rows. v7–v9 also preserve
those rows. v4–v6 preserve their nine stored rows and explicitly initialize the
previously absent tenth row to the default palette. v3 has no color array and
continues to initialize the entire default cycle. Invalid palette values still
use the existing per-position fallback.

## Finding classification

**AUTO-FIX:** The Output screen did not expose stored curve-instance colors, and
accepting initial values overwrote them. Reproduced with multiple first-order ICs;
fixed through the existing color state and existing consumer paths.

**AUTO-FIX:** The absent tenth color row in v4–v6 records was zero-filled (Blue).
Because that color now becomes independently visible, initialize the absent row
with the established default, while preserving every existing stored row.

**REVIEW REQUIRED, unchanged:** Per-IC ON/OFF would add a new visibility state and
require consistent Table/CSV column policy. It is outside the requested independent
color correction. The existing shared component visibility remains explicit.

**INFO:** Palette colors repeat after six entries because the existing chooser has
six colors. All ten preferences remain independently editable, including repeated
initial values, which still represent distinct curve instances.

## Verification

`output_instances` drives the real Output handler with F3, palette arrows,
EXE, F1, UP/DOWN and F6 for 1, 2, 5 and 10 ICs. It checks complete Document
isolation, exact preview/Graph/TRACE/G-Solve pixels, curve counts, Table mapping,
SAVE/RCL, hidden color retention, deterministic growth and no stale curve ordinal.

`output_instances_ui` runs the actual app entry workflow and checks row names,
truthful ON/OFF ownership, every IC's reachability, wrap/page behavior, palette
cancel restoration, INIT and unchanged higher-order/system component rows.
The existing `test_list`, `test_initial`, `test_field_storage` and `simplified_ui`
regressions remain active; frozen legacy layout tests verify stored colors and
the absent tenth row's migration. Full build/package gate results are recorded
with the milestone's overall validation.

On hardware, retest 1/2/5/10 IC Output navigation; F3 changes and cancellation;
Graph/TRACE/G-Solve matching curve colors; all-IC visibility in Table/CSV; scoped
INIT; and SAVE/RCL after shrinking and expanding the initial-value list.
