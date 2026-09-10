#!/usr/bin/env python3
"""BOX and graph interaction frames from production handlers, without manual imagery."""
import os,re
from pathlib import Path
import subprocess
import tempfile
from PIL import Image,ImageDraw,ImageFont

root=Path(__file__).resolve().parents[1];out=root/'docs/ui-review'
out.mkdir(parents=True,exist_ok=True)
plain='1 4 F6 F6 F6 '
sys='4 2 F6 F6 F6 F6 '
phase=sys+'F4 F2 '
event='1 4 A:SUB F6 F6 F2 F1 RIGHT DOWN A:SUB SUB 1 0 EXE DOWN RIGHT F6 EXIT F6 '
zoom=plain+'F2 '
point1=zoom+'F5 '+('LEFT '*20)+('UP '*10)
point2=point1+'EXE '+('RIGHT '*40)+('DOWN '*20)
box=point2+'EXE '
domain='1 4 S:SQUARE 1 SUB XOT RIGHTP F6 F6 F6 '
pole='1 4 1 DIV LEFTP XOT SUB 1 RIGHTP F6 F6 DOWN DOWN DOWN 0 DOT 2 5 EXE F6 '
sinh='1 4 LEFT ACON F2 F2 A:SUB RIGHTP F6 F6 F6 '
cases=[
 ('main','Main / compact graph tiles',''),
 ('equation-select','Equation SELECT / INIT','4 2 F6'),
 ('equation-edit','Equation EDIT / INIT FUNC VAR','4 2 F6 LEFT'),
 ('ic','Initial Conditions / INIT','1 4 F6 DOWN'),
 ('settings','Style / INIT on every row','1 4 F6 F6 F5 DOWN DOWN'),
 ('plain','Ordinary ODE / no TIME',plain),
 ('evt','Ordinary ODE / EVT only',event),
 ('time','SYS2 / TIME',sys),
 ('phase','SYS2 / PHASE',phase),
 ('trace','TRACE / local9px cross','2 F6 F6 F6 F1 RIGHT RIGHT'),
 ('icpt','G-Solve ICPT / shared cross','2 F6 F6 F6 F5 F5'),
 ('busy','Delayed G-Solve / deterministic clock','2 F6 F6 DOWN DOWN RIGHT F6 TICKS:8 F5 F5'),
 ('zoom','ZOOM / F5 BOX',zoom),
 ('point1','BOX / center start',zoom+'F5'),
 ('rectangle','BOX / Point2 and pale stipple',point2),
 ('result','BOX / committed bounds',box),
 ('init','Graph F6 INIT / factory window',box+'F6'),
 ('domain','Domain limit / valid curve retained',domain),
 ('domain-trace','Domain status / TRACE usable',domain+'F1'),
 ('domain-gsolve','Domain status / G-Solve usable',domain+'F5 F4'),
 ('pole','Pole sampled exactly / nonfatal status',pole),
 ('sinh','Magnitude limit / valid prefix',sinh),
 ('blank-equation','Blank Equation draft / no error','1 4 LEFT ACON EXIT'),
 ('error-equation','Equation NEXT / focus and inline error','1 4 LEFT ACON F6'),
 ('blank-ic','Blank IC draft / no error','1 4 F6 LEFT ACON EXIT'),
 ('error-ic','IC NEXT / focus and inline error','1 4 F6 LEFT ACON F6'),
 ('phase-box','PHASE BOX / separate window',phase+'F2 F5 '+('LEFT '*12)+('UP '*8)+'EXE '+('RIGHT '*24)+('DOWN '*16)),
 ('phase-init','PHASE INIT / view retained',phase+'F2 F1 EXIT F6'),
]
font=ImageFont.load_default();sheet=Image.new('RGB',(816,264*((len(cases)+1)//2)+8),'#e8eef5')
draw=ImageDraw.Draw(sheet)
for i,(name,title,keys) in enumerate(cases):
 with tempfile.TemporaryDirectory() as directory:
  run=subprocess.run([str(root/'build-host/host_app')],cwd=directory,
      env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_OUT=directory,DIFFEQ_HOST_MAX_FRAMES='10000',
          **({'DIFFEQ_HOST_TICK_LIMIT':'128'} if name=='busy' else {})),
      capture_output=True,text=True,timeout=30,check=True)
  assert 'SCRIPT COMPLETE' in run.stdout and 'runtime error:' not in run.stderr
  frames=sorted(Path(directory).glob('*.ppm'))
  if name=='busy':
   ids=re.findall(r'CALCULATING[^\n]*\n.*?FRAME (\d+)',run.stdout,re.S)
   assert ids,'Busy callback did not paint a frame'
   frame=Image.open(Path(directory)/f'{int(ids[0]):03d}.ppm').convert('RGB')
  else:frame=Image.open(frames[-1]).convert('RGB')
  frame.save(out/f'interaction-{name}.png',optimize=True)
 x=8+(i%2)*404;y=8+(i//2)*264
 draw.text((x,y),title,fill='#193857',font=font);sheet.paste(frame,(x,y+25))
sheet.save(out/'interaction-overview.png',optimize=True)
print(f'Saved {len(cases)} current UI frames. HARDWARE TEST REQUIRED.')
