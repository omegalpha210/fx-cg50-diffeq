# v0.12.0-beta.8 preflight

Baseline audit → three approved changes → targeted/existing/full regression →
strict clean SH and package → renderer review/docs → development commit → clean
public snapshot → exact candidate and tag gates → prerelease → asset re-download.

Required gates:59 host/UBSan groups,28 strict SH C units,zero warnings,13 package
checks. Compare source to development, validate README links/GitHub rendering,
scan candidate/public history/binary, preserve original MIT/notices and public
parent1a5a49d. Tags are immutable. Verify downloaded bytes/SHA256/GitHub digests,
remote source tree and all prior tags. R1/UIR1 are closed; physical validation
remains pending. Exact evidence is release VALIDATION.md.
