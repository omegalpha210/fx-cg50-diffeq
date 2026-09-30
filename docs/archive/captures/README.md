# Frozen historical contact sheets

These eight compact sheets preserve the existing review evidence. They are
production-host renderings, not calculator screenshots. Their associated reports
specify the milestone, the later superseding contracts and any limitations.

| Sheet | Evidence/scenarios |
|---|---|
| [initial-ui-overview](initial-ui-overview.png) | Initial six-view gallery; the former individual copies had identical cells here |
| [ux-field-overview](ux-field-overview.png) | Earlier editor/field/IC/Table review |
| [polish-overview](polish-overview.png) | Context help and TRACE/IC refinements |
| [trace-ic-overview](trace-ic-overview.png) | TRACE Y follow, speed, ten IC and limit messages |
| [consistency-overview](consistency-overview.png) | Semantic colors and labels |
| [interaction-overview](interaction-overview.png) | Draft validation, BOX transaction and domain behavior |
| [overlay-overview](overlay-overview.png) | G-Solve result/selection and busy/cancel overlays |
| [audit-overview](audit-overview.png) | SF50, colors, busy and Table review |

Intermediate individual/3x PNG copies were removed only after comparing the
396×224 RGB pixels with their retained contact-sheet cells. Exact historical
files also remain in development Git history. None is a test golden input.

`capture_ui`, `capture_consistency`, `capture_interaction`, `capture_overlays`
and `capture_audit` still reproduce their scenarios against the current source
under `build/captures/`. Running them does not overwrite these frozen sheets.
Exact old UI pixels require the corresponding historical source, rather than
claiming that a newer renderer recreates a previous release.
