"""Production handler/rendering contract frozen before structural cleanup."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

app = str(Path(sys.argv[1]).resolve())
cases = json.loads((Path(__file__).parent / "golden/ui-beta9.json").read_text())
for name, case in cases.items():
    env = dict(os.environ, DIFFEQ_HOST_KEYS=case["keys"],
               DIFFEQ_HOST_MAX_FRAMES="2000", DIFFEQ_HOST_TICK_LIMIT="128")
    with tempfile.TemporaryDirectory(prefix="diffeq-ui-golden-") as work:
        result = subprocess.run([app], cwd=work, env=env, text=True,
                                capture_output=True, check=True, timeout=30)
    assert not result.stderr, (name, result.stderr)
    lines = [line for line in result.stdout.splitlines()
             if line.startswith(("PLOT ", "REPORT ", "FRAME ", "TEXT "))]
    digest = hashlib.sha256(("\n".join(lines) + "\n").encode()).hexdigest()
    assert len(lines) == case["lines"] and digest == case["sha256"], (name, len(lines), digest)
print(f"{len(cases)} workflows match beta.9 pixel, text, plot and report hashes")
