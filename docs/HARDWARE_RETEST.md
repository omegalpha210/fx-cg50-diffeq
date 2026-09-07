# fx-CG50 hardware retest — contextual hints, PREV, color swatches and SF

**HARDWARE RETEST REQUIRED — every case below is pending on a physical calculator.**
Baseline `79e5d9d` and all earlier history are preserved. Final binary/commit:
[ACCEPTANCE.md](ACCEPTANCE.md). Record device/OS, SHA256, complete inputs and
key sequence. Host UI rendering is not SH/OS emulation.

## First priority — UI polish (28 cases, all pending)

1. OUTPUT shows exactly `LEFT/RIGHT: ON/OFF toggle`, fully visible on one line. LEFT/RIGHT, F3 COLOR, INIT and DONE retain their actions.
2. Equation SELECT has contextual help and no commit/next or EXIT-commit hint.
3. Equation EDIT shows `EXE: commit / next   EXIT: commit` on both visible and hidden cursor blink frames. Include Bernoulli n and N-th/SYS expressions.
4. IC numeric and scalar/list expression EDIT show the same hint; include `{0,1}` and SYS9's last state.
5. V-WIN numeric EDIT shows the same hint, including its seventh row.
6. Parameters numeric EDIT shows the same hint, in first-order and higher-order modes. Also check order/variable-count input.
7. EXE/EXIT commit returns to SELECT context. Invalid draft popup owns its message; dismissing restores EDIT. FUNC/VAR and numeric function picker retain their controls and restore the underlying edit state.
8. All bottom hints fit one line without clipping or softkey overlap. Check the longest V-WIN context, all Parameters rows, and full seven-row SYS9 IC/OUTPUT pages.
9. Linear second-order IC and other ordinary forms have no generic `UP/DOWN: select`. Table/G-Solve/TRACE retain their special arrow interactions.
10. First-order IC has no SHIFT/braces shortcut hint; the `y0: scalar or {values}; at most 9` description remains. Physical SHIFT+multiply/divide still inserts braces.
11. First-order IC SELECT retains `Comma: separator`; physical comma inserts list separators and EDIT uses the shared commit hint.
12. Graph base F6 reads PREV, including initial render and redraw after calculation.
13. Graph PREV matches the existing magenta PREV buttons. TRACE BACK and ZOOM/G-Solve bars retain their own labels; EXIT restores base PREV and its color.
14. Graph F6 PREV returns to Solver Parameters with the existing navigation state.
15. First-order Graph Settings Color has a clearly visible Pale Blue rectangular swatch at factory defaults; compare its frame with OUTPUT.
16. Select all six pale colors: Blue/Red/Cyan/Magenta/Gold/Gray. Swatch updates on selection, cancel preserves it, SAVE/RCL restores it, and Graph Settings INIT restores Arrow/Pale Blue.
17. Separable, Linear, Bernoulli and General first-order Parameters show SF between Step and Max steps; select/edit it normally.
18. Linear second-order Parameters has no SF row.
19. N-th Parameters has no SF row for orders 1, 2 and 9, matching the existing scalar-field capability.
20. SYS Parameters has no SF row for sizes 1, 2 and 9.
21. Higher-order Parameters UP/DOWN visits exactly five visible rows and clamps at both ends; auxiliary V-WIN/OUTPUT/SET returns preserve the selector.
22. Higher-order Step EDIT EXE commits and selects Max steps. Edited last-row EXE commits/stays, then SELECT EXE calculates normally.
23. Higher-order Parameters never shows SF help; every visible row has its matching context and edit value.
24. Set first-order SF=20, visit 2nd/N-th/SYS, then return to first-order: SF remains 20.
25. Repeat the mode round trips with higher-order Parameters INIT: visible solver defaults reset, hidden SF remains 20. Field Style/Color also remain.
26. First-order Parameters INIT resets SF to 12 with solver defaults.
27. SAVE/RCL preserves SF20 both in first-order and when saved from higher-order after INIT; return to first-order to inspect it. Retain existing old-session migration and recovery checks.
28. MENU/reentry during ordinary EDIT, palette, Parameters and Graph retains appropriate state, cursor timing and input controls. No reboot, stuck modifier or unintended storage write; test a true cold launch separately.

## Retained simplification regression (37 cases, all pending)

1. Scalar first-order Parameters shows SF between Step and Max steps; edit SF 0/1/12/100 and reject invalid values.
2. Graph Settings has Grid/Axis Label and field Style/Color only; no density editor.
3. Slope Field heading spacing is readable and visually grouped with Style/Color on the LCD.
4. A new first-order document defaults to Arrow; verify horizontal/right, positive/up and negative/down slope.
5. Factory field color is Pale Blue; compare all six field colors with the existing solution palette.
6. Graph Settings INIT restores Grid/Label ON and Arrow/Pale Blue; SF=24 must remain 24.
7. Scalar first-order Parameters INIT restores SF12, h/Step/Max Steps and automatic range; selected field color/style remain.
8. OUTPUT has no x, Fixed or CSV X row in any of the seven equation kinds.
9. LEFT/RIGHT toggles dependent output ON/OFF across Graph, TRACE, G-Solve, Table and STAT; a hidden derivative still participates in the numerical solution.
10. F3 COLOR opens only the selected state palette; cancel preserves it. Multiple first-order ICs cycle distinct colors from the selected y seed.
11. Table x is always the first visible column, including all outputs OFF.
12. CSV/STAT first column is always x, including an old x-OFF session and all outputs OFF. Actually load the file into STAT.
13. F5 SET enters Graph Settings directly; EXIT restores the exact Parameters selector and draft commit policy.
14. No Private Constants selector or insertion route remains; pi/e, arithmetic and numeric functions still work.
15. Load same-device v3/v4/v5 files, including before first calculation (no recall). Preserve equation/SF/IC/settings; v3/v4 get Arrow/Pale Blue and v5 explicit Segment/color stay. Check migration notice and fallback after a bad newest slot.
16. First-order y0=0 and singleton {0}: one curve, one y Table column. F4/F5 blank/inert.
17. Physical SHIFT+×, comma, SHIFT+÷ enter y0={0,1}; one common x0, two independent curves.
18. Enter y0={0,1,-1}; change x0 and confirm all three starts move together. Test duplicate values and {1/4,sqrt(2),pi}.
19. Reject empty/malformed/unbalanced/nested list and division by zero; dismiss error and repair draft without losing committed values.
20. Tenth list item is rejected. Nine-item long edit scrolls correctly; nine values with default range/h=.0005 are rejected by total-work preflight before calculation.
21. Manual separable example: f=1, g=y^2-1, x0=0, y0={0,1}, Solver -5..5, h=.1; V-Window x ±6.3, y ±3.1, scales1. Show decreasing first curve and equilibrium y=1.
22. Manual example Table shows x/y1/y2, with x=0 values 0/1. Export the two solution columns in the same order.
23. Second-order IC has exactly x0/y0/y'0 for one solution; no ADD/DROP. Edit each and verify oscillator.
24. N-th orders1..9 retain x0 plus all n state initial values, scrolling through the ninth; no multiple-vector syntax.
25. SYS sizes1..9 retain x0 and all y1_0..ym_0 values. Conversion from N-th retains the complete vector.
26. Table first page centers 0 for the default symmetric range. Check a fractional x0 with no zero grid sample, a positive-only range, asymmetric -2..8 and a range with fewer than seven rows.
27. UP moves toward smaller x in the unified Table and clamps at TOP.
28. DOWN moves toward larger x and clamps at BTM; rows always strictly ascend.
29. F1 TOP shows the smallest reachable x, including an off-Step terminal point.
30. F2 BTM shows the largest reachable x and the final page; repeated BTM is stable.
31. F3 MID returns to exactly the initial center page after TOP/BTM/scroll; repeat many times.
32. No DIR mode or IC+ action remains; F4 is blank/inert on Table.
33. Table x column remains frozen under repeated LEFT/RIGHT and TOP/BTM/MID.
34. Six/nine first-order solutions scroll y columns; labels mean solution index. High-order derivatives and SYS state labels retain their own meaning. Show horizontal help only for more than two dependent columns.
35. Separable sinh(x)*(y^2-1), (0,0), h=.1: TOP near -4.4 and BTM near +4.4 with inline END: Numerical limit, no modal. Step3 still includes terminal points. Mixed reachable lengths show --/CSV blank only in unavailable solution cells; verify actual STAT blank-cell alignment.
36. SAVE/RCL roundtrip list values, output flags, solution colors, SF and field appearance. Repeated slots recover, cancelled export removes only closed incomplete files, legacy source remains untouched on LOAD.
37. MENU/reentry during list edit, palette, SF100 render, Table/STAT, TRACE repeat/extension and Graph; no reboot, stuck modifier, unwanted write or lost timer. Cold launch still starts defaults.

## Retained solver/navigation regression checklist (49 cases)

1. First-order → Separable: f(x)=sinh(x), g(y)=y²-1, IC (0,0), default window and h=.1. Compare near-origin values with -tanh(cosh(x)-1); recognize that the analytic solution is bounded.
2. GRAPH: magnitude warning preserves the valid computed prefix in both directions.
3. No point above 1e100, NaN or Inf is plotted; no line bridges invalid data.
4. TABLE → BTM: remain on the last valid page near +4.4, not x0.
5. TABLE → TOP: remain on the last valid page near -4.4.
6. Repeated BTM at either numerical end never opens a separate Table End modal.
7. Numerical end displays `END: Numerical limit`; a normal bounded run reaching its requested endpoint displays END.
8. MID after BTM restores the central page. Repeat TOP/BTM/MID, including Step=3.
9. Main F1–F4 labels are blank and keys have no action.
10. Main digits 1–4 and UP/DOWN+EXE open the four existing type workflows.
11. RCL description `Recall saved session` fits. Test both RAM last calculation and saved-session load.
12. SAVE still requires explicit F6; verify cold defaults and subsequent saved load. No automatic writes on ordinary edits/EXIT.
13. FUNC open in EDIT: EXE validates/commits/advances. FUNC is absent in SELECT; ordinary SELECT EXE performs NEXT. Invalid draft remains editable.
14. VAR open in N-th/SYS: EXE follows EDIT/SELECT rules, EXIT closes only the overlay, and arrows retain ordinary editor semantics. F1–F5 insert tokens; F6 paginates.
15. Equation SELECT EXE reaches IC from first, middle and last equation rows.
16. Equation EDIT EXE commits and selects the next field without opening its editor.
17. Last-field EDIT EXE commits/stays; second EXE reaches IC.
18. IC SELECT EXE reaches Parameters from any IC field; edited values are preserved.
19. Parameters SELECT EXE reaches Graph from any row when settings are valid; test Equation→EXE→EXE→EXE.
20. VAR is hidden/inert in all first-order subtypes, linear second-order and N-th 1. Physical x/y input still works.
21. N-th 9 shows y1–y8, SYS 9 shows y1–y9 with correct pagination and insertion. SYS 1 exposes y1.
22. Table x remains fixed on the left, including after all dependent outputs are disabled.
23. With at least four enabled dependent variables, LEFT/RIGHT scrolls only those variables. Compare x values before/after scrolling.
24. Horizontal-scroll help appears only with more than two enabled dependent columns. STAT CSV always includes x first.
25. TRACE RIGHT moves continuously into the right margin and auto-pans X without resetting x or the selected curve.
26. TRACE LEFT performs the symmetric operation, including repeated pans past the initial range.
27. TRACE UP/DOWN selects visible curves at the same x where valid; it never pans the screen.
28. TRACE blink/crosshair survives movement and auto-pan. Black alternates with Blue; no colored strokes appear on blank background.
29. Hold movement during extension; EXIT cancels promptly and retains the last complete graph/window/cache, without `Partial: Cancelled` replacing it.
30. Compare h and Step before/after TRACE, pan, ZOOM and ORIG; manual Solver range must remain unchanged.
31. In NORMAL at fractional Xdot, one key moves approximately one display pixel. Compare the interpolated y with F1 x= explicit RK4; assess accuracy/resolution with many ICs and long domains.
32. ZOOM submenu LEFT/RIGHT/UP/DOWN pans the current graph.
33. Every ZOOM arrow pan keeps IN/OUT/AUTO/ORIG visible.
34. IN, OUT, AUTO and ORIG each keep the ZOOM submenu; ORIG restores the default window without changing h.
35. First EXIT closes ZOOM only; next EXIT follows Graph→Parameters.
36. h=0 is rejected at validation; retain the draft and correct it without leaving Parameters.
37. Negative h is rejected similarly.
38. h=1e-300 (or another representable tiny positive h) is blocked by preflight before calculation; test NaN/Inf-producing expressions as invalid numbers.
39. h=.001, x0=0, Solver -6..6, default Max Steps=20000 is allowed.
40. h=.0001 with that range is rejected before integration (60000 steps per direction).
41. h=.0001, Solver 0...01, x0=0 is allowed; small h alone must not be forbidden.
42. Per-path rejection suggests increasing h or Max Steps. Raising Max Steps can allow an affordable path; aggregate limits still apply.
43. Repeated rejected calculations cause no large RAM allocation or progressive memory loss. Try tiny h repeatedly, then a normal calculation.
44. After rejection, preserve equation/IC/V-Window/manual range/h and select h in Parameters. Correct only the intended field.
45. Multiple IC workload: ordinary nine-IC h=.1 calculations work; excessive total paths are blocked before integration.
46. Dimension workload: ordinary N-th/SYS 9 cases work; nine states × one IC × h=.000125/default range with Max Steps=100000 is blocked by the aggregate work guard.
47. During a long but accepted calculation or extension, EXIT remains responsive and returns to a usable state. Retest at complex RHS expressions.
48. Repeated allowed/rejected/cancelled runs do not hang or reboot. Record elapsed time and observed memory symptoms; do not label numerical instability as a resource failure.
49. MENU during TRACE repeat/extension, Graph and editor → calculator menu → add-in re-entry preserves the appropriate state and restored repeat/timer policy. Test a true cold launch separately.

## Retained baseline checks

- Real Fugue/BFile SAVE, saved load and STAT CSV export/import: handles close, no
  reboot, valid file, incomplete-file cleanup, explicit writes only.
- OUTPUT keeps its separate list EXE behavior: nonlast next row, modified last
  commit/stay, next EXE DONE; unmodified last DONE. LEFT/RIGHT toggles ON/OFF; F3 opens the palette.
- INIT in Parameters, V-Window, Output and Graph Settings returns to the first
  selector with visible defaults (higher-order Parameters retains hidden SF); YES/NO and palette selection retain their own controls.
- ROOT/MAX/MIN/Y-ICPT/X-CAL/Y-CAL/ICPT, off-screen Y candidates, reverse direction,
  independently anchored reachable regions, multiple results and no gap roots.
- Phase portrait with two selected axes, curve selection/blink, SAVE/RCL colors,
  higher-order conversion and nine-variable IC entry remain usable.
- Cancel and MENU under held keys; no stuck modifiers, lost EXIT, duplicate timer,
  continuous repeat acceleration or unintended storage writes.

## Evidence record

Use one record per failure/pass: case number, device/OS, SHA256, complete inputs,
key sequence, observed versus expected behavior, screenshot/video and repeat count.
The 28 UI polish, 37 simplification and 49 solver/navigation cases may be marked PASS only after an actual device run. Identify the checklist section as well as its case number.
