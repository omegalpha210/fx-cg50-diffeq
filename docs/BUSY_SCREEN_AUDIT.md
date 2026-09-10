Current beta.8 changes are authoritative in [VISIBILITY_PROMPT_AUDIT](VISIBILITY_PROMPT_AUDIT.md).
R1/UIR1 are resolved by owner decision, implemented and validated. Drawing now
keeps Graph with a bottom bar; Output has per-IC ON/OFF and color; SAVE is v11;
numeric EXIT cancels immediately to G-Solve page2. The beta.7 report below is
historical evidence, including its former review choices and original controls.

---

# Dedicated preparation screens — beta.7

Table and actual Graph construction share a small busy-screen helper. After the
existing20/128-second delay, the current native21-pixel blue header contains
`Preparing Table... /` or `Drawing... /` in white. A normal white row below says
`EXIT cancels`; the rest of the396x224 LCD is white, including the F-key region.
The spinner cycles `/ - \ |`, with at least16/128 seconds between frames. Fast
operations show no screen. Cancellation is checked before the clock or paint.
TRACE and G-Solve retain their separate lower-left CALCULATING indicator.

## LCD ownership, without a new framebuffer

The production graph stream continues to own VRAM. The preparation screen is a
temporary LCD view, transferred in strips using installed gint clipping and the
synchronous rectangular driver. Each strip is saved in the unused tail of the
existing TRACE staging union, painted, uploaded, then immediately restored.
At most396x3 uint16 pixels (2376 bytes) are borrowed. The first visible frame
transfers75 strips to clear the full canvas; subsequent frames transfer only
the21 header rows in7 strips. No whole-screen upload per spinner character.

The owner replaces the screen after commit or rollback. `ui_busy_end` never uploads
a cancelled in-progress Graph framebuffer. Existing lower-panel busy cleanup
remains byte-exact. Native driver tests assert every strip's bounds and size;
host LCD emulation accumulates the same rectangles separately from VRAM so review
captures show the visible screen accurately. That extra LCD buffer is host-only.
Installed `dwindow_set` uses exclusive bounds; r61524_display_rect uses inclusive
bounds and waits for prior DMA before its synchronous CPU transfer.

## Cancellation and stable state

Table index/page work is local. Cancellation returns to Graph without entering a
half-built Table. The existing local TablePage is reused for column changes and
unchanged TOP/BTM/MID requests; these have no solve or spinner. A changed page can
show the same delayed preparation screen.

Drawing stages Event markers and solver diagnostics in existing scratch rather
than replacing the canonical report before completion. Cancel discards the new
capture/report. For a first calculation, the parent Parameters screen is restored.
For pan/zoom/BOX/INIT redraw, prior geometry and the last graph result are restored
and the existing committed display samples are repainted; no second ODE solve is
started by cancellation. Empty-output views restore their backdrop. With no
compatible prior display cache, return to the safe parent rather than committing
a partial framebuffer. Cancelled attempts also preserve Last calculation recall.

Completed nonfatal prefixes are retained as display caches for rollback without
being promoted to complete SYS2 trajectory caches. This retains numerical status,
connected intervals and coordinates. The existing258-point budget can rasterize
slightly differently from the original full-resolution stream: the tested partial
RK45 example differs by two colored pixels. Do not claim a full framebuffer copy
or byte-identical original rendering in every rollback case. Default constant/
harmonic rollback examples do compare byte-for-byte.

This supersedes beta.6's user-facing Partial: Cancelled on Graph construction.
The numerical cancellation return code still exists; the new UI treats Drawing
as a transaction. Non-cancelled numerical limits and valid-domain operations retain
their existing red status and bounded-prefix behavior.

`GRAPH-1` (P1, AUTO-FIX) corrected a pending redraw that restored the old window
after the renderer had committed a noncancel numerical-limit prefix. A completed
prefix now retains its window, report and display cache together; cancellation
still rolls back first. A zero-step plan/phase preflight rejection leaves the
previous report and cache untouched. Its caller restores the pending geometry and
shows a contextual Graph or Phase notice. Initial calculation validation retains
the Parameters inline error and EDIT action.

## Verification

`busy_screen_canvas` verifies header/cancel row/white body/hidden softkeys, all four
frames, delay/cadence, exact VRAM preservation and cancel priority. Native-key
tests check75 initial /7 subsequent strips. `busy_screen_ui` verifies index/page
cancel, same-page reuse, slow success, initial Drawing cancel, ordinary and nonfatal
partial pan rollback, report/window ownership and held/fresh EXIT. Existing
G-Solve/TRACE busy and overlay regression groups remain enabled.

The targeted five groups passed after the transaction fix. `busy_screen_ui`
checks a Second-order RK45 Max steps=10 partial RIGHT pan, the committed
X window -3.78..8.82, and reuse of its report/cache on return from V-Window.
It also checks rejected RK4 factory INIT from X=-1.5..1.5 with Max steps=10,
preserving the prior window/report/cache. `phase_renderer` independently asserts
that dry step-limit and phase-input preflight errors preserve pixels and report.
Physical confirmation is hardware retest case27.

[Current production renderer frames](ui-review/audit-overview.png) include the
dedicated Table/Drawing screens and their integer3x enlargements. RTC/poll fixtures
are host-only. Actual CPU upload duration, contrast, physical repeat, MENU/Fugue
and RTC timing remain HARDWARE TEST REQUIRED.
