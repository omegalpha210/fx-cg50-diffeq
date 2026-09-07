# TRACE event, cache and auto-follow audit

Current base: `f7e1093`; stride/field implementation: `1f354de`. Earlier cache/auto-follow milestone: `1ad95d8` (rollback `671186f` remains in history).
Current behavior is verified by production-source host tests and SH compilation;
physical key timing and LCD behavior remain **HARDWARE TEST REQUIRED**.

## Movement and viewport

The previous code moved to the next retained RK4 sample. Long trajectories used
sparse retained points, so one key could move much farther than a screen pixel.
TRACE advances by the existing fractional `model_xdot()` value `(Xmax-Xmin)/378`
multiplied by the local stride: F2 NORMAL=1, F3 FAST=2, F4 FASTER=3. It linearly
interpolates connected cached states. The mode starts at NORMAL on every entry
and is not serialized. A button changes only that small local value and its
outline: x, curve, blink phase, window, h, Step and solve count stay unchanged.
One effective arrow computes one target, with no synthesized keys or Xdot rounding.
A nonfinite or unrepresentable target is rejected before cache fallback. Phase
TRACE uses the same independent-x stride and retains family selection; phase
auto-follow/extension remains disabled. This changes navigation spacing, not
numerical accuracy or the repeat interval.

Entering the outer 10% of the X viewport requests a 20% horizontal pan via the
same `graph_zoom` geometry used by Graph, G-Solve and ZOOM arrow pan. UP/DOWN still
selects visible curves; it never pans TRACE. Selection preserves x where the new
curve has valid cached data. Blink phase and current pointer are retained through
successful pan. EXIT restores the current committed viewport, not the original
viewport before intentional pan. Graph/ZOOM pan and AUTO/ORIG never change h.

## Bounded cache and numerical meaning

A total of 258 `TracePoint` slots is shared by enabled IVP families. Each family
receives two branches with `floor(258 / (2 * enabled_families))` slots per branch:
129 for one IC, 14 for nine. Each point holds x and at most nine states. Capture
uses a bounded stride, always retaining the first and last accepted sample.
Explicit link flags mark segment starts; interpolation and cached polylines never
bridge a validity gap. Replacing the last full slot conservatively retains a
broken link. Invalid RK4 stages cannot enter the cache.

Displayed TRACE y values and cached pan curves are approximations, especially for
many ICs, long domains or rapid oscillation. They are not fresh RK4 evaluations.
F1 x= performs an explicit integration from the original IC, using the runtime
extent and unchanged h. This also works after moving outside a manual Solver
range. Normal Graph and G-Solve still use their existing streamed numerical paths.
The selected XOR mask is restricted to actual base-color pixels of the canonical
plot, preventing sparse cache geometry from drawing synthetic highlight strokes
on white. Black's alternate remains Blue. EXIT restores all changed pixels.

## Runtime extension and cancellation transaction

The initial cache covers the current Solver range. A pan beyond it, or a TRACE
step at a narrower manual range boundary, requests a range extended by half a
viewport beyond the requested edge. Already computed range is retained. A later
pan inside that range only rebuilds axes/slope field and cached polylines; it does
not integrate. One synchronous extension can run at a time. At a numerical guard,
TRACE stops or uses a separately supplied trusted segment; RK4 never invents a
restart beyond a failed step.

An extension preflights all configured IC paths before capture. It recomputes the
requested range from original ICs with the existing RK4/provider, rather than
continuing from rounded/interpolated cache values. Each successful RK4 step polls
cancellation. The old canonical plot and sample cache remain intact while the new
samples are staged. Success publishes the fixed-size cache, then performs a
bounded pixel/RHS redraw. Cancellation or a work-limit failure discards staging;
old cache, document window, manual parameters and VRAM remain unchanged.

The staging array is a **union with the pre-existing overlay storage**: a 9,504-byte
bit mask plus 14,592-byte footer. Before staging, XOR/pointer/footer overlays are
restored to VRAM, so that memory can be reused. After success or cancellation,
the footer is recaptured from the canonical plot and the selected mask rebuilt.
There is no added framebuffer, heap allocation, or second permanent trajectory
array. A compile-time assertion bounds staging to the existing 24,096 bytes.
The committed cache remains 21,168 bytes on SH. The earlier auto-follow milestone
added 496 BSS bytes; this stride/field milestone adds 16 total linker BSS bytes.
The Document is now version 5, but its two new field bytes fit prior tail padding:
SH Document remains 2,920 bytes, App 13,960 bytes and the work union is unchanged.
Cached redraw streams the field once, polls queued EXIT/MENU each column, and
finishes the bounded raster commit before returning; it allocates no field cache.

AUTO Solver ranges follow committed view changes through the existing sync
helper. Manual Solver xmin/xmax remain byte-for-byte unchanged; only the temporary
TRACE extent grows. h, Step and Max Steps remain unchanged in both modes.

## Input ownership and evidence

The earlier repeat regression treated a full event queue as cancellation after
clearing graph VRAM. That path remains removed. `ui_trace_cancel` coalesces repeat
intent and prioritizes EXIT/MENU/F6. Ordinary arrows do not cancel integration.
The target repeat override remains 400 ms then 125 ms with no acceleration; it is
restored when TRACE exits. The existing single blink timer lifecycle is unchanged.

- `test_trace.py`: 50 holds each way, 100 blinks, 100 curve-switch and entry cycles;
  no move/blink solves and exact plot restoration before any pan.
- `test_trace_cache.c`: fractional movement, analytic interpolation check,
  cancellation after capture has begun, identical old VRAM/cache/view on failure,
  cache-only subsequent pan, manual range preservation, nine ICs × nine states,
  and a single enabled late-index family.
- `test_safety_ui.py`: 310 pixel moves in each direction past x=±10, at most two
  extension passes, same-x switching/blink, exact x= beyond manual range,
  unchanged manual range/h on return, persistent ZOOM arrow pan.
- `test_key_lifecycle.c`: production target input/cancel/timer branches, pending
  repeats, EXIT/MENU priority and re-entry policy. This is not OS emulation.
- `test_ux_field.py`: fixed Xdot=.025, x=.6, both directions at 1/2/3×; mode
  changes without movement/solves, fresh NORMAL, 260 repeated FAST/FASTER moves,
  curve switching/blinks, numerical/work boundaries and normal EXIT restoration.
- `test_field.c`: field/TRACE overlay exact framebuffer restoration, including
  valid solution data beside invalid field grid points.
- Existing segmented provider tests still prove gap safety and Black↔Blue.

See [ACCEPTANCE.md](ACCEPTANCE.md) for build/hash/memory and
[HARDWARE_RETEST.md](HARDWARE_RETEST.md) for physical follow-up.
