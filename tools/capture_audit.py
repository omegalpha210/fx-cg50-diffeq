#!/usr/bin/env python3
"""Beta.7 review frames from production handlers and the host LCD adapter."""
import os,subprocess,tempfile
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'docs/ui-review';out.mkdir(exist_ok=True)
def values(count):return ' COMMA '.join(' '.join(str(i)) for i in range(1,count+1))
def params(count):
    initial='1' if count==1 else 'S:MUL '+values(count)+' S:DIV'
    return '1 4 0 EXE F6 DOWN '+initial+' EXE F6 '
five=params(5);ten=params(10)
colors='F4 '
for choice in ['RIGHT ','','DOWN RIGHT RIGHT ','RIGHT RIGHT ','DOWN ']:
    colors+='DOWN F3 LEFT LEFT UP '+choice+'EXE '
colored=five+colors+'F6 F3 NEG 6 EXE 6 EXE 1 EXE DOWN NEG 1 EXE 6 EXE 1 EXE F6 F6 '
cases=[
 ('table-busy','Table: blue header, cancel row, hidden softkeys','2 F6 F6 F6 TICKS:8 F4','Preparing Table... /'),
 ('drawing-busy','Drawing: same dedicated preparation canvas','2 F6 F6 TICKS:8 F6','Drawing... /'),
 ('sf50','SF50 accepted; default remains12','1 4 F6 F6 '+'DOWN '*5+'5 0 EXE',None),
 ('sf51-error','SF51: inline range error, field retained','1 4 F6 F6 '+'DOWN '*5+'5 1 EXE',None),
 ('output-single','Single IC: existing y row',params(1)+'F4',None),
 ('output-five','Five independently editable curve colors',five+'F4 DOWN',None),
 ('output-five-colors','Each selected IC owns its color',five+colors,None),
 ('output-ten-first','Ten IC: shared visibility and first six colors',ten+'F4',None),
 ('output-ten-last','Ten IC: last four colors remain accessible',ten+'F4 UP',None),
 ('colors-graph','Five actual trajectories with independent colors',colored,None),
 ('colors-gsolve','G-Solve uses those same curve colors',colored+'F5 F1',None),
 ('colors-trace','TRACE retains the selected family color',colored+'F1 DOWN DOWN',None),
 ('sys9-output','SYS9: last output remains in bounds','4 9 F6 F6 F6 F4 UP',None),
 ('nth9-edit','N-th9: last IC and scientific notation edit','3 9 F6 F6 UP 1 DOT 2 3 4 5 6 7 8 9 EXP 9 9',None),
 ('rk45-table','RK45 Table reports actual display dx','2 F6 F6 DOWN DOWN DOWN DOWN 7 EXIT UP UP RIGHT F6 F4',None),
 ('drawing-cancel','Cancelled initial Drawing returns Parameters','2 F6 F6 TICKS:8 CANCEL:12 F6',None),
]
sheet=Image.new('RGB',(816,264*((len(cases)+1)//2)+8),'#e8eef5');draw=ImageDraw.Draw(sheet)
for i,(name,title,keys,match) in enumerate(cases):
    with tempfile.TemporaryDirectory() as folder:
        p=subprocess.run([str(root/'build-host/host_app')],cwd=folder,env=dict(os.environ,
            DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_OUT=folder,DIFFEQ_HOST_MAX_FRAMES='2000',DIFFEQ_HOST_TICK_LIMIT='128'),
            capture_output=True,text=True,check=True,timeout=30)
        assert 'SCRIPT COMPLETE' in p.stdout and 'runtime error:' not in p.stderr
        frames=sorted(Path(folder).glob('*.ppm'));index=-1;found=False
        if match:
            for line in p.stdout.splitlines():
                if line.startswith('TEXT ') and match in line:found=True
                if found and line.startswith('FRAME '):index=int(line.split()[1]);break
            assert index>=0,(name,match)
        picture=Image.open(frames[index]).convert('RGB');picture.save(out/f'audit-{name}.png',optimize=True)
        if name in ['table-busy','drawing-busy','output-five-colors','output-ten-last']:
            picture.resize((1188,672),Image.Resampling.NEAREST).save(out/f'audit-{name}-3x.png',optimize=True)
        x=8+i%2*404;y=8+i//2*264;draw.text((x,y),title,fill='#193857');sheet.paste(picture,(x,y+25))
sheet.save(out/'audit-overview.png',optimize=True)
print('16 production UI/LCD-adapter frames +4 integer3x previews; HARDWARE TEST REQUIRED.')
