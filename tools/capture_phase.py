#!/usr/bin/env python3
"""Reproduce Phase README images with the real app handlers and host framebuffer."""
import os
from pathlib import Path
import subprocess
import tempfile

from PIL import Image

root = Path(__file__).resolve().parents[1]
output = root / "docs/images"
output.mkdir(parents=True, exist_ok=True)
# Factory SYS2: y1'=y2, y2'=-y1, (y1,y2)(0)=(1,0). The projected
# cached trajectory initializes a separate Phase window on first entry.
equations = "4 2 F6 "
phase = equations + "F6 F6 F6 F4 F2 "
cases = [
    ("phase-system-input", equations),
    ("phase-field", phase),
    ("phase-equilibrium", phase + "F5 F2 F3"),
]
for name, keys in cases:
    with tempfile.TemporaryDirectory() as directory:
        env = dict(os.environ, DIFFEQ_HOST_KEYS=keys, DIFFEQ_HOST_OUT=directory)
        run = subprocess.run(
            [str(root / "build-host/host_app")], cwd=directory, env=env,
            capture_output=True, text=True, timeout=30, check=True)
        assert "SCRIPT COMPLETE" in run.stdout, (name, run.stdout[-1000:])
        assert "runtime error:" not in run.stderr, (name, run.stderr)
        assert "Invalid value" not in run.stdout and "Invalid parameter" not in run.stdout, name
        if name == "phase-equilibrium":
            assert "EQPT 1/1" in run.stdout, run.stdout[-2000:]
            assert "Center / Neutral candidate" in run.stdout, run.stdout[-2000:]
        frames = sorted(Path(directory).glob("*.ppm"))
        assert frames, name
        image = Image.open(frames[-1]).convert("RGB")
        assert image.size == (396, 224), (name, image.size)
        image.save(output / (name + ".png"), optimize=True)
        print(f"Saved {name}.png ({image.width}x{image.height}, {len(frames)} frames)")
print("Three app-rendered Phase views; lossless PNG without metadata. HARDWARE TEST REQUIRED.")
