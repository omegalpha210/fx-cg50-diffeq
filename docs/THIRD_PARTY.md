# Third-party components

The native add-in links gint, fxlibc, OpenLibm and the GCC runtime. Exact revisions and repositories are in [the toolchain lock](../tools/toolchain-lock.json); source checkouts remain in `.local/src/`. Local source/license copies are retained alongside the deliverable:

- gint: upstream permission statement in [its README](licenses/gint-README.md). The host test adapter also includes its proportional font atlas and key constants, credited in `tests/host/vendor/README.md`.
- fxlibc: [CC0](licenses/fxlibc.txt); its floating-point formatting includes [Grisu2's MIT notice](licenses/grisu2.txt).
- OpenLibm: [upstream license collection](licenses/OpenLibm.md), covering its MIT/BSD/ISC/FDLIBM-derived components. No OpenLibm test program is included in the add-in.
- libgcc: [GCC Runtime Library Exception](licenses/GCC-RUNTIME.txt) and [GPLv3](licenses/GPL-3.0.txt).

fxSDK, GCC/binutils, CMake, Python, Pillow, PyMuPDF and Poppler are build/verification tools, not bundled host executables in `DIFFEQ.g3a`. The menu icons are original generated assets. The CASIO reference PDF was supplied by the user and is a specification reference, not an application asset; it is not embedded in the binary. This is an independent implementation, not an official CASIO application.
