# Dormand–Prince RK45 — v0.11.0-beta.1

RK45 is an optional explicit adaptive solver for all seven equation types and
1–9 state components. Classical RK4 remains the factory default. Its step
arithmetic, fixed h, Step decimation, preflight and existing status values are
unchanged. The higher-order-to-system transformation and expression bytecode
evaluator are shared by both solvers.

## Method and coefficient provenance

Dormand–Prince 5(4) advances the fifth-order estimate and estimates local error
with an embedded fourth-order formula. Bibliographic origin: J. R. Dormand and
P. J. Prince, *A family of embedded Runge-Kutta formulae* (1980),
[DOI 10.1016/0771-050X(80)90013-3](https://doi.org/10.1016/0771-050X(80)90013-3).
The original article was not directly accessible during this audit. Every
coefficient below was checked against two independent primary implementations:

- [SciPy v1.16.2 RK45](https://raw.githubusercontent.com/scipy/scipy/v1.16.2/scipy/integrate/_ivp/rk.py), C/A/B/E definitions.
- [Boost.Odeint boost-1.89.0 runge_kutta_dopri5](https://raw.githubusercontent.com/boostorg/odeint/boost-1.89.0/include/boost/numeric/odeint/stepper/runge_kutta_dopri5.hpp), a/b/c and dc definitions.

This is an independent C implementation of mathematical coefficients; no SciPy
or Boost source code or runtime dependency was imported. SciPy's error vector
has the opposite sign, which gives the same absolute error norm.

| Stage c | Nonzero a coefficients (preceding stages, including zero positions) |
|---|---|
| 0 | — |
| 1/5 | 1/5 |
| 3/10 | 3/40, 9/40 |
| 4/5 | 44/45, -56/15, 32/9 |
| 8/9 | 19372/6561, -25360/2187, 64448/6561, -212/729 |
| 1 | 9017/3168, -355/33, 46732/5247, 49/176, -5103/18656 |
| 1 | 35/384, 0, 500/1113, 125/192, -2187/6784, 11/84 |

b5 is the final row followed by zero. b4 is
`[5179/57600, 0, 7571/16695, 393/640, -92097/339200, 187/2100, 1/40]`.
The implementation directly accumulates `h*sum((b5-b4)*k)` to avoid subtracting
two nearly equal state values. The resulting weights are
`[71/57600, 0, -71/16695, 71/1920, -17253/339200, 22/525, -1/40]`.

For each component, scale is
`AbsTol + RelTol * max(abs(old), abs(candidate))`.
The maximum of `abs(error)/scale` across all components is the norm.
Only norm ≤1 commits the candidate. Rejected candidates never advance x or
enter the trajectory, Table or TRACE. Tolerances describe estimated local error,
not a rigorous global error bound or a guarantee of digits at a singularity.

## Controller, safety and diagnostics

Defaults: Method RK4; RK45 RelTol **1e-6**, AbsTol **1e-9**, Initial h **0.1**,
Max steps **20000**. Shared h retains its value when switching methods: fixed
step for RK4, initial proposal for RK45. Tolerances and hidden RK4 Step persist.
Parameters INIT retains Method, resets shared h/Max steps and the selected
method's settings; RK45 INIT preserves hidden Step, RK4 INIT preserves tolerances.
SF is still scalar-first-order-only and is independent of solver selection.

The proportional controller uses `0.9*err^(-1/5)`, clamped to [0.2,5]. Zero error
uses growth 5; an accepted retry cannot immediately grow. Invalid late stages
shrink by 0.2; an invalid RHS at the starting state terminates immediately.
The maximum step is span/8, raised to ten initial-coordinate ULPs for tiny spans,
and capped by the entire span. Output/end targets further cap each attempt.
The minimum uses ten ULPs at the current x toward the target (`nextafter`).
A representable final remainder may be smaller; `x+h==x` always stops.
An unsuccessful retry below that minimum returns Step underflow or the actual
domain/magnitude failure. The final state lands at the requested coordinate;
there is no overshoot followed by linear correction. Backward control uses
positive step magnitudes and a negative integration direction.

RelTol and AbsTol must be finite and positive. RelTol below `100*DBL_EPSILON`
(about 2.22e-14) and AbsTol below `DBL_MIN` (about 2.23e-308) are rejected as
Tolerance too small. Thus RelTol=1e-300 is rejected. AbsTol=1e-300 can describe
very small units when paired with valid RelTol; it is not a subnormal floor.
Error/progress and runtime limits still prevent unbounded work. NaN/Inf and
magnitudes above 1e100 are rejected at each stage and accepted-state boundary.

Max steps counts **attempts, including rejected/partially cancelled trials**, per
integration path (IC/direction or a numerical query); range 1–100000. Adaptive
counts cannot be inferred from span/initial h. Preflight checks input, finite
range/output spacing and initial-coordinate progress. Runtime caps one operation
at **200000 attempts, 800000 vector RHS evaluations, 240000 RHS×dimension work**.
Graph, TRACE preparation/extension, Table index/page, AUTO and G-Solve share
their operation budget across ICs, directions and refinements. CSV/STAT shares
one budget across its index and all pages; ordinary Table navigation starts a
new page budget. Repeated user actions begin new bounded operations.
An RHS count is one whole state-vector evaluation, not one expression or CPU
instruction; separate bounded field/Jacobian/G-Solve derivative probes are not
included in the integration diagnostic. Complex expressions cost more time.

Cancellation is checked at attempts, every stage and before state commit.
Complete, Cancelled, Step limit reached, Total calculation too large, Step
underflow, Tolerance too small, Math domain / singularity, NaN or infinity and
Magnitude >1e100 remain distinct. A non-cancel terminal failure may emit its
last accepted point; it never creates a new seed beyond the invalid prefix.

**FSAL is deliberately omitted.** Each complete attempt costs seven RHS calls.
Fresh trials/directions/ICs have no cached derivative to invalidate. Tests check
the seven-call accounting and cancelled/rejected state isolation. Optimization
can be considered separately after real-device correctness and timing tests.

## Output and consumers

RK4 Step is a fixed-step output decimation factor, not an accuracy parameter.
Table uses h×Step; higher-order/system graph redraw decimates while scalar
curves retain each RK4 step. RK45 hides and ignores Step throughout output.

RK45 streaming lands on an IC-anchored uniform grid with spacing
`max(TIME Xdot, requested integration span/1024)`, plus actual endpoints and
trusted terminal points. This bounds output density independently of accepted
adaptive steps. Internal steps advance between targets but are never all stored.
The same time-domain stream drives Graph and Phase; Phase field/nullcline/EQPT
evaluation is unchanged and solver-independent. SYS2 reprojects the same cache.

Table retains ascending x, zero/terminal extras, TOP/BTM/MID, frozen x and output
columns. RK45 recomputes each numerical row from the original IC to its exact x;
it never supplies Table/STAT values by linear interpolation. Each page stores
only 7+1 rows. A target at a singular terminal can fail on recomputation because
adaptive partitions differ; its cell remains empty. No invalid value is invented.
CSV still caps 998 data rows. For RK45 reduce Xrange or increase TIME Xdot if
the grid exceeds this; hidden Step has no effect.

G-Solve uses target landing in refinement and Y-CAL/Y-ICPT queries. Scan density
and bracketing still limit discovery: roots/extrema/intersections can be missed.
TIME TRACE explicit F1 x= also lands at the requested valid coordinate. Arrow
movement remains 1/2/3×Xdot and uses the existing **258-point total display cache**.
Its linear interpolation (including Phase x=) is display-only, **not a claim of
RK45 tolerance accuracy between retained points**. It never crosses known gaps.
TIME extension remains one cancellable transaction, preserves configured solver
settings, and discards temporary results on failure. Phase does not extend time.
There is no dense-output polynomial and no enlarged full-trajectory storage.

SAVE format **v9** appends Method/tolerances. Frozen v3–v8 readers default to RK4
and the tolerance defaults while preserving their existing migration behavior.
Current and recall records are covered; unknown/invalid settings are rejected.

## Quantitative checks

Reproduce with `tools/test.sh`, `build-host/test_rk45` and
`build-host/benchmark_rk45`. Host Clang/UBSan uses double precision; the SH binary
uses the installed double math library. No Python numerical dependency is needed:
the comparisons below use analytic exp/cos/sin/tanh solutions.

Core-only benchmark, initial/fixed h=.1, RK45 RelTol=1e-6 / AbsTol=1e-9;
output landing/raster excluded. A/R are accepted/rejected attempts. Error is
relative for exp, maximum state absolute for oscillators, absolute for sinh.

| Equation / requested x | Method | Final x | Error | A / R | RHS | Status |
|---|---|---|---|---|---|---|
| y'=y / 5 | RK4 | 5 | 3.834e-6 | 50 / 0 | 200 | Complete |
| y'=y / 5 | RK45 | 5 | 8.711e-7 | 21 / 0 | 147 | Complete |
| y''=-y / 40 | RK4 | 40 | 2.660e-5 | 400 / 0 | 1600 | Complete |
| y''=-y / 40 | RK45 | 40 | 3.573e-6 | 209 / 58 | 1869 | Complete |
| sinh(x)(y²-1) / 7 | RK4 | 4.4 | 8.239e51 | 44 / 0 | 177 | Magnitude limit |
| sinh(x)(y²-1) / 7 | RK45 | 7 | 1.780e-7 | 350 / 79 | 3003 | Complete |
| y'=y² / 2 | RK4 | 1.1 | undefined at/beyond pole | 11 / 0 | 48 | Magnitude limit |
| y'=y² / 2 | RK45 | 1.0000002541 | undefined at/beyond pole | 212 / 212 | 2968 | Step underflow |
| SYS y1'=y2, y2'=-y1 / 40 | RK4 | 40 | 2.660e-5 | 400 / 0 | 1600 | Complete |
| same SYS / 40 | RK45 | 40 | 3.573e-6 | 209 / 58 | 1869 | Complete |

The oscillator amplitude errors are RK4 2.774e-6 / RK45 3.583e-6; phase errors
3.321e-5 / 1.355e-6 radians. At these defaults RK45 uses more RHS evaluations
and its amplitude error is slightly worse. A host timing sample for exp,
oscillator, sinh, pole and SYS respectively was RK4
0.181/0.130/0.014/0.003/0.131 ms, RK45 0.014/0.170/0.211/0.191/0.168 ms.
These very short instrumented timings are noisy runtime proxies, not calculator
speed measurements or evidence of a universal speedup. RHS counts are repeatable.

RK45 accepted h spans: exp [.09745,.25338], oscillator [.1,.24034], bounded sinh
[.001643,.20892], singular [2.66e-15,.12555]. The bounded sinh equation has exact
solution `-tanh(cosh(x)-1)` from y(0)=0. Completing x=7 with a small error is an
observed test result, not an assertion that all larger x are inexpensive/stable.

The singular y'=y², y(0)=1 example terminates near the true pole x=1 with finite
last state and underflow. Global numerical error shifts the estimated blow-up:
at tighter 1e-8/1e-11 it stops at 1.000000000821. It never resumes a trajectory
after failure or reaches the requested x=2. This solver has no symbolic/event
pole detector and **cannot certify staying on the exact mathematical side of a
singularity**. A tiny numerical overshoot is not valid continuation of 1/(1-x).

For exp at x=5 (initial h=1), RelTol/AbsTol pairs 1e-3/1e-6, 1e-5/1e-8 and
1e-7/1e-10 give errors 3.659e-5, 8.132e-6, 9.552e-8; A/R counts 8/0, 12/1,
33/2 and RHS counts 56,91,245. Tests also check fifth-order local convergence,
embedded error scaling, cubic-in-x RHS quadrature, oversized rejected trials,
both directions, off-grid and one-ULP endpoints, every-stage cancellation,
domain recovery/termination, all budget types, all equation modes, 9 states,
10 ICs, all G-Solve modes, Table/STAT, Phase cache reuse, TRACE extension/cancel,
save migrations, dynamic Parameters rows and 100 method-toggle cycles.

## Memory and limitations

SH GCC 14.1 strict full build: text **194928 B**, data **704 B**, BSS **64896 B**.
Baseline Phase milestone: text187848/data704/BSS64816; BSS delta **+80 B**,
data delta **0 B**. Fixed RK45 workspace **652 B** = seven 9-double stage vectors
(504 B), candidate/error vectors (144 B), stage index (4 B). It is caller-stack
storage, with no global mutable stage buffer. Mathematical coefficient tables
are 448 bytes of read-only storage. Adaptive prefs are 20 B per target Document;
work diagnostics 32 B per CompiledModel; linker alignment accounts for remaining
BSS growth. No new framebuffer, heap allocation or trajectory array.

Target `.su` frames: RK45 integration **988 B**, single step **100 B**, Parameters
**872 B**. Largest overall remains ui_graph **2572 B**; equilibrium2144/Table1768.
Individual frames exclude callers/callees; G-Solve callbacks can nest numerical
queries. Parser depth remains bounded and numerical loops are nonrecursive.
Stack high-water, long-expression timings, physical cancellation/repeat, LCD,
MENU/Fugue and native v9/legacy SAVE/RCL are **HARDWARE TEST REQUIRED**.

RK45 is an explicit adaptive Runge–Kutta method, **not a stiff ODE solver**.
Stiffness, discontinuities, severe scaling or tight tolerances can force very
small steps and work/underflow termination. Discrete error estimates cannot
guarantee detection of every discontinuity, pole or narrow feature. Compare
tolerances/ranges and analytical knowledge. Events, parameter sweeps,
bifurcation/Poincaré tools, implicit BDF/Rosenbrock, CAS and FSAL optimization
are intentionally deferred from this milestone.
