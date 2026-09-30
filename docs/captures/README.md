# Current renderer gallery

These are lossless images from production C handlers through
`tests/host/gint_host.c` and the attributed gint font atlas. They are not
calculator photographs or CPU-emulator captures. Display contrast, refresh,
physical keys and SYSTEM power behavior remain **HARDWARE TEST REQUIRED**.

The retained gallery contains16 representative images used by the English/Korean
public READMEs and5 current review sheets. The eight original menu previews remain
in [assets/menu](../../assets/menu/README.md); required G3A icons remain in `assets/`.

| Review | Retained image | Generator |
|---|---|---|
| Main/Subtype and fixed-horizontal TRACE | [18 views](tiles-overview.png), [Main](tiles-main-first.png), [Subtype](tiles-subtype-first.png) | `capture_tiles.py` |
| General workflow and field/editor/Table cases | [66 views](host-overview.png), [8 staged views](workflow-overview.png) | `capture_ui.py` |
| IC visibility, Drawing bar and numeric cancellation | [12 views](visibility-overview.png) | `capture_visibility.py` |
| Long-list header positions | [4 views](power-overview.png) | `capture_power.py` |
| Pictured scalar example, TRACE/G-Solve/Table and icon | `equation-entry`, `initial-conditions`, `solver-parameters`, `graph-solution`, `graph-slope-field`, `graph-trace`, `graph-gsolve`, `table-view`, `diffeq-icon` | `capture_readme.py` |
| Event definition and numerical diagnostics | `event-settings`, `solver-diagnostics` | `capture_events.py` |
| SYS2 input, field and local linear analysis | [Phase provenance](PHASE.md) | `capture_phase.py` |

Build the current host harness, then generate reviews:

```sh
./tools/test.sh
python3 tools/capture_ui.py
python3 tools/capture_visibility.py
python3 tools/capture_power.py
```

By default, each suite writes individual frames, enlarged previews and contact
sheets only to `build/captures/<suite>/`. Frame/process scratch space is temporary
under `build/tmp/captures/`. Production source directories receive no new files.
An intentional gallery refresh uses the same command with `--update-docs`:

```sh
python3 tools/capture_readme.py --update-docs
python3 tools/capture_tiles.py --update-docs
python3 tools/capture_phase.py --update-docs
python3 tools/capture_events.py --update-docs
python3 tools/capture_ui.py --update-docs
python3 tools/capture_visibility.py --update-docs
python3 tools/capture_power.py --update-docs
```

Only the retained, fixed allowlist is copied. `capture_tiles.py --update-docs`
also refreshes the8 authored menu previews from the actual C renderer. Reproducible
intermediate frames and3x variants stay in build. Tests assert drawing/key behavior
directly; no docs PNG acts as a numerical or visual-regression golden.

Contact-sheet captions use Pillow's classic bitmap font explicitly. The installed
Pillow9.3 and12.3 APIs were checked against all95 printable ASCII glyphs; their
caption pixels and bounds match. Production framebuffer pixels use the separate
credited gint atlas. PNG compression can differ between Pillow/zlib versions, so
cross-version image comparison uses dimensions and decoded RGB pixels; a release
binary's SHA256 still identifies its exact published bytes.

Earlier compact sheets remain in [archive/captures](../archive/captures/README.md)
with their milestone evidence. They are frozen and are not overwritten by a
current gallery refresh.
