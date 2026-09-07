# Solver safety audit

Rollback point: `671186f`; implementation milestone: `1ad95d8`.
No toolchain reinstall, adaptive integrator, automatic h adjustment, numerical
engine replacement, or weakened magnitude guard was introduced.

## Preflight and limits

`model_preflight` in `src/ode/model.c` runs before the Calculate transition publishes
Recall or enters Graph, before Graph render/AUTO integration, and before TRACE
cache preparation/extension. Pan/zoom candidates that exceed limits restore the
previous window and Solver settings. A rejected Calculate returns to Parameters
with h selected and preserves equations, ICs, h, range and V-Window.

`ode_validate` requires finite h > 0. The previous arbitrary h >= 1e-12 restriction
was removed. Runtime `x + signed_h == x` checks still reject an unrepresentable
step. Parameter input rejects zero/negative/nonfinite values and retains the
invalid draft for correction. A positive tiny h can be committed, but preflight
rejects an excessive requested calculation before RK4 or integer conversion.

For each configured IC and each executed direction, preflight calculates:

`ceil(abs(target - IC.x) / h)`

A positive branch is skipped if IC.x exceeds xmax; a negative branch is skipped
if IC.x precedes xmin, matching the existing trajectory provider. ICs outside the
range therefore still pay for the full necessary path from their actual anchor.
Nonfinite quotients or estimates above Max Steps are rejected before casting.

- **Max Steps = per IC / per direction**, unchanged semantics; default 20,000,
  configurable from 1 to 100,000. The raw integrator retains its loop guard and
  partial-result behavior for streaming consumers.
- Aggregate planned RK4 steps: **200,000**.
- Aggregate planned `steps × dimension`: **600,000** state-steps.
- All configured ICs are counted, even if outputs are currently hidden; Table/Output
  can select them later. Zero-IC scalar slope-field mode has no RK4 paths to count.
- Step is output spacing only; it never reduces the estimated integration work.
- h=1e-300 cannot overflow an integer cast or cause a giant preallocation.

Per-path rejection displays `Too many integration steps` / `Increase h or Max
Steps`. Aggregate rejection asks for larger h, a shorter range or fewer ICs;
increasing Max Steps alone does not bypass the aggregate bound. User-edited h is
never silently changed by preflight, Graph, TRACE, ZOOM, AUTO or ORIG.

## Budget rationale and measured behavior

solver-benchmark.txt (local development log; see ACCEPTANCE.md) records five-run host CPU
means using the actual compiled model/RK4 code with Clang and UBSan. On this host:

| Case, X=-6..6 unless noted | Steps | State-steps | Host CPU mean |
|---|---:|---:|---:|
| 1 state / 1 IC / h=.1 | 120 | 120 | 0.018 ms |
| 1 state / 1 IC / h=.001 | 12,000 | 12,000 | 1.788 ms |
| 9 states / 1 IC / h=.001 | 12,000 | 108,000 | 8.221 ms |
| 1 state / 9 ICs / h=.001 | 108,000 | 108,000 | 15.486 ms |
| 9 states / 9 ICs / h=.1 | 1,080 | 9,720 | 0.762 ms |
| 1 state / h=.0001 / X=-.01...01 | 200 | 200 | 0.028 ms |

9 states × 9 ICs × h=.001 would require 972,000 state-steps and is rejected.
The chosen limits admit the ordinary small-h, multiple-IC and nine-state cases
while bounding large combinations. The measured dimension/IC costs justify
counting both; state-steps are a conservative proxy, not an exact timing model.
Expression complexity and slope-field work also affect time. These host numbers
are **not fx-CG50 latency estimates**. Hardware responsiveness and any future budget
tuning require the physical checklist. No claim that these are optimal hardware
limits is made.

The allowed numerical paths remain cancellable at each RK4 step. Existing
G-Solve scan/refinement/result limits and CSV bounds remain in force; their repeated
point queries are separately bounded by their existing algorithms, not represented
as one complete cost prediction by this preflight estimate.

## Retention and RAM

| Consumer | Retention policy |
|---|---|
| Graph | Streams segments; no complete trajectory array |
| Table | 7 visible rows + 1 lookahead; recomputes requested page |
| TRACE | 258 points total across enabled ICs, decimated with segment links |
| G-Solve | Existing fixed maximum of 32 result points |
| STAT CSV | At most 998 data rows, streamed via fixed page buffers; overflow removes incomplete file and asks for larger Step |

No buffer is sized from `(xmax-xmin)/h`, and no small-h heap growth exists. TRACE
stages into its existing overlay union; there is no new large memory allocation.
Historical solver-safety milestone: SH BSS **62,736 → 63,232 (+496) bytes**; data **704 → 704**. Document v4 remained
2,920 bytes; App remains 13,960 bytes. See the map and memory log in ACCEPTANCE.

## Numerical / Table regression

Separable f(x)=sinh(x), g(y)=y²-1, IC (0,0), default range and h=.1 has bounded
analytic solution `-tanh(cosh(x)-1)`. The host's last accepted x is ±4.4000000000000004
with numerical y≈8.23876260708e51; the following RK4 stage exceeds the existing
1e100 guard. This is fixed-step numerical instability, **not mathematical
solution divergence or proof of a hardware resource limit**. Guard-valid values
are not an accuracy guarantee. With user-selected h=.001, both endpoints are
reached and match the bounded reference within 1e-8.

BTM previously calculated a valid final page offset but discarded it because the
terminal status was numerical. It now accepts that offset and displays
`END: Numerical limit` in place. TOP, BTM and central MID are repeatable. The
last accepted off-Step-grid point is also retained at a numerical end. Ordinary
endpoints show END. No failed stage, NaN/Inf or value above 1e100 is stored/plotted,
and no RK4 restart is guessed beyond the guard.

Table and CSV always put x first; only ON dependent columns scroll/export.
First-order lists expand into solution columns. Two or fewer dependent
columns omit the horizontal-scroll guide.

## Verification

`test_solver_safety.c` checks default h=.1/.001 allowance, .0001/default rejection,
explicit Max Steps increase, .0001/short-range allowance, invalid/tiny h,
out-of-range IC anchors, multiple ICs, dimension 9, both unified Table endpoints and
Step=3 terminal retention. `test_safety_ui.py` checks actual dialogs, no solves on
rejection, parameter preservation, EXE/FUNC/VAR and Table navigation. Existing
numerical/parser/model/G-Solve/storage/navigation tests remain enabled.

Physical tests: [HARDWARE_RETEST.md](HARDWARE_RETEST.md). Final results and artifact:
[ACCEPTANCE.md](ACCEPTANCE.md).

## Current UI/field adaptation (UI polish base 79e5d9d)

Work limits above are unchanged. SF has one editor in scalar first-order
Parameters and still uses `solver.sf` (0..100). First-order INIT resets it to 12;
2nd/N-th/SYS (including N-th1/SYS1) have no SF row, skip it in navigation and
preserve its hidden value on INIT, mode changes and SAVE/RCL. Graph Settings INIT
resets only Grid/Label and Arrow/Pale Blue appearance. Field evaluation streams one RHS
call per grid point, with no RK4 or stored grid and cancellation/MENU per column.
TRACE NORMAL/FAST/FASTER retains the same preflight, extension transaction,
258-point storage and 400/125 ms repeat policy. First-order y0 lists expand into
the existing IC records and contribute every IC to preflight. Higher-order/SYS
UI is one complete vector; the multi-vector backend remains for core regression.
Unified Table uses a small coordinate index plus 7+1 rows, not full trajectories.
Final memory/stack figures and artifacts are in [ACCEPTANCE](ACCEPTANCE.md).
