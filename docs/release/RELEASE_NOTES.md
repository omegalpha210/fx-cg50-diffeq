# DIFFEQ v0.9.0-beta.2

A public beta update for the **CASIO fx-CG50**, with a clearer input workflow,
richer graph exploration, simpler outputs and visual English/Korean documentation.

## Changes since beta.1

- Ordinary SELECT EXE now runs NEXT/GRAPH/DONE from any row. EDIT EXE commits and
  selects the next visible field; EXIT commits/stays. FUNC is shown in EDIT,
  VAR only in relevant modes, and Main uses digits1–4 or arrow selection.
- First-order initial values use one common x0 and `y0={...}` for up to nine
  solutions. Higher-order/SYS input shows one complete initial-state vector.
- OUTPUT now has a single dependent-state ON/OFF setting shared by graph, TRACE,
  G-Solve, Table and CSV. x is always the first Table/CSV column. The existing six
  solution colors are retained; F3 opens the color chooser.
- Slope fields have Segment/Arrow styles and six pale colors, with Arrow/Pale Blue
  defaults. Graph Settings shows a real color swatch. SF density appears only in
  scalar first-order Parameters; unsupported modes and their INIT preserve it.
- TRACE adds NORMAL/FAST/FASTER, interpolated movement and viewport following,
  retaining cancellation and bounded-cache behavior during extension.
- Table combines both integration directions in ascending x. TOP/BTM/MID jump to
  reachable ends or the center, while horizontal scrolling keeps x fixed. Valid
  terminal samples remain visible and numerical ends use inline notices.
- Preflight checks reject excessive RK4 work before calculation, with guidance
  for h and Max steps. Numerical failures preserve valid computed regions.
- Shared EDIT hints, clearer IC/OUTPUT help and magenta Graph PREV complete the
  calculator UI polish. Graph PREV returns to Parameters; submenus keep their controls.
- New visual README in English and Korean, using original icons and eight
  reproducible captures from the app renderer.

RK4, G-Solve/ICPT, the existing device crash/lifecycle fixes and explicit SAVE/RCL
remain part of the app. This release imports a validated development milestone;
it is not a claim that all the retained functionality is newly added in beta.2.

## Upgrading saved sessions

Back up your DIFFEQ session files before upgrading. New saves use **v6** and can
load same-device v3/v4/v5 records through the existing two-slot recovery path.
Old graph/list flags are combined with ON if either was enabled. Private Constants
are removed; old references are converted to numeric literals where possible,
otherwise the retained expression needs review. Built-in pi/e remain.

Legacy scalar ICs at the first common x0 are retained; higher-order records retain
the first complete vector. The app reports adaptations and leaves source files
unchanged on LOAD; subsequent explicit SAVE can rotate over old slots. Older
add-ins may reject new saves. See [migration details](../OUTPUT_LIST_AUDIT.md).

## Download and install

Copy **DIFFEQ.g3a** from this release to the fx-CG50 USB drive root, safely disconnect,
and launch **DIFF EQ** from Main Menu. Check the file with **SHA256SUMS.txt**.
**THIRD_PARTY_NOTICES.txt** accompanies the linked libraries and copied materials.
**VALIDATION.md** records the exact public commit, build/test results and artifact.

## Public beta status

Core first/second-order workflows have been tested on real fx-CG50 hardware by the
project owner. **HARDWARE TEST REQUIRED** for the newest UI, LCD contrast/colors,
held-key/blink timing, MENU/Fugue reentry, native SAVE/RCL and STAT interactions.
Host/UBSan PASS is not a physical-device pass. Other calculator models are untested.

Fixed-step RK4 has no adaptive error estimate or stiff-equation handling. G-Solve
can miss unsampled features, TRACE uses bounded retained samples, and CSV needs
manual STAT import. Read the README numerical notes and hardware checklist.

Report calculator/OS and beta version, equation, IC, solver/window settings,
exact keys and expected/actual results. Project code is MIT; dependencies retain
their notices. Unofficial community project, not affiliated with or endorsed by CASIO.
