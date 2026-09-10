# DIFFEQ v0.12.0-beta.5 — Native tiles and fixed TRACE

- Main is 2×3: four graph tiles plus a short RECALL/SAVE row. First-order subtype
  is 2×2 using the identical 184×58 tile helper. Native frame, 2D wrap, digits,
  EXE/F6 OPEN, safe SAVE confirmation and partial focus repaint remain.
- Eight original 108×34 motifs use 832 B const geometry plus an 8 B panel palette.
  Individual PNG exports total 3378 B; those review PNGs are not embedded images.
- TRACE fixes entry horizontal bounds/scale/Xdot. Navigation stays in the current
  connected visible valid component, with no X pan or cache extension. Y-only
  translation retains span, scale and solver settings; invalid translations stop.
- F1 INIT replaces x= and restores entry cursor/curve while keeping speed.
  NORMAL is orange, FAST green, FASTER cyan, with 1/2/3× Xdot strides. F5/F6 solver
  endpoints clamp to reachable visible boundaries, including Event/domain limits.
- PHASE fixes horizontal state bounds and follows vertical state, retaining
  integration-time order. Offscreen retained states stop traversal; a narrow slab
  with no retained state can make TRACE unavailable. No fabricated edge projection.
- All Graph numerical warnings share a top-left anchor with red normal text and a
  small opaque text-sized white backplate; Event STOP remains neutral. Existing
  valid-domain TRACE/G-Solve remain available, including active overlays.
- Ordinary Graph/ZOOM/G-Solve-menu pan and safe expansion are preserved. Graph INIT,
  ORIG/BOX, draft validation, Event/Phase/Table/SAVE/RCL and numerical algorithms remain.

47 host/UBSan groups (all previous 44 retained), clean 28-unit SH compile/link with
zero warnings and 13 package checks are required from candidate and exact tag.
Text 214208 (+1660), BSS 72352 (+48), data 752 (unchanged), max application frame
2664 (+16), G3A 243640 (+1660) B versus beta.4. No second framebuffer, trajectory
or menu heap cache. The 36 B TRACE viewport metadata is static; anchor is small stack state.
Largest frame is not the total device stack high-water.

SAVE still writes v10 and reads same-device v3–v9. Back up sessions before upgrading;
older add-ins can reject newer saves. The numerical/parser/storage implementation
is unchanged from beta.4. Public history, MIT and dependency notices are preserved.

[Complete audit](../TILES_TRACE_AUDIT.md), [user guide](../USER_GUIDE.md),
[18 current renderer frames](../ui-review/tiles-overview.png),
[original icons](../../assets/menu/README.md).
**HARDWARE RETEST REQUIRED:** all 16 new priority cases in
[HARDWARE_RETEST](../HARDWARE_RETEST.md) remain pending. These frames are production
host-renderer output, not calculator photographs or an SH emulator.
