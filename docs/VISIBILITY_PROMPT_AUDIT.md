# Drawing, trajectory visibility and numeric EXIT — beta.8

Baseline: beta.7, development 297b109, public 1a5a49d. The owner explicitly
resolved FULL_AUDIT R1 and UIR1. Both are **Resolved by user decision / Implemented /
Validated**. No open review decision remains for these two requests. This milestone
changes UI, output filtering and persistence, not numerical algorithms.

## A. Drawing owns exactly the softkey rectangle

Beta.7 incorrectly reused Table's dedicated preparation page for Drawing.
`busy_draw_paint` now replaces only logical **(0,198)–(383,215)**, width384,
height18. The native LCD rectangle is **(6,202)–(389,219)** inclusive. It includes
all six former separators. Background is existing UI_BLUE, C_RGB(3,10,24).
White `Drawing... <spinner>` begins logical(5,202); white `EXIT cancels` is
right-aligned with a five-pixel inset using the installed font measurement.
There are no button labels/cells or separator pixels in this continuous bar.

The existing LCD Graph remains visible while its replacement is constructed in
VRAM. On first Graph entry, there is no previous Graph: axes and ordinary Graph
controls are established once after input preflight, before slow numerical work.
Recalculation/pan keeps the previously displayed Graph. Neither path redraws the
plot at the spinner cadence. Shared RTC rules: first visible after20 ticks at128Hz
(156.25ms), subsequent refresh at least16ticks apart (at most8Hz), phases / - \ |.
Cancellation is polled before clock/paint, including the tick when feedback begins.
Fast work never shows the busy bar.

Each bar update uses six synchronous three-row native rectangle uploads. Existing
TRACE staging-tail scratch stores384*3*2=2304B, below its2852B target capacity.
Source pixels and dwindow are restored after each upload/batch; no partial
construction is published. Success replaces the bar with the accepted Graph and
normal controls. Cancellation discards the temporary result and restores prior
geometry/report/display cache; first calculation cancellation returns Parameters.
Last calculation and accepted Event data retain beta.7 transaction ownership.
Rollback uses bounded retained display samples and need not reproduce every pixel
of a former dense drawing; it must preserve the accepted numerical state.

Table remains its dedicated blue `Preparing Table...` header, white `EXIT cancels`
row/body and hidden softkeys:75 strips initially, seven header strips afterward.
TRACE/G-Solve retain their lower CALCULATING feedback. No new framebuffer or heap.
Installed gint dwindow and synchronous r61524 headers were checked and reused.

Evidence: `test_busy_screen`, `test_key_lifecycle`, `test_busy_screen_ui` check
delay, four phases, six strips, exact bounds, every outside pixel unchanged,
VRAM preservation, cancelled/rejected/successful transactions and HOLD priority.

## B. A trajectory's existence is independent of its visibility

`Document.ic_enabled` is an appended uint16_t with ten valid bits; unused slots
are retained. `model_curve_visible(document,family,variable)` is the shared
consumer predicate. Scalar first-order modes use the IC bit, while higher-order
and SYS retain their original dependent-component `enabled` mask. Bounds checks
precede shifts/indexing. No existing field or padding is reinterpreted.

Output has one `y` row for one IC and IC1 y..IC10 y for multiple ICs. LEFT/RIGHT
toggles only that row; F3 COLOR changes only its color. OFF retains its actual
colored line swatch. Seven-row paging and selector wrap expose IC10 above help.
F1 INIT restores all ten ON bits and the existing six-color default cycle; it
does not reset SF, solver or window. IC editing/INIT preserves Output preferences.
Shrink/re-expand preserves inactive bits and colors. Previously untouched slots
are ON with default color; document reset and Output INIT restore these defaults.
Visibility-only edits also participate in the existing equation-size replacement
guard, just as color edits do; a reproduced omission was corrected and tested.

| Consumer | beta.8 behavior |
|---|---|
| Graph and cached repaint | Paint only ON families, using stored family colors; preserve axes/grid/SF independently. |
| TRACE | Select/cycle ON curves only. Hidden explicit prepare/select fails before mutating selection; fresh entry normalizes to first visible curve. |
| G-Solve | Count/ordinal/validation use visible curves; one visible curve skips selection; ICPT requires at least two. Numerical search and Event endpoint algorithms unchanged. |
| Table | x remains first; only ON trajectory columns, original family indices and order. |
| CSV/STAT CSV | Reuse Table mapping, machine-safe y1..y10; hiding IC2/4 gives x,y1,y3,y5, not renumbered columns. Single IC uses y. Existing leading-quote CSV header convention is retained. |
| All OFF | Graph axes/grid and enabled field; TRACE says No visible graph, G-Solve Not available; Table/CSV/STAT are x-only. No hidden fallback. |
| Event | Integrate/detect all scalar ICs, preserve hit data and work diagnostics; hide markers belonging to OFF families. |
| Numerical state | Graph capture and Table indexing still calculate required hidden scalar families. Visibility enters display identity, not numerical identity. Direct model queries remain available internally. |

Thus an all-OFF Graph can still report genuine domain/step/Event outcomes from
the work actually done. It does not claim zero solve work. SYS2 PHASE retains its
existing state-pair visibility policy and independent TIME/PHASE geometry.

### SAVE v11 and migration

v11 streams the extended current and recall Document; all-OFF is valid. A frozen
v10 structure, with compile-time prefix assertion, reads the exact old layout.
All v3–v10 readers initialize every new IC bit ON, including inactive positions.
**This explicit migration also turns scalar ICs ON if an old shared y mask was
OFF.** The old field remains stored, but is no longer scalar output authority.
Old colors and their established absent-slot defaults remain unchanged; SF clamp,
Phase settings, v9 RK45 and v10 Event settings retain their previous migration.
v3–v9 still default Event OFF. Loading does not rewrite the old files; a later
explicit SAVE rotates the existing slots using v11. Invalid new mask bits reject
the record using existing validation/backup policy. Raw ABI saves are device
specific; host .dat files are not interchangeable with calculator files.

Evidence: `trajectory_visibility` tests RK4/RK45 ×1/2/5/10IC ×sparse/all-OFF/all-ON
masks, visible pixels, hidden markers, exact unchanged numerical reports, direct
hidden queries, selection mappings, Table values, CSV/STAT columns, cold v11
current/recall, inactive preferences and invalid masks. `full_persistence_matrix`
retains46 complete Document roundtrips with the mask added. `field_storage`
retains independent v3–v10 fixtures and checks ON migration and original colors.
`output_instances` and actual Output UI cover 1/2/5/10 rows, colors, toggles,
INIT, shrink/grow, scrolling/wrap and SAVE. Existing higher/SYS tests remain.

## C. G-Solve numerical draft EXIT is unconditional cancel

Root cause: `gsolve_input` shared its validation/commit branch across EXE and
EXIT, so an empty draft stayed at Invalid number and a valid draft could commit.
EXIT now branches first: restore overlay, stop cursor blink, return false. The
local draft/target/query scope ends; the operation unwinds to the existing
G-Solve menu's retained page2 (F1 Y-CAL, F2 X-CAL, F6 previous). It does not return
to an intermediate graph selector or page1, publish a Cancelled report, rerun the
solver, or change Graph/window/settings/preferences/Last calculation.

Only fresh EXE validates and commits. Invalid EXE keeps the draft and red Invalid
number; valid EXE follows unchanged X/Y-CAL search/result/marker/Y-follow paths.
F6 RUN was removed from this temporary prompt and is inert; all six cells are
blank. Generic Equation/IC/Parameters/V-WIN editors were not changed. Existing
fresh/held key filtering, including blink waits, consumes held EXIT at the next
boundary: one press returns page2; a second released-and-pressed EXIT returns Graph.

`visibility_prompt_ui` executes both operations on one curve and sparse IC2/IC5
with empty, minus, decimal, exponent, valid-uncommitted and EXE-error drafts.
It verifies exact return softkeys, stable Graph/report, unchanged solve/search
counters, no Partial: Cancelled, held EXIT confinement, fresh EXIT, valid X/Y-CAL,
single-visible bypass, ICPT availability, all-OFF behavior and visibility-only
dimension replacement guard. Original G-Solve numerical tests remain enabled.

## Validation and remaining device work

Baseline57/57 passed; final **59/59 host/UBSan PASS (39.72s)**, preserving all
previous57 groups and adding two focused groups. Clean **28 C-unit SH compile/link,
zero warnings**, **13/13 package checks**. Exact public candidate and tag repeat
these gates; Release VALIDATION.md records their commits/timings/binary hash.
No ASan or SH-emulator result is claimed.

| Bytes | beta.7 | beta.8 | Delta |
|---|---:|---:|---:|
| text |219056|220020|+964|
| data |768|768|+0|
| BSS |72192|72192|+0|
| largest single application frame |2664|2664 (app_run)|0|
| G3A |248504|249468|+964|

SH Document is3044B (v10:3040B); current+recall grow App by8B, from15160 to15168.
Linker alignment absorbs this in total BSS, which remains72192B. Existing load/model
union needs no added allocation. Heap changes: none. The existing IC draft limit
remains1920B. Single .su frames do not establish cumulative stack or heap margin.

`tools/capture_visibility.py` creates [12 production renderer/LCD frames and four
integer3x previews](ui-review/visibility-overview.png), including all eight
requested views. Drawing3x shows the retained Graph and uninterrupted bar;
Output5/10, sparse Graph/selection/TRACE, numeric draft/error and page2 return were
visually reviewed. Existing generators were rerun. These are host production-C
renders, not device photographs, visual mockups or physical timing measurements.

**HARDWARE TEST REQUIRED / HARDWARE RETEST REQUIRED:** all34 current priority cases
in [HARDWARE_RETEST](HARDWARE_RETEST.md) remain pending, including physical LCD,
key repeat, delay/cancellation, MENU/Fugue, native storage migration and STAT import.
