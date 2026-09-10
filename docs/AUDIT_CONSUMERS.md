Current beta.8 changes are authoritative in [VISIBILITY_PROMPT_AUDIT](VISIBILITY_PROMPT_AUDIT.md).
R1/UIR1 are resolved by owner decision, implemented and validated. Drawing now
keeps Graph with a bottom bar; Output has per-IC ON/OFF and color; SAVE is v11;
numeric EXIT cancels immediately to G-Solve page2. The beta.7 report below is
historical evidence, including its former review choices and original controls.

---

# Graph, numerical consumers and domain audit — beta.7

## Scope and evidence

This audit began after the four explicit fixes passed 53 host/UBSan groups and
the strict 28-unit SH/package gates. It covers G-Solve, TRACE, Table/CSV consumers,
Event and SYS2 Phase, with the capability matrix in [FULL_AUDIT.md](FULL_AUDIT.md).
It does not replace the solver, parser or persistence audit, and does not claim
the full Cartesian product of modes and settings was executed.

The 24 selected consumer groups passed after the consumer change; the separate
`test_valid_regions` and `test_segment_stream` groups also passed. Existing
fixtures were read against the implementation, and new gaps are fixed in
[test_full_consumers.c](../tests/test_full_consumers.c). The final complete
suite and exact public build are reported in the release validation.

| Consumer / class | Executed representatives and invariants | Evidence |
|---|---|---|
| G-Solve operations | ROOT, MAX, MIN, Y-ICPT, X-CAL, Y-CAL, ICPT; scalar polynomial/constant/exponential, harmonic second order, RK4/RK45; zero, multiple and no results; Y-window independence | `test_gsolve`, `test_rk45_model`, `test_graph_overlay_ui` |
| Selectable curves | Single graph skips selector; second order and N-th outputs; first-order IC1/2/5/10; sparse SYS9 mask; ordinal -1/count rejected; same curve ICPT rejected | `test_full_consumers`, `test_output_instances`, `test_output_instances_ui`, `test_graph_overlay_ui` |
| G-Solve domain | Valid prefixes, two disconnected synthetic islands, secondary IC initially unreachable then reachable, Event STOP endpoints/common domain; no invented cross-gap root | `test_valid_regions`, `test_segment_stream`, `test_event_consumers`, `test_full_consumers` |
| G-Solve result boundary | 32 sorted finite roots retained; result marker keeps X and Y span, follows Y only when needed; cycling does not solve again | `test_full_consumers`, `test_graph_overlays`, `test_graph_overlay_ui` |
| TRACE | Increasing/decreasing linear curves, oscillation, multiple curves/ICs, RK4/RK45, valid-domain endpoints, Event STOP, PHASE exit/re-entry boundary, INIT/speed, held arrows | `test_trace_fixed`, `test_trace_cache`, `test_trace_polish`, `test_tiles_trace_ui`, `test_event_consumers` |
| TRACE invariants | Entry X bounds/scale/Xdot fixed, Y-only follow, no horizontal extension solve, connected visible samples only; preparation/Phase follow cancel retains window/cursor/report | `test_trace_fixed`, `test_phase_cache`, `test_rk45_model` |
| Table order / terminals | Strictly ascending x, TOP/BTM/MID, common zero/nearest midpoint, exact last row, numerical and Event terminals, finite cells/explicit missing cells | `test_table_unified`, `test_table_storage`, `test_event_consumers`, `test_rk45_model` |
| Table output mapping | IC1/2/5/10 independent solution columns; sparse SYS9 component mapping; all outputs OFF retains only x; nic=0 safely rejected | `test_full_consumers`, `test_table_unified`, `test_output_instances` |
| Table / CSV / STAT | Both export entry points use the same `TableIndex`, labels, page reader and cell-validity bits; 998 data-row cap; interrupted export cleanup | `src/storage.c:export_table`, `test_table_storage`, `test_output_instances`, `test_rk45_ui` |
| Event direction / action | ANY/RISING/FALLING mean increasing-x direction in forward/backward integration; MARK/STOP; initial zero and zero plateau; repeated crossings; 10 ICs and SYS9 | `test_events`, `test_event_consumers`, `test_events_ui` |
| Event safety | Rejected RK45 trials cannot trigger; singular event expression does not create a pole crossing; invalid event gaps; cancellation during refinement; 32-marker cap and evaluation budget | `test_events` |
| Phase | TIME/PHASE separate windows/cache, state-coordinate axes, FIELD, NULL, EQPT, INFO, hidden TIME components, autonomous classification and non-autonomous restrictions | `test_phase`, `test_phase_cache`, `test_phase_renderer`, `test_phase_ui`, `test_rk45_model` |
| Phase bounds | 16 equilibrium candidates, truncation, grids through 20, 8192 evaluation budget; zero/invalid vectors, poles do not make nullclines, cancellation preserves results | `test_phase` |
| Busy / state | Dedicated delayed Table/Drawing canvas, four spinner frames, white body and no softkeys, header-only refresh, exact VRAM preservation; Table page reuse; rollback and EXIT/HOLD | `test_busy_screen`, `test_busy_screen_ui`, `test_graph_overlay_ui` |

## AUTO-FIX: CONS-1 — missing Event terminal G-Solve result (P1)

**Reproduction before the change:** `y'=y`, `y(0)=1`, Event `y-10`, STOP,
X-CAL target 10 returned no result in both RK4 and RK45, although the accepted
Graph/Table endpoint was approximately `(ln(10),10)`. For the harmonic second
order example, Event `y-y'`, STOP, ICPT returned zero endpoints under RK4 and
only one under RK45 instead of both valid terminal intersections.

The scanner required an exact zero or a later sign change. Event refinement can
leave a tiny residual at the last accepted point, where no later sample exists.
ICPT also reintegrated the other component of the same state vector and could
reject its STOP status even at the requested endpoint. A separate refinement
query could cross STOP by a few floating-point units and erase the trusted
stream endpoint candidate.

The minimal consumer correction in `src/ode/gsolve.c`:

- Same-family ICPT reads both components of the same accepted state, including
  the refinement query; cross-family integration remains separate.
- An exactly requested secondary STOP state is usable, while its directional
  boundary remains recorded so later samples cannot continue through STOP.
- A completed Event STOP may recognize its retained ROOT/X-CAL/ICPT endpoint
  within the Event refiner's **local x precision**, converted to metric units
  with the actual derivative at the accepted state. The allowance is capped by
  existing search precision. No additive absolute-y epsilon, whole-window
  allowance or coarse secant slope is used. Constant nonzero curves get no
  residual allowance. MAX/MIN endpoint conventions are unchanged.
- A failed interior query does not discard an otherwise trusted Event endpoint;
  cancellation and actual invalid samples still invalidate temporary candidates.

Regression coverage includes both methods; forward/backward ROOT and X-CAL;
two harmonic ICPT terminal points; exact three-point directed-event ICPT sets;
distinct-family vector endpoints; and a secondary STOP at x=.25 which must
exclude an apparent intersection at x=.5. Y-CAL at the endpoint succeeds, while
a later query returns Event STOP without changing its output point.

Negative guards reject a constant `1e-90`, an unrelated stopping Event, a target
`1e12+1.001` beyond a STOP at x=1, target 1.5 with a ±1e9 viewport and STOP at
x=1, and a quadratic root at 1.000001 beyond STOP at x=1. These guards prevent
false roots from absolute scale, viewport span or local curvature. The large
offset and wide-window counterexamples were independently reviewed and retested.

No RK4/RK45 stepping, Event refinement, parser, save schema, user workflow or
production heap allocation changes were needed. A small terminal state vector
is local to the existing G-Solve search; no trajectory/framebuffer was added.

## Review-required issues

No additional consumer feature or ambiguous UX change was implemented. The
shared component ON/OFF versus per-IC visibility decision remains the review
item documented in [MULTI_IC_COLOR_AUDIT.md](MULTI_IC_COLOR_AUDIT.md).

## INFO / intentional limits

- **CONS-I1, bounded numerical discovery:** 32 G-Solve results and 32 Event
  markers are bounded stores, not a claim that all roots/crossings are retained.
  Coincident curves have a continuum of intersections; the scanner reports
  bounded valid representatives, without promising a unique count. Separate
  numerical trajectories whose apparent endpoint coincidence exceeds local
  Event precision can conservatively return no intersection.
- **CONS-I2, display approximation:** TRACE/cached rollback uses the existing
  258-point shared display budget and connectivity flags, not fresh numerical
  table values. A completed RK45 Max-steps prefix followed by cancelled pan
  differed from the original full-resolution raster at two pixels in one
  fixture. It exactly matched the committed-cache repaint and preserved the
  window, solver settings and canonical report. Ordinary fixtures still verify
  byte-exact plot restoration. No broad domain shading is introduced.
- **CONS-I3, remaining wait paths:** ZOOM AUTO, Phase EQPT and STAT retain their
  existing cancel/static-feedback policies. The existing host Phase benchmark
  measured EQPT at 0.039–0.119 ms/pass for oscillator/nonlinear examples, FIELD
  20×11 at 0.042–0.048 ms and NULL 20×20 preflight at 0.115–0.221 ms. This does
  not establish calculator latency. No additional spinner was added on the
  basis of speculative waiting time; slow expression/SH timing needs hardware
  measurement before a further busy-UX decision.
- **CONS-I4, mathematical scope:** Bounded Phase candidate search is not an
  exhaustive equilibrium proof. Non-autonomous trajectories are allowed,
  FIELD/NULL freeze the reference x, and EQPT is unavailable. There is no new
  stiff solver, multiple-event system or continuation across a solver failure.
  A legacy field-only document with no IC has no Table data and safely returns
  unavailable input rather than indexing a nonexistent trajectory.

## Findings and hardware boundary

This consumer audit found **one P1 AUTO-FIX (CONS-1), now fixed**, and no remaining
P0/P1 defect in its executed matrix. There are no additional P2/P3 AUTO-FIX or
consumer REVIEW REQUIRED items; the four INFO items above are limitations, not
four newly discovered defects. Whole-project counts are consolidated separately.

**HARDWARE TEST REQUIRED:** physical key repeat and release ordering, MENU/Fugue,
RTC cadence, complete/partial LCD uploads and contrast, slow calculation cancel
latency, and cumulative stack high-water. Host LCD shadow captures prove the
software's intended transfer contents, not the physical driver's performance.
