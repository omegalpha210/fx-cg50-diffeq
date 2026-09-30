# Hardware-reported UI fixes

Historical record through the pre-06a491e UI/crash fixes. Current field rules,
explicit SAVE/RCL, v4 color compatibility and TRACE scratch behavior supersede
the old policies below; see [ACCEPTANCE.md](../ACCEPTANCE.md) and
[TRACE_EVENT_AUDIT.md](../audits/TRACE_EVENT_AUDIT.md).

Baseline: `aa55902`; crash-fix rollback point: `df7bdc8`; work branch: `hardware-crash-fix`. The 1st/2nd-order RK4 results and solution renderer already confirmed on a physical fx-CG50 are treated as regression-sensitive and are not redesigned here.

## P0: EXIT/checkpoint reboot

**Observed:** Graph → EXIT and the much shorter Main → SET → Solver Parameters → EXIT → EXIT path could show an OS busy indicator, reboot the calculator and return to initial setup.

**Confirmed code cause:** Both paths invoked automatic session persistence. Installed gint 2.11 explicitly requires BFile-backed Fugue Unix/C99 calls to execute through `gint_world_switch()`; the baseline session and STAT/CSV paths called them directly from the gint world. This is a confirmed API-contract violation. Physical proof that it was the only reboot cause remains **HARDWARE TEST REQUIRED**.

**Change:** Every complete filesystem transaction now runs in a synchronous OS-world worker. Session format v3 and two-slot recovery remain, while streaming serialization removes the 5,696-byte Record duplicate. An iterative top-level screen dispatcher owns explicit parents and performs dirty checkpoints only after screen handlers unwind. A clean form visit no longer sets dirty. See [CRASH_AUDIT](CRASH_AUDIT.md), [NAVIGATION_CALL_AUDIT](../development/NAVIGATION_CALL_AUDIT.md) and [MEMORY_AUDIT](../audits/MEMORY_AUDIT.md).

**Regression evidence:** SH builds compile/link against the installed headers with warnings as errors. Host tests cover 1,000 navigation cycles, autosave ON/OFF isolation, corrupted-slot fallback, failed-load immutability and all pre-existing numerical/UI paths. Calculator reboot elimination remains the first item in [HARDWARE_RETEST](../HARDWARE_RETEST.md).

## P0: numeric fields and Table cells were blank

**Observed:** Solver Parameters and V-Window labels appeared without current decimal values. After editing, the new value still did not appear. Graph -> Table displayed its column structure but no calculated floating values.

**Proven cause:** The `Document` fields and streamed `TablePage.row` values were populated; host numerical/table tests demonstrated this. Every missing target string used `%g` through `snprintf`, `vsnprintf` or `fprintf`. The installed fxlibc 1.5.1 `stdio.h` states that `%e`, `%f`, `%g` formatters are disabled by default. The `aa55902` startup did not call `__printf_enable_fp()`. Thus labels and integer fields could render while floating conversions produced no text. CSV floating cells were affected by the same root cause.

**Change:** `src/main.c` now calls `__printf_enable_fp()` on every native entry before any UI or storage formatting. The host build excludes this target-only header/call. No model, table sampling or RK4 code changed. The final SH ELF contains `__printf_enable_fp`, increasing binary text by about 11 KiB as documented by fxlibc.

**Regression evidence:** Target compiler/linker and 13 G3A checks pass. The table regression now explicitly solves `y'=-y`, y(0)=1, h=.1 and verifies x=0/y=1 plus the Step=2 row x=.2/y≈exp(-.2). Existing y'=y, oscillator, higher-order/SYS conversion and parser tests remain unchanged and pass. Physical text and CSV rendering require retest.

**Risk:** The formatter adds target code/data and exercises fxlibc's Grisu-based float path. Static size remains within the prior architecture budget, but actual display and CSV bytes must be checked on hardware.

## P0: add-in exit and immediate re-entry

**Reported risk:** After EXIT to the CASIO Main Menu, immediately selecting DIFF EQ and pressing EXE could appear to exit/crash; visiting another app first could make re-entry possible.

**Source audit:** gint 2.11 `start2()` reloads `.data`, clears user `.bss`, initializes drivers, calls `exit(main())`, runs destructors and calls `kquit()` to restore the OS. The keyboard driver initializes its queue from the physically held matrix so launch keys are not new presses. `getkey()` handles MENU with a full gint world switch. Return value 1 is normal in the installed fxSDK template and is retained. DIFF EQ owns no heap trajectory, persistent callback or long-lived file handle. The later curve-selection blink owns one bounded timer that is stopped before its screen returns.

**Change:** New `src/app_state.c` makes the app lifecycle explicit: it clears the complete `App`, applies defaults, then attempts saved-session load on every `app_run()`. `src/main.c` clears pending key events after gint/display/formatter setup. This removes dependence on implicit BSS state and prevents queued stale events from becoming first-screen input.

**Regression evidence:** A host test fills `App` with two different nonzero patterns and calls initialization twice in the same process; both runs produce the same clean default and no false Recall. The existing two-process saved Recall workflow still passes. The target build uses only installed, documented gint APIs.

**Risk:** `clearevents()` consumes only already queued events; it does not fabricate a key-release wait or alter repeat profiles. The reported launcher sequence cannot be faithfully emulated by the host adapter, so execution -> EXIT -> immediate icon EXE remains a required physical retest.

## Files intentionally unchanged for P0

`src/ode/rk4.c`, `src/ode/model.c`, `src/math/expression.c`, `src/ode/sampling.c` and higher-order/SYS conversion logic were not modified. The Table test was strengthened, while its production data path remained unchanged.

## P1: in-place entry and full graph viewport

**Cause:** The baseline reused `ui_edit()`/`ui_number()` as full-screen editors. Parameter and V-Window were generic chooser lists, so selection, editing and the resulting value were never visible together. The renderer reserved 44 pixels above and 23 pixels below the plot for a title, legend, IC count and completion text.

**Change:** `src/ui/forms.c` keeps an edit buffer inside the selected Parameter, V-Window, IC or constant row. Direct typing replaces the current value; EXE starts editing the visible value; EXE/F6 accepts it; EXIT cancels it. Numeric expressions still use the existing parser with state variables disabled. `src/app.c` applies the same inline editor to equation and Bernoulli exponent rows; the independent vertical IC screen owns all initial-value editing. `src/ui/graph_screen.c` uses an in-place trace-x field. `include/graph.h` and `src/graph/renderer.c` expand the viewport to x=0..383, y=0..197, with only the softkey strip below it. Normal title, legend, IC and completion text are removed. A numerical stop uses a one-time message and then redraws the retained partial graph.

**Regression evidence:** Host workflows enter `y'=-y`, x0=0, y0=1 without entering a separate editor and trace y(1)=0.3678798. Additional flows edit h and Xmin in place and assert that 0.05 and -7 immediately appear in their original rows. The SH build treats all warnings as errors.

**Risk:** Host font metrics closely follow gint's built-in font, but physical key repeat, modifiers and LCD colors are not emulated. The enlarged plot changes pixel mapping while preserving the same V-Window math; geometry tests were updated for the 384x198 area. **HARDWARE RETEST REQUIRED.**

## P2: Algebra FX DIFF EQ screen structure

**Cause:** The baseline used spacious two-column cards and separate equation/IC screens. Output exposed only one IC set at a time. This hid the original manual's dense field sequence and Output matrix.

**Change:** `src/ui/common.c` provides dense, full-width selected fields. Main keeps F1 1st, F2 2nd, F3 N-th, F4 SYS and F5 RCL, with F6 SET retained as an extension. After later hardware feedback, the equation workspace now contains only equation/coefficient rows. LEFT/RIGHT immediately starts cursor editing; UP/DOWN validates and moves fields. F1 opens a separate vertical IC block editor for all numeric initial values and ADD/DROP. Output remains a variable-by-IC matrix. A compile error returns to the failing equation row.

**Regression evidence:** Scripted UI workflows cover type selection, equation cursor movement, vertical IC editing, syntax recovery, N-th conversion, 9-state scrolling, Output, slope-only graph, phase, Parameter, V-Window, Table/STAT and automatic persistence.

**Adaptations:** F6 SET on Main, native Table/STAT export, private constants, direct phase, fx-CG50-style ODE G-Solve/ICPT and complete IC vectors remain documented extensions. Unsupported SKTCH, PICT, solution integral and OS G-Mem/List memory operations are not shown as working features.

**Risk:** A 9-state SYS has 19 rows and requires paging. The original monochrome pixel layout is translated to the 396x224 color framebuffer rather than enlarged pixel-for-pixel. **HARDWARE RETEST REQUIRED.**

## P3: high-contrast curve colors

**Cause:** Neighboring baseline colors included similar blue/green hues and the permanent legend consumed graph space.

**Change:** `src/graph/renderer.c` cycles adjacent curves through magenta, cyan, light green, orange, bright blue, gold, violet and additional high-contrast colors. Every solution segment draws a second perpendicular pixel; grid, axes and slope fields remain one pixel. The palette is fixed and therefore consistent across equation modes.

**Regression evidence:** Host rendering exercises the same RGB565 selection and segment code. Perceived color/thickness on the physical LCD remains **HARDWARE RETEST REQUIRED**.

## P0: Table export reboot risk

**Observed:** Table → CSV could power off or reboot a physical fx-CG50 and lose recent settings.

**Audit:** The old sample callback called floating `fprintf()` and file I/O while RK4, model RHS and expression evaluation frames were active. The file was streamed correctly in host tests, but the nested target call chain combined the largest avoidable stack/stdio/filesystem risks. Installed public gint/fxlibc headers provide Fugue files and no supported Main Memory List API.

**Change:** Table F5 is now STAT. It finishes each fixed Table page before formatting, with bounded page/output scratch in BSS. It exports current IC/direction and selected List columns to `DIFFSTATnn.csv`, caps label+data at the official 999 CSV rows, and performs every create/append/close/cleanup transaction inside an OS-world worker. A partial file is removed only after close is known to have succeeded. The built-in List Editor imports it through the documented CSV menu. See [STAT_EXPORT_AUDIT](../audits/STAT_EXPORT_AUDIT.md).

**Evidence and boundary:** Host tests cover exact file bytes, capacity, cancellation and cleanup. SH compiler/linker/package checks pass. The actual reboot regression and STAT import are **HARDWARE RETEST REQUIRED**.

## Physical subtraction and negative keys

Installed gint defines subtraction as `KEY_SUB=0x25`, unary negative as `KEY_NEG=0x14`, and X,θ,T as `KEY_XOT=0x61`. `getkey()` supplies one enriched press event with `shift`/`alpha` and removes modifier/release events. The editor now maps ALPHA+SUB to `y`, KEY_NEG to unary `-` regardless of ALPHA, XOT to `x`, SHIFT+x² to `sqrt(` and SHIFT+×10^x to `pi`. Host events cover all five mappings; physical timing is **HARDWARE RETEST REQUIRED**.
