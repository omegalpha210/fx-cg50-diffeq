# TRACE event audit (baseline 06a491e)

## CONFIRMED ROOT CAUSE

Source ui_trace called graph_render, graph_highlight_curve and ode_integrate on
every directional move and timer wake. Those calls polled ui_cancel. In 06a491e,
ui_cancel returned true when its eight-event retained-input queue became full,
even if every event was a normal directional repeat. graph_render had already
cleared VRAM and then displayed the resulting ODE_CANCELLED partial curve. EXIT
could remain behind those repeats. This exactly establishes a normal-repeat to
cancel/partial/starvation code path; elimination of the physical report still
requires a device retest. It is not a RAM or CPU-overload diagnosis.

## CONTRIBUTING FACTOR

Installed gint keydev.c generates KEYEV_HOLD with a default 400 ms initial delay
and 40 ms interval (25/s); its native queue has 32 slots. No application trace
acceleration existed. Repeated full integration and redraw made eight pending
events reachable. TRACE clamped x to the current view/solver intersection; it
did not extend the domain. Graph arrow pan separately updates V-Window and calls
model_sync_solver_window, extending the integration domain only in AUTO mode.

## FIX

TRACE now prepares one selected-IVP cache in bounded scratch, with at most 129
exact RK4 samples in each direction, and a one-bit selected-curve raster mask.
Ordinary LEFT/RIGHT moves between retained canonical samples; blink and movement
perform no RK4 calls. For very long trajectories the retained Step grid is
decimated to fit the cap (coordinates show the actual retained x). X= still
performs one explicit exact query. Changing the selected curve may prepare it
once. No interpolation changes numerical results and no full framebuffer copy
is allocated. The sample budget does not limit the number of ICs/states.

Preparing scratch never alters plot VRAM. Usable finite points are published even
when a numerical guard stops a branch; explicit cancellation discards preparation
and leaves the graph untouched. A cache without usable points remains unavailable. A reversible XOR mask and
crosshair plus a saved 19-row footer restore every plot pixel on EXIT. TRACE is
a bounded Graph mode, so EXIT restores softkeys without a dispatcher redraw.
TRACE stays within its calculated domain; the already-working Graph pan/auto
extension remains separate, with no repeat-triggered extension thrashing.

TRACE polling coalesces directional repeats to one intent and gives EXIT/MENU/
BACK priority. Ordinary repeats never return solver cancellation. A custom
gint repeat profile uses 400 ms then constant 125 ms (8/s), restores the previous
profile on exit and does not mutate gint's global standard-repeat intervals.
This conservative ceiling reduces the previous 25/s input load; real target
latency/tuning is unmeasured, not inferred from host timing. Timers are stopped
on every return; MENU retains the normal OS-world mechanism.

## HOST VERIFICATION

trace_repeat simulates 50 HOLD events (two seconds at the previous 25/s rate)
for both LEFT and RIGHT, then immediate EXIT. It checks no Partial: Cancelled,
zero additional movement/blink integration, immediate Graph bar and exact plot
hash restoration. It also repeats entry/exit, blink and curve switches 100 times
and checks Graph pan past the original domain. All 17 host/UBSan groups pass;
color tests also verify six exact rendered colors and distinct black highlights. Target-branch key_lifecycle
feeds a pending repeat flood followed by EXIT and verifies priority/coalescing,
bounded repeat intervals and restoration, released-HOLD discard, deferred MENU
priority, plus 100 balanced timer lifetimes. MENU during preparation leaves the
plot intact and retries the cache safely on resume.

## HARDWARE RETEST REQUIRED

Hold LEFT and RIGHT for 2-5 seconds, release and immediately EXIT. Verify complete
graph, no partial/cancel, immediate control response, curve switching, blink,
X=, MENU resume and panning. The host adapters do not emulate keyscan timing,
LCD, native gint MENU or OS scheduling. The fixed TRACE buffers total 44,756 bytes, without heap allocation. Static memory and cache costs are measured
in MEMORY_AUDIT.md; no claim of physical runtime high-water is made.


## Follow-up against 6addf4d

Black's alternate is now Blue through the shared graph_highlight_color helper;
other TRACE colors keep their XOR mappings. A two-byte highlight XOR value fits
alongside existing static objects; cache/mask/footer sizes are unchanged.

Numerical invalidity no longer rejects both branches. NULL delimiters reset mask
continuity, finite retained points remain navigable, and a failed boundary move
can show TRACE: invalid region. Independent exact x= queries reject unreachable
values. Cancellation of an exact query routes pending EXIT/MENU through the same
priority handler before waiting again, avoiding a stranded control event.
See [NUMERICAL_VALIDITY_AUDIT.md](NUMERICAL_VALIDITY_AUDIT.md) for the IC-connected
prefix guarantee and synthetic post-gap fixture boundary.
