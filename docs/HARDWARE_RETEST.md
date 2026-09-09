# Graph interaction priority retest — v0.12.0-beta.4

**HARDWARE RETEST REQUIRED — all46 new cases pending.** Record OS/version, release
SHA256, keys, expected/observed result, duration, LCD and storage behavior.

1. Main six items remain on one page.
2. Pale-blue divider has clear3px thickness.
3. Divider does not overlap help or softkeys.
4. Numeric1–6 shortcuts and selector indices unchanged.
5. Graph Settings Style F1 INIT visible/works; F2 blank.
6. Graph Settings Color F1 INIT visible/works; F3 COLOR retained.
7. Plain first-order no TIME label.
8. Second-order no TIME label.
9. N-th and non-SYS2 no TIME label.
10. EVT still appears independently when enabled.
11. SYS2 TIME label matches VIEW.
12. SYS2 PHASE label matches VIEW.
13. Delayed neutral G-Solve busy appears without quick-operation flicker.
14. Slow RK45 second-order y/y-prime ICPT and SYS ICPT complete.
15. EXIT/MENU cancellation while busy; no remaining busy text.
16. TRACE local9px cross, six curve colors and Black/Blue blink.
17. Cross center exact; edges clipped and old pixels restored.
18. G-Solve point marker matches TRACE; Event/EQPT distinct.
19. Graph F6 INIT, EXIT back; restore Event markers/status after a narrow redraw.
20. Graph INIT after pan and after returning from V-WIN/Table.
21. Graph INIT after Zoom/BOX, retaining selected TIME/PHASE.
22. ZOOM ORIG still restores factory window, different from custom graph-entry INIT.
23. BOX cursor starts at nearest plot center.
24. BOX Point1 movement/repeat clipped to plot; independent of h/Xdot.
25. Fresh EXE locks Point1; held EXE cannot advance twice.
26. BOX pale-blue stipple/outline retains curves and protects status/footer.
27. Point2 EXE commits only then and returns Graph base.
28. BOX all reverse-corner orders yield correct min/max.
29. BOX EXIT cancels in both stages; exact original view, ZOOM retained.
30. BOX same point/tiny rectangle rejected; movement clears warning.
31. BOX TIME keeps MAN bounds; AUTO follows window.
32. BOX PHASE does not change TIME window; Phase INIT retains PHASE.
33. Equation VAR hidden in SELECT.
34. Equation F3 VAR visible only in supported EDIT; FUNC/VAR EXIT preserves cursor.
35. Equation F1 INIT resets expressions/n only, including unfinished draft.
36. IC F1 INIT resets current IC only, including list/vector/draft.
37. Equation blank/partial draft has no immediate required warning.
38. Equation NEXT blocks incomplete fields and focuses first error; INIT remains usable.
39. IC blank/partial draft preserved across back/forward navigation.
40. IC NEXT validates all before numeric commit; first-error focus, no draft leaks.
41. Existing red domain/singularity/magnitude status stays nonfatal.
42. Valid-domain TRACE remains usable; no invalid continuation/false segment.
43. Valid-domain G-Solve remains usable with readable result footer.
44. MENU/Fugue reentry including RTC-driven busy; no clock-setting changes.
45. SAVE/RCL/migration unchanged; IC runtime drafts remain unsaved until NEXT.
46. LCD contrast, repeat response, long draft/navigation soak and heap/stack margin.

Earlier milestones below are historical.

---

# UI consistency priority retest — v0.12.0-beta.3

**HARDWARE TEST REQUIRED — all36 new cases pending.** Record OS/version, release
SHA256, exact keys, expected/observed screen, timing and save-file behavior.

1. Main MENU-only hint.
2. Main MENU red.
3. Equation no generic EXE NEXT hint.
4. IC no generic EXE NEXT hint.
5. Parameters no redundant EXE hint.
6. EDIT blue EXE normal weight.
7. Color chooser EXE normal weight.
8. All retained EXE glyph thickness matches normal text.
9. RK4 h.
10. RK45 h0.
11. AUTO indicator follows existing window policy.
12. MAN indicator preserves direct range commits.
13. Main SYS/RCL divider contrast.
14. Numeric 1–6 unaffected.
15. TIME label.
16. PHASE label.
17. EVT only when enabled.
18. TIME+EVT collision and END status.
19. PHASE+EVT collision and N1/N2 legend.
20. TRACE cursor/selected-curve/label/footer priority.
21. G-Solve prompt and bottom-edge result pointer priority.
22. Event squares centered on curve; equilibrium diamonds remain visible.
23. Selected2px curve visibility/blink, including Black↔Blue.
24. Output horizontal color line preview.
25. Output OFF retains color preview.
26. Field-inline input error vs plot/table END vs critical confirmation hierarchy.
27. SELECT highlight vs EDIT caret clarity.
28. Semantic F-key colors in every screen/slot.
29. All INIT yellow/black with separate reset scopes.
30. ADV black/white; EVENT/INFO no solve or I/O.
31. V-WIN access only Parameters/Graph.
32. 1/3,2/3,3/3 alignment; submenus no progress.
33. SAVE/RCL/load/resize/convert fresh F5 NO/F6 YES, EXE/EXIT, opening HOLD ignored; native migration unchanged.
34. MENU/Fugue reentry and session retention.
35. LCD contrast: divider, AUTO/MAN, h0, view labels, lines and blue EXE.
36. Key-repeat responsiveness and graph/menu navigation soak.

Earlier milestone records below are historical.

---

# UI/UX polish priority retest — v0.12.0-beta.2

**HARDWARE TEST REQUIRED — all36 new cases pending.** Record OS/version, release
SHA256, exact keys, expected/observed screen, timing and save-file behavior.

1. Main numeric1–6 opens the corresponding item.
2. Main F6 OPEN equals EXE for the current selector; returning retains it.
3. Main F1–F5 blank/inert, especially former RCL shortcut F5.
4. SAVE F5 NO returns Main without file I/O.
5. SAVE F6 YES performs one save after confirmation only.
6. SAVE EXE equals YES; held opening EXE/F6 cannot auto-approve.
7. SAVE EXIT equals NO; failure and No session keep Main selector safe.
8. Parameters F1 INIT retains Method/Event/V-Window and resets its numerical defaults.
9. V-WIN F1 INIT resets geometry only; retains Grid/Label/projection/manual solver settings.
10. Graph Settings F1 INIT resets its appearance only, retaining SF density.
11. Output F1 INIT restores dependent outputs/colors only.
12. Every visible INIT has Yellow background/Black text.
13. Parameters F2 ADV has Black background/White text.
14. ADV F1 EVENT retains existing Event editing and validation.
15. ADV F2 INFO remains read-only, no solve or I/O.
16. ADV has no INIT; EXIT closes only ADV and retains Parameters selector.
17. Style LEFT/RIGHT toggles; F1/F2 are blank/inert on that row; other rows offer F1 INIT.
18. Style hint LEFT/RIGHT: SEGMENT/ARROW toggle fits.
19. Color hint RIGHT/F3: COLOR fits and both keys open the existing palette.
20. Output one-line hint includes F3 COLOR; no second explanation; SYS9 row7 remains clear.
21. First SELECT row UP wraps to last.
22. Last SELECT row DOWN wraps to first; paged lists follow selection.
23. Dynamic RK4/RK45 rows and scalar-only SF wrap visible rows only.
24. Every Equation mode shows1/3 without title/formula overlap.
25. IC shows2/3; EXIT returns1/3 and retains fields.
26. Parameters shows3/3; EXIT returns2/3; auxiliary screens have no progress.
27. Equation F3 is blank/inert outside FUNC; no V-WIN entry.
28. IC F1–F5 are blank/inert, including during draft entry.
29. Parameters V-WIN returns to3/3 and prior selector.
30. Graph V-WIN returns to Graph with its existing numerical behavior.
31. Main hint: first MENU Red, EXE Blue, other text normal; one line.
32. Global EXE help token Blue, including EDIT, palettes and graph notices; no other MENU/EXIT recoloring.
33. Renderer/screenshot comparison for all updated screens and actual LCD contrast/clipping.
34. MENU/reentry, repeat/cancel response and long navigation soak; no reboot/reset/write.
35. SAVE/RCL v10 and older same-device sessions unchanged; cold launch and native Fugue behavior.
36. RK4/RK45/Event/Diagnostics/TRACE/G-Solve/Phase/Table/STAT smoke; data navigation must not become cyclic.

Earlier feature checks remain relevant but their former control labels are historical.
Use the current [USER_GUIDE](USER_GUIDE.md) control map.

---

# Event / Solver Diagnostics priority retest — v0.12.0-beta.1

**HARDWARE TEST REQUIRED — all 29 new cases are pending.** Record release SHA256,
calculator OS, method/settings, observed x/state, elapsed time and exact keys.

1. Parameters F2 SOLVE shows EVENT/INFO/blank/INIT/blank/blank; main bar unchanged.
2. SOLVE EXIT closes only the submenu and retains selector and values; F4 INIT works.
3. EVENT rows/editor/FUNC/VAR in first/second/N-th9/SYS9; invalid draft preserves input.
4. Event OFF with blank or preserved draft calculates normally; toggles do no work.
5. y'=y, y(0)=1, E=y-10 RISING STOP at x≈ln(10), FALLING does not stop.
6. MARK continues the same equation to the requested end.
7. y'=-y, E=y-.5 FALLING at x≈ln(2).
8. IC E=0: ANY immediate; directed first nonzero policy, constant zero and zero plateau.
9. Harmonic oscillator repeated MARK roots, no duplicate root; >32 keeps counting.
10. TIME Event squares use first visible output, remain clipped during pan/zoom.
11. SYS2 TIME/PHASE shows the same Event state and distinct equilibrium diamonds.
12. STOP curve ends at the refined root, including Step>1 and redraw.
13. STOP Table BTM/TOP exact terminal x/state and inline END: Event; no modal end screen.
14. TRACE cannot pass STOP by arrows, fast movement, endpoint jumps or x= input.
15. Ten first-order ICs stop independently; other branches continue; Table blanks beyond each STOP.
16. Backward Event accuracy and increasing-x RISING/FALLING meaning.
17. RK4 h=.1/.05 comparison including y'=y², E=y-10 before its singularity.
18. RK45 tolerance comparison, large initial h and rejected attempts; no rejected-state marker.
19. EXIT/AC cancellation during trajectory/refinement; actual CANCELLED report; no false marker.
20. RK4 INFO Steps/RHS, no adaptive rejection/tolerance rows.
21. RK45 INFO settings/status/Accepted/Rejected/Attempts/RHS, scrolling and blank softkeys.
22. Accepted/Rejected work includes scratch; rejection can be normal; counters do not wrap.
23. h min/max finite and ordered, magnitudes including scratch steps.
24. Hits versus Stored/32, initial-hit dedup and single-hit Stopped x.
25. Repeated INFO/SOLVE enter/exit performs no recalculation, file write or reset.
26. SAVE v10 current/recall Event settings; same-device v3–v9 load OFF, v9 RK45 retained.
27. MENU/reentry from Event EDIT/INFO/Graph/Table; fresh launch/load No solver run yet.
28. Physical LCD marker contrast/shape, labels and scroll hint; no overlap or color artifacts.
29. Key repeat/cancel responsiveness, cancelled TRACE-extension preservation and long SYS9/10-IC
    stack high-water with gint/OS interrupts; no reboot or unintended I/O.

Event math-domain gaps, G-Solve's STOP bounds and mixed-family numerical-limit
status should also be checked with the recorded host cases in [EVENTS](EVENTS.md).
The earlier feature checks below remain applicable; their old version labels are
historical. Parameters INIT now lives at F2 SOLVE → F4 INIT.

---

# RK45 priority retest — v0.11.0-beta.1

**HARDWARE TEST REQUIRED — all new RK45 cases remain pending.**
Record release SHA256, calculator OS, settings, observed values/time and keys.

1. Parameters Method LEFT/RIGHT in first/second/N-th9/SYS9: no immediate solve,
   RK45 Initial h/RelTol/AbsTol, hidden Step, first-only SF, Max steps scrolling.
2. Scientific tolerance edits, invalid 0/negative/1e-300 RelTol, INIT retention,
   repeated Method changes and F6 GRAPH/PREV/EXIT.
3. Compare y'=y, y''=-y and harmonic SYS with the numerical benchmark; backward
   and off-grid TIME x=/Table/Y-CAL; tighten tolerance and compare results.
4. Bounded sinh through x=7; singular y'=y² near x=1 must stop, never restart a
   far branch. Do not interpret the slightly shifted numerical pole as continuation.
5. Long SYS9 and ten-IC runs: EXIT/MENU during accepted and rejected attempts,
   workload status, no reboot; measure actual stack high-water and key latency.
6. TRACE NORMAL/FAST/FASTER, endpoint following/extension cancellation; Phase
   projection/field/nullclines/EQPT; verify the old graph/cursor survives cancellation.
7. Table TOP/BTM/MID and empty terminal cells, STAT import and export cancellation.
8. SAVE v9 with RK45/tolerances, RAM recall preference retention, same-device
   frozen v3–v8 sessions load as RK4; MENU/Fugue and restart behavior.

The earlier feature checks remain applicable below.

# fx-CG50 hardware retest — TRACE X/Y, endpoints and ten initial values

**HARDWARE TEST REQUIRED — every case below is pending on a physical calculator.**
Baseline `0de88c7` is preserved. See [ACCEPTANCE](ACCEPTANCE.md) for the validated
source and binary. Record device/OS, binary SHA256, inputs and exact key sequence.
Host UI rendering does not emulate SH execution, LCD timing or OS lifecycle.

## First priority — TRACE/IC polish (32 cases, all pending)

1. TRACE Y upper auto-follow: y exceeds the upper 10% margin and remains visible.
2. TRACE Y lower auto-follow: y exceeds the lower margin and remains visible.
3. TRACE X/Y simultaneous auto-follow: one combined pan, without two redraw flashes.
4. Y span and Yscale unchanged; also retain X span, h and Step. No jitter after landing.
5. TRACE F6 BACK absent; EXIT alone closes TRACE and restores the graph base bar.
6. NORMAL has a yellow background, including when selected.
7. FAST has the existing Bright Green background, including when selected.
8. FASTER has a cyan background, including when selected.
9. All speed text is black and readable; selected speed has a visible black border.
10. F5 LEFT jumps to configured Solver Xrange min, preserving curve and speed.
11. F6 RIGHT jumps to configured Solver Xrange max, preserving curve and speed.
12. Endpoint jump is exact when valid; invalid endpoints stop at a real nearest valid point with a warning.
13. No prefetch near endpoint: runtime [-6,6], x=5.9 then x=6 then F6 RIGHT perform no extra solve.
14. Only a beyond-runtime-edge movement extends; test both directions, EXIT during work, Max Steps and total-work rejection. Keep prior complete graph on cancellation.
15. FAST/FASTER crossing extends in the same input: x=5.95, Xdot=.1, FASTER RIGHT -> 6.25. Solver endpoints remain -6/6; F5/F6 still jump there.
16. Graph Settings has no fixed Density SF: Parameters help.
17. Grid selected: LEFT/RIGHT: ON/OFF toggle fits one line and arrows toggle.
18. Axis Label selected: same exact toggle hint and arrow behavior.
19. Grid F1/F2 are blank and inert; EXE keeps its screen completion action.
20. Axis Label F1/F2 are blank and inert; EXE keeps its screen completion action.
21. Style SEG/ARROW softkeys and left/right controls still work.
22. Color chooser, swatch, all six presets, cancel and INIT still work.
23. y0 accepts {0,1,2,3,4,5,6,7,8,9}; hint says at most 10. Higher/SYS retain one complete vector.
24. 11 y0 values are rejected with Too many initial values / Max: 10; retain draft and prior values.
25. 191-character boundary is separate: overflow is blocked before writing; show Input too long / Max: 191 characters, then DEL/AC and retry.
26. Ten solution curves render with the existing six-color cycle; TRACE/G-Solve reach family 10.
27. Ten-column Table: x frozen, horizontal navigation reaches y9/y10; TOP/BTM/MID and STAT remain correct.
28. Ten-IC workload guard: h=.0005 and range [-6,6] reject 240,000 total steps before calculation.
29. Main bottom hint is MENU: return to MAIN MENU and fits one line.
30. MENU actual behavior still returns to calculator Main Menu; EXIT navigation unchanged.
31. SAVE/RCL: ten values and colors roundtrip; load old v3/v4/v5/v6 and fallback after a damaged slot. Load never rewrites old files.
32. MENU/reentry from TRACE, repeat/extension, EDIT, palettes and Table: retain state, no reboot/stuck keys/unintended write. Also test a true cold launch.

## Retained contextual UI polish (28 cases, all pending)

1. OUTPUT shows exactly `LEFT/RIGHT: ON/OFF toggle`, fully visible on one line. LEFT/RIGHT, F3 COLOR, INIT and DONE retain their actions.
2. Equation SELECT has contextual help and no commit/next or EXIT-commit hint.
3. Equation EDIT shows `EXE: commit / next   EXIT: commit` on both visible and hidden cursor blink frames. Include Bernoulli n and N-th/SYS expressions.
4. IC numeric and scalar/list expression EDIT show the same hint; include `{0,1}` and SYS9's last state.
5. V-WIN numeric EDIT shows the same hint, including its seventh row.
6. Parameters numeric EDIT shows the same hint, in first-order and higher-order modes. Also check order/variable-count input.
7. EXE/EXIT commit returns to SELECT context. Invalid draft popup owns its message; dismissing restores EDIT. FUNC/VAR and numeric function picker retain their controls and restore the underlying edit state.
8. All bottom hints fit one line without clipping or softkey overlap. Check the longest V-WIN context, all Parameters rows, and full seven-row SYS9 IC/OUTPUT pages.
9. Linear second-order IC and other ordinary forms have no generic `UP/DOWN: select`. Table/G-Solve/TRACE retain their special arrow interactions.
10. First-order IC has no SHIFT/braces shortcut hint; the `y0: scalar or {values}; at most 10` description remains. Physical SHIFT+multiply/divide still inserts braces.
11. First-order IC SELECT retains `Comma: separator`; physical comma inserts list separators and EDIT uses the shared commit hint.
12. Graph base F6 reads PREV, including initial render and redraw after calculation.
13. Graph PREV matches the existing magenta PREV buttons. TRACE LEFT/RIGHT and ZOOM/G-Solve bars retain their own labels; EXIT restores base PREV and its color.
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

## v0.10.0-beta.1: SYS 2D Phase (HARDWARE TEST REQUIRED)

- [ ] SYS2 oscillator: GRAPH → VIEW → PHASE, clockwise closed trajectory; y1/y2 labels.
- [ ] TIME ↔ PHASE retains both windows, equation, IC, solver settings and Output.
- [ ] Phase V-WIN, arrows, IN/OUT/AUTO/ORIG affect only phase geometry.
- [ ] Phase TRACE NORMAL/FAST/FASTER, held arrows, endpoint buttons and numeric x=;
      x/y1/y2 progression, single-family UP/DOWN, immediate EXIT and MENU return.
- [ ] Field OFF/ON, independent NULL OFF/ON; arrow directions under unequal spans;
      visible pale field, red N1, blue N2, solution and black diamond on actual LCD.
- [ ] Harmonic oscillator, saddle, nodes and spirals: EQPT and INFO match known
      local linearizations. Center/Neutral is not nonlinear stability assurance.
- [ ] Multiple fixed points: LEFT/RIGHT traversal; no roots, domain boundary and
      near-degenerate Jacobian report bounded/Unavailable/Inconclusive results.
- [ ] Non-autonomous x+y2,-y1: trajectory allowed, Field at x=IC x0, EQPT disabled.
- [ ] Long valid expressions and rapidly oscillatory contours: measure full
      preflight + bounded paint time; EXIT/MENU responsiveness, no damaged old plot.
      Atomic paint retains keys for its completion; host timings are not SH timings.
- [ ] SAVE v8 / cold RCL retains independent windows and toggles. Load actual
      same-device v7 and older slots; old active SYS2 phase geometry migrates and
      configured automatic/manual Solver endpoints remain unchanged.
- [ ] Existing scalar slope field, 1–9-state modes, TIME G-Solve, ten-IC Table,
      STAT export and repeated MENU/Fugue lifecycle remain stable.
