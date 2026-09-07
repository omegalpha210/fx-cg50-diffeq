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
| solver-parameters.png | Range -3..3, h=.1, Step1, SF12, Max steps20000 |
| graph-solution.png | Same example, SF0 |
| graph-slope-field.png | Same example, SF12, default Arrow/Pale Blue; 396×224 → 792×448 nearest-neighbor |
| graph-trace.png | Same example, NORMAL TRACE after three RIGHT events; colored speeds and LEFT/RIGHT endpoint keys |
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

The beta.3 captures use development source 0fbe0de. TRACE softkeys and the IC limit
hint were regenerated with the exact public candidate renderer.

Captures were visually checked. Host output is not evidence of physical LCD
contrast, calculator key timing or OS behavior; hardware retesting remains pending.
