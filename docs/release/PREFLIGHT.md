# Initial public beta preflight

## Source and history strategy

The source snapshot comes from the validated local milestone **671186f**. Its
application src/, include/ and tests are preserved byte-for-byte. Only public
packaging/configuration, version metadata, docs and license/ignore policy change.
The intended Git tag is v0.9.0-beta.1; numeric G3A metadata is 00.09.0000.

The original development history is not suitable for a public push: a CASIO manual
PDF was committed early, and personal absolute paths/raw diagnostics and generated
binaries occur in previous commits. An additional manual exists in local tool
capture refs, although it is untracked in the development branch. A current-tree
removal would not remove those objects from public commit ancestry.

A separate clean main repository is prepared from the stable source. It does not
share Git objects/alternates with the original repository. The original local main,
hardware-crash-fix and ui-reference-refactor branches, metadata, manuals and SDK are
preserved; no reset/rebase/filter/force push is used. Publication of this clean-history
alternative requires the owner's confirmation under the requested history-audit stop
rule. Only its main and beta tag are candidates for push.

## Audit scope and findings

- Scanned all 780 objects reachable from original refs (524 distinct blobs), including
  old source/docs and local capture refs. Common private-key, GitHub/API/cloud-token
  and credential-assignment patterns had no findings. This bounded pattern audit
  is not a guarantee that arbitrary secrets can never exist.
- Personal absolute paths were in DEVELOPMENT and historical diagnostic files;
  public documentation is rewritten without them. Email matches were attributed
  upstream license contacts; required credits are retained.
- Largest reachable blob was a 6.43 MB manual in local capture refs. Public snapshots
  omit both manual PDFs, extracted manual images, historical diagnostics and binary
  releases. Original Git history was about 11 MiB of loose objects; no history is deleted.
- Included images are project-generated application frames/icons or attributed gint/
  fxSDK material. No proprietary OS font or manual screenshot is included.
- The owner chose MIT for project-authored materials. Installed linked dependencies,
  borrowed font/key data and example icons have original notices in
  [THIRD_PARTY_NOTICES.md](../../THIRD_PARTY_NOTICES.md).
- Public ignores cover local SDK/venv, CMake/object/ELF/map/stack output, raw logs,
  generated G3A, sessions, editor state and the local-reference folder.
- GitHub authentication is available and the intended account/name was checked for
  a collision. No existing origin is overwritten. This audit does not create a remote.

## Clean-checkout packaging correction

The first source-only build exposed an existing assumption that dist/ already
existed. fxgxa reported an output-open error even though the linker command returned
success; the independent package check correctly failed on the missing artifact.
CMake now creates dist/ during configuration. The release must pass a fresh strict
build, host/UBSan tests and independent package checks after this fix.

## Final publication gates

Confirm the clean public history; audit the final tree/history for secrets, private
paths, excluded PDFs and build junk; verify MIT/notices and relative README links;
commit/tag the candidate; clean-build that exact tagged commit and run all 17 tests;
check the G3A and assemble SHA256SUMS plus redistribution notices. Push only main/tag,
publish a prerelease, then verify public visibility/default branch, license detection,
tag commit, uploaded asset bytes and absence of excluded files.

Device-specific testing is separate in [HARDWARE_RETEST.md](../HARDWARE_RETEST.md);
automated build/tests do not establish all physical LCD, OS/Fugue or key/timer cases.
