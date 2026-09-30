# Source snapshot and release preflight

1. Preserve local edits, protected manuals/dist, the installed SDK, public main
   ancestry and every existing tag. Inventory before mutation.
2. Validate source changes: all 66 host/UBSan groups, including frozen numerical
   and 18 UI contracts; strict clean 29-C-unit SH compile/link, zero warnings,
   13 independent G3A checks. Retain stack/section measurements.
3. Prepare an explicit public-safe snapshot with `tools/public_snapshot.py`.
   Review exact additions/removals; scan source, reachable public-history blobs
   and binary for private paths/credentials/artifacts. Keep MIT and dependency
   notice bytes. Check relative links and current renderer provenance.
4. Run the same gates from that exact clean public candidate, then from the exact
   immutable annotated tag. Use the next available beta if the binary changes.
   No force-push, tag movement or development-history import.
5. Assemble DIFFEQ.g3a, SHA256SUMS.txt, VALIDATION.md and complete dependency
   notices. Publish main/tag normally and create a beta prerelease. Re-download
   every asset, compare bytes/SHA256/GitHub digest, remote tree and old tag refs.

The independent `verify_g3a.py` is retained separately from fxgxa. Relinking may
change embedded timestamp and hash. Optional ASan is unverified on the audited
host runtime. Native hardware results remain HARDWARE TEST REQUIRED until actually
performed. Stop publication for unresolved required gate, auth, history, security
or license failures.
