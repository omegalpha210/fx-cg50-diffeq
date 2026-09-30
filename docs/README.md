# Documentation map

Start with the [user guide](USER_GUIDE.md), [current validation](ACCEPTANCE.md),
[UI conventions](UI_CONVENTIONS.md), and [pending hardware retest](HARDWARE_RETEST.md).
Host rendering and successful compilation do not substitute for physical testing.

| Location | Purpose |
|---|---|
| [audits](audits/) | Current numerical, memory, storage, power and regression evidence |
| [development](development/) | Actual source structure, staged workflow and completed development plan |
| [captures](captures/README.md) | Current README gallery and representative production-renderer sheets |
| [archive](archive/README.md) | Superseded milestone reports and meaningful design/compatibility evidence |
| [licenses](licenses/) | Required dependency redistribution notices |
| `reference/` | Local original manuals; protected, not distributed with the public source |

Current behavior comes from the source, USER_GUIDE and UI_CONVENTIONS. Archived
reports explain earlier decisions and measurements; their old controls or save
versions are not current instructions. Old save migration explanations remain
evidence for supported compatibility, rather than candidates for deletion.

Build and contributor commands are in [DEVELOPMENT](../DEVELOPMENT.md).
Disposable captures belong in `build/captures/`; fresh renderer commands and the
explicit gallery-update option are documented in [capture provenance](captures/README.md).
