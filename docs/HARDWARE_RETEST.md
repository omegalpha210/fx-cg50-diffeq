# fx-CG50 hardware feedback retest

**HARDWARE TEST REQUIRED** for every item below. Current source milestone:
`33bcd2a`, branch `hardware-crash-fix`; rollback point `6addf4d` is retained.
Host PASS does not establish native keyscan, LCD, MENU, timer, Fugue or runtime
stack/allocator margins. Record model/OS, commit, SHA256 from ACCEPTANCE.md,
exact equation/IC/h/window, keys, latency and observed outcome. Preserve user
files and a known-good session slot; use copies for corruption tests.

## Current milestone: 42 priority cases

1. Edit a nonlast field → EXE: validate/commit, select the next field, no automatic edit entry.
2. Edit the last field → first EXE: commit, stay on the last field in SELECT.
3. Equation last field already SELECT → EXE: same NEXT action as F6.
4. IC last field already SELECT → EXE NEXT; edited last field requires commit then a second EXE.
5. Parameters last field SELECT → EXE GRAPH; edited last field first commits and stays.
6. OUTPUT x row → EXE → y row. EXE must not toggle G or open the palette.
7. OUTPUT unmodified last variable → EXE immediately DONE.
8. OUTPUT last variable after G/L/color change → first EXE confirms/stays, second EXE DONE. Modified nonlast row confirms and advances.
9. OUTPUT state row → RIGHT activates COLOR and opens chooser; subsequent family/cell navigation remains usable.
10. OUTPUT F3 COLOR opens the current state variable's chooser; arrows/EXE accept, EXIT cancels. x has no curve color.
11. COLOR label has a white background and separate red/orange/bright-green/cyan/magenta letters. Check LCD readability.
12. OUTPUT F1 GRAPH is standard blue; actual Parameters F6 GRAPH remains red.
13. OUTPUT F4 INIT resets all IC G/L flags and color assignments to the common six-color defaults. Confirm actual graph and SAVE/RCL colors.
14. INIT on V-WIN, Parameters, OUTPUT and Graph Settings resets values, selects the first item, ends editing and stays on the screen.
15. Equation FUNC opens only the softkey bar. Opening, paging and EXIT preserve draft, cursor, selection and equation pixels.
16. FUNC has no asin/acos/atan entries; softkey-only menu EXE inserts nothing.
17. SHIFT+SIN/COS/TAN still inserts inverse trig and evaluates correctly; test modifier state after MENU.
18. FUNC page 1: ABS/SINH/COSH/TANH/ASINH/next; page 2: ACOSH/ATANH/blanks/back. Insert at a middle cursor position.
19. VAR replaces only softkeys; F6 pages mode-valid variables (N-th 9: y1–y8, SYS 9: y1–y9). EXIT/EXE-no-op preserve input.
20. OUTPUT Black curve → TRACE blinks Black↔Blue. Other five colors keep visible alternates; no white disappearance.
21. Black curve → G-Solve selection blinks Black↔Blue. Verify UP/DOWN switching and EXIT pixel restoration.
22. Graph → ZOOM → IN redraws and keeps the ZOOM submenu.
23. ZOOM → OUT keeps the submenu.
24. ZOOM → AUTO keeps the submenu and current X domain; fits finite selected Y values.
25. ZOOM → ORIG keeps the submenu, restores default V-WIN and preserves equation/IC/h/manual Solver range. Repeat IN/IN/OUT/AUTO/ORIG 100 times.
26. EXIT from ZOOM closes only its submenu, restoring the Graph bar; next EXIT follows the existing parent path.
27. Main numeric 1/2/3/4 opens exactly the same types as F1/F2/F3/F4. UP/DOWN/EXE and SAVE/RCL remain; numeric 5/6 performs neither SAVE nor RCL.
28. Main `Recall / load session` fits. RCL still offers Last calculation (RAM) and Load saved session, so the description is truthful.
29. General y'=100*y, y(0)=1, h=.01, Solver/View X=-4..4: magnitude guard retains usable graph and shows ERROR rather than disabling all curves. Repeat -100*y and the user's original equation with recorded parameters.
30. No false line/clipping bridge across a known invalid gap. The host analytic gap fixture proves the consumer path; on hardware test only independently trusted reachable values, never invent a post-singularity continuation.
31. TRACE remains available on finite points despite overflow elsewhere; compare with the analytic exponential near x=0.
32. TRACE cannot display NaN/Inf or >1e100 coordinates at the boundary. Stop with an inline invalid-region notice, or jump only to a supplied trusted next segment. Fast repeats preserve graph pixels.
33. X-CAL finds y=2 on the reachable pre-overflow exponential branch (x approximately ±log(2)/100 for ±100*y).
34. X-CAL after an overflow gap: search a separately anchored, reachable IC curve on that side. Results require trusted values; the current RK4 does not restart the same IVP beyond failure. The host analytic fixture tests both islands on one supplied stream and sorted previous/next results.
35. Y-CAL at a reachable x returns the expected y, including x=0 and off-screen Y values.
36. Y-CAL in an unreachable/invalid region returns a safe inline error without disabling other locations; blank/invalid input does not reuse an earlier value.
37. ROOT/MAX/MIN on valid regions remain usable with overflow elsewhere; Y-ICPT/ICPT likewise. Exercise SYS y1'=1, y2'=-100*y2, IC1=(0;0,0), IC2=(4;4,1), h=.001, X=-1..4: IC1 y1 versus IC2 y2 has a reachable intersection near x=3.986 despite a failed secondary lookup near zero. Never report a root inferred only across a gap.
38. X-CAL numeric prompt starts empty, cursor blinks, only F6 RUN is labeled. F1–F5/OPTN do not open FUNC/VAR.
39. Y-CAL has the same empty scalar-only RUN prompt. Exercise decimal, NEG and scientific exponent input.
40. X-CAL valid input → EXE validates and runs immediately, identical to F6 RUN. Empty/invalid remains with a short error. Edit EXIT commits/stays, then EXIT cancels.
41. Y-CAL valid input → EXE validates and runs immediately; verify the same error and EXIT behavior.
42. EXIT/MENU/reentry regression: hold TRACE LEFT/RIGHT 2–5 s, release and immediately EXIT; also interrupt preparation and exact x= queries. No repeat backlog, reboot, stale modifier, duplicate MENU, leaked timer or damaged graph; repeat 100 cycles.

The current IVP solver retains accepted prefixes in each direction from each IC.
A failure does not establish post-gap values. Record case 34 as unsupported for
same-IVP restart when no trusted far-side anchor exists; do not mark it PASS by
joining two independent ICs. See [validity audit](NUMERICAL_VALIDITY_AUDIT.md).

## Retained regression and failure cases

- Invalid expression/number + EXE, EXIT or available stage softkey retains draft,
  cursor and screen after the short error. A valid unfinished draft commits before
  PREV/V-WIN/ADD/DROP/OUTPUT/SET/NEXT/GRAPH. On/Off LEFT/RIGHT toggles; EXE advances.
- Complete all seven Equation → IC → Parameters → Graph paths through order/state
  9. Formula is above fields, with no generic selector counter or equation h footer.
- ALPHA+ADD is x, plain ADD is +, XOT is x, ALPHA+SUB is y, plain SUB is subtraction,
  NEG remains unary minus even with ALPHA, SHIFT+square is sqrt, SHIFT+EXP is pi.
  Repeat after blink timeout and MENU; one-shot modifiers must not leak.
- Main EXIT stays; Graph EXIT follows Parameters → IC → Equation → type/order/Main.
  Same-process MENU resumes RAM input. True fresh execution uses defaults and does
  not automatically read slots. SAVE before choosing a document says No session.
- SAVE and both RCL choices retain equation/IC/masks/colors/constants and intended
  global settings. v3 saved records load with default colors; v4 retains selected
  colors. Unsupported/corrupt/newest-truncated slots fail or fall back safely.
  Old builds cannot read v4; use copies of known-good v3 data when testing rollback.
- Ordinary editing/NEXT/PREV/EXIT must never write session files. Repeat SAVE twice,
  STAT success/cancel/storage-full and newest-slot corruption using disposable data.
- ZOOM/G-Solve menu-only entry/page/exit has no full redraw or integration. AUTO
  fits selected finite Y at unchanged X even outside the old Y viewport.
- Test ROOT/MAX/MIN/Y-ICPT/Y-CAL/X-CAL and two/many-curve ICPT, including results
  outside Y viewport. Numeric G-Solve EXE and F6 RUN both validate and run immediately; edit EXIT commits and stays.
- Table TOP/BTM/DIR/IC+ and STAT import through OS List Editor preserve G/L meaning,
  headers, backwards direction and the 998-data-row limit. No leaked descriptor
  or unlabeled incomplete export is acceptable.
- Run a 15-minute mixed UI/TRACE/Graph/STAT/SAVE soak. Physical key latency, fixed
  repeat tuning, deepest stack/allocator margins and power-loss durability remain
  unmeasured until tested here. No item is premarked PASS.
