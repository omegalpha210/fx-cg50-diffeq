"""Compare production numerical output to the pre-cleanup beta.9 baseline."""
import math
from pathlib import Path
import subprocess
import sys

expected = (Path(__file__).parent / "golden/numerical-beta9.txt").read_text().splitlines()
actual = subprocess.check_output([sys.argv[1]], text=True).splitlines()
assert len(actual) == len(expected), (len(actual), len(expected))
floats = 0
for line, (before, after) in enumerate(zip(expected, actual), 1):
    left, right = before.split(), after.split()
    assert len(left) == len(right), (line, before, after)
    for old, new in zip(left, right):
        if "0x" in old and "p" in old:
            # libc may differ in the last few bits across host architectures.
            # Integer counters, record order and statuses must match exactly.
            a, b = float.fromhex(old), float.fromhex(new)
            assert math.isfinite(b) and math.isclose(a, b, rel_tol=1e-11, abs_tol=1e-12), (line, old, new)
            floats += 1
        else:
            assert old == new, (line, old, new)
print(f"{len(actual)} numerical records, {floats} values match beta.9")
