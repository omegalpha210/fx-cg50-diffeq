# Bounded two-variable system analysis

The implementation in `src/ode/phase.c` analyzes compiled, two-variable SYS
equations. It reuses the existing expression evaluator and validity policy:
coordinates and values must be finite and have magnitude at most `1e100`.
This is numerical local analysis, with no symbolic solving or global stability
claim. Actual calculator timing and interaction remain **HARDWARE TEST REQUIRED**.

## Scope and reference time

`phase_autonomous()` examines compiled instructions through `expr_uses_x()`.
It does not search text: `exp(y1)` is autonomous, whereas `x`, `X`, and even
`0*x` contain an independent-variable instruction. The latter conservative
case is intentionally not simplified symbolically.

The vector field and nullclines can be evaluated at an explicit reference
independent value. Equilibrium search accepts autonomous systems only; a zero
of a non-autonomous vector field at one time is not reported as a fixed point.
The view rectangle contains dependent coordinates `(y1,y2)`.

## Normalized direction

The screen vector is proportional to
`(f1 * pixel_width / y1_span, -f2 * pixel_height / y2_span)`.
Mantissa/exponent arithmetic avoids intermediate overflow or underflow from
directly dividing by an extremely small span. The result has unit Euclidean
length; field magnitude does not determine arrow length. An exactly zero or
invalid vector has no direction. No absolute near-zero threshold is imposed,
since changing equation units must not erase otherwise valid directions.

## Streaming nullclines

The caller requests a square grid of 2 through 20 cells per dimension. Only two
rows of two-component samples are retained. Invalid corners discard the cell.
Marching-square edges are processed separately for `f1=0` and `f2=0`.

- A zero endpoint is accepted. An edge with two zero endpoints is handled by
  neighboring edges; an entirely zero cell is not drawn as an arbitrary curve.
- Opposite signs trigger at most 18 safeguarded secant/bisection evaluations.
  Every fourth iteration uses bisection; a secant fraction outside
  `[1e-6, 1-1e-6]` also uses bisection.
- An edge intersection must reach zero or a residual no greater than `1e-8`
  times the larger original endpoint magnitude. Sign changes across poles or
  finite jumps are insufficient by themselves.
- Intersections within `1e-12` of each other in both normalized view coordinates
  are merged within a cell. Two intersections form a candidate segment. Four
  intersections use the evaluated center sign to resolve the ambiguity; an
  exact zero center permits four segments through that center.
- Each candidate segment is evaluated at its quarter, middle, and three-quarter
  positions. Invalid samples, or component residuals exceeding half the largest
  corner magnitude, discard the segment. Successful segments go directly to the
  callback; there is no accumulated contour array.

These checks reject sampled invalid regions and common discontinuity artifacts;
finite sampling cannot prove continuity of an arbitrary user expression. Narrow
features, unresolved oscillations, contours touching a corner, or sharp curvature
can be omitted. Grid resolution limits geometric accuracy. Adjacent cells can
emit the same segment along a shared exact-zero edge.

## Equilibrium discovery and refinement

Search scans a deterministic 12-by-12 grid, using the closed view rectangle.
Exact simultaneous zero values at grid vertices become candidates. Otherwise,
valid cell corners and the center are inspected: for each component, its sampled
range must include zero or its smallest sampled magnitude must be at most 15%
of its largest. Cells satisfying both component tests seed refinement at their
centers. No random initial guesses are used.

Refinement performs at most 24 Newton iterations. The two equations and the two
coordinates are scaled before solving the 2-by-2 linear system. A normalized
Jacobian determinant smaller than `128 * DBL_EPSILON` cannot support a Newton
step. The initial step is limited to one-quarter of the view span in either
coordinate. At most ten successive halvings seek a valid in-window point with
strictly smaller scaled residual. Failure abandons that candidate.

An exact simultaneous zero is accepted even if derivatives are unavailable.
Otherwise, the acceptance residual is

```text
max_i |f_i| / (|J_i1| * y1_span + |J_i2| * y2_span) < 1e-9
```

A zero denominator with a nonzero numerator cannot converge. `PhaseRoot.residual`
stores this dimensionless measure, not an absolute residual. It accommodates
different equation units without interpreting a tiny constant equation as zero.

Roots within `1e-6` of each other in both normalized view coordinates are
deduplicated. At most 16 roots are retained. Discovery of a seventeenth distinct
root sets `truncated` and stops; successful results are sorted by `y1`, then `y2`.
This is a bounded candidate search, not an exhaustive root-isolation algorithm.
It can miss closely spaced roots, tangencies between samples, roots near invalid
boundaries, or roots whose Newton steps cannot safely refine. A continuum of
zeros can produce a truncated list of inconclusive candidates, not an isolated
fixed-point enumeration.

## Jacobian and local linear classification

Each derivative uses central differences with both `h` and `h/2`, where

```text
h = min(cbrt(DBL_EPSILON) * max(|coordinate|, 0.01 * span),
        0.001 * span)
```

The actual represented separation between floating-point samples is used as the
denominator. Samples that collapse onto the center, invalid neighboring function
values, or invalid derivative magnitudes make the Jacobian unavailable.
Comparing symmetric curvature at both step sizes rejects unresolved corners such
as `abs(y1)` at zero. The rejection requires the finer symmetric bend to exceed
80% of the coarser bend and also exceed rounding noise and `0.001` of the derivative
magnitude. This permits smooth quadratic/cubic degeneracies while refusing the
tested cusp. It is a heuristic check, not a differentiability proof.

The largest difference between fine and coarse derivatives, and a rounding-noise
estimate, are carried into classification. The matrix is divided by its largest
entry magnitude before computing trace, determinant, and discriminant, avoiding
overflow in their products. The normalized classification tolerance is
`max(256 * DBL_EPSILON, 16 * estimated_error / matrix_scale)`.

Negative determinant outside that tolerance gives a saddle. Positive determinant
with well-separated nonzero real eigenvalues gives a stable or unstable node;
a confidently negative discriminant and signed trace gives a stable or unstable
spiral. Approximately zero trace with a complex pair is a **Center / Neutral
candidate**. Approximately zero determinant/eigenvalue real parts, or insufficient
numerical separation, remain **Inconclusive**. Invalid derivatives give
**Unavailable**. Approximate eigenvalues are retained for an inconclusive matrix
when evaluable; for example, the linearization of `(y1^2,-y2)` has eigenvalues
`0,-1`, not `0,0`.

The real-eigenvalue calculation obtains one eigenvalue from a sign-aware quadratic
formula and the other from determinant divided by the first, reducing subtraction
cancellation. A zero matrix is inconclusive. These labels describe the local
linearization: they do not determine nonlinear or global stability when the
linearization is non-hyperbolic.

## Work bounds, cancellation, and memory

Each nullcline or equilibrium call permits at most 8,192 vector evaluations;
each evaluates at most two compiled expressions, so the expression-evaluation
ceiling is 16,384. Domain failures count toward the same budget. Budget exhaustion
returns `ODE_WORK_LIMIT`. The cancellation callback runs before each evaluation;
equilibrium search also checks it immediately before committing results.

Equilibrium results are assembled in an automatic temporary. Cancellation,
work-limit failure, and bad input leave the caller's result byte-for-byte
unchanged. Successful searches preserve a `has_invalid` flag for sampled failures.
Nullclines stream through a callback, so their caller owns drawing rollback and
must not commit partial display changes on cancellation. Callback refusal returns
`ODE_SAMPLE_STOP`. Domain-invalid cells alone are skipped without failing the
whole nullcline operation. A null callback performs the same validation and
evaluation sequence without drawing, allowing a renderer to preflight the work.

Measured with the installed SH GCC 14.1 `-Os` build:

| Item | Bytes |
| --- | ---: |
| `PhaseRoot` on SH | 92 |
| `PhaseResults` on SH | 1,484 |
| `PhaseResults` on host | 1,552 |
| Core permanent mutable data/BSS | 0 / 0 |
| Largest core individual frame (`phase_equilibria`) | 2,148 |
| Nullcline frame | 1,268 |
| Nested root-refinement frame | 548 |
| Standalone phase object text/constant data | 10,141 |

These are core/object measurements; whole-program memory and cumulative stack
depth include callers, expression evaluation, and platform functions. There is no
dynamic allocation, full trajectory copy, or framebuffer copy in this module.

`tests/test_phase.c` covers the oscillator, saddle, stable/unstable and repeated
nodes, stable/unstable spirals, smooth non-hyperbolic points, cusp/domain failures,
three and nine nonlinear roots, deduplication/truncation, off-grid and translated
coordinates, tiny coordinates and equation scales, invalid values, autonomous
detection, cancellation preservation, exact and curved nullclines, discontinuities,
normalized directions, and budget exhaustion. The current known-system equilibrium
suite needs at most 2,158 vector evaluations for one search. The oscillator
nullcline test needs at most 681 across grids 9 through 20. An oscillatory contour
stress case safely stops at 8,192 evaluations. These counts establish bounds and
regressions; they are not fx-CG50 timing measurements.

## Renderer density benchmark

`build-host/benchmark_phase` benchmarks the actual evaluator and direction,
nullcline dry pass/segment emission, and equilibrium functions (20–100 repeats).
A UBSan host run measured the following at the chosen density; these are CPU
algorithm times without raster/LCD work, not SH latency predictions.

| Case | Field 20×11 | NULL 20×20 dry / emit | EQPT |
| --- | ---: | ---: | ---: |
| Oscillator | 220 eval, 0.0215 ms | 681 eval, 0.075 / 0.074 ms | 394 eval, 0.026 ms |
| Nonlinear three-root example | 220 eval, 0.0306 ms | 1473 eval, 0.185 / 0.190 ms | 1025 eval, 0.115 ms |

15-column and 30-column fields used 120 and 480 evaluations respectively;
20 columns balances readable arrows and bounded work. The renderer repeats
field/contour work after a successful dry pass, so a full redraw uses up to
440 field vector evaluations plus twice the contour budget (32,768 expression
calls at the contour cap). EQPT has its separate 8,192-vector budget.
Actual long-expression input latency and final atomic paint time remain
**HARDWARE TEST REQUIRED**.

The full linked target build has different inlining from the standalone object:
`phase_equilibria` frame 2,144 B and `phase_nullclines` frame 1,192 B; use the
whole-program [memory report](MEMORY_AUDIT.md) for current build measurements.
