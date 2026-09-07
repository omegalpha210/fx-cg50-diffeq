#!/usr/bin/env python3
"""Reproduce README images with production handlers; no calculator source edits."""
import os
from pathlib import Path
import subprocess
import tempfile
from PIL import Image

root = Path(__file__).resolve().parents[1]
out = root / 'docs/images'
out.mkdir(parents=True, exist_ok=True)
# General first-order y'=1-y^2, x0=0, y0={0,0.5}; window [-3,3] x [-1.5,1.5].
equation = ('1 4 1 SUB A:SUB SQUARE EXE F3 NEG 3 EXE 3 EXE 1 EXE DOWN '
            'NEG 1 DOT 5 EXE 1 DOT 5 EXE 0 DOT 5 EXE F6 ')
ic = equation + 'F6 DOWN S:MUL 0 COMMA 0 DOT 5 S:DIV EXE '
parameters = ic + 'F6 '
graph = parameters + 'F6 '
cases = [
    ('equation-entry', equation, 1),
    ('initial-conditions', ic, 1),
    ('solver-parameters', parameters + 'DOWN DOWN RIGHT', 1),
    ('graph-solution', parameters + 'DOWN DOWN DOWN DOWN DOWN 0 EXE F6', 1),
    ('graph-slope-field', graph, 2),
    ('graph-trace', graph + 'F1 RIGHT RIGHT RIGHT', 1),
    ('graph-gsolve', graph + 'F5 F4 EXE', 1),
    ('table-view', graph + 'F4', 1),
]
for name, keys, scale in cases:
    with tempfile.TemporaryDirectory() as directory:
        env = dict(os.environ, DIFFEQ_HOST_KEYS=keys, DIFFEQ_HOST_OUT=directory)
        run = subprocess.run([str(root/'build-host/host_app')], cwd=directory,
                             env=env, capture_output=True, text=True, timeout=30, check=True)
        assert 'SCRIPT COMPLETE' in run.stdout and 'runtime error:' not in run.stderr
        assert 'Invalid value' not in run.stdout and 'Invalid parameter' not in run.stdout, name
        frame = sorted(Path(directory).glob('*.ppm'))[-1]
        image = Image.open(frame).convert('RGB')
        if scale != 1:
            image = image.resize((image.width*scale, image.height*scale), Image.Resampling.NEAREST)
        image.save(out/(name+'.png'), optimize=True)
icon = Image.open(root/'assets/icon-uns.png').convert('RGB')
icon.resize((184,128), Image.Resampling.NEAREST).save(out/'diffeq-icon.png', optimize=True)
print('Saved 8 app-rendered views and the original project icon; PNG, no metadata, nearest-neighbor enlargement.')
