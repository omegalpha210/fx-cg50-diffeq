# DIFFEQ v0.12.0-beta.7 — Busy screens, IC colors and systematic audit

Long Table/Drawing work uses the requested dedicated preparation canvas, spinner,
EXIT cancellation and hidden softkeys. Fast/cached work avoids flashing; cancel
preserves accepted state. SF is 0–50 (default 12), with safe legacy clamping.
First-order IC1–IC10 colors use the existing persisted matrix and palette; shared
component visibility remains explicit. No save-version bump or new heap buffer.

The completed [full audit](../FULL_AUDIT.md) records the exact capability matrix,
objective corrections, severity counts, two review decisions and known limits.
It includes G-Solve Event STOP endpoint handling, RK45 Table dx, cancelled-drawing
Recall ownership and maximum-row geometry. See its final issue registry for the
additional final-review transaction regression and all evidence.

57 host/UBSan groups, clean 28-unit strict SH compile/link, zero warnings and
13 package checks are repeated on the exact public source/tag. All earlier 49
groups remain. BSS stays 72192 B; maximum single application frame stays 2664 B.
Exact binary hash, memory totals and validation are included in Release assets.

MIT, dependency notices, public ancestry and existing tags are preserved.
No reference PDF or private development history is published.
[16 renderer frames](../ui-review/audit-overview.png), [user guide](../USER_GUIDE.md),
[memory](../MEMORY_AUDIT.md), [hardware checklist](../HARDWARE_RETEST.md).
**HARDWARE TEST REQUIRED:** host tests and frames do not establish device PASS.
