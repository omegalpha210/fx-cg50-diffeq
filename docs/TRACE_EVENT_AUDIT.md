# TRACE event, cache and auto-follow audit

The TRACE/IC milestone follows baseline `0de88c7`. Runtime implementation is
`e2c3e64`; [ACCEPTANCE](ACCEPTANCE.md) records checks and memory. Earlier stride,
field and extension audits remain in Git history. **HARDWARE TEST REQUIRED.**

## Movement and viewport

NORMAL/FAST/FASTER use 1/2/3 times the actual `(Xmax-Xmin)/378` Xdot. The mode
starts at NORMAL, is not serialized, and changes only the selected button state.
Fixed yellow/Bright Green/cyan backgrounds have black text and a black selected
border. F1 x= retains explicit evaluation; F5 LEFT/F6 RIGHT use configured Solver
Xrange endpoints while preserving the selected curve and speed. EXIT restores
the Graph bar; F6 is an ordinary jump event, not a cancellation control.

Connected retained points are linearly interpolated. An exact cached endpoint is
preferred, including the final RK4 sample. Explicit jumps fall back to the nearest
valid cached point with an inline warning when the endpoint is invalid. Ordinary
movement retains directional valid-point fallback. No interpolation crosses a
broken segment link. F1 x= evaluates from the original IC with captured runtime
settings. Guard-valid values do not establish numerical accuracy.

For valid non-phase points, crossing a 10% X/Y margin translates the needed axes
to put the cursor 30% inside. This gives hysteresis, preserves spans and scales,
and permits a far jump in one update. Both axes are committed together and drawn
once. Invalid/nonfinite or magnitude-invalid states never pan the window. A pan
whose span cannot be represented safely is refused. Curve switching follows the
new curve at the retained x when valid. Phase still uses independent-x steps and
has no automatic pan/extension. Intentional viewport changes persist on EXIT.

## Calculated range and transaction

Configured Solver Xrange, runtime calculated extent and displayed V-Window are
separate. TRACE preserves both automatic and manual configured Solver ranges,
h, Step and Max Steps. User Graph/ZOOM/V-WIN actions retain their existing sync
policy; runtime TRACE following does not call that sync helper.

Initial cache extent is the configured range. Pan alone never integrates. In
runtime [-6,6], x=5.9, x=6 and F6 RIGHT reuse the cache. Only an actual requested
target outside the extent starts extension, up to that target without speculative
padding. x=5.95 + FASTER at Xdot=.1 requests and reaches 6.25 in the same input
when valid. Later F6 RIGHT still selects configured xmax=6. Every further
out-of-cache move may need a calculation; there is no background worker/prefetch.

Preflight includes every configured IC/direction before capture. The extended
range is recomputed from original ICs by the existing RK4/segmented provider,
never from interpolated display values. Every accepted step polls cancellation.
One synchronous transaction runs at a time; repeated intentions are coalesced,
and EXIT/MENU retain priority. Success commits the staged cache and one bounded
redraw. Cancellation or work-limit failure retains the old cache, cursor,
window/settings and complete graph. The native repeat transform remains 400 ms
then 125 ms, with one blink timer and restoration on exit.

## Storage budget and rendering

The total is still **258 TracePoint slots**, each x plus at most nine states.
Each enabled family has two branches of floor(258/(2*families)) slots: 129 for
one, 14 for nine, 12 for ten. Capture retains first/last accepted points and
explicit segment links. Many families or a long domain reduce retained detail.

Staging aliases the existing overlay union: 9,504-byte mask plus 14,592-byte
footer, bounded by a static assertion. No second permanent trajectory, full
framebuffer or heap allocation is added. SH committed samples are 21,192 bytes
(+24 of branch metadata); staging/overlay remains 24,096 bytes.

Overlays are restored before staging. On success or failure the footer and
selected mask are rebuilt. XOR highlighting touches only actual base-color
pixels; Black alternates with Blue. Cached redraw streams the optional field
once and polls pending EXIT/MENU without interrupting its bounded raster commit.

## Evidence

- `test_trace_cache.c`: analytic interpolation; exact +/- configured endpoints
  without RHS work; first requested extension; cancellation/preflight preserving
  cache/VRAM/cursor/settings; ten families; retained nine-state/nine-family and
  late-family enable/disable stress; invalid endpoints; joint X/Y one redraw.
- `test_graph.c`: inside/no-pan, upper/lower, joint and far pan, span/scale,
  hysteresis, NaN/Inf/magnitude rejection and phase behavior.
- `test_trace_polish.py`: production key handlers, pixel colors/black text and
  borders, retained curve/speed across endpoint jumps, same-input crossing,
  configured range persistence, displayed Y window and EXIT restoration.
- `test_trace.py`, `test_ux_field.py`, `test_safety_ui.py` and
  `test_key_lifecycle.c`: existing repeats/blink/switch/entry cycles, fixed Xdot,
  beyond-edge movement, work rejection, manual range, ZOOM, key/timer policy and
  F6 jump versus EXIT priority. `test_field.c` and segmented-region tests retain
  overlay/gap/field regressions.

Physical LCD, worst-case out-of-cache responsiveness and cancellation, MENU/Fugue
and SH stack high-water remain in [HARDWARE_RETEST](HARDWARE_RETEST.md).
