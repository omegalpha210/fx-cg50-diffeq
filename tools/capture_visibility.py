#!/usr/bin/env python3
"""Current review frames from production handlers and the host LCD adapter."""
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
for i,choice in enumerate(['RIGHT ','','DOWN RIGHT RIGHT ','RIGHT RIGHT ','DOWN ']):
    colors+=('DOWN ' if i else '')+'F3 LEFT LEFT UP '+choice+'EXE '
colored=five+colors+'F6 F3 NEG 6 EXE 6 EXE 1 EXE DOWN NEG 1 EXE 6 EXE 1 EXE F6 F6 '
mixed=five+colors+'DOWN DOWN RIGHT DOWN DOWN RIGHT '
window='F3 NEG 6 EXE 6 EXE 1 EXE DOWN NEG 1 EXE 6 EXE 1 EXE F6 '
subset=mixed+'F6 '+window+'F6 '
cases=[
 ('normal','Normal Graph: six softkeys',colored,None),
 ('drawing','Drawing: same Graph, one blue bottom bar',colored+'TICKS:8 RIGHT','Drawing... /'),
 ('table','Table keeps its dedicated preparation page',colored+'TICKS:8 F4','Preparing Table... /'),
 ('output-five','Five IC: independent visibility and retained colors',mixed,None),
 ('output-ten-first','Ten IC: seven rows on first page',ten+'F4 DOWN RIGHT',None),
 ('output-ten-last','Ten IC: IC10 OFF and its color retained',ten+'F4 UP RIGHT',None),
 ('subset','Graph: only IC1 / IC3 / IC5',subset,None),
 ('selection','G-Solve selects only visible families',subset+'F5 F1',None),
 ('trace','TRACE skips hidden IC2 and IC4',subset+'F1 DOWN',None),
 ('prompt','Y-CAL: EXE confirms; EXIT discards draft',subset+'F5 F6 F1 EXE 2 DOT 5',None),
 ('return','Numeric EXIT returns to G-Solve page 2',subset+'F5 F6 F1 EXE NEG EXIT',None),
 ('invalid','EXE still validates an empty numeric prompt',subset+'F5 F6 F2 EXE EXE',None),
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
        picture=Image.open(frames[index]).convert('RGB');picture.save(out/f'visibility-{name}.png',optimize=True)
        if name in ['drawing','output-five','output-ten-last','prompt']:
            picture.resize((1188,672),Image.Resampling.NEAREST).save(out/f'visibility-{name}-3x.png',optimize=True)
        x=8+i%2*404;y=8+i//2*264;draw.text((x,y),title,fill='#193857');sheet.paste(picture,(x,y+25))
sheet.save(out/'visibility-overview.png',optimize=True)
print('12 production UI/LCD-adapter frames +4 integer3x previews; HARDWARE TEST REQUIRED.')
