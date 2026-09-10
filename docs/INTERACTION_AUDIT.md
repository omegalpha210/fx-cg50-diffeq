> Historical beta.4 audit. Main layout, TRACE navigation/keys and warning placement are superseded by [TILES_TRACE_AUDIT](TILES_TRACE_AUDIT.md). Other INIT/BOX/draft contracts remain.

# Interaction audit — beta.4 baseline

Baseline: development15fb84f / publicf95cf5f,42 passing host/UBSan groups.
Installed tools and both local-only reference PDFs are retained.

| Area | Observed beta.3 source | Planned change |
|---|---|---|
| Main | six indices,4px gap,1px UI_LINE |3px pale-blue divider |
| Settings | Style deliberately blanks/ignores F1 | INIT on every row |
| Graph | TRACE/ZOOM/V-WIN/TABLE/G-SLV/PREV; SYS2 uses VIEW and Phase ANLYS | F6 INIT, EXIT back |
| Status | TIME/PHASE on all modes; enabled EVT | view label only model_phase_supported (SYS2); EVT independent |
| ZOOM | IN/OUT/AUTO/ORIG/blank/blank | F5 BOX, transactional two-point selection |
| G-Solve | algorithms receive ui_cancel; no busy feedback | delayed neutral footer via same cancellation callback |
| TRACE | reversible full horizontal/vertical XOR pointer | shared local9px black/white point cross |
| G-Solve point |11px blue cross, white center | shared9px point primitive |
| Equation | F1 VAR even SELECT; F2 FUNC only EDIT; parse on edit exit | F1 INIT, F2 FUNC/F3 VAR edit-only, validate on NEXT |
| IC | canonical numbers only, parse on every edit exit, no INIT | F1 INIT, bounded owned UI drafts, validate on NEXT |
| Graph entry | no persistent viewport snapshot; dispatcher reenters after subviews | small geometry snapshot across reentry, current view retained |
| Red status | graph_status: END: reason for ODE_HAS_INVALID, Partial: reason for other limits, END: Event neutral | retain wording, location y20, nonfatal behavior |

No new numerical algorithms. Default helper extraction must reproduce existing
Document defaults byte-for-byte. UI drafts must never become compiled numeric
state until validated. BOX borrows the existing inactive TRACE overlay workspace;
no second framebuffer. Graph INIT reuses the existing bounded trajectory cache
when available, with normal safe redraw only if its numerical identity/coverage
is no longer reusable. ORIG keeps factory-window semantics.


## Implemented behavior

- Main uses a3px C_RGB(21,25,30) band at local x10..374,y115..117. All six
  rows remain on one screen, with unchanged indices, MENU help and softkeys.
- Graph Settings exposes yellow/black F1 INIT on every row. Style keeps only
  LEFT/RIGHT toggling; Color retains F3 COLOR. No SEG/ARROW direct buttons.
- TIME/PHASE is shown only for model_phase_supported: SYS with exactly2 states,
  the same condition as F4 VIEW. Other legacy projections do not gain VIEW or a
  mode label. Enabled EVT is independent of equation type.
- G-Solve receives the same cancel policy through ui_busy_cancel. EXIT/MENU is
  polled first; installed rtc_ticks supplies128Hz time, including midnight wrap.
  After20 ticks (156.25ms) a blue CALCULATING... spinner/EXIT hint appears in the
  footer, at most8 updates/sec. There is no percentage, interrupt drawing or new
  timer. Fast operations have no busy frame. Prior selection softkeys are cleared
  when busy becomes visible. Success/not-found/failure/cancel end the indicator
  before the existing graph/result redraw. Host fixtures advance/freeze this same
  clock for deterministic lifecycle tests; device duration is not inferred from them.
- TRACE/G-Solve/BOX share9×9 point geometry: black one-pixel arms, white3×3 center,
  plot clipping. TRACE saves only81 pixels and restores before removing the selected
  curve XOR. Event squares/EQPT diamonds remain distinct;2px curve blink is unchanged.
- Graph base F6 is yellow/black INIT; EXIT returns Parameters. A156-byte static
  graph-entry snapshot survives V-WIN/Table/Settings dispatcher returns. It restores
  the active window's geometry/scales, retaining TIME/PHASE selection, appearance,
  equation, IC, method/h/tolerances, manual solver range, Event and Output. AUTO
  retains its normal window-follow policy. A previously uninitialized Phase view
  records its first successfully fitted window. A new calculation replaces entry.
- INIT uses the existing bounded sample cache when its numerical identity and
  required original solver coverage still match. Initial non-SYS render streams
  now also feed that already allocated cache; their normal pan/redraw and TRACE
  integration policies are unchanged. No second solve is added to initial drawing.
  Replaced/incompatible/incomplete caches fall back to the existing safe renderer.
  A narrow later Event report cannot be paired with an older broad trajectory:
  report coverage is checked too. Thus INIT is not an unconditional no-solve promise.
  Cached repaint is the existing decimated display approximation, not a second
  canonical trajectory. Domain gaps/status remain intact. ORIG remains factory reset.
- ZOOM F5 BOX starts at nearest plot-center pixel(192,99), geometric center191.5,98.5.
  Arrows/repeats move4 pixels, clamped to0..383/0..197, independently of h/Xdot.
  Fresh EXE locks Point1; arrows move Point2; fresh EXE commits only if both spans
  are at least6 pixels and finite ordered coordinate bounds result. Reverse corners
  work. Success returns Graph base; either-stage EXIT returns ZOOM unchanged.
  Invalid size stays in Point2 with BOX TOO SMALL; movement restores normal help.
- BOX paints a pale-blue1-in16 stipple and blue outline using saved touched pixels
  in the inactive TRACE mask plus existing footer backup. At most4512 uint16 color
  samples fit the4752-slot mask region. It does not allocate another framebuffer
  or trajectory. Top status rows0..34 and footer179..197 are protected; the local
  cursor is restored exactly before any following overlay. Cancel is byte-exact.
- Equation F1 INIT restores only expressions/Bernoulli n. IC F1 INIT restores only
  IC defaults. Both stay on screen, first row selected, no warning or automatic
  NEXT. Existing new-document defaults were extracted into two helpers with full
  Document scope tests across all kinds/dimensions. No default values were changed. An independent comparison with the frozen beta.3
  model_defaults body under the same host -O1/UBSan flags matched all63 complete
  kind/dimension Documents byte-for-byte.
- Equation F2 FUNC and F3 VAR are EDIT-only; VAR retains its existing N-th/SYS scope.
  Token palette bars temporarily own their token keys, as before; EXIT restores the
  exact editor. Event's separate VAR position is unchanged.
- Equation/IC edit EXE and EXIT commit draft text without required/syntax checks;
  SELECT NEXT/EXE validates the complete stage and focuses the first bad field.
  Red field-inline warnings leave INIT/NEXT usable. Parent EXIT does not validate.
  Equation text remains uncompiled until validation; Bernoulli n is numeric only
  after a successful parse. IC drafts allocate only edited strings (maximum10×192
  bytes plus allocator overhead), are owned by navigation state, and are freed on
  successful NEXT, IC INIT, mode replacement, successful recall/load or conversion.
  All IC fields validate before canonical numeric IC values change. Drafts survive
  backward/forward navigation; runtime IC drafts are not part of SAVE v10. SAVE/RCL
  backend and readers are unchanged. Allocation failure retains the active draft.
- Existing red status wording is preserved: END: Math domain / singularity,
  END: NaN or infinity, END: Magnitude > 1e100 where applicable; other limited paths
  retain Partial: reason, and Event STOP keeps neutral END: Event. Status stays at
  local x7/y20 under corner legends. Valid-side TRACE/G-Solve continue; no guessed
  continuation or artificial segment is introduced.

## Verification

All prior42 groups remain enabled; new graph_interaction and graph_interaction_ui
cover geometry, exact overlay restoration, reverse corners/minimum spans, scoped
INIT/defaults, cache identity/Event coverage, deferred draft focus and no-work BOX
navigation. Busy tests include fast ROOT, RK4/RK45 second-order ICPT, SYS ICPT,
not-found and cancellation lifecycle. sqrt-domain, exactly sampled pole (h=.25)
and sinh(y) limit cases retain usable valid-side TRACE and Y-ICPT. Existing parser,
RK4/RK45/Event/G-Solve/TRACE/Phase/storage/migration/cancellation tests are preserved.
A pole between sampled points is not newly certified: numerical detection is unchanged.

[28 production frames](ui-review/interaction-overview.png) are reproduced by
capture_interaction.py; busy capture uses the documented deterministic host clock.
They are not device photos or an SH emulator. Current memory/build totals and exact
release-source evidence are in MEMORY_AUDIT, ACCEPTANCE and Release VALIDATION.md.
**HARDWARE RETEST REQUIRED:** all46 priority cases in HARDWARE_RETEST are pending.
