#!/usr/bin/env python3
"""Two Event/Info images from production UI handlers and the host framebuffer."""
import os
from pathlib import Path
import subprocess
import tempfile
from PIL import Image

root = Path(__file__).resolve().parents[1]
output = root / 'docs/images'
output.mkdir(parents=True, exist_ok=True)
# y'=y, y(0)=1, RK45, E=y-10 / RISING / STOP.
parameters = '1 4 A:SUB EXE F6 DOWN 1 EXE F6 DOWN DOWN RIGHT '
event = parameters + 'F2 F1 RIGHT DOWN A:SUB SUB 1 0 EXE RIGHT DOWN RIGHT '
cases = [('event-settings', event, 'Event Settings'),
         ('solver-diagnostics', event + 'F6 EXIT F6 EXIT F2 F2 ' + 'DOWN '*7, 'Solver Info')]
for name, keys, title in cases:
    with tempfile.TemporaryDirectory() as directory:
        run = subprocess.run([str(root/'build-host/host_app')], cwd=directory,
            env=dict(os.environ, DIFFEQ_HOST_KEYS=keys, DIFFEQ_HOST_OUT=directory),
            capture_output=True, text=True, timeout=30, check=True)
        assert 'SCRIPT COMPLETE' in run.stdout and 'runtime error:' not in run.stderr
        assert 'Invalid parameter' not in run.stdout and 'Check Event E' not in run.stdout
        final = run.stdout[run.stdout.rfind('\nKEY '):]
        assert title in final and ('RHS evals' in final if name=='solver-diagnostics' else 'STOP' in final)
        image = Image.open(sorted(Path(directory).glob('*.ppm'))[-1]).convert('RGB')
        assert image.size==(396,224)
        image.save(output/(name+'.png'), optimize=True)
        print(f'Saved {name}.png: real host framebuffer, 396x224; HARDWARE TEST REQUIRED.')
