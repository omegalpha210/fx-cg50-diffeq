#!/usr/bin/env python3
"""Native production frames and exact firmware menu geometry, never mockup crops."""
import os
from pathlib import Path
import subprocess
import tempfile
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1]
out = root/'docs/ui-review'
icons = root/'assets/menu'
out.mkdir(parents=True, exist_ok=True)
icons.mkdir(parents=True, exist_ok=True)
graph = ('1 4 1 F6 DOWN S:MUL 0 COMMA 5 S:DIV F6 '
         'NEG 4 EXE 4 EXE F3 NEG 1 EXE 1 EXE DOWN DOWN '
         'NEG 0 DOT 2 5 EXE 0 DOT 2 5 EXE F6 F6 ')
phase = '4 2 F6 F6 F6 F6 F4 F2 F3 NEG 0 DOT 5 EXE 0 DOT 5 EXE F6 '
domain = '1 4 S:SQUARE 1 SUB XOT RIGHTP F6 F6 F6 '
event = '1 4 A:SUB F6 F6 F2 F1 RIGHT DOWN A:SUB SUB 1 0 EXE DOWN RIGHT F6 EXIT F6 '
cases = [
    ('main-first', 'Main: 1st selected / 2 columns', ''),
    ('main-system', 'Main: SYSTEM selected', 'DOWN RIGHT'),
    ('main-recall', 'Main: compact RECALL row', 'DOWN DOWN'),
    ('main-save', 'Main: compact SAVE row', 'DOWN DOWN RIGHT'),
    ('subtype-first', 'Subtype: same tile geometry', '1'),
    ('subtype-others', 'Subtype: Others selected', '1 DOWN RIGHT'),
    ('trace-origin', 'TRACE: entry cursor / INIT + orange NORMAL', graph+'F1'),
    ('trace-upper', 'TRACE: Y follow / fixed X right edge', graph+'F1 F4 F6'),
    ('trace-lower', 'TRACE: Y follow / fixed X left edge', graph+'F1 F3 F5'),
    ('trace-curve', 'TRACE: changed IC / Y-only follow', graph+'F1 F4 F6 DOWN'),
    ('trace-init', 'TRACE INIT: origin + curve / speed retained', graph+'F1 F4 F6 DOWN F1'),
    ('trace-window', 'TRACE: X bounds and scales retained', graph+'F1 F4 F6 DOWN EXIT F3'),
    ('phase-trace', 'PHASE: bounded y1, integration-time cursor', phase+'F1 F4 F6'),
    ('domain', 'Domain warning: top-left opaque backplate', domain),
    ('domain-trace', 'Domain warning: valid-side TRACE', domain+'F1 F6'),
    ('domain-gsolve', 'Domain warning: valid-side G-Solve', domain+'F5 F4'),
    ('event', 'Normal Event STOP: neutral text / independent EVT', event),
    ('phase-legends', 'PHASE warning / legends / frozen field', '4 2 F6 DOWN S:SQUARE 1 SUB XOT RIGHTP F6 F6 F6 F4 F2 F5 F2'),
]
sheet = Image.new('RGB', (816, 264*((len(cases)+1)//2)+8), '#e8eef5')
draw = ImageDraw.Draw(sheet)
for i, (name, title, keys) in enumerate(cases):
    with tempfile.TemporaryDirectory() as folder:
        run = subprocess.run([str(root/'build-host/host_app')], cwd=folder,
            env=dict(os.environ, DIFFEQ_HOST_KEYS=keys, DIFFEQ_HOST_OUT=folder,
                     DIFFEQ_HOST_MAX_FRAMES='10000'), capture_output=True, text=True, timeout=30, check=True)
        assert 'SCRIPT COMPLETE' in run.stdout and 'runtime error:' not in run.stderr
        frame = Image.open(sorted(Path(folder).glob('*.ppm'))[-1]).convert('RGB')
        frame.save(out/f'tiles-{name}.png', optimize=True)
        if i < 6:
            frame.resize((1188,672), Image.Resampling.NEAREST).save(out/f'tiles-{name}-3x.png', optimize=True)
        x=8+(i%2)*404; y=8+(i//2)*264
        draw.text((x,y), title, fill='#193857');sheet.paste(frame,(x,y+25))
sheet.save(out/'tiles-overview.png', optimize=True)
names = ['first','second','higher','system','separable','linear','bernoulli','others']
with tempfile.TemporaryDirectory() as folder:
    run = subprocess.run([str(root/'build-host/test_menu')], cwd=folder,
        env=dict(os.environ, DIFFEQ_HOST_OUT=folder), capture_output=True, text=True, timeout=30, check=True)
    frames = sorted(Path(folder).glob('*.ppm'))
    assert len(frames)==8 and 'runtime error:' not in run.stderr
    for name, path in zip(names, frames):
        Image.open(path).crop((6,4,114,38)).save(icons/f'{name}.png', optimize=True)
print('18 app-rendered views, six additional 3x menu previews, and eight exact 108x34 firmware icon PNGs. HARDWARE TEST REQUIRED.')
