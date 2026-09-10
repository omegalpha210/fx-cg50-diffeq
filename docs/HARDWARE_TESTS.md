# fx-CG50 acceptance checklist

User testing confirmed 1st/2nd calculation and solution graphs on baseline `aa55902`. The modified UI reference build still requires the focused [HARDWARE_RETEST](HARDWARE_RETEST.md); unchecked stress and filesystem cases below remain **HARDWARE TEST REQUIRED**.

Record calculator model, OS version, G3A SHA256, date and observations when testing. Use [USER_GUIDE](USER_GUIDE.md) for key sequences; [ACCEPTANCE](ACCEPTANCE.md) identifies the delivered artifact. Compiler success alone does not satisfy these checks.

| Area | Hardware test | Expected observation |
|---|---|---|
| Installation | Copy G3A to storage root and disconnect normally | DIFF EQ icon/name in main menu; both selected/unselected icons legible |
| Lifecycle | Launch, navigate, EXIT at home; relaunch repeatedly | Stable display, OS return, no accumulated driver or memory failures |
| Input | Numbers, ALPHA+SUB, NEG, SHIFT+ln, SHIFT+x², functions, y9 menu | Correct distinct tokens, cursor/backspace/cancel, no stuck modifier or repeat |
| Long input | Edit near 191 characters; move between all five rows | No text/framebuffer overrun; capacity dialog; cancel preserves old value |
| Types | All four first-order modes, 2nd, N-th 1/9, SYS 1/9 | Correct equations and valid scrolling through state 9 |
| Known solution | y'=y, (0,1), x=1, h=.1/.05/.025 | Values approach exp(1), roughly fourth-order convergence |
| Oscillator | y1'=y2, y2'=−y1, (1,0), h=.01 | cos(x), −sin(x); bounded phase curve with all axes selectable |
| Conversion | N-th 3: sin(x)−y1−y2 → SYS | State equations and all ICs preserved; matching numerical values |
| Graph | 1st separable defaults, multiple colors, all 9 ICs | Complete family; selected components and window clipping correct |
| SF | Set SF=0/1/12/50; reject 51; load an old SF=100 session; first-order zero IC sets | Disabled/normal/dense fields; old SF clamps to 50; zero IC + SF>0 valid; unsupported modes reject missing IC |
| Navigation | Trace, pan, zoom, auto window, grid, V-Window | Accurate readout; responsive redraw; invalid window rejected |
| Output/Table | Toggle masks, scroll 10 columns, page ±x, switch IC | Selected values/rows agree with trace; Step changes sampling, not h |
| Cancellation | Small h/long range, EXIT/AC during graph, auto, table, STAT export | Prompt stop; UI remains usable; partial result correctly labeled |
| Errors | `1/(x-1)` near singularity, `sqrt(-1)`, huge magnitudes, low max steps | Numerical stop; last accepted point retained; no OS crash |
| Storage | SET → Save session; exit, reopen; saved Recall | Both current and last-calculation data round trip on native filesystem |
| Recovery | On backed-up app-private test files only, truncate the newer slot | Older valid record loads; next save repairs alternate slot |
| STAT CSV | Export current Table IC/direction, import in List Editor | Distinct names; selected columns map to Lists; ≤998 data rows; no reboot |
| Storage full | Export with insufficient available storage | Clear I/O error and no successful-partial-file claim; old export/save survives |
| Runtime budget | Dense fields, 9×9 states/IC, long run | Measure latency, cancellation latency, stack/heap high-water; no unbounded growth |
| OS coexistence | Inspect existing GRAPH, List, STAT, shared variables before/after | No application writes to those OS datasets |

Do not interpret two-slot checksums as proof of power-loss durability. Physical power interruption testing, if later performed with backed-up disposable app data, needs a separate result record. Runtime performance has no measured hardware numbers yet.
