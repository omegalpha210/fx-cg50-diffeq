# Historical RK4 methods and validation baseline

This report records the original RK4-only milestone. Current RK4/RK45 arithmetic,
Event and SYS2 extensions are documented in [RK45](../audits/RK45_NUMERICS.md),
[Events](../EVENTS.md) and [Phase](../audits/PHASE_NUMERICS.md); current regression
counts and measurements are in [ACCEPTANCE](../ACCEPTANCE.md). Old claims about
missing adaptive integration or automatic saves below are historical evidence.

## Baseline method

[ORIGINAL] The local manual PDF p.2 specifies classical fourth-order Runge-Kutta and distinguishes integration h from graph/List Step. No adaptive method or error estimator is claimed in this release.

For a normalized system s'=F(x,s):

```
k1 = F(x, s)
k2 = F(x+h/2, s+h*k1/2)
k3 = F(x+h/2, s+h*k2/2)
k4 = F(x+h, s+h*k3)
s_next = s + h*(k1+2*k2+2*k3+k4)/6
```

A single call advances 1-9 components together. Negative h performs backward integration. Each stage and resulting state is checked for nonfinite values and magnitudes above 1e100. The caller's state changes only after a successful step. The final step is shortened to meet the requested endpoint. No `-ffast-math` is used.

[INFERRED] These implementation details, double precision, the magnitude guard, h minimum 1e-12 and finite step budget are implementation policies, not documented bit-for-bit behavior of the Algebra FX. SF defaults to 12, Step to 1, h to .1 as shown in manual screenshots. All trig functions in this app use radians.

## Executed host checks

2026-09-06, macOS arm64, Apple Clang 17, `-Wall -Wextra -Wpedantic -Werror`, UndefinedBehaviorSanitizer, assertions enabled. `./tools/test.sh` configures, builds and executes the independent host tests. Native SH compilation/linking is performed separately with `./tools/build.sh`.

Initial solver results:

| Problem | h | Computed / error |
|---|---|---|
| y'=y, y(0)=1, x=1 | .1 | 2.71827974413517; abs error 2.08432388e-6 |
| same | .05 | 2.71828169265633; abs error 1.35802711e-7 |
| same | .025 | 2.71828181979285; abs error 8.66619176e-9 |
| oscillator, (1,0), x=2*pi | .01 | (0.999999999996, 5.23228362113e-10) |
| logistic y'=y(1-y), y(0)=.2, x=3 | .01 | 0.833925230195 |

Halving h reduced exp global error by about 15.35 and 15.67, approaching the expected factor 16. Tests also compare y'=-y and backward y'=y against exp(-1), oscillator against cosine/sine, and logistic against `1/(1+4*exp(-x))`. Endpoint clipping (.3 steps to x=1), sample counts, step-budget termination, NaN rejection, invalid range/tiny h, and cancellation passed.

Raw concise results: `docs/archive/build-logs/numerical-results.txt`. Additional parser/model/graph acceptance results are added as modules become available.

## Test environment caveat

Apple's supplied AddressSanitizer runtime deadlocked **before main** on this macOS build. A native `sample` trace showed `__asan::InitializeShadowMemory` re-entering the malloc initializer and waiting on `StaticSpinMutex`. Only the two test processes created by this task were terminated. Host tests were rebuilt with UndefinedBehaviorSanitizer, which executes successfully. AddressSanitizer coverage is not claimed. This is a host runtime issue, not evidence of a solver loop hanging.

## Limits and user guidance

Fixed-step RK4 is not a stiff solver and cannot detect every singularity or guarantee accuracy from a finite result. Decrease h and compare; shorten the range near discontinuities. Some discontinuities can lie between sampled stages without being detected. Avoid using graph appearance as an error estimate. Cancellation/overflow/step-limit results are partial. The original manual p.22 gives analogous advice about reducing h/range and continuing from intermediate values.

User testing on baseline `aa55902` confirmed working 1st/2nd calculations and solution graphs. This is useful functional evidence, but it does not measure target math-library error or performance. The subsequent UI/formatter/lifecycle build remains **HARDWARE RETEST REQUIRED**.

## Expression compiler milestone

Passed precedence (`-2^2=-4`, `2^3^2=512`, `2^-3=.125`), scientific literals, trig/inverse trig/hyperbolic/inverse hyperbolic functions, exp/ln/log/sqrt/abs, pi/e, private constants, mode-dependent yN scope, invalid token/parenthesis/multiplication forms, zero division/domain/nonfinite results, bounded nesting, and 10,000 deterministic malformed-input strings under UBSan. Explicit checks cover asin/acos outside [−1,1], acosh below 1, and atanh at/outside ±1. A compiled program occupies 904 host bytes. `ln` is natural logarithm; `log` is base 10. Multiplication must be explicit. The failed evaluation does not modify its output argument.

## Equation model milestone

All seven input modes compiled and integrated. Tests cover the manual's separable/linear/Bernoulli families, harmonic second-order form, fourth-order polynomial, the p.17 third-order to SYS conversion (matching all states to 1e-14), and independently decaying SYS components for every dimension 1 through 9. A ninth-order problem with y^(9)=0 and y^(8)(0)=40320 gives y(1)=0.999999453880 at h=.01 (exact: 1); h=.1 did not meet the original tight test threshold, illustrating the need to choose h for the problem. Invalid coefficient-variable scope and field-only/zero-SF states are rejected. Current host sizes: Document 2,856 bytes, compiled model 8,152 bytes.

## UI integration milestone

The same app/UI C sources now execute in a host adapter using gint's actual font atlas and key codes. Five scripted workflows pass: editing `sin(x)-y` and IC y(0)=2, 9-state SYS navigation, syntax-error recovery, N-th→SYS, and recall. The calculation gives y(5)=-0.604448018 at h=.1. UI review exposed an extra near-zero terminal step due to accumulated x rounding; the boundary comparison now uses a scale-aware floating-point tolerance while still rejecting h that cannot advance x. A regression requires exactly 50 steps from 0 to 5 with h=.1. Host framebuffer images are visual/layout evidence only, not hardware execution.

## Graph milestone

Five test groups pass after connecting the streaming renderer. Geometry tests cover off-screen rejection, clipping horizontal/vertical endpoints at magnitude 1e100 before integer conversion, center mapping, invalid zero zoom and panning. The retained host adapter renders the real UI/graph C sources with the upstream gint font. Visual inspection confirmed the manual's y'=y^2-1 family (IC 0 and 1), slope field, axes/ticks, and harmonic-oscillator direct phase plot. First rendering uses stride 1; redraw uses the independently configured Step. Slope field is first-order-only. Render and auto-window loops support EXIT cancellation. Auto-window refuses to fit an incomplete trajectory. Physical timing/display remain untested.

## Table and storage milestone

At the table/storage milestone, all six CTest groups passed. Table checks cover seven-row paging plus one lookahead, independent Step sampling, backward traversal and partial step-limit results. Storage checks cover two successive saves, current/recall restoration, a truncated newest slot falling back to the older slot, repair on the next save, CSV headers and values, exclusive filenames, and removal of a canceled new export. Nine scripted UI workflows covered table direction/page changes, CSV, field-only rendering, phase plots and explicit session save. That milestone produced the earlier 136,208-byte G3A; current artifact data is maintained in [ACCEPTANCE](../ACCEPTANCE.md). Calculator filesystem semantics and interrupted physical writes remain **HARDWARE RETEST REQUIRED**.


## Final 0.1.0 validation

Nine CTest groups pass with warnings as errors and UBSan. The separate G-Solve test uses y=x²−1 and y=1−x² solution curves to verify two roots, extrema, Y-CAL and two X-CAL results after numerical refinement; it also checks curve enumeration and a constant-zero solution without duplicate results. UI coverage exercises physical SHIFT/ALPHA event flags, Xdot coupling, graph settings, official two-page G-Solve actions, multiple-curve selection, curve-blink frame changes and LEFT/RIGHT results. The two-process test still saves `y'=-y`, restarts the application, recalls the saved calculation and traces y(1)≈0.3678798. Autosave ON/OFF isolation and a 1,000-cycle fixed-state navigation test cover the crash-fix boundaries. Final native link/package evidence and memory figures are in [ACCEPTANCE](../ACCEPTANCE.md). Nine current host framebuffer views were visually inspected and retained in `docs/captures/`.

## Hardware-feedback regression milestone

G-Solve now searches only the intersection of Solver X and the latest V-Window X. Numerical candidates and refined points are never rejected by V-Window Y. Tests cover off-screen ROOT/MAX/MIN, direct off-screen Y-ICPT/Y-CAL, X-CAL with an off-screen target, X exclusion, Y-window invariance, and harmonic ICPT intersections in sorted order. ICPT solves `curveA(x)-curveB(x)=0` and uses the same bounded refinement path.

Table/storage tests now cover BTM counting, STAT-compatible apostrophe headers, numeric mapping, exclusive files, cancellation cleanup and the 998-data-row safety bound. Export formats fixed pages after RK4 has returned rather than invoking floating stdio and filesystem code inside a sample callback. The host UI suite covers physical `ALPHA+SUB` versus `NEG`, independent vertical IC blocks, parent EXIT hierarchy, global V-Window retention, menu-state panning, multi-curve same-x TRACE, inline ICPT, off-screen ROOT, TOP/BTM/DIR, STAT export and automatic cross-process persistence.
