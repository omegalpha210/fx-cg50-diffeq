# OUTPUT, field settings and session migration audit

2026-09-08; baseline `309a9db`, which preserves the previous `1f354de` G/L work.
This is a requested UI/persistence adaptation around the existing numerical
engine. The earlier independent-L policy remains documented in Git history.

## Audit before unifying G/L

The previous `graph_mask[IC]` selected Graph, TRACE and G-Solve candidates.
`list_mask[IC]` selected Table and STAT dependent columns, and bit 0 selected
CSV x. General `storage_csv` also consumed L, using blank unselected cells in
its fixed schema; this API has no current UI caller. SAVE and RAM recall kept
both masks per IC. Neither mask reduced RK4 dimension or equations.

**There is a real presentation tradeoff:** G OFF/L ON allowed numeric-only data,
G ON/L OFF allowed graph-only output, and per-IC masks allowed different subsets
for each solution. Original DIFF EQ p.3 describes separate graph/List selection;
p.20 assigns x/y1/y2 to List1/2/3 independently. Therefore unification is not
claimed to be feature-identical or original-binary-compatible. Following the
user's explicit simplification direction, this build removes those independent
presentation combinations and records their loss here. Numerical state and the
existing solution algorithms are preserved.

| New control | Consumer | Meaning |
|---|---|---|
| `Document.enabled` bit k | Graph/auto-window/TRACE/G-Solve | State k is ON for every IC; phase requires both selected axes |
| Same bit | Table and STAT/general CSV | State column included; scalar multi-IC y expands to one column per solution |
| x | All Table/CSV/STAT paths | Always first; no field, mask bit, or UI toggle |
| F3 COLOR | Existing six-color solution palette | Selected state color; first-order families automatically cycle from that seed |
| F4 INIT | Output only | All valid state bits ON, default solution colors; field settings unchanged |
| SF | `solver.sf` | Single 0..100 integer in scalar first-order Parameters; default 12 unchanged |
| Graph Settings | Grid/Label/Style/Color | Direct F5 SET destination; Arrow/Pale Blue factory defaults |

LEFT/RIGHT only toggles the selected output; UP/DOWN selects a state. F1/F2 are
blank, F3 opens COLOR, F4 INIT, F6 DONE. Existing row-next/modified-last-stay/next-
DONE semantics are retained. Private Constants menu, parser scope, live document
array and editor insertion routes were removed. Builtin pi/e and numeric
expressions remain.

Graph Settings INIT restores Grid/Label ON and Arrow/Pale Blue without changing
SF. Scalar first-order Parameters INIT restores SF12 with solver defaults,
retaining field style and color. Unsupported 2nd/N-th/SYS modes (including N-th1
and SYS1) hide both SF and field appearance rows; Parameters INIT preserves the
hidden SF preference. Existing SAVE/RCL stores it in every mode. The Color row
shares OUTPUT's swatch frame/geometry and retains its own pale palette. Field
rendering remains once per background, independent of output bits and IC count.

## Versioned storage: v6 and frozen old layouts

v6 removes the 28-double private constant array, nine graph masks and nine list
masks from the live/new serialized Document, replacing the masks with one
16-bit dependent-output mask. Existing equation, IC, Solver/SF, V-Window, curve
colors and field preferences remain. Target record sizes/ABI and final memory
are in [ACCEPTANCE](ACCEPTANCE.md).

Old v3/v4/v5 layouts are frozen explicitly in `storage.c` for streaming offsets.
Each version has its own expected size/padding, checksum and validation. No old
bytes are read into the new raw struct. Constants need only 224 transient stack
bytes for conversion; no full old record or trajectory scratch is allocated.
The inactive recall region is hashed but is not mistaken for a required valid
Document when `has_recall=0`.

| Legacy field | Mapping |
|---|---|
| G/L state bits | OR of `graph[f]` and `list[f] >> 1`, over active ICs, masked to valid state count; ON if any previous consumer selected it |
| CSV x OFF | Ignore; x always first |
| v3 curve colors absent | Existing per-curve default sequence |
| v4/v5 curve colors | Preserve valid choices; invalid index gets its per-curve default |
| v3/v4 field style/color absent | Arrow / Pale Blue |
| v5 Segment/Arrow and color | Preserve valid explicit saved values, including Segment |
| Invalid new style/color | Arrow / Pale Blue fallback |
| Private constant reference | Replace recognized A..Z except X/Y, r, theta with parenthesized round-trip numeric literals; keep functions, pi/e and scientific notation intact |
| Nonfinite referenced constant or expansion >191 chars | Keep the original entire expression, flag for editing; new parser rejects unsupported variables rather than substituting zero |
| Unused invalid constant | Skip; it does not reject the session |
| High-order/SYS old nic>1 | Keep first complete vector; report IC adaptation |
| Scalar old ICs with different x0 | Keep first and subsequent exactly common-x records, with their colors; report adaptation; never re-anchor values automatically |

Masks are combined before IC adaptation. The corresponding output state may
thus remain ON if it was selected only in a family removed by the new UI policy.
Migration also applies to stored recall. The one load notice reports expression
review with priority over IC adaptation; both bits remain in the in-memory
migration result. Under ordinary migration it reports combined outputs and
converted constants. The record itself need not compile merely to load/edit it;
calculation performs the existing expression validation.

Load never changes the old files. Explicit SAVE writes a verified v6 slot using
the existing generation/recovery strategy. Repeated SAVE can eventually replace
both old slots, so an archival copy is needed to retain the original indefinitely.
World-switch ownership, close-before-return and close-failure cleanup policy are
unchanged. Same-target raw ABI files are not portable host/device exchange.

RAM Last calculation recalls equation/IC/Output while retaining current global
Solver/View/Field preferences. Explicit Load saved session restores all saved
current/recall data. This established distinction is preserved.

## Unified Table and export

The Table index merges the regular `x0+n*h*Step` grid with accepted terminal
points and an interpolated zero row when 0 is a natural center. Rows ascend across
both internal integration branches. MID uses 0 in the middle half of the valid
range, otherwise the nearest sampled numerical midpoint; page start is clamped
to put the anchor in row 4 of 7 where possible. TOP/BTM/MID do not reset columns. Large-coordinate rounding cannot
conflate distinct sampled states; output spacing below the local representable
coordinate resolution is rejected before a Table or export is constructed.

Both CSV APIs now use the same ascending schema: apostrophe label row, x first,
then enabled states/first-order solution columns. All outputs OFF exports x alone.
Each solution's unreachable cells are blank, not fabricated zeros. `fx-CG50_manual.pdf`
printed 3-18/3-19 (PDF 134–135) documents List Editor CSV loading, 26 columns and
999 lines and ignored apostrophe label row. We retain the 998-data-row cap and
unique filenames. The manual does not establish how this exact mixed-length
blank-cell CSV is aligned on the actual STAT importer: **HARDWARE RETEST REQUIRED**.
Host tests validate blank-cell positions/ascending coordinates, not OS behavior.

The general CSV API's old fixed all-state header/selected blank-cell schema is
intentionally replaced by this shared schema; no existing UI uses the old API.
Exports pre-count rows before creating files and preserve cancel/error cleanup.
No private List-memory writes or undocumented syscalls were introduced.

## Verification

- `test_list.c`: consumers, ninth-state mapping, common flags, always-x/all-off,
  color seed, SAVE and bitwise unchanged full numerical state when outputs change.
- `test_field_storage.c`: independent frozen v3/v4/v5 fixtures, poisoned padding,
  absent recall, constants, masks, IC policy, v6 current+recall roundtrips and
  malformed/unsupported newest-slot recovery with unchanged old file bytes.
- `test_table_storage.c`: CSV ordering/cap/unique names and cancellation after
  file creation; current data survives failure of both saved slots.
- `test_ux_field.py`, `test_feedback.py`, `test_simplified_ui.py`: production UI
  controls, selectors, INIT boundaries, six-color choices, physical list entry,
  direct SET, all-off CSV, Table navigation and explicit persistence.
- Existing numerical/parser/G-Solve/TRACE/field/validity/navigation/key tests
  remain enabled; final logs and SH package evidence: [ACCEPTANCE](ACCEPTANCE.md).
