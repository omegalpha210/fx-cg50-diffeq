# DIFFEQ v0.9.0-beta.1

Initial public beta of the unofficial native DIFFEQ add-in for **CASIO fx-CG50**.
Not affiliated with or endorsed by CASIO. Other calculator models are untested.

## Included

- First/second/N-th order ODEs and systems (1–9 states), bidirectional fixed-step RK4.
- Multiple ICs, first-order slope fields, V-Window, color output, pan/zoom and phase portraits.
- TRACE and G-Solve ROOT/MAX/MIN/Y-ICPT/X-CAL/Y-CAL/ICPT.
- Paged Table, STAT-compatible CSV, explicit SAVE/RCL and two-slot session recovery.
- Unified field completion, inline FUNC/VAR, persistent ZOOM and retained valid regions.

## Install

Download DIFFEQ.g3a, connect the fx-CG50 by USB and copy the file to the calculator
drive root. Safely disconnect and launch DIFF EQ from Main Menu. SHA256SUMS.txt
identifies the exact binary. THIRD_PARTY_NOTICES.txt accompanies its library code.

## Status and limitations

Core functionality has been tested on real fx-CG50 hardware by the project owner;
additional hardware testing and UI refinement are ongoing. Host/UBSan and strict
SH/package checks verify the release build but do not establish every device case.

Fixed-step RK4 has no adaptive error estimate or stiffness handling. The 1e100
magnitude guard remains; computed valid regions remain usable, but failed IVPs
are not restarted across unknown gaps. G-Solve may miss unsampled roots and is
limited to 32 results. TRACE can decimate retained points. STAT exports CSV for
manual OS import; session files are target-ABI-specific. Back up saved data when
trying beta upgrades. See README and the hardware retest checklist for details.

Please report your calculator OS version, equation/IC/settings, exact key sequence,
and expected/actual behavior using the issue template. Photos are welcome.

Project code: MIT. Linked and copied dependencies retain their individual notices.
