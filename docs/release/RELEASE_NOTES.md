# DIFFEQ v0.10.0-beta.1

SYS 2D now offers **VIEW → TIME / PHASE / TABLE**. PHASE plots y2 versus y1
from the existing RK4 trajectory cache, with an independent V-Window and TRACE.
PHASE **ANLYS → FIELD / NULL / EQPT / INFO** adds normalized vector arrows,
red/blue numerical nullclines, bounded fixed-point search and numerical-Jacobian
local linear classification. TIME G-Solve and time-domain tables remain available.

Non-autonomous trajectories are supported. Their field and nullclines are frozen
at IC x0 and labeled; EQPT is autonomous only. Center/Neutral and Inconclusive
labels do not establish nonlinear or global stability. A finite grid may miss
roots or narrow structures; the 258-point retained trajectory may lose detail
on long intervals. No RK45, symbolic solver, bifurcation or 3D feature is included.

New sessions use **v8** and retain independent phase windows and toggles.
Same-device v3–v7 sessions remain readable; old active SYS2 phase windows migrate
without changing configured solver endpoints. Back up older sessions before
explicit SAVE; older add-ins may reject v8. Analysis results are not serialized.

The 33 host/UBSan groups, strict SH compile/link, bounded-density benchmark,
package checks and three new project-renderer README screenshots are part of
this release's verification. The release asset `VALIDATION.md` identifies the
exact public source, binary size/hash and final measurements.

**HARDWARE TEST REQUIRED:** the new phase workflow needs physical fx-CG50 LCD,
held-key/cancel timing, long-expression performance, stack high-water, MENU/Fugue
and native session migration retests. Prior first/second-order hardware results
do not imply these new paths have been tested on hardware.
