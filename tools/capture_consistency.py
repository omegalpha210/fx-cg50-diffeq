#!/usr/bin/env python3
"""UI consistency review frames from production handlers, without manual imagery."""
import os
from pathlib import Path
import subprocess
import tempfile
from PIL import Image,ImageDraw,ImageFont

root=Path(__file__).resolve().parents[1];out=root/'docs/ui-review'
out.mkdir(parents=True,exist_ok=True)
params='1 4 F6 F6 '
event='1 4 A:SUB EXE F6 DOWN 1 EXE F6 F3 DOWN DOWN DOWN DOWN NEG 1 EXE 1 2 EXE F6 F2 F1 RIGHT DOWN A:SUB SUB 1 0 EXE DOWN RIGHT F6 EXIT F6 '
phase='4 2 F6 F6 F6 F6 F4 F2 '
phase_event='4 2 F6 F6 F6 F2 F1 RIGHT DOWN A:SUB 1 EXE F6 EXIT F6 F4 F2 '
cases=[
 ('main','Main / grouped choices',''),
 ('equation-select','Equation SELECT','1 4'),
 ('equation-edit','Equation EDIT / normal EXE','1 4 LEFT'),
 ('ic','Initial Conditions','1 4 F6 DOWN'),
 ('rk4-auto','RK4 / AUTO',params),
 ('rk45-auto','RK45 / h0 / AUTO',params+'DOWN DOWN RIGHT'),
 ('range-man','Range / MAN',params+'NEG 3 EXE'),
 ('output-on','Output / line colors','2 F6 F6 F4'),
 ('output-off','Output / OFF preserves color','2 F6 F6 F4 RIGHT'),
 ('settings','Graph Settings / Style',params+'F5 DOWN DOWN'),
 ('palette','Palette / normal EXE','2 F6 F6 F4 F3'),
 ('time','TIME',params+'F6'),
 ('phase','PHASE',phase),
 ('time-event','TIME EVT / STOP',event),
 ('phase-event','PHASE EVT / analysis',phase_event+'F5 F2 F3'),
 ('trace','TRACE / selected curve','2 F6 F6 F6 F1 RIGHT RIGHT'),
 ('gsolve','G-Solve / curve choice','2 F6 F6 F6 F5 F1'),
 ('gsolve-result','G-Solve / bottom-edge result','1 4 0 EXE F6 F6 F3 DOWN DOWN DOWN DOWN 0 EXE F6 F6 F5 F4'),
 ('save','SAVE confirmation','2 EXIT 6'),
 ('input-error','Inline syntax error','1 4 SIN EXE'),
 ('event-error','Inline Event error','2 F6 F6 F2 F1 DOWN ADD EXE'),
 ('rk45-error','Inline tolerance error',params+'DOWN DOWN RIGHT DOWN DOWN 0 EXE'),
 ('phase-time-ref','Nonautonomous Phase / legend','4 2 F6 DOWN NEG A:SUB 1 ADD XOT EXE F6 F6 F6 F4 F2 F5 F2'),
 ('time-event-trace','TIME EVT / TRACE',event+'F1 F6'),
]
font=ImageFont.load_default();sheet=Image.new('RGB',(816,264*((len(cases)+1)//2)+8),'#e8eef5')
draw=ImageDraw.Draw(sheet)
for i,(name,title,keys) in enumerate(cases):
 with tempfile.TemporaryDirectory() as directory:
  run=subprocess.run([str(root/'build-host/host_app')],cwd=directory,
      env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_OUT=directory),
      capture_output=True,text=True,timeout=30,check=True)
  assert 'SCRIPT COMPLETE' in run.stdout and 'runtime error:' not in run.stderr
  frame=Image.open(sorted(Path(directory).glob('*.ppm'))[-1]).convert('RGB')
  frame.save(out/f'consistency-{name}.png',optimize=True)
 x=8+(i%2)*404;y=8+(i//2)*264
 draw.text((x,y),title,fill='#193857',font=font);sheet.paste(frame,(x,y+25))
sheet.save(out/'consistency-overview.png',optimize=True)
print(f'Saved {len(cases)} current UI frames. HARDWARE TEST REQUIRED.')
