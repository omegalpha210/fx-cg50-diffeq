English | [한국어](README_KO.md)

<p align="center">
  <img src="docs/images/diffeq-icon.png" width="184" height="128" alt="DIFFEQ's original solution-curve and slope-field icon">
</p>

# DIFFEQ for CASIO fx-CG50

Solve, graph, and explore ordinary differential equations on your calculator.
DIFFEQ is a native **fx-CG50 add-in** with colorful solution curves, slope fields,
TRACE, G-Solve, numerical tables, and phase analysis for two-variable systems.

**Public Beta · v0.10.0-beta.1 · [MIT License](LICENSE)**

**[Download the beta](https://github.com/omegalpha210/fx-cg50-diffeq/releases/tag/v0.10.0-beta.1)**
· [All releases](https://github.com/omegalpha210/fx-cg50-diffeq/releases)
· [Report a bug](https://github.com/omegalpha210/fx-cg50-diffeq/issues/new/choose)

![Two DIFFEQ solution curves for y'=1-y^2 with a pale blue arrow slope field](docs/images/graph-slope-field.png)

*Screens on this page come from DIFFEQ's actual application renderer running in
the host test harness. They are not hardware photographs or CPU-emulator captures.
Core first/second-order workflows have been tested on an fx-CG50 by the project
owner; this beta's latest UI and device behavior still need hardware retesting.*

Open any image for its full-size view, especially on a phone.

## From an equation to a graph

**Equation → Initial Conditions → Solver Parameters → Graph**

| 1. Enter an equation | 2. Set initial conditions |
|---|---|
| ![General first-order editor containing 1-y^2](docs/images/equation-entry.png) | ![Initial conditions x0=0 and y0={0,0.5}](docs/images/initial-conditions.png) |
| Choose a type, then enter the right-hand side. | A first-order list draws a solution for each initial y value. |

| 3. Choose solver settings | 4. Draw and explore |
|---|---|
| ![Solver range -3 to 3, h=0.1, Step=1 and slope-field density 12](docs/images/solver-parameters.png) | ![Magenta and cyan solution curves with slope field disabled](docs/images/graph-solution.png) |
| Set the integration range, h, and optional field density. | Press GRAPH. Pan, zoom, trace, or open a table. |

Use **F6 NEXT** between stages and **F6 GRAPH** to calculate. In a selected,
unedited ordinary field, **EXE** performs the same primary action from any row.

## Take a closer look

| TRACE | G-Solve |
|---|---|
| ![TRACE cursor with x and y values and NORMAL FAST FASTER controls](docs/images/graph-trace.png) | ![G-Solve finds the first solution's y-intercept at x=0, y=0](docs/images/graph-gsolve.png) |
| Move along a solution and switch between curves. | Find roots, extrema, intercepts, and intersections. |

| Numerical table | Slope field |
|---|---|
| ![Ascending x table centered at zero with two solution columns and TOP BTM MID STAT controls](docs/images/table-view.png) | ![Two solutions over the arrow field for y'=1-y^2](docs/images/graph-slope-field.png) |
| Keep x in view while scrolling solutions; export for STAT. | Adjust density in Parameters and style/color in SET. |

## Phase Portraits for 2D Systems

| Enter a two-variable system | Explore its phase trajectory |
|---|---|
| ![System editor with the oscillator y1'=y2 and y2'=-y1](docs/images/phase-system-input.png) | ![Oscillator phase trajectory and normalized direction arrows](docs/images/phase-field.png) |
| Choose SYS → 2. The pictured oscillator starts at (y1,y2)=(1,0). | After calculating, use F4 VIEW → F2 PHASE. |

![Numerical nullclines, an equilibrium near the origin, and its Center / Neutral candidate classification](docs/images/phase-equilibrium.png)

**F5 ANLYS** opens **F1 FIELD**, **F2 NULL**, **F3 EQPT**, and **F4 INFO**.
Toggle arrows/nullclines, search for equilibria, then inspect the numerical Jacobian
and eigenvalues. LEFT/RIGHT cycles found equilibria; EXIT leaves the analysis menu.
N1 (red) is `f1=0`; N2 (blue) is `f2=0`. Use **F4 VIEW → F1 TIME** to return to
the time plot, or **F3 TABLE** within VIEW. Time and Phase keep separate windows.

Equilibria require an autonomous system. Non-autonomous trajectories remain
available; their field/nullclines use the displayed reference **x=x0**.
Stability labels describe the **local linearization**: a Center / Neutral
candidate is not a nonlinear or global stability result. Searches can miss roots;
inconclusive or unavailable classifications are retained honestly.

Both projections reuse a bounded **258-point trajectory cache**. Reprojection,
pan and zoom do not rerun RK4; closely spaced features can exceed the retained
resolution. Phase TRACE uses this cache and stays within its calculated time
range. See [numerical methods and bounds](docs/PHASE_NUMERICS.md).
**HARDWARE TEST REQUIRED:** these new Phase views and interactions are host-tested.

## Features

- **Seven equation types:** separable, linear, Bernoulli and general first-order;
  linear second-order; general N-th order; systems of ODEs. Orders and systems
  support **1–9 states**, with N-th-to-system conversion and two-state phase portraits.
- **Classical RK4**, integrated in both directions from the initial condition.
  First-order `y0` accepts up to **10 values** at a common `x0`; higher-order and
  system input uses one complete initial-state vector.
- **Slope fields** for the four scalar first-order modes: SF density 0–100,
  Segment/Arrow styles and six pale colors. Default: Arrow / Pale Blue.
- **2D SYS Phase:** normalized vector fields, numerical nullclines, up to 16
  equilibrium candidates, and local linear stability with Jacobian/eigenvalue details.
- **V-Window, pan and zoom**, six solution colors and dependent-output ON/OFF.
- **TRACE:** NORMAL / FAST / FASTER movement, curve switching, x queries and
  X/Y viewport following, and jumps to configured solver endpoints.
- **G-Solve:** ROOT, MAX, MIN, Y-ICPT, ICPT, X-CAL and Y-CAL.
- **Table:** ascending x, TOP / BTM / MID, a fixed x column and horizontally
  scrollable solution columns. **STAT-compatible CSV** exports up to 998 data rows.
- **Explicit SAVE / RCL** with recoverable session slots and older-session migration.

TIME TRACE speed buttons are **yellow / bright green / cyan**, with black text and a
border on the active mode: NORMAL=1×Xdot, FAST=2×, FASTER=3×. F5 LEFT and F6 RIGHT
jump to configured Solver Xrange endpoints without changing curve or speed.
Reaching an endpoint does not prefetch; a move beyond the calculated range extends
in that input. X/Y follow pans the view while preserving spans and solver settings.
Explicit TIME `x=` queries retain RK4 evaluation at the requested in-range x;
they are not replaced by Phase's cached interpolation.

Graph Settings toggles Grid/Axis Label with LEFT/RIGHT; their F1/F2 are blank.
First-order lists keep the **191-character** editor limit, with separate count
and length messages. Up to ten solution columns can be explored with x frozen.

## Try the pictured example

Choose **1st → Others** (general first-order) and enter:

```text
y' = 1-y^2
x0 = 0
y0 = {0,0.5}
h = 0.1
```

Set V-WIN to Xmin `-3`, Xmax `3`, Xscale `1`, Ymin `-1.5`, Ymax `1.5`,
Yscale `0.5`. Xdot updates automatically. Leave Parameters at Step `1`, SF `12`
and Max steps `20000`. The automatic solver range becomes `-3` to `3`.
The two curves approach y=1 to the right and y=-1 to the left.
Set **SF=0** to reproduce the solution-only view.

Use ALPHA+SUB for `y`, SHIFT+multiply/divide for `{`/`}`, and the physical comma
key between values. Multiplication is explicit (`2*y`); trigonometric functions
always use radians. See the [full controls and examples, in Korean](docs/USER_GUIDE.md).

## Install on your calculator

1. Open the [current beta release](https://github.com/omegalpha210/fx-cg50-diffeq/releases/tag/v0.10.0-beta.1).
2. Download **DIFFEQ.g3a**. `SHA256SUMS.txt` is available to check your download.
3. Connect the fx-CG50 by USB, select USB Flash mode, and open its storage drive.
4. Copy `DIFFEQ.g3a` to the drive's **root directory**, outside `@MainMem`.
5. Safely eject the drive and finish the USB connection.
6. Launch **DIFF EQ** from the calculator's Main Menu.

These steps follow [CASIO's add-in installation guide](https://edu.casio.com/content/dam/casio/global/edu-casio-com/download/files/fx-cg50-series/Inst_Users_Guide.pdf).
You only need the `.g3a` on the calculator. Keep a backup of existing session files
when upgrading: this beta writes format v8 and can read same-device v3–v7 files.
Older add-ins may reject new saves; adaptations are explained in the
[upgrade notes](docs/release/RELEASE_NOTES.md).

## Essential controls

| Context | Keys |
|---|---|
| Main Menu | Digits 1–4 or UP/DOWN + EXE choose a type; F5 RCL, F6 SAVE |
| Selected ordinary field | UP/DOWN selects; LEFT/RIGHT starts editing; EXE runs NEXT/GRAPH/DONE/OPEN |
| Editing | EXE commits and selects the next field; the last field stays. EXIT commits and stays |
| Equation | FUNC is available in EDIT; VAR appears only where relevant. EXIT closes the token bar first |
| OUTPUT | LEFT/RIGHT toggles ON/OFF; F3 COLOR, F4 INIT, F6 DONE. EXE follows the output rows |
| Parameters | F3 V-WIN, F4 OUTPUT, F5 SET, F6 GRAPH |
| Graph (except 2D SYS) | Arrows pan; F1 TRACE, F2 ZOOM, F3 V-WIN, F4 TABLE, F5 G-SLV, F6 magenta PREV |
| 2D SYS Graph | F4 VIEW → F1 TIME / F2 PHASE / F3 TABLE; Phase uses F5 ANLYS |
| Phase analysis | F1 FIELD, F2 NULL, F3 EQPT, F4 INFO; LEFT/RIGHT cycles equilibria, EXIT returns |
| TIME TRACE | LEFT/RIGHT moves, UP/DOWN switches curves; F1 x=, F2–F4 speed, F5/F6 configured endpoints, EXIT returns |
| Phase TRACE | Shows x, y1 and y2 along the retained trajectory; cached/interpolated x queries, no time-range extension |
| Table | UP/DOWN pages, LEFT/RIGHT scrolls columns; TOP/BTM/MID, F5 STAT |
| Session | SAVE is explicit; RCL offers last calculation or saved load. MENU returns to the calculator OS |

SF appears only in scalar first-order Parameters. Higher-order/SYS, including
N-th1 and SYS1, hide it and preserve its value, even after Parameters INIT.
First-order INIT resets SF to12. Style and color stay in Graph Settings.

## Build from source

With an existing compatible fxSDK/gint installation on PATH:

```sh
fxsdk build-cg -j8
python3 tools/verify_g3a.py dist/DIFFEQ.g3a
./tools/test.sh
```

The build uses C11/CMake and requires gint ≥2.11. Development and release builds
have been tested with fxSDK/gint 2.11.0, SH GCC 14.1.0, binutils 2.42, fxlibc 1.5.1
and the pinned OpenLibm SH port. These are tested versions, not universal minimums.
The host script uses Clang, CMake and Python 3 with UBSan enabled by default.

See [DEVELOPMENT.md](DEVELOPMENT.md) for the existing SDK setup, exact release
checks and image reproduction. Release binaries are built from the public tag;
timestamps mean a later rebuild can have a different file hash.

## Numerical notes and compatibility

**Target: CASIO fx-CG50.** Other calculator models and emulators are untested.

`h` controls the RK4 step. A smaller value can improve accuracy but increases work;
compare results at different h values. Preflight workload checks and Max steps
prevent excessive calculations. Fixed-step RK4 is not an adaptive or stiff solver:
rapid variation and singularities can still cause numerical failure.

Nonfinite values and magnitudes above `1e100` are not plotted. Valid computed
prefixes remain usable; the solver does not continue across an unknown gap.
G-Solve can miss unsampled features. TRACE uses a bounded retained/interpolated
sample set; Table may recompute values. CSV requires manual import into STAT and
does not directly write OS lists. See [numerical safety](docs/SOLVER_SAFETY_AUDIT.md)
and [release validation](docs/ACCEPTANCE.md).

**HARDWARE TEST REQUIRED:** Phase rendering/analysis, the latest LCD layout/colors, held-key and blink timing,
MENU/Fugue reentry, native SAVE/RCL and STAT behavior need device retesting.
The [hardware checklist](docs/HARDWARE_RETEST.md) keeps these separate from host PASS.

## Contribute and report bugs

Please include your calculator/OS version, beta version, equation, initial values,
solver/window settings, exact keys, and expected versus observed behavior.
A DIFFEQ photo or video helps; remove personal details before attaching it.
[Open an issue](https://github.com/omegalpha210/fx-cg50-diffeq/issues/new/choose)
or read [CONTRIBUTING.md](CONTRIBUTING.md).

## License and credits

Project-authored code, documentation and original icons use the [MIT License](LICENSE).
Dependencies retain their own terms in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
Thanks to fxSDK/gint, fxlibc, OpenLibm and the GNU toolchain contributors. The host
font/key data comes from the credited gint revision. Image sources and reproduction
are listed in [docs/images/README.md](docs/images/README.md).

Inspired by the DIFF EQ application on the Algebra FX 2.0. Reference manuals and
manual screenshots are not redistributed. This is an unofficial community project,
not affiliated with or endorsed by CASIO. CASIO and product names are trademarks
of their respective owners.
