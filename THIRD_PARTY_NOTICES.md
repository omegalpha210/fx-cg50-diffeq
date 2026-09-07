# Third-party notices and redistribution

The project's MIT License covers project-authored material, not the dependencies
under the terms below. Exact upstream commit IDs are in
[tools/toolchain-lock.json](tools/toolchain-lock.json). Notices were checked against
the installed source revisions used by the release linker, not inferred from names.

| Component | Verified revision/version | Role | Terms and supplied notice |
|---|---|---|---|
| [gint](https://git.planet-casio.com/Lephenixnoir/gint) | 2.11.0, `badbd0f` | Linked native runtime; host font/key constants | Custom permissive statement in [upstream README](docs/licenses/gint-README.md); permits use, sharing/modification for any purpose; no standard SPDX license is asserted |
| [fxlibc](https://git.planet-casio.com/Vhex-Kernel-Core/fxlibc) | 1.5.1, `e2f458f` | Linked C library | [CC0-1.0](docs/licenses/fxlibc.txt), except identified third-party components |
| Grisu2 | bundled `grisu2b_59_56` | fxlibc floating-point formatting | [MIT, Florian Loitsch](docs/licenses/grisu2.txt); copyright/permission notice retained |
| [OpenLibm SH port](https://git.planet-casio.com/Lephenixnoir/OpenLibm) | 0.7.0-based, `9828d2e` | Linked math library | [MIT/BSD/ISC/FDLIBM notices](docs/licenses/OpenLibm.md); retain applicable copyright, conditions and disclaimers with source and binary distributions |
| [GCC libgcc](https://gcc.gnu.org/) | GCC 14.1.0 | Linked arithmetic/soft-float runtime | [GPLv3](docs/licenses/GPL-3.0.txt) with [GCC Runtime Library Exception 3.1](docs/licenses/GCC-RUNTIME.txt) |
| [fxSDK](https://git.planet-casio.com/Lephenixnoir/fxsdk) | 2.11.0, `09e2cf5` | Build/packaging tools; retained minimal-example icons and packaging-format reference | [MIT, gint/fxSDK contributors](docs/licenses/fxsdk-MIT.txt); preserve notice for copied assets/derived portions |

## Linked binary and license boundary

The target link command/map resolves libgint-cg, libc, OpenLibm and libgcc. It does
not link OpenLibm's LGPL test programs, libstdc++, a Python runtime or host SDK tools.
GCC's SH/soft-float runtime headers carry the Runtime Library Exception. This C
add-in is independently authored source compiled with the GCC toolchain without
nonfree compiler plugins; the exception permits the resulting combined target code
to use terms consistent with its independent modules. It does not relicense libgcc
itself or remove obligations when distributing the compiler/runtime separately.
See the [FSF exception and FAQ](https://www.gnu.org/licenses/gcc-exception-3.1-faq.en.html).

No linked component audited here imposes GPL on the application's independent
source. The project owner selected MIT. Keep these notices when redistributing
source, and provide the assembled THIRD_PARTY_NOTICES.txt alongside the release
binary. Library-specific notices remain authoritative; this table is an inventory.

## Fonts, icons, screenshots and borrowed materials

- `tests/host/vendor/font8x9.png` and `tests/host/gint/keycodes.h` are byte-for-byte
  copies of the pinned gint source. `tests/host/font_data.h` is generated from that
  atlas. They retain gint's upstream permission statement and attribution. The
  target links the corresponding gint font; no proprietary OS font was extracted.
- `assets/icon-uns.png` / `icon-sel.png` are original mathematical drawings from
  `tools/make_icons.py`, with no external image or font source.
- Minimal example icons under `examples/hello/assets-cg/` originate in the pinned
  fxSDK MIT templates. The example's binary and unused template artwork are excluded.
- `tools/verify_g3a.py` checks the format documented by fxSDK's fxgxa sources; the
  fxSDK MIT notice is retained for any derived format/source portions.
- UI PNGs come from this application's host drawing adapter and the permitted gint
  atlas. They are not CASIO manual extracts and are labeled as host renders. The
  optional overview captions use Pillow's bundled default font through Pillow;
  no standalone Pillow font is copied into the source or add-in.
- CASIO/manual PDFs, extracted page images and private local diagnostics are not
  redistributed. CASIO trademarks do not imply affiliation or endorsement.

The installed fxlibc also includes BSD-3-Clause TinyMT sources, but no TinyMT/rand object
occurs in this add-in link map. Those sources are not copied into this repository.

## Development-only tools

GCC/binutils (GPL), fxSDK (MIT), CMake (BSD-3-Clause), Python (PSF), Pillow (MIT-CMU)
and optional PyMuPDF (AGPL/commercial) are external tools, not bundled executables
in this source snapshot or add-in. Optional PyMuPDF was used only for local manual
inspection; ordinary builds/tests do not depend on it. Bootstrap retrieves them
under their own terms and does not convert them to MIT. If redistributing toolchain
binaries separately, audit that separate distribution and its source obligations.
