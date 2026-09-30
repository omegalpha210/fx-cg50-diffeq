#!/usr/bin/env python3
"""Long-list position review from production UI handlers (not hardware)."""
import os,subprocess,tempfile
from pathlib import Path
from PIL import Image,ImageDraw
import sys
sys.dont_write_bytecode = True
from capture_paths import CapturePaths, caption_font

paths = CapturePaths(__file__, "power")
root = paths.root
font = caption_font()
out = paths.output
ten='1 4 F6 DOWN S:MUL '+(' COMMA '.join(' '.join(str(i)) for i in range(1,11)))+' S:DIV EXE F6 F4 '
cases=[('output-first','Output: position1 of10',ten),
       ('output-last','Output: wrap to10 of10',ten+'UP'),
       ('functions-first','Function list: position1 of10','1 4 F6 RIGHT OPTN'),
       ('functions-last','Function list: last item','1 4 F6 RIGHT OPTN UP')]
sheet=Image.new('RGB',(816,536),'#e8eef5');draw=ImageDraw.Draw(sheet)
for i,(name,title,keys) in enumerate(cases):
    with tempfile.TemporaryDirectory(dir=paths.temporary_root) as folder:
        p=subprocess.run([str(paths.app)],cwd=folder,env=dict(os.environ,
            DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_OUT=folder),capture_output=True,text=True,check=True,timeout=15)
        assert 'SCRIPT COMPLETE' in p.stdout and 'runtime error:' not in p.stderr
        assert 'of 10' in p.stdout
        frame=Image.open(sorted(Path(folder).glob('*.ppm'))[-1]).convert('RGB')
        frame.save(out/f'power-{name}.png',optimize=True)
        x=8+i%2*404;y=8+i//2*264;draw.text((x,y),title,fill='#193857',font=font);sheet.paste(frame,(x,y+25))
sheet.save(out/'power-overview.png',optimize=True)
print('4 production list frames; power/LCD behavior still HARDWARE TEST REQUIRED.')

paths.finish()
