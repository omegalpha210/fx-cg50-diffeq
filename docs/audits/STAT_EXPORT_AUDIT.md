# Table export and STAT integration audit

## Finding

The installed public gint 2.11.0 and fxlibc 1.5.1 headers expose documented storage-memory file operations but no supported API for creating or replacing fx-CG50 Main Memory List variables. A repository/header search found `open`, `write`, `close`, Fugue/BFile and ordinary file interfaces; it found no public List/STAT variable API. DIFF EQ therefore does not patch undocumented RAM or call firmware-dependent private syscalls.

The fx-CG50 manual documents a safe interchange route. Its List Editor can import a CSV stored on the calculator through `F6(g) → F6(g) → F1(CSV) → F1(LOAD) → F2(FILE)` (PDF pages 134–135, printed 3-18–3-19). It accepts comma or semicolon delimiters, ignores the first row when its fields contain a single or double quotation mark, and rejects more than 26 columns or 999 rows. Each List itself accepts up to 999 cells.

## Fatal-path audit and change

The previous Table → CSV path invoked `fprintf()` with floating conversion from inside the RK4 sample callback. On target that stacked solver/RHS/parser frames with fxlibc's floating formatter and stdio stream implementation for every sample. It also performed BFile-backed file I/O directly from the gint world. Installed gint 2.11 explicitly requires these calls to run through `gint_world_switch()`; the old path violated that contract.

The new exporter:

- computes one fixed eight-row `TablePage` at a time;
- lets the ODE/RK4 callback return before formatting or filesystem calls;
- keeps one export `TablePage` and one 2,048-byte page-format buffer in static BSS;
- performs each create or page-append transaction, including `open`, checked short writes and `close`, synchronously in the OS world through `gint_world_switch()`;
- writes only the current Table IC/direction and Output-selected `L` columns;
- prefixes every header field with `'`, so List Editor ignores the label row;
- caps data at 998 rows, keeping the label plus data within the documented 999 CSV-line limit;
- removes a newly created file on cancellation, numerical failure, capacity overflow or I/O failure only after its descriptor is known to have closed successfully.

The filename is the first free `DIFFSTAT00.csv`…`DIFFSTAT99.csv`. In STAT, the imported first numeric column becomes List1, the next becomes List2, and so on; with default Output selection this maps first-order data to x/y, second-order to x/y/y', and an N-state system to x/y1/…/yN.

No descriptor crosses an integration call, key wait or screen transition. The application uses no buffered `FILE` stream in this path and does not attempt unsafe removal after an unconfirmed close.

Host tests verify headers, numeric rows, exclusive filenames, cancellation cleanup and capacity cleanup. The SH target verifies the installed gint call signature at compile/link time. Actual Fugue writes, STAT import, interruption behavior, storage-full behavior and the absence of the prior reboot are **HARDWARE RETEST REQUIRED**.
