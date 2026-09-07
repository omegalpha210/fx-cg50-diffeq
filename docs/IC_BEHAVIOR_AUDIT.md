# Initial-condition behavior audit — scalar lists and complete state vectors

2026-09-08; baseline `309a9db`. Evidence: the complete 22-page local
`docs/reference/diff_eq_manual.pdf`, including every initial-value procedure and
example. Text and the relevant formula/screens were checked; derivative marks
in PDF extraction were checked against rendered pages. References remain local;
no manual screenshot is a repository asset. This replaces the earlier inferred
ADD/DROP design, whose record remains in Git history.

## Evidence and UI decision

PDF page 3 (printed 3-1-2) describes general Output columns numbered 1st..9th as
initial values. The SYS example's matrix on p.20 also has these column headings.
This suggests a shared family capability in the original mode, but it does **not**
specify how lists of multiple high-order/state initial values pair or broadcast.
Only the separable example explicitly supplies a list and demonstrates multiple
solutions. We do not claim that the original calculator forbids higher-order
families. Under the user's requested rule, the new higher-order/SYS **UI accepts
one complete vector**, because no explicit multiple-vector input syntax or
example was found in this full manual. No tuple or broadcasting syntax is invented.

| Mode | Required values for one solution | Manual multiple-IC evidence | New UI syntax | Multiple independent solutions? | ADD/DROP |
|---|---|---|---|---|---|
| Separable | x0, y0 | Explicit `{0,1}` family on p.5 / 3-2-2 | common x0; scalar or y0 list | Yes, 1..9 | Absent |
| Linear first | x0, y0 | Procedure p.6; p.7 example only y0=-2 | Same scalar/list input | Yes; consistent first-order adaptation | Absent |
| Bernoulli | x0, y0 | Procedure p.8; p.9 example only y0=1 | Same scalar/list input | Yes; consistent first-order adaptation | Absent |
| General first | x0, y0 | Procedure p.10; p.11 example only y0=1 | Same scalar/list input | Yes; consistent first-order adaptation | Absent |
| Linear second | x0, y0, y'0 | pp.12–13 give one complete triple | Three scalar fields | One vector in UI | Absent |
| Nonlinear second (N-th2) | x0, y0, y'0 | General N-th procedure on p.14 | Three scalar fields | One vector in UI | Absent |
| N-th1..9 | x0 plus n state values | pp.14–15 fourth-order vector; pp.16–17 third-order conversion vector | x0, y0, y'0, …, y(n−1)0 | One vector in UI, including n=1 | Absent |
| SYS1..9 | x0 plus m state values | pp.18–20 single vectors; p.21 analyzes the same solution | x0, y1_0, …, ym_0 | One vector in UI, including m=1 | Absent |

All example values were checked: p.5 `(0,{0,1})`; p.7 `(0,-2)`;
p.9 `(0,1)`; p.11 `(0,1)`; p.13 `(0,1,1)`; p.15 `(0,0,-2,0,3)`;
p.17 `(0,0,1,0)` and its SYS conversion; p.19 `(0,1,.1)`;
p.20 `(0,1,1/4)`. Page 22 discusses restarting numerical calculation from
intermediate values, not a simultaneous multiple-vector entry syntax.

## Input and numerical semantics

`x0=0, y0={a,b,c}` expands to three existing backend `InitialCondition` records:
`(0,a)`, `(0,b)`, `(0,c)`. Scalar `0` and singleton `{0}` both produce one record.
Common x0 edits update every scalar family. The higher-order initial state values
are all retained; for order n there are exactly n state values plus x0.
The numerical backend still supports up to nine full IC records for existing
solver/TRACE validation; the simplified UI does not expose high-order ADD/DROP.

The list parser reuses the production numeric-expression parser per item, with
no dynamic allocation: at most nine values and 191 input characters. It rejects
empty/malformed/nested lists, empty items, mismatched braces/parentheses, variables,
undefined values, NaN/Inf and magnitude above 1e100. Expressions such as
`{1/4,sqrt(2),pi}` are valid. Duplicates are deliberately allowed as independent
ICs and can overlap visually. A failed edit does not change the committed values.
Stored values are doubles; reopening formats their values, not their original
expression text. SHIFT+physical multiplication/division produce braces; physical
COMMA separates items. These keys were checked in the installed gint keyboard
header and against the reference's p.5 key sequence.

F4/F5 are blank/inert in SELECT and EDIT. F3 V-WIN and F6 NEXT retain the ordinary
validation and return behavior. A zero-IC legacy first-order record can still
load for field-only compatibility; NEXT asks for y0 before proceeding in the new
editor. New field-only use is y OFF with SF ON.

List count enters the unchanged per-direction and total-work preflight. Nine
values over -6..6 with h=.0005 require 216,000 RK4 steps and are rejected before
integration. No full trajectories are copied per list value. Graph and TRACE
reuse their streaming/bounded buffers.

## Presentation and migration

OUTPUT has one y row in first-order mode regardless of list count. Its ON/OFF
applies to all families. The selected y color seeds the existing six-color
sequence rather than making all solutions identical. Table labels y1..y9 mean
**solution index only in first-order multiple-IC mode**; higher-order labels are
state/derivative names and SYS y1..ym remain state variables.

Legacy v3/v4/v5 high-order/SYS multi-vector records keep the first complete vector
with a load notice. Scalar records retain the first IC and all subsequent ICs
whose x0 is exactly the same; other x0 records are not silently re-anchored.
This policy can reduce the represented solution family. Load does not rewrite
source files; explicit later SAVE writes the v6 two-slot format. See the complete
[migration audit](OUTPUT_LIST_AUDIT.md).

## Evidence and hardware boundary

`test_initial.c`: scalar/list expressions, transactional rejection, nine-item
bounds, unchanged preflight, two reference solutions and all dimensions 1..9.
`test_simplified_ui.py`: physical braces/comma, malformed edits/retry, complete
high-order fields, same-x TRACE selection, two-curve Table/CSV and SAVE/RCL.
`test_table_unified.c`: shared ascending x, solution columns, zero/midpoint pages,
terminal points and cells outside individual reachable prefixes.

**HARDWARE RETEST REQUIRED**: physical modifiers, long list cursor/scrolling,
LCD curve distinction, actual RK4 behavior, Table/STAT import and Fugue durability.
