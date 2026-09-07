# Public beta release validation

Release candidate: **v0.9.0-beta.1**. Numeric project/G3A versions: 0.9.0 / 00.09.0000.
The application source is unchanged from the validated private development milestone
671186f; packaging/version/docs/license are the public-release changes.

Candidate verification passed: all 17 host/UBSan groups, a full clean SH compile/link
with -Wall -Wextra -Werror, and all 13 independent G3A package checks. The release
process repeats these on the tagged commit before uploading. Target text/data/BSS
is 154,712 / 704 / 62,736 bytes; the G3A is 184,092 bytes. Binary size/SHA256 is provided in the Release and its
SHA256SUMS.txt, because timestamps make hashes specific to each link invocation.

The public source excludes manual PDFs, internal development history, local paths,
raw diagnostics, toolchains and compiled binaries. See [preflight audit](release/PREFLIGHT.md).
Real-device testing was reported by the owner for core functionality; the
[42-case hardware retest checklist](HARDWARE_RETEST.md) remains **HARDWARE TEST REQUIRED**.
