# DIFFEQ v0.12.0-beta.8 — Drawing bar, per-IC visibility and numeric EXIT

- Drawing retains Graph; only the bottom softkey rectangle becomes one blue busy
  bar with white spinner/EXIT text and no separators. Table keeps its dedicated
  preparation page. Shared delayed/cancel-first feedback and rollback remain.
- Each first-order IC has independent ON/OFF and color. Graph, TRACE, G-Solve,
  Table and CSV/STAT share visible-family mapping. Hidden numerical/Event data
  remain intact. All-OFF is safe; Table/exports keep x only.
- SAVE v11 persists ten IC bits including inactive preferences. Old v3–v10
  sessions initialize every IC ON and retain existing colors/migration rules.
  Output INIT restores all ON/default palette; shrink/grow preserves preferences.
- X/Y-CAL numeric EXIT discards every draft without validation and returns to
  originating G-Solve page2. EXE alone validates/commits; held EXIT stays one level.
  Generic editors and numerical algorithms remain unchanged.
- FULL_AUDIT R1/UIR1 are resolved by owner decision, implemented and validated.

59/59 host/UBSan groups (all previous57 retained), clean 28-unit strict SH build,
zero warnings and 13/13 package checks repeated on exact candidate/tag source.
BSS72192B, data768B, maximum single application frame2664B. No new heap/framebuffer.
Exact source, text/G3A totals and binary SHA256 are recorded in release VALIDATION.md.

[Complete audit](../VISIBILITY_PROMPT_AUDIT.md),
[12 production renderer frames](../ui-review/visibility-overview.png),
[user guide](../USER_GUIDE.md), [memory](../MEMORY_AUDIT.md).
**HARDWARE TEST REQUIRED:** all34 [priority cases](../HARDWARE_RETEST.md) remain pending.
MIT/notices, public ancestry and prior tags are preserved; no manuals, private
paths/logs, toolchains or development history are published.
