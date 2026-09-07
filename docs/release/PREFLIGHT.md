# Public beta.3 source and publication audit

## Source and history

The source baseline is development commit
`0fbe0de864204754659c3a3f2e5478efb4832149`, whose strict SH build, package checks
and 29 host/UBSan groups passed. The public update descends directly from the
existing `main`/`v0.9.0-beta.2` snapshot `6405296`. Development history is not merged,
rewritten or pushed; public history remains a sequence of safe snapshots.

94 existing source/header/test/tool/icon files are copied byte-for-byte from that
commit. The extra tool is host-only `tools/capture_readme.py`. CMake differences
from development are the existing public numeric version (`0.9.0`, `00.09.0000`)
and creation of dist/ for source-only builds. VERSION/tag identify `0.9.0-beta.3`;
the numeric container format cannot express a beta suffix. No numerical or UI
feature is added solely for this publication.

## Public-file and asset audit

- Public tree and existing public-history blobs were scanned for common private
  keys, GitHub/API/cloud tokens, credential assignments and personal absolute
  paths. No findings. This bounded pattern scan does not prove absence of every
  possible arbitrary secret.
- No PDF, manual extract, raw private log, local SDK, environment credential file,
  editor state, object, ELF, map, stack file or release binary is tracked.
  `.gitignore` retains those exclusions. Source/test data and dependency notices
  remain; the calculator does not need local reference files.
- README visuals are eight regenerated DIFFEQ app frames and the project icon.
  All were reviewed, optimized losslessly and recorded in docs/images/README.md.
  No hardware photograph, CASIO screenshot, internet image or mockup was used.
- MIT LICENSE, THIRD_PARTY_NOTICES.md and every docs/licenses/ notice are unchanged
  from beta.1. Linked library notices accompany the release binary.
- The verified repository is `omegalpha210/fx-cg50-diffeq`; authentication, owner,
  main ancestry and existing beta.1/beta.2 tags/releases match the prior public snapshot.
  Existing description/topics are already appropriate and are retained.

## Release gates

A beta.3 prerelease is published only after committing and tagging this public
candidate, building from that exact clean tag, running all 29 host/UBSan groups,
performing a full strict SH compile/link, and passing all 13 G3A checks. The release
assets include the binary, SHA256SUMS, dependency notices and a validation record.
Raw logs stay local. Only main and the new tag are pushed, without force.

After publishing, verify remote main/tag ancestry, prerelease status, both README
renders and assets, and download the uploaded G3A/checksum for byte verification.
See [ACCEPTANCE.md](../ACCEPTANCE.md) and the release's VALIDATION.md for the evidence
boundary. Hardware cases remain pending in [HARDWARE_RETEST.md](../HARDWARE_RETEST.md).
