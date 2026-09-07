# Host UI review frames

These images are framebuffer dumps from the real UI sources linked to the host drawing/key adapter. They verify layout, visible values, selection rows and workflow transitions; they are not emulator screenshots and do not establish exact physical fx-CG50 color or font rendering.

`host-overview.png` places eighteen review views together: Main, equation-only 1st-order editor, Parameter, V-Window, vertical Initial Conditions, Output matrix, full-size Graph, inline G-Solve result, numerical Table/STAT, ZOOM ORIG, nine-state Equation/IC, color chooser, TRACE, inline FUNC, paged VAR, scalar Y-CAL prompt and Graph Settings INIT. Individual PNG files preserve the 396 x 224 framebuffer size.

The layout choices are traced to `docs/reference/diff_eq_manual.pdf` in `../reference/README.md`. Final display, keyboard feel, float text, graph palette and immediate EXIT/re-entry remain **HARDWARE RETEST REQUIRED**.

Regenerated with `tools/capture_ui.py` and visually inspected for this milestone. COLOR letters and the four new views were also inspected at the native 396×224 size. Host pixels cannot establish LCD contrast or physical cursor/timer cadence.
