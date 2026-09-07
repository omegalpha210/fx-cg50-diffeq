# Numerical validity audit

Baseline: 6addf4d. The RK4 state/intermediate/derivative guard at magnitude 1e100
remains. ODE_DIVERGED means that application guard, ODE_NONFINITE means NaN/Inf,
and ODE_DOMAIN means an undefined expression. None is user cancellation.

## Confirmed disable paths and changes

Previously TRACE rejected its entire cache if either direction stopped with an
error. G-Solve first tried to reach the left search edge from the IC, aborting if
that failed; a later scan error also hid already-found results. Graph retained
some pixels but labeled the whole calculation Partial. These are code paths,
not a diagnosis of the user's unspecified equation or of physical overflow cause.

model_path_branch now streams trusted bounded samples with a NULL delimiter for
invalid gaps and direction boundaries. Graph resets its previous-point state at
each delimiter, so neither clipping nor drawing connects across a gap. Invalid
regions produce ERROR plus the reason; explicit cancellation remains Partial:
Cancelled. Setup/step-limit errors remain distinct from numerical invalid regions.
No new trajectory array, framebuffer clone or persistent Document field is added.

TRACE retains usable points from both directions in its existing 258-point cache.
Its mask resets continuity at gaps; LEFT/RIGHT selects another finite retained
point (jumping a supplied gap), or stops at the boundary with an inline notice.
Only a cache without usable points is unavailable after numerical invalidity.
Explicit cancellation still discards preparation and restores the prior graph.
Exact x= and Y-CAL independently integrate to the requested x and reject an
unreachable/invalid target without disabling other locations.

ROOT/X-CAL/extrema/ICPT scan both IC-connected branches within Solver X intersect
V-Window X, retaining and sorting results from usable regions. Brackets, zero
runs and secondary-curve continuity reset at gaps. Failed refinement contributes
no result. Extrema at exact zero derivative require trusted values on both sides;
no stationary inflection or unverified gap extremum is manufactured. An ICPT
secondary branch that fails caches its accepted boundary. Outside that boundary
it is not retried for every point; when an inward scan reaches the accepted side,
it queries from the original secondary IC again. A new trusted segment resets
secondary continuity. The failed RK4 state is never used as a new seed. Cancellation and fatal limits still stop the operation.

## Mathematical boundary

An RK4 failure does not supply a new initial condition on the far side. A general
ODE solution cannot be recovered by skipping x, clipping y, using zero, or copying
the last value across a singularity. This implementation does **not** do that.
With the current IVP solver, each direction retains only the accepted prefix
reachable from its IC; the other direction and other independently anchored ICs
remain usable. Post-gap values that cannot be reached from that IC are unknown.
An independent IC is a separate curve, never silently merged into the old one.

The streaming consumers support VALID / INVALID / VALID when a provider supplies
independently trusted post-gap values. The synthetic analytic test exercises that
case, including sorted roots on both islands, without claiming RK4 can cross it.
This distinction preserves mathematical meaning and the numerical safety guard.
Search results mean found in computed valid regions; no-result does not rule out
features in uncomputed/invalid regions or roots missed by the fixed sampling grid.

## Verification and hardware boundary

test_valid_regions uses actual RK4 y'=+/-100y with h=.01: one side exceeds 1e100,
yet TRACE, X-CAL and Y-CAL on reachable points work. Invalid Y-CAL and explicit
cancellation remain errors of their respective kinds. test_segment_stream links
an analytic sample/value provider into the production validity, Graph, TRACE and
G-Solve consumers: valid/gap/valid, >1e100/NaN/Inf, no bridging line/root, TRACE
finite jump and exact pixel restore, before/after X/Y-CAL, sorted X-CAL/ROOT/MAX/MIN results and no invented ICPT across
the gap. An additional real-RK4 SYS test initially failed because a secondary ICPT
lookup near the primary IC was unreachable: y1'=1, y2'=-100*y2, IC1=(0;0,0),
IC2=(4;4,1), h=.001. It now finds the reachable intersection near x=3.986.
The existing numerical, parser, model, G-Solve and lifecycle suites remain enabled.

The common highlight policy maps Black to Blue in TRACE and G-Solve. Other TRACE
colors keep their XOR alternate, all distinct from white; restoring the same XOR
preserves overlaps and original pixels. No second mask or color framebuffer exists.

Physical overflow examples, key latency, LCD Black/Blue contrast and MENU/reentry
are HARDWARE TEST REQUIRED. Host fixtures cannot establish those outcomes.



Public beta validation and memory evidence are summarized in [ACCEPTANCE.md](ACCEPTANCE.md).
