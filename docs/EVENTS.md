# Event Detection and Solver Diagnostics — v0.12.0-beta.1

One document has one Event definition: Enabled, Expression, Direction and Action.
Factory settings are OFF, empty expression, ANY and MARK. This feature works with
both existing solvers; it adds no ODE method or production dense interpolant.
All new physical-calculator paths remain **HARDWARE TEST REQUIRED**.

## Entry and expression scope

Parameters **F2 ADV → F1 EVENT** opens Event Settings; **F2 INFO** opens the
read-only report. Since beta.2, Parameters has **F1 INIT**, preserving its
method-specific reset behavior and Event settings. F3 V-WIN/F4 OUTPUT/F5 SET/F6 GRAPH
remain on the normal Parameters bar. EXIT closes just the submenu or child screen.
Editing Event settings performs no solve or file write. GRAPH validates and
compiles the expression once, including evaluation at every configured IC.
An ON empty/invalid/domain-invalid expression prevents calculation. OFF preserves
the expression/direction/action and bypasses all Event evaluation in integration.

The existing expression editor, FUNC/VAR menus and parser are reused. `x` is the
independent coordinate. In first-order modes `y` is the solution. In second-order
and N-th modes `y1` means y', through `y8` for the ninth-order state. In SYS, `y1`
through `yn` are the state components; `y` aliases the first state. N-th → SYS
converts the Event expression transactionally along with the ODE expressions,
including a stored OFF draft. An overlong conversion changes neither expression.
Trigonometry uses radians and multiplication must be explicit.

## Detection, direction and the initial zero

Detection uses finite Event values at consecutive **accepted numerical steps**.
For RK45 these are tolerance-accepted internal states, not rejected trials and
not merely displayed output points. Sign comparisons do not multiply the values.
Event values outside ±1e100, NaN/Inf or expression-domain errors break continuity;
Event analysis skips that interval and counts it as unavailable. The valid ODE
itself can continue. An invalid interior refinement is also skipped; its endpoints
are never connected into an asserted root. ODE-invalid gaps remain disconnected.

ANY accepts either crossing. RISING means negative to positive as **x increases**;
FALLING means positive to negative as x increases. The same meaning applies during
backward integration. Direction is determined using the endpoint sign progression
and the sign of the x step, not the order in which the loop visits points.

At an exact initial zero, ANY triggers immediately. A directed Event waits for
the first valid nonzero progression: its sign and increasing-x direction decide
whether the *original initial point* matches. A constant-zero interval does not
repeatedly trigger; an identically zero directed Event has no hit. For directed
STOP, this decision uses a bounded scratch probe before any later points are
published, so a zero plateau cannot leave samples beyond an eventual initial STOP.
The probe uses the selected method and the same output partition when applicable;
all of its work is counted. A nonmatching probe is followed by the normal stream.
An invalid Event value ends the initial-direction probe without claiming a root.

After a hit at an exact endpoint zero, detection rearms after a nonzero value.
The same root x is also suppressed within a scale-aware 64-epsilon tolerance.
An initial hit is counted once per IC across its forward/backward branches, while
both branches can independently report STOP at that point. No wide fixed zero
band merges nearby crossings.

## Refinement and commit

An accepted sign bracket invokes at most **48 safeguarded secant/bisection
iterations**. Secant fractions outside [0.1,0.9] use the midpoint; every third
iteration also bisects. Each trial x is evaluated by integrating in scratch from
the original accepted reference state with the **currently selected solver**.
RK4 retains fixed h with a clipped last substep; RK45 lands directly at the trial
x. Refinement integrations disable Event hooks, preventing recursive root searches.
No linear endpoint interpolation is presented as an accurate Event state.

Let epsilon be DBL_EPSILON and S the larger original endpoint |E|. An exact zero
is accepted. Otherwise acceptance requires either residual |E|/S ≤128 epsilon
together with a bracket width ≤sqrt(epsilon) times the larger of |trial x| and
original step span, or width ≤64 epsilon times the bracket's original x scale
and residual ≤sqrt(epsilon). The x scale includes endpoint magnitude, span and
DBL_MIN. Failure to make representable progress or satisfy these checks within
48 iterations safely skips the candidate. A sign-changing pole with a residual
that does not shrink is not accepted just because the bracket is narrow.

MARK preserves the original canonical accepted endpoint. STOP commits the
successful refined x/state as the terminal valid point, discarding the overshoot.
Failed analysis leaves the original valid numerical step intact; cancellation or
exhausted shared work budget discards scratch and stops at the prior canonical
point. A successfully detected STOP before a later ODE singularity reports Event.
Accepted-endpoint detection cannot recover a crossing inside an ODE step whose
endpoint never passed numerical validity checks.

## Work, storage and consumers

Each IC and each integration direction stops independently. The run stores at
most **32 markers** (x, up to nine state values, family and direction). Total hits
keep counting after storage fills; MARK integration continues. Action belongs to
the one report definition, so it is not duplicated in every marker.

TIME draws a black outlined, orange centered square on the first visible output
for each marker. Phase projects the same state onto its axes; the square differs
from equilibrium diamonds. Rendering visits only the bounded list, performs no
solver calls, and clips pixels to the plot. Existing TRACE cache/overlay storage
is reused. A second small marker list stages TRACE work so cancelled extensions
preserve the previous cache, markers and report. Successful extensions replace
them together; switching an already cached SYS2 TIME/PHASE view does not rescan
the Event trajectory. Separate first-order families retain their own STOP bounds.

STOP is a valid terminal condition, not an invalid gap. Table TOP/BTM include
the exact terminal x/state and show `END: Event`; cells beyond an individual IC's
STOP are empty. RK45 terminal rows replay the index's trajectory/output partition
to recover the same root state, with no interpolation. MARK leaves the existing
Table grid unchanged. TRACE snaps to STOP and cannot extend beyond that branch;
G-Solve searches only the valid prefix. Other families/directions still run.
An earlier family error is not hidden by a later family's successful STOP.

Both ordinary and refinement work share 200,000 attempt, 800,000 RHS-call and
2,400,000 RHS×dimension limits. Event evaluations have a separate 200,000 limit.
The existing per-call Max steps and cancellation checks remain. Counters are
uint32_t and cannot overflow before these bounded work limits. Empty OFF Event
settings incur no parser/evaluator calls per step.

## Solver Info meaning

INFO shows the **last trajectory run**, aggregated over its computed ICs and
directions: a new GRAPH calculation or successful TRACE preparation/extension.
Configured IC count, state dimension and requested range describe that run;
the actual terminal range may be shorter. Table, G-Solve and Phase field,
nullcline or equilibrium probes do not overwrite this report. Editing settings
does not relabel an old report as a new calculation.

- RK4: Method, Status, ICs, State dim, range, h, Steps, actual RHS evals, h min/max.
- RK45: Method, Status, ICs, State dim, range, Initial h, RelTol, AbsTol, Accepted,
  Rejected, Attempts, actual RHS evals, h min/max.
- Event: ON/OFF; when ON, Direction, Action, total Hits, Stored/32 and Unavailable.
  A single STOP hit also shows Stopped x. Multiple hits remain an aggregate.

Accepted/Steps counts successful **numerical trials**, including refinement and
initial-direction scratch work, and a tolerance-accepted candidate truncated by
STOP. It is not the displayed-row count or only canonical ODE steps. Rejected
counts unsuccessful numerical trials, including failed stage evaluations;
Attempts is Accepted+Rejected. Interrupted or failed numerical trials can count as
rejected with fewer stage calls than a complete trial. RHS is counted
at real solver function calls, not inferred as four times a display step count.
One SYS RHS call evaluates the full vector. Phase analysis calls are excluded.
h min/max are magnitudes of accepted numerical trial steps including scratch.

Rejected RK45 trials are normally part of adaptive error control, not necessarily
an error. COMPLETE, EVENT, CANCELLED, MAX STEPS, WORK LIMIT, STEP UNDERFLOW,
MAGNITUDE LIMIT and DOMAIN ERROR remain distinct. A new Graph run clears old
counters and retains performed work/status even on failure/cancellation. Failed
transactional TRACE extension keeps the previous successful report with its
graph. INFO UP/DOWN scrolls; EXIT returns. It never integrates, writes files,
changes parameters or resets counters. New document/load/recall resets it to
`No solver run yet`. Diagnostics and marker arrays are runtime-only.

SAVE **v10** adds Event settings to current and recall Documents. Independently
frozen v3–v9 readers initialize Event OFF. v9 preserves RK45 preferences, and
v8/v9 preserve independent Phase windows. Loading does not rewrite old sessions.

## Quantitative verification

Reproduce with `tools/test.sh` and `build-host/test_events`. Host double results
below use h=.1, RK45 RelTol=1e-6/AbsTol=1e-9, y(0)=1 and direct target queries.
The exact formula, not pixels, is the reference. Graph output landing can change
step partition and counts; the screenshot's whole-Graph totals are different.

| Test / exact root | Method | Computed x | Absolute x error | Accepted / rejected | RHS |
|---|---|---:|---:|---:|---:|
| y'=y, E=y-10 / ln(10) | RK4 | 2.3025868565885 | 1.76359e-6 | 52 / 0 | 208 |
| y'=y, E=y-10 / ln(10) | RK45 | 2.3025847322732 | 3.60721e-7 | 218 / 0 | 1526 |
| y'=y², E=y-10 / .9 | RK4 | .900713818333163 | 7.13818e-4 | — | — |
| y'=y², E=y-10 / .9 | RK45 | .900000223504279 | 2.23504e-7 | — | — |

RK4 square-problem error improves by more than 8× when h decreases from .1 to
.05. The root state has |y−10|<1e-10, but that small residual is **not** a guarantee
of equally small analytic x error: the underlying solver's solution error remains.
Refinement can cost many solver calls; no FSAL optimization is included.

All 39 strict host/UBSan groups include the previous 36 regression groups and
three new Event numerical, consumer and UI groups. They cover forward/backward
ANY/RISING/FALLING, initial roots and zero plateaus, oscillatory MARK/dedup, >100
hits with 32 stored, rejected RK45 trials, singularity precedence, invalid Event
gaps/poles, refinement cancellation/work limits, ten independent ICs, SYS9,
N-th→SYS Event conversion/overflow, TIME/Phase marker reuse, Table terminal state,
TRACE extension/cancellation, G-Solve bounds, actual RHS counters, Graph cancel
status, v10 roundtrip/frozen v3–v9 migration and repeated no-work/no-I/O menus.
Physical SH/math-library accuracy, stack high-water and key timing are pending.

## Limits and primary reference

Endpoint bracketing can miss multiple zeros inside one accepted step when its
endpoint signs agree, and tangential roots that do not land on a sampled zero.
Smaller RK4 h or tighter RK45 tolerances can help. RK45 controls **ODE solution
error**, not Event oscillation frequency: reducing Initial h alone does not cap
later adaptive steps and cannot guarantee finding every rapidly oscillating E.
Finite bracketing/refinement cannot prove continuity of an arbitrary expression.
These are numerical Event estimates, not analytic root certificates.

The inspected primary reference is SciPy 1.16.2's
[solve_ivp Event implementation and limitations](https://github.com/scipy/scipy/blob/v1.16.2/scipy/integrate/_ivp/ivp.py).
It illustrates accepted endpoint bracketing and direction masks. This project
implements its own bounded secant/bisection with solver target landing; it does
not copy SciPy code, use its dense interpolant/Brent solver or add a dependency.
Existing RK coefficients and accuracy limits remain documented in
[RK45_NUMERICS](RK45_NUMERICS.md).

Multiple Event definitions, FSAL, sweeps, Poincaré/bifurcation, stiff/implicit
solvers, new ODE methods, CAS, Laplace and special functions are deferred.
