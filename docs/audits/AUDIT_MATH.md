# Mathematical and structural audit — beta.7

## Scope and evidence

This pass began after the four explicit beta.7 fixes passed 53 host/UBSan groups
and a strict 28-unit SH build/package verification. The capability matrix was
recorded first in [FULL_AUDIT](FULL_AUDIT.md). The solver architecture, RK4/RK45
arithmetic and expression parser are preserved; this pass found no new objective
numerical or indexing defect requiring a production change.

The new `full_math_matrix` test fills concrete gaps in the previous coverage:
both methods at every N-th/SYS dimension, distinct derivative indices and
conversion at every order, forced Second coefficients, all scalar subtype signs,
nonzero common IC x, and regular-domain logarithm/square-root/tangent values.
It makes 682 analytic/mapping comparisons in one bounded test group. This count
is evidence of tested cases, not a correctness proof for arbitrary expressions.
The complete application is also checked by the retained dedicated tests below.

Host command: configure the existing `tests` CMake project with Clang and its
default UBSan, then run `full_math_matrix` and the listed numerical groups.
No numerical package/runtime dependency is added. Fourteen selected groups passed
in 0.58s, including the final tangent/domain assertions.
The final whole-project gate is reported in FULL_AUDIT/ACCEPTANCE.

## Mode, dimension and analytic equivalence classes

| Class | Direct structure and mapping | Analytic integration, both RK4/RK45 and directions |
|---|---|---|
| General 1 |one state and y0|y'=y →exp(x); y'=-2y →exp(-2x); y'=x →1+x²/2|
| Separable 1 |f(x)g(y)|f=.5,g=y →exp(.5x)|
| Linear 1 |g(x)-f(x)y|f=-.5,g=0 →exp(.5x)|
| Bernoulli 1 |g(x)y^n-f(x)y|n=1,f=.25,g=.75 →exp(.5x); n=0 →2-exp(-x); n=2 logistic →2/(1+exp(-2x))|
| Second 2 |state[y,y']; RHS[y',h-fy'-gy]|y''+y=0 →[cos,-sin]; y''-y=0 →[cosh,sinh]; y''+2y'+3y=6exp(x) →[exp,exp]|
| N-th 1..9 |n states, y/Y1..Y(n-1), all n IC values|y^(n)=y with every initial derivative 1 →exp(x) in every state|
| N-th 9 polynomial |highest IC derivative is 1|y^(9)=0 →state j=x^(8-j)/(8-j)!|
| SYS 1..9 |m states, y1..ym, unequal state values and rates|y_j'=((-1)^(j-1)j/10)y_j, y_j(0)=j →j exp(((-1)^(j-1)j/10)x)|
| SYS 2 coupled |horizontal y1, vertical y2|y1'=y2,y2'=-y1 →[cos,-sin]; radius invariant 1|
| First IC 1/2/5/10 |common x0=.25; unequal y0 values|every independent decay trajectory reaches both off-grid targets with its own amplitude|

Each N-th dimension separately evaluates a weighted expression containing every
allowed derivative with distinct values, checks the shift rows, rejects Y_n,
converts to SYS, and compares the RHS and numerical result while asserting the
full IC array is unchanged. This avoids relying only on equal-component examples
that could hide swapped indices. All mode defaults expose the intended state
dimension and one complete vector where applicable. Existing IC tests cover
1/9-state full vectors, 10/11-value list boundary, malformed/oversized input,
duplicate values and transactional rejection.

The original local DIFF EQ manual was visually checked at PDF pages 4, 6, 8
(printed 3-2-1, 3-2-3, 3-2-5). Its Separable, Linear and Bernoulli definitions match
the direct mappings above. Reference artwork and the private PDF stay local.

## Errors and invariants

For the new matrix RK4 uses h=.005; RK45 uses initial h=.4, RelTol 1e-9,
AbsTol 1e-12. Analytic comparisons use off-grid coordinates in both directions;
maximum observed absolute errors across 682 comparisons were:

| Method | Maximum observed absolute error | General asserted bound |
|---|---:|---:|
| RK4 |1.36156508e-9|3e-8|
| RK45 |3.50168605e-9|3e-8|

Polynomial/mapping comparisons use tighter bounds (3e-9 and 2e-14 respectively).
These are observed double-precision host errors for the selected finite test
intervals. They do not assert a global error bound for arbitrary ODEs or all
configured tolerances. SH numerical execution remains a hardware test.

Every successful new matrix solve checks exact requested endpoint equality,
finite/magnitude-bounded state, operation budgets and accounting. Event-free
RK45 satisfies attempts=accepted+rejected and RHS=7×attempts; RK4 successful
steps=accepted and RHS=4×steps. Terminal RK45 cases also check attempt accounting.
Event-enabled refinement may add its own counted integrations, so its totals
are checked by the dedicated Event tests rather than applying an incorrect
single-path formula. Harmonic SYS checks radius; conversion checks IC and state
mapping; Table checks strict ascending x and trusted terminal values.

Dormand–Prince coefficients and embedded error weights were rechecked against
[SciPy's pinned v1.16.2 primary implementation](https://raw.githubusercontent.com/scipy/scipy/v1.16.2/scipy/integrate/_ivp/rk.py).
The stored mathematical coefficients agree, with the existing opposite sign of
the embedded error vector harmless under absolute norm. DIFF EQ intentionally
uses a maximum component norm, seven fresh RHS calls and its existing bounded
controller; it does not claim identical SciPy step selection or dense output.
[RK45_NUMERICS](RK45_NUMERICS.md) documents these established choices.

## Domain, cancellation and bounded-work coverage

New analytic checks compare ln(x), log10(x) and sqrt(x) primitives on x=.75/1.25,
and tan(x) against -ln(cos(x)) on x=±.75. Terminal tests use actual compiled model
RHS functions, without replacing the solver or inventing values after failure.

| RHS and test direction | RK4 terminal | RK45 terminal |
|---|---|---|
| ln(x), x0=1 toward -1 |Domain at last x≈.005|Domain at a tiny positive last x|
| log10(x), x0=1 toward -1 |Domain at last x≈.005|Domain at a tiny positive last x|
| sqrt(x), x0=1 toward -1 |Domain at last x≈.005|Domain at a tiny positive last x|
| 1/(x-.5), x0=1 toward -1; RK4 h=.125 |Domain at last x=.625|Step underflow near x=.500000000000315|
| exp(1000x), x0=0 toward 1 |Magnitude limit at last x≈.23|Magnitude limit at last x≈.230258509299404|

All retained terminal states remain finite and at most 1e100 in magnitude; none
of these paths reports the requested out-of-domain target as reached. The pole
RK4 grid is deliberately exactly representable so its stages encounter the
actual invalid point. The separate sampling limitation below is not hidden by
this regression. Adaptive approaches to zero may require thousands of bounded
RHS evaluations; no unsupported hardware performance claim is made.

| Existing group | Actual checks retained and rerun |
|---|---|
| numerical_core |RK4 fourth-order convergence; forward/backward growth/decay, oscillator/logistic, partial final step, max steps, nonfinite RHS, immediate cancel|
| expressions |operator precedence/right-associative powers, radians/log bases, all variable scopes, invalid domains, expression/depth limits and 10000 deterministic malformed inputs|
| equation_models |all seven modes; original examples, SYS 1..9 decay, higher polynomial and conversion|
| rk45_core |embedded local order, tighter-tolerance trend, oversized rejected trials, backward/off-grid/one-ULP endpoints, tolerance floors, underflow, all operation budget types, every trial stage/precommit cancel, nine-state scaling|
| solver_safety |RK4 preflight dimensions/IC/range/work, bounded exact solution becoming numerically unstable at coarse h, explicit smaller h recovery|
| test_initial |numeric scalar/list validation, complete state vectors, common x,10/11 boundary, duplicates, no mutation on invalid input|
| gsolve_numerics |polynomial/harmonic roots/extrema/X/Y-CAL/ICPT, sorted results, zero roots, X-domain restriction and Y-window invariance|
| test_valid_regions |real RK4 valid prefix/opposite direction, magnitudes, TRACE bounds, valid G-Solve after termination and distinct cancel|
| test_segment_stream |production consumers fed a trusted valid/gap/valid stream: no false gap lines/roots, sorted extrema, invalid lookup and restoration; this is explicitly a consumer fixture, not a solver restart claim|
| test_table_unified |strict ascending grid, zero/terminal merge, both endpoints, missing multi-IC cells, tiny/large steps and large-x distinguishability|
| rk45_integration |all modes/9 states, 10 IC, exact Table/G-Solve coordinates, state/work budgets, Phase cache projection, singular terminal|
| event_numerics |ANY/RISING/FALLING under forward/backward MARK/STOP, initial/plateau roots, repeated crossing/capacity, invalid Event gaps, 10 IC, cancellation/refinement, rejected trials never create events|
| phase_numerics |harmonic/linear/nonlinear equilibria and eigenvalues, scale extremes, root capacity, nonautonomous reference-x field/nullclines and disabled EQPT, discontinuity rejection, bounded cancellation|

## Findings and intentional limits

No new P0/P1/P2/P3 objective numerical defect was established in this pass; no
production numerical fix or solver rewrite was made. Two INFO findings apply:

**MATH-I1 — INFO, discrete sampling cannot certify an entire domain.**
A concrete RK4 observation is y'=1/(x-.5), x0=1,y0=0,h=.005,target=-1.
Floating-point trial coordinates can miss the exact pole. The preexisting solver
then finishes its configured steps (observed y≈3.7529996894765e12), although there
is no continuous IVP solution through x=.5. This is a known black-box fixed-step
sampling limitation, not evidence that this value is a valid continuation.
The exact-hit h=.125 case correctly stops; RK45 in the audited case shrinks and
stops near the pole. Neither method can guarantee discovering every unsampled
pole/discontinuity. Complete indicates finished numerical steps, not a proof of
an entire mathematical domain. Symbolic pole detection or a new sampling policy
would be a separate feature and was not introduced. Existing invalid segments
still remain disconnected, and no whole-domain shading was added.

**MATH-I2 — INFO, local tolerance and display sampling are different contracts.**
RK45 tolerance controls the embedded local estimate, not a global certified error
or digits between retained display points. TRACE/repaint caches are bounded and
may interpolate only connected valid samples; Table/G-Solve numerical queries
land on their requested coordinates. The maximum-norm, no-FSAL implementation,
nonstiff solver scope and state/magnitude/work limits remain intentional.
No new REVIEW REQUIRED numerical product decision was introduced by this audit.

## Hardware-only gaps

HARDWARE TEST REQUIRED: actual SH double-math results/timing, cancellation latency
near domains and long expressions, RTC behavior, cumulative stack high-water and
MENU/Fugue. Host/UBSan tests prove the listed host executions and invariants;
compilation/package success does not substitute for calculator execution.
