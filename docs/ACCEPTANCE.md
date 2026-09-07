# TRACE X/Y follow, endpoint controls and ten-IC acceptance

Runtime sources, tests, tools and original assets match validated development
milestone `0fbe0de864204754659c3a3f2e5478efb4832149`. The implementation commit
is `e2c3e64`. This update continues public main `6405296` / v0.9.0-beta.2;
internal development branches/history and reference PDFs are not imported.

## Behavior

- TRACE valid points pan both X/Y at a 10% edge margin, landing 30% inside.
  One combined window update performs one cached redraw. Spans, scales, h and
  Step remain unchanged; invalid/magnitude-invalid values cannot move the view.
- Configured Solver Xrange, runtime calculated extent and displayed V-Window
  remain separate. TRACE preserves automatic and manual configured endpoints.
- No prefetch for x=5.9, x=6 or F6 RIGHT in runtime [-6,6]. Only an actual target
  outside the runtime extent requests extension to that target. FAST/FASTER can
  cross in the same input. Subsequent out-of-cache steps may each require work;
  one controlled transaction, repeat coalescing, EXIT priority and all work caps
  remain. Cancellation/preflight failure preserves cache, cursor, view and plot.
- TRACE F1 x=, F2 NORMAL, F3 FAST, F4 FASTER, F5 LEFT, F6 RIGHT. Speeds remain
  1×/2×/3×Xdot. Yellow/Bright Green/Cyan backgrounds stay fixed, with black text
  and a black selected border. EXIT restores the graph base bar.
- F5/F6 use configured Solver endpoints, retaining curve and speed even after
  extension. Exact cached/interpolated endpoints are preferred; invalid targets
  fall back to a real nearest valid point with an inline numerical-limit hint.
- Grid/Axis Label use arrows only; F1/F2 are blank/inert. Their contextual help
  is `LEFT/RIGHT: ON/OFF toggle`. No fixed density help. Style/Color keys remain.
- First-order maximum is ten ICs; dimensions remain nine and higher/SYS UI still
  accepts one full vector. Model, graph, TRACE/G-Solve, Table/CSV and colors cover
  all ten. Table has ten solution columns plus x; x remains frozen.
- The editor remains bounded to 191 characters. Count and length errors are
  separate. Excess insertion is rejected before writing, with visible feedback;
  DEL/AC clears the length condition before the IC draft is committed.
- Session v7 writes expanded current/recall. Frozen v3–v6 layouts stream safely
  into the new Document; old files are never rewritten during load.
- Main hint is `MENU: return to MAIN MENU`. Actual MENU behavior is unchanged.

## Verification

Baseline: 28/28 host/UBSan groups, full SH link and 13/13 package checks passed.
Final development: **29/29 host/UBSan groups passed (14.47s)**.
The public candidate independently passed **29/29 (21.78s)**. Existing RK4/parser/model,
segmented-validity, G-Solve/ICPT, table/STAT, storage/recovery, tiny-h/work-limit,
key lifecycle and navigation tests remain enabled.

Expanded tests cover joint pan/one redraw, hysteresis/spans, invalid Y, ten
families, exact configured endpoints without extra solves, requested extension,
cache/VRAM preservation on cancellation or preflight failure, speed/curve
retention, target-branch F6 vs EXIT priority, actual pixel colors/black text and
borders, settings keys, 1/9/10/11-item lists, long numeric lists, bounded length,
tenth-slot validity, Table/CSV columns, current/recall and frozen v6 migration.

The clean SH build compiled and linked all 21 target C units with **zero
warnings/errors**, using `-Wall -Wextra -Werror -Wframe-larger-than=3072`.
`verify_g3a.py`: **13/13 checks passed**; fxgxa dump/signature valid.
The public release is rebuilt from the exact tagged source with numeric metadata
00.09.0000. Its **VALIDATION.md** and **SHA256SUMS.txt** assets record the public
commit, final test/build/package results and artifact hash. Relinking changes the
container timestamp. The candidate and final tag are independently checked.

[Release validation assets](https://github.com/omegalpha210/fx-cg50-diffeq/releases/tag/v0.9.0-beta.3)

| Linker section | Baseline | Final | Change |
|---|---:|---:|---:|
| text | 164,960 | 165,872 | +912 |
| data | 704 | 704 | 0 |
| BSS | 62,720 | 62,992 | +272 |

No new large buffer. TRACE remains 258 total points; committed sample storage
is 21,192 bytes (+24 of branch metadata), overlay/staging scratch remains
24,096 bytes. App is 13,628 bytes (+176 from two expanded documents). The Table
page is bounded to eight rows and ten solution columns plus x. Largest static
function frame remains ui_graph 2,468 bytes; ui_table is 1,768 bytes (+84).

Actual target-font widths (available bottom-line width 368 px): toggle hint
189, ten-IC hint 247, Main MENU hint 194, overflow hint 266. Count error lines
169/49 px; length error lines 108/147 px. All fit.

[65 own-renderer views](ui-review/host-overview.png) were regenerated, including
[a nine-view TRACE/IC review](ui-review/trace-ic-overview.png). The latter was
visually inspected for follow, speed colors, settings, ten-column Table and
separate error dialogs. These are host renderings, not calculator screenshots.

**HARDWARE TEST REQUIRED.** The [32 priority cases](HARDWARE_RETEST.md) remain
pending, especially LCD/repeat timing, cancellation during expensive extension,
Fugue MENU/reentry, physical SAVE/RCL and on-device legacy session recovery.
