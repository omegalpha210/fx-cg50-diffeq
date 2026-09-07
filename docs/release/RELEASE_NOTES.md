# DIFFEQ v0.12.0-beta.1 — Events and Solver Diagnostics

Added one user-defined **Event Detection** condition with **ANY / RISING /
FALLING** direction and **MARK / STOP** actions for both RK4 and RK45.
Parameters **F2 SOLVE → F1 EVENT** edits settings; **F2 INFO** shows read-only
runtime solver statistics. The former Parameters INIT is **SOLVE → F4 INIT**.

Events use accepted numerical steps only, never rejected RK45 candidates.
Direction always means increasing x, including backward integration. Refinement
uses at most48 safeguarded secant/bisection iterations and selected-method
scratch target landing. Initial-zero, invalid-gap, cancellation and shared-work
limits are handled without publishing a canonical overshoot.

MARK continues integration and stores at most32 black/orange squares on Graph
and Phase, while total hits keep counting. STOP terminates each IC/direction
independently at the Event state. Table retains the root as its terminal valid
row, TRACE cannot extend beyond it, and G-Solve remains in the valid prefix.

Solver INFO reports method/settings/status, accepted/rejected attempts, actual
RHS calls, accepted h min/max and Event totals. Counts include refinement and
initial-direction probe work; Phase analysis is excluded. INFO does no solve or
file I/O. A new Graph run replaces the report even on cancellation; transactional
TRACE failures preserve the previous graph/report.

SAVE **v10** stores Event settings in current/recall; same-device **v3–v9** readers
load Event OFF, preserving v9 RK45 preferences. Runtime diagnostics are not saved.

**39/39 host/UBSan tests**, strict clean SH compile/link with **zero warnings**
and **13/13 package checks** pass before publication. BSS64896→71952 (+7056 B),
data704 unchanged, largest single application frame2572 B unchanged. EN/KO READMEs
and Parameters captures are updated; two actual-renderer Event/Info images added.
Exact public source, binary hash and build results accompany the Release in
VALIDATION.md and SHA256SUMS.txt.

Accepted-endpoint signs can miss multiple crossings within one step or tangential
roots. Event x accuracy remains limited by the underlying numerical solution;
RK45 tolerances control ODE error, not arbitrary Event oscillations. RK45 remains
an explicit non-stiff solver. See [EVENTS](../EVENTS.md) and [USER_GUIDE](../USER_GUIDE.md).

**HARDWARE TEST REQUIRED:** all new Event/Info device paths, target numeric accuracy,
LCD/keys, cancellation timing, stack high-water, MENU/Fugue and native SAVE/RCL/STAT.
Multiple Event definitions, FSAL, sweeps, Poincaré/bifurcation, new/stiff/implicit
solvers, CAS, Laplace and special functions remain deferred.
