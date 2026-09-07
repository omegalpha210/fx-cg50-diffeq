# Phase screenshots

These images come from the production app's handlers and drawing code through
the host adapter. They are not photographs or calculator-emulator captures.
Run `python3 tools/capture_phase.py` after building `build-host/host_app` to
reproduce them. Pillow is required. Images retain the native 396-by-224 host
framebuffer size and contain no capture metadata.

- **phase-system-input.png** — Two-variable SYS equation editor with the harmonic
  oscillator `y1'=y2`, `y2'=-y1`.
- **phase-field.png** — The same numerical trajectory projected onto `(y1,y2)`;
  normalized arrows show the vector field in the separate Phase window.
- **phase-equilibrium.png** — Numerical nullclines (N1 red, N2 blue), the selected
  equilibrium near `(0,0)`, and its local linear **Center / Neutral candidate**
  classification. This wording makes no claim about nonlinear global stability.

Rendering and key paths are host-verified. Display appearance, responsiveness,
and navigation on the physical fx-CG50 remain **HARDWARE TEST REQUIRED**.
