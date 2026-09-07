# Contributing

Bug reports and focused fixes are welcome. Use the issue template and include your
fx-CG50 OS version, DIFFEQ version, equation mode/text, initial conditions, Solver
settings, V-Window and exact key sequence. Describe expected versus actual behavior;
a photo or screenshot helps if it contains no private information.

For code changes, preserve the numerical/parser regressions and run `./tools/test.sh`.
UI/target changes also need a strict native compile and G3A package check when the
SDK is available. State which hardware tests you actually performed; host screenshots
are not hardware evidence. Keep changes small and avoid unrelated refactors.

Do not contribute secrets, calculator session files, local paths, copyrighted manual
PDFs/screenshots, toolchains or generated binaries. Original code contributions are
under the repository's MIT License; third-party materials need their own preserved
license/provenance notice.
