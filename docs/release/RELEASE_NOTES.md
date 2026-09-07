# DIFFEQ v0.11.0-beta.1 — Adaptive RK45

Added optional **Dormand–Prince RK45** alongside the unchanged default Classical
RK4. Parameters Method uses LEFT/RIGHT; RK45 exposes Initial h, RelTol, AbsTol
and Max steps. Defaults .1/1e-6/1e-9/20000; rejected attempts count toward limits.
Adaptive internal steps are separate from the TIME Xdot-based output grid.

All seven equation modes, 1–9 states, ten first-order ICs, Graph/TRACE/Table,
G-Solve and SYS2 Phase use the selected solver. Table/G-Solve/TIME x= evaluations
land at the requested x; ordinary TRACE and Phase x= remain display-only cached
interpolation. Step is hidden for RK45; scalar-only SF and existing RK4 semantics
remain. Stage cancellation, attempt/RHS/work budgets and Step underflow prevent
unbounded retries. No extra full-trajectory buffer or FSAL optimization is used.

New SAVE format **v9** stores Method/tolerances in current and recall records.
Frozen same-device v3–v8 readers load as RK4. Back up old sessions before saving;
older add-ins may reject new records. Native save/migration needs hardware retest.

36 host/UBSan groups, all previous regressions, strict SH compile/link with zero
warnings, 13 package checks and updated Parameters renderer images pass before
publication. The release asset VALIDATION.md records the exact tag build, memory
and binary SHA256. See [RK45_NUMERICS](../RK45_NUMERICS.md) for coefficients and
quantitative comparisons: RK45 can improve automatic accuracy control but is
not always cheaper than RK4 and is not a stiff solver.

Local error estimates do not certify global accuracy or a singularity location.
A numerical pole can shift slightly; the solver stops and does not restart a
far continuation. Fine features may be missed by finite grids and retained
TRACE samples. **HARDWARE TEST REQUIRED:** target-math numerical results,
long 9-state/10-IC execution, rejection cancellation, physical keys/LCD, stack
high-water, MENU/Fugue and SAVE/RCL/STAT. Events, sweeps, bifurcation/Poincaré,
implicit/BDF/Rosenbrock, CAS and FSAL remain intentionally deferred.
