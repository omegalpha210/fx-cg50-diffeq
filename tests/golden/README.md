# Cleanup regression baseline

These fixtures were recorded from the unchanged beta.9 development source
(`2fb4f9f`) before removing functions or separating Graph responsibilities.
They are test/reference source, not disposable captures.

`numerical-beta9.txt` contains every sample in the representative scalar
trajectories, order-9/system-9 states, exact work/step counters, all Event
directions/actions with locations, partial-domain termination, G-Solve results,
multi-IC visibility/colors, and SYS2 vectors/nullclines/equilibria. Hexadecimal
floating values retain the original bits; the comparator allows 1e-11 relative
or 1e-12 absolute error for host libc differences. Counters/status/order are exact.
`numerical_probe.c` calls the production model/solver; it contains no second solver.

`ui-beta9.json` stores 18 key sequences and hashes of production framebuffer,
text, plot and numerical report output. These require exact equality, using the
credited checked-in host font and deterministic timer adapter. Existing semantic
UI tests explain individual controls and remain enabled alongside this contract.

Run both through `./tools/test.sh`. Do not regenerate fixtures just to make a
failure pass: any intentional behavior change needs a separate review.
