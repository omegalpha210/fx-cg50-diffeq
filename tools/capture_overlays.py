#!/usr/bin/env python3
"""Production beta.6 frames; deterministic host RTC/poll fixtures for busy states."""
import os,re,subprocess,tempfile
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'docs/ui-review';out.mkdir(exist_ok=True)
base='2 F6 F6 F6 '
small='2 F6 F6 F3 NEG 6 EXE 6 EXE 1 EXE DOWN NEG 0 DOT 0 1 EXE 1 EXE 0 DOT 2 EXE F6 F6 '
multi='3 3 F6 F6 F6 F6 '
domain='1 4 S:SQUARE 1 SUB XOT RIGHTP F6 DOWN S:MUL 0 COMMA 1 S:DIV EXE F6 F6 '
cases=[
 ('min-select','MIN: common top-left instruction',base+'F5 F3',None),
 ('max-select','MAX: same instruction and blue EXE',base+'F5 F2',None),
 ('root-select','ROOT: same selection channel',base+'F5 F1',None),
 ('icpt-first','ICPT: first graph selection',multi+'F5 F5',None),
 ('icpt-second','ICPT: second graph selection',multi+'F5 F5 EXE',None),
 ('root-result','ROOT: marker above the fixed data panel',small+'F5 F1 EXE',None),
 ('min-result','MIN: minimal Y-only visibility adjustment',small+'F5 F3 EXE',None),
 ('max-result','MAX: marker visible near upper boundary',small+'F5 F2 EXE',None),
 ('icpt-result','ICPT: existing result, Y-only display',small+'F5 F5 RIGHT',None),
 ('domain','Domain: red warning, small white backing',domain,None),
 ('domain-select','Active instruction temporarily owns channel',domain+'F5 F3',None),
 ('domain-exit','EXIT: warning restored, G-Solve submenu',domain+'F5 F3 EXIT',None),
 ('trace-busy','TRACE preparation in lower data panel',base+'TICKS:8 F1','CALCULATING... /'),
 ('trace-ready','TRACE: X-fixed / Y-follow, cursor INIT',base+'TICKS:8 F1',None),
 ('table-busy','Table: dedicated preparation canvas',base+'TICKS:8 F4','Preparing Table... /'),
 ('drawing-busy','Drawing: retained Graph and bottom busy bar','2 F6 F6 TICKS:8 F6','Drawing... /'),
 ('factory-init','Graph INIT: factory geometry, same as V-WIN',small+'RIGHT UP F2 F1 EXIT F6',None),
 ('factory-window','Graph INIT factory values',small+'RIGHT UP F6 F3',None),
 ('scratch-cancel','G-Solve scratch cancel: original graph retained','2 F6 F6 DOWN DOWN RIGHT F6 TICKS:8 CANCEL:12 F5 F5',None),
 ('box','BOX instruction shares the top-left channel',base+'F2 F5 EXE RIGHT DOWN',None),
]
sheet=Image.new('RGB',(816,264*((len(cases)+1)//2)+8),'#e8eef5');draw=ImageDraw.Draw(sheet)
for i,(name,title,keys,match) in enumerate(cases):
 with tempfile.TemporaryDirectory() as folder:
  p=subprocess.run([str(root/'build-host/host_app')],cwd=folder,
   env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_OUT=folder,DIFFEQ_HOST_MAX_FRAMES='2000',DIFFEQ_HOST_TICK_LIMIT='128'),
   capture_output=True,text=True,check=True,timeout=30)
  assert 'SCRIPT COMPLETE' in p.stdout and 'runtime error:' not in p.stderr
  frames=sorted(Path(folder).glob('*.ppm'))
  index=-1
  if match:
   for segment in p.stdout.split('FRAME '):
    if match in segment:
     # TEXT precedes the following FRAME delimiter, whose index is next.
     index=int(segment.split('\n',1)[0].split()[0])+1 if segment[0].isdigit() else 0
     break
   assert index>=0,(name,match)
  image=Image.open(frames[index]).convert('RGB');image.save(out/f'overlay-{name}.png',optimize=True)
  if name in ['min-select','root-result','trace-busy','table-busy','drawing-busy']:
   image.resize((1188,672),Image.Resampling.NEAREST).save(out/f'overlay-{name}-3x.png',optimize=True)
  x=8+i%2*404;y=8+i//2*264;draw.text((x,y),title,fill='#193857');sheet.paste(image,(x,y+25))
sheet.save(out/'overlay-overview.png',optimize=True)
print('20 production renderer views +5 integer enlargements. RTC/poll fixtures are host-only; HARDWARE TEST REQUIRED.')
