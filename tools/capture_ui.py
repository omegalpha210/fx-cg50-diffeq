#!/usr/bin/env python3
"""Render the actual UI sources through the host adapter; not an emulator."""
import os
from pathlib import Path
import subprocess
import tempfile
from PIL import Image, ImageDraw, ImageFont

root=Path(__file__).resolve().parents[1]
executable=root/'build-host/host_app'
output=root/'docs/ui-review'
output.mkdir(parents=True,exist_ok=True)
cases=[('main','Main',''),
    ('equation','Equation / NEXT','F1 4 NEG A:SUB EXE'),
    ('parameter','Parameters / GRAPH','F1 4 F6 F6 DOWN DOWN 0 DOT 0 5 EXE'),
    ('vwindow','V-Window','F1 4 F3 NEG 7 EXE'),
    ('initial-conditions','Vertical ICs / ADD-DROP','F1 4 F6 F4 DOWN'),
    ('output','Output matrix','F2 F6 F6 F4 DOWN'),
    ('graph','Full-size Graph / 2 IC colors','F1 4 F6 F4 DOWN 2 EXE F6 F6'),
    ('gsolve','G-Solve ROOT result','F2 F6 F6 F6 F5 F1 EXE'),
    ('table','Numerical Table','F2 F6 F6 F6 F4'),
    ('zoom','Inline Zoom menu','F2 F6 F6 F6 F2'),
    ('system9','Nine variables / formula above fields','F4 9 F6 '+ 'DOWN '*8),
    ('ic9','Nine-state IC scroll','F4 9 F6 F6 '+'DOWN '*9),
    ('color-chooser','Output / six-color chooser','F2 F6 F6 F4 DOWN F3'),
    ('trace','TRACE / retained samples','F2 F6 F6 F6 F1 RIGHT'),
    ('func','FUNC / softkeys only','F1 4 LEFT F2'),
    ('var','VAR / second page','F4 9 F6 LEFT F1 F6'),
    ('scalar-prompt','Y-CAL / scalar RUN','F2 F6 F6 F6 F5 F6 F1 EXE 2 DOT 5'),
    ('graph-settings','Graph settings / INIT','F2 F6 F6 F5 1')]
sheet=Image.new('RGB',(816,2384),'#e8eef5')
draw=ImageDraw.Draw(sheet)
font=ImageFont.load_default()
for i,(name,label,keys) in enumerate(cases):
    with tempfile.TemporaryDirectory(dir=root/'build-host') as directory:
        env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_OUT=directory)
        subprocess.run([str(executable)],cwd=directory,env=env,check=True,
                       stdout=subprocess.DEVNULL,timeout=30)
        frame=sorted(Path(directory).glob('*.ppm'))[-1]
        im=Image.open(frame).convert('RGB')
        im.save(output/f'{name}.png')
    x=8+(i%2)*404;y=8+(i//2)*264
    draw.text((x,y),label,fill='#193857',font=font)
    sheet.paste(im,(x,y+25))
sheet.save(output/'host-overview.png')
print('Saved eighteen host framebuffer views to docs/ui-review/. HARDWARE RETEST REQUIRED.')
