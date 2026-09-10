# README image sources

Every UI image comes from this project's production app/rendering code, executed
by `build-host/host_app` with scripted key events. No hardware photographs,
calculator manual screenshots, downloaded calculator photos or mockups are used.
The permitted gint font is attributed in THIRD_PARTY_NOTICES.md.

| File | Source and displayed state |
|---|---|
| diffeq-icon.png | Project's original assets/icon-uns.png from tools/make_icons.py; 92×64 → 184×128 with nearest-neighbor |
| equation-entry.png | General first-order equation 1-y^2 |
| initial-conditions.png | x0=0, y0={0,0.5}; current ten-value help |
| solver-parameters.png | Range -3..3, Method RK45, h0=.1, RelTol1e-6, AbsTol1e-9, SF12 (Max steps below scroll) |
| graph-solution.png | Same example, SF0 |
| graph-slope-field.png | Same example, SF12, default Arrow/Pale Blue; 396×224 → 792×448 nearest-neighbor |
| graph-trace.png | Same example, NORMAL TRACE after three RIGHT events; F1 cursor INIT, orange NORMAL/green FAST/cyan FASTER and bounded LEFT/RIGHT endpoints |
| graph-gsolve.png | Same example, first solution's Y-ICPT result |
| table-view.png | Same example, initial table page centered at zero |

All use V-Window X=-3..3/scale1, Y=-1.5..1.5/scale.5. Native frames are
396×224. Lossless optimized RGB PNGs retain readable pixels and contain no EXIF
or personal metadata. The nine README PNGs total less than 50 KiB.

Reproduce with an existing compiler/CMake/Python and Pillow:

```sh
./tools/test.sh
python3 tools/capture_readme.py
```

The v0.12.0-beta.5 Parameters capture shows RK45 h0/AUTO rows, F1 INIT and F2 ADV. Scalar Graph frames have no TIME label; SYS2 uses TIME/PHASE and enabled Event adds EVT.
Equation/IC show F1 INIT, Graph shows F6 INIT, and TRACE uses a 9px local cross.
Equation/IC/Parameters headers show 1/3, 2/3 and 3/3. V-WIN setup uses Parameters. All eight original gallery
frames are reproduced with the exact public candidate renderer; graph examples
retain default RK4. Two additional Main/subtype frames are linked directly from docs/ui-review/tiles-main-first.png and tiles-subtype-first.png.

Captures were visually checked. Host output is not evidence of physical LCD
contrast, calculator key timing or OS behavior; hardware retesting remains pending.

Three additional Phase screens and reproduction details are documented in [PHASE.md](PHASE.md).

Native menus and warnings: `python3 tools/capture_tiles.py` reproduces 18 current
frames in docs/ui-review, six 3x nearest-neighbor menu previews, and eight individual
PNGs in assets/menu. The renderer uses the same const geometry as the add-in;
[provenance and motif formulas](../../assets/menu/README.md). README warning examples
link the current tiles overview. Source images contain no CASIO/manual artwork.
