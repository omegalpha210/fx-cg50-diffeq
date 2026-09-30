# Historical design and milestone evidence

These records explain confirmed root causes, earlier UI decisions and storage
migrations. Their supersession notes and dates are part of the evidence. Read
[current conventions](../UI_CONVENTIONS.md), [current validation](../ACCEPTANCE.md)
and the [user guide](../USER_GUIDE.md) for current behavior.

The numerical methods, migration and failure-path records are retained because
they explain existing safeguards and supported older sessions. Cleanup preserves
all runtime numerical algorithms, parser semantics and SAVE v3–v11 reading paths.
Moving a report here does not remove the feature it describes.

| Evidence | Records |
|---|---|
| Confirmed reboot and hardware UI root causes | [CRASH_AUDIT](CRASH_AUDIT.md), [HARDWARE_FIXES](HARDWARE_FIXES.md) |
| Original RK4/domain-policy baseline | [NUMERICAL_METHODS](NUMERICAL_METHODS.md), [NUMERICAL_VALIDITY_AUDIT](NUMERICAL_VALIDITY_AUDIT.md) |
| IC/output policy and legacy-save rationale | [IC_BEHAVIOR_AUDIT](IC_BEHAVIOR_AUDIT.md), [OUTPUT_LIST_AUDIT](OUTPUT_LIST_AUDIT.md), [AUDIT_PERSISTENCE](AUDIT_PERSISTENCE.md), [MULTI_IC_COLOR_AUDIT](MULTI_IC_COLOR_AUDIT.md) |
| Milestone interactions and deferred drafts | [INTERACTION_AUDIT](INTERACTION_AUDIT.md), [TILES_TRACE_AUDIT](TILES_TRACE_AUDIT.md) |
| Overlay, consumer and busy changes | [OVERLAY_AUDIT](OVERLAY_AUDIT.md), [AUDIT_CONSUMERS](AUDIT_CONSUMERS.md), [BUSY_SCREEN_AUDIT](BUSY_SCREEN_AUDIT.md), [AUDIT_UI](AUDIT_UI.md) |
| Frozen visual evidence | [contact sheets](captures/README.md) |

Earlier report references to build logs are historical measurements. New build
logs and temporary screenshots belong under build rather than the source tree.
