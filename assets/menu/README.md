# Original DIFFEQ menu graphics

Eight illustrative graph motifs were authored for this project, under its MIT
license. No CASIO icon, reference screenshot, PDF crop or generated mockup is
redistributed here. The supplied visual references guided the pastel-panel,
graph-curve and badge style; native text/borders are drawn separately by the app.

| PNG | Menu label | Illustrative motif |
|---|---|---|
| first.png | 1st | smooth increasing curve |
| second.png | 2nd | damped oscillation |
| higher.png | N-th | three modal curves |
| system.png | SYSTEM | phase spiral |
| separable.png | Separable | exponential decay |
| linear.png | Linear | linear growth |
| bernoulli.png | Bernoulli | logistic-shaped saturation |
| others.png | Others | nonmonotone curve |

These shapes do not uniquely identify an ODE's order/type. They are explicitly
authored parametric illustrations, not live numerical solution plots.

Reproduce with the existing host build and Pillow:

```sh
python3 tools/generate_menu_icons.py --check
cmake --build build-host -j8
python3 tools/capture_tiles.py
```

The generator documents each formula and emits src/ui/menu_icons.inc: 378 uint8
coordinate pairs, 10 stroke records, 8 ranges, **832 B** of read-only geometry.
Firmware uses those coordinates through ui_menu_icon; no PNG decoder, RK4/RK45
calculation, heap image or extra framebuffer is involved in menu display.
Each **108x34 PNG** is cropped from the host execution of that same C renderer.
Their combined **3378 B** is separate from firmware asset size. Main/Subtype
native and integer-enlarged screen previews are in docs/ui-review/tiles-*.png.
