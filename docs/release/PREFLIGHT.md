# Phase beta public-source and publication audit

The validated development milestone is `6bc10c9` (implementation `dc14ca7`), preserving all earlier local
history. This clean public update descends from beta.3 `ad02d8c`; no development
branch/history or reference manual is imported. Runtime sources, headers, tests,
icons and tools match the development commit byte-for-byte. The existing public
README helper is retained. Public CMake differences remain release metadata and
dist directory creation; numeric container version is 00.10.0000 and VERSION/tag
identify v0.10.0-beta.1.

Public paths and reachable history blobs are scanned for common credential and
private-path patterns. PDFs, raw private logs, toolchains, environment secrets,
editor state and compiled artifacts stay excluded. This bounded pattern audit
is not a proof that arbitrary unrecognized secrets cannot exist. MIT and all
third-party notices remain unchanged from the original public snapshot.

Only after exact-source host/UBSan, strict full SH compile/link and all package
checks pass is this source tagged and published. Only public main and the new
tag are pushed without force. The beta release includes the built binary,
SHA256SUMS, dependency notices and VALIDATION. Uploaded assets are downloaded
and compared byte-for-byte, and remote refs/source tree are checked afterward.
Existing tags/releases are never moved or overwritten.

Three new Phase screenshots use the project's renderer; no manual or external
image is included. README links/renders are checked alongside exact-source
reproducibility. Final results and evidence boundary are in the release's
VALIDATION.md; hardware tests remain pending in [HARDWARE_RETEST](../HARDWARE_RETEST.md).
