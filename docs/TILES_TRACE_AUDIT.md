> Historical milestone record. Beta.7 busy/cancel, SF and Output rules are in
> [FULL_AUDIT](FULL_AUDIT.md) and [UI_CONVENTIONS](UI_CONVENTIONS.md).

> Beta.5 record: Graph INIT, G-Solve overlays and busy rendering are superseded by [OVERLAY_AUDIT](OVERLAY_AUDIT.md). Tile and fixed-TRACE contracts remain.

# Native tiles and fixed-horizontal TRACE — beta.5

Baseline: development 83de313 / public 4deabae (v0.12.0-beta.4).
The baseline rerun passed all 44 host/UBSan groups, a clean 27-unit SH build with
zero warnings, and 13 package checks. Its text/data/BSS were 212548/752/72304 B,
largest application frame 2648 B, and G3A 241980 B. Existing tools were reused.

## Warning rendering

`graph_status` is the common numerical/domain status renderer. Text starts at
logical plot (7,4), physical framebuffer (13,8). The white opaque backplate starts
at logical (5,2), with actual normal-font text width/storage height plus 2px padding per
side. Installed gint reports line_height9 but data_height11; the latter includes
descenders and is used for the 15px-high white backplate. Text width is capped at 284px with the existing ellipsis helper so the right
TIME/PHASE/EVT corner remains free. There is no full-width strip or reduced plot.

Fresh rendering, cached Y follow and active TRACE/G-Solve overlays all use the
same status. The old TRACE-only `ERROR: Numerical limit` is removed; the original
END/Partial reason now survives cached repaint unchanged. Domain failure is not
relabelled as a proven singularity. Event STOP stays neutral `END: Event`.
Phase N1/N2/frozen-field labels and G-Solve selection help use the compact second
row. Cursor/highlight painting is followed by status/labels; results keep their
existing footer. Full/cached base redraw restores the previous text/backplate
when a message changes or disappears. No status framebuffer backup was added.

## TRACE capability and bounds

`trace_prepare` captures a 36-byte SH `TraceViewport`: horizontal min/max/scale,
actual integration-time Xdot and projection/axis metadata. The existing samples
retain their entry extent and linked valid branches. `ui_trace` owns the original
cursor and selected curve on its stack. No trajectory is cloned.

TIME navigation is restricted to the entry X viewport intersected with the
selected curve's connected valid cached component. Configured solver bounds,
cache extent and viewport are distinct; a manual solver range can be wider or
narrower than the window. The current cursor's connected component is grown from
linked branch runs, joining the common initial point where appropriate. It cannot
jump over an invalid gap merely because a later valid sample exists.

The former out-of-cache `prepare_range` branch is removed from `trace_navigate`.
LEFT/RIGHT, held repeats, speed changes, curve changes, endpoint jumps, INIT and
Y follow cannot extend the cache or pan horizontally. Preparation itself remains
bounded/cancellable and can reproduce the already configured graph range.
Normal display interpolation within linked finite samples is retained; cached y
values are display approximations, not new solver evaluations at arbitrary x.

F5 LEFT/F6 RIGHT still target configured solver endpoints. Unreachable targets
clamp to the nearest boundary of the current connected visible component; Event
STOP/numerical boundaries remain respected. FASTER cannot overshoot that boundary.
No fake coordinate is introduced across a missing interval.

`graph_follow_y` translates Y only. It retains X bounds/scale/Xdot, Y span/scale,
solver settings, h/Step, selected curve and target integration x. Existing 10%
trigger / 30% landing hysteresis keeps the cursor inside the plot. NaN/Inf,
magnitude-invalid states or unrepresentable translated spans are rejected. Phase
layer preflight still occurs before the window/cursor transaction commits.

F1 INIT replaces x= input and restores the entry cursor/curve, applying Y follow
if needed. Current NORMAL/FAST/FASTER choice is retained. Equation/IC/solver/Event/
Output/session and the separate Graph entry snapshot are untouched. Arbitrary
numeric TRACE input is gone; G-Solve Y-CAL retains its numerical x query.

The bar is INIT / NORMAL / FAST / FASTER / LEFT / RIGHT. Colors are yellow/black,
orange/black, bright green/black, cyan/black, then the existing blue/white auxiliary
style. The selected speed outline remains. Speeds are 1/2/3 times entry Xdot,
not faster repeat clocks; SYS2 PHASE uses TIME Xdot for integration-time movement.

## PHASE and ordinary graph navigation

PHASE freezes phase_view horizontal state bounds (y1 in SYS2); vertical follow
uses y2. The cursor's integration x retains trajectory order and is never compared
as if it were phase y1. Traversal conservatively stays within the current linked
run of retained in-slab states. It stops before an offscreen sample even if a
later part of the trajectory reenters. It never projects an outside y1 onto a
fabricated boundary point. If a very narrow slab contains no retained state,
TRACE reports unavailable in that view and retains the graph; changing the
ordinary Phase window makes other retained states accessible. No new Phase solve.

General Graph, ZOOM-menu and G-Solve-menu pan are unchanged. On leaving TRACE,
ordinary AUTO TIME pan again synchronizes the configured range using the existing
ceil(Xmin)/floor(Xmax) policy and redraws/integrates. MAN remains manual. Graph
INIT, ORIG and BOX keep their distinct beta.4 contracts.

## Native menu geometry and assets

The framebuffer is 396x224; logical UI is 384x216 at (6,4). Header remains 21px,
help baseline remains y184 and softkeys remain y198..215. Main title is DIFF EQ;
subtype retains First-order equation. No mockup background or battery ornament.

Main is 2 columns x 3 rows in order 1st/2nd, N-th/SYSTEM, RECALL/SAVE. Shared large
tiles are 184x58 at x4/196, y27/89. The last row uses the same widths, y151 and
height25; RECALL/SAVE have no graphic. Subtype is 2x2, using exactly the same first
four tile rectangles/helper and leaving the last-row space empty. Large labels
are drawn at tile y+44; each upper-right badge contains one digit only. Labels,
digits and borders are normal native text/drawing, never rasterized mockup text.

LEFT/RIGHT wraps within the row; UP/DOWN wraps within the column. Digits and
EXE/F6 OPEN use the existing handlers. Main EXIT stays; subtype EXIT returns to
Main's 1st tile. SAVE still uses the existing opening-HOLD-safe confirmation.
F1-F5 remain blank blue; F6 is OPEN. MENU is the only red help token.
Focus changes repaint only the old and new tiles, preserving graph colors under
a clear 2px cyan-blue outline and thin neutral tile border.

Eight original 108x34 graph motifs use 378 uint8 coordinate pairs, 10 small stroke
records and 8 ranges: **832 B of const geometry** plus the separate 8-byte pastel
palette. Runtime menu drawing does not solve ODEs or allocate images. Motifs are
illustrative rather than a classification of equation type by curve shape.
`tools/generate_menu_icons.py --check` verifies the exact firmware geometry.
`tools/capture_tiles.py` exports each PNG by executing the same C renderer.
The eight PNGs total **3378 B** and are review artifacts, not embedded bitmaps.
See [asset provenance](../assets/menu/README.md).

## Evidence and remaining device work

All 44 previous groups remain enabled. Obsolete Main list/x-input/X-extension
assertions are replaced with 2D grid/fixed-boundary/reset checks. Cache-preparation
cancellation rollback, adaptive/Event reports and Phase layer cancellation remain
tested. Three new groups cover fixed TRACE/warning geometry, menu geometry and
actual key/pixel behavior, including zero additional solve counts during long
boundary holds and ordinary Graph expansion after EXIT.

[18 production frames](ui-review/tiles-overview.png), six native plus 3x menu
previews, and eight exact firmware icon PNGs are generated by capture_tiles.py.
They are host renderer captures, not the supplied mockups or device photographs.
Memory totals and exact build results are in MEMORY_AUDIT/ACCEPTANCE; release
VALIDATION records exact public commit/tag and binary checksum.

**HARDWARE RETEST REQUIRED:** the current checklist covers native LCD/outline
visibility, two-tile refresh, repeat/EXIT, Y-follow rounding, Phase boundaries,
Fugue, SAVE/RCL and actual heap/stack margin. Host PASS is not calculator PASS.
