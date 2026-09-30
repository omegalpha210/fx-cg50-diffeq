# Project structure

| Location | Ownership / source of truth |
|---|---|
| `src/ode/`, `src/math/` | Shared production solver/model/parser/Event/G-Solve/Phase calculations |
| `src/graph/` | Coordinate conversion, renderer, retained trajectory/TRACE and Phase display |
| `src/ui/` | Native forms, editors, dialogs, menus, Graph/Table interaction |
| `src/app.c`, `app_state.c`, `main.c`, `power.c`, `storage.c` | Navigation/state/startup, main-thread OS power policy, explicit SAVE/RCL and frozen migrations |
| `include/` | Authored runtime APIs; no generated font header |
| `tests/` | Same core sources, host adapters/fault injection, regression/golden reference fixtures; never linked into target |
| `assets/` | Original package icons and menu previews; compact runtime geometry is `src/ui/menu_icons.inc` |
| `tools/` | Canonical build/test, guarded cleanup/snapshot, independent package verifier, generators and capture suites |
| `docs/audits/` | Current numerical/memory/safety/acceptance evidence |
| `docs/development/` | Source/state/compatibility map and design/workflow records |
| `docs/captures/` | Reviewed current README/gallery images; not disposable render output |
| `docs/archive/` | Superseded meaningful design/migration/contact-sheet evidence |
| `docs/licenses/`, `docs/release/` | Required notices and publication checks |
| `docs/reference/` | Protected local manuals, never in the public snapshot or needed for builds |
| `build/host`, `build/target` | Reproducible CMake caches/objects/ELF/map/stack reports; ignored |
| `build/captures`, `build/tmp` | Disposable renderer output and managed scratch; ignored |
| `dist/` | Final user package; preserved rather than treated as cache |
| `examples/hello/` | Retained minimal package reference; optional cache in `build/hello-target` |
| `.local/` | Installed SDK/venv, private validation/release history and public checkout; never published |

[Build/test/release commands](DEVELOPMENT.md) · [Responsibility and Graph state map](docs/development/CODE_MAP.md)
· [Cleanup evidence](docs/audits/CODE_CLEANUP_AUDIT.md) · [Documentation index](docs/README.md)

The public snapshot excludes internal policy and manual-analysis/environment
records. It preserves its own main ancestry and releases. Authored sources and
required attributed generated inputs stay tracked; reproducible build/render
intermediates do not. There is no parallel numerical-core copy or new platform
architecture.
