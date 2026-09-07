"""Contextual help, semantic PREV/swatch pixels and capability-aware SF rows."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
EDIT='EXE: commit / next   EXIT: commit'
BASE=['TRACE','ZOOM','V-WIN','TABLE','G-SLV','PREV']
def run(keys,directory=None,image=False):
    if directory is None:
        with tempfile.TemporaryDirectory() as directory:return run(keys,directory,image)
    env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_MAX_FRAMES='10000')
    if image:env['DIFFEQ_HOST_OUT']=directory
    p=subprocess.run([app],cwd=directory,env=env,capture_output=True,text=True,timeout=30)
    assert p.returncode==0 and 'runtime error:' not in p.stderr,(p.stderr,p.stdout[-2500:])
    assert 'SCRIPT COMPLETE' in p.stdout
    if image:return p.stdout,sorted(Path(directory).glob('*.ppm'))[-1].read_bytes().split(b'\n',3)[3]
    return p.stdout
def tail(out):return next(p for p in reversed(out.split('\nKEY ')) if 'TEXT ' in p)
def plot(out):return re.findall(r'^PLOT (\w+)',out,re.M)[-1]
def bar(out):
    return re.findall(r'TEXT \d+ 206 ([^\n]*)',tail(out))[-6:]
def pixel(rgb,x,y):return rgb[(y*396+x)*3:(y*396+x)*3+3]
def rgb565(c):return bytes([(c>>11&31)*255//31,(c>>5&63)*255//63,(c&31)*255//31])

# Hint ownership follows logical edit state, including simulated blink events.
forms=['1 1','1 2','1 3 DOWN DOWN','1 4','2','3 9 F6','4 9 F6',
       '1 1 F6','1 1 F6 DOWN','2 F6','4 9 F6 F6 '+('DOWN '*9),
       '1 4 F6 F6','2 F6 F6','2 F3','3','4']
for prefix in forms:
    base=run(prefix)
    assert EDIT not in tail(base) and 'up/down: select' not in tail(base).lower()
    for entered in ['LEFT','RIGHT','2']:
        for blink in ['', ' BLINK', ' BLINK BLINK']:
            out=run(prefix+' '+entered+blink)
            assert EDIT in tail(out),(prefix,entered,tail(out))
    assert plot(run(prefix+' LEFT EXIT'))==plot(base),prefix
    assert EDIT not in tail(run(prefix+' LEFT EXE'))
    # Error popup has its own content; dismissing restores the edit hint/draft.
    out=run(prefix+' LEFT ACON SIN EXE')
    assert EDIT not in tail(out)
    assert EDIT in tail(run(prefix+' LEFT ACON SIN EXE EXE'))
assert 'Comma: separator' in tail(run('1 1 F6'))
assert all(x not in tail(run('1 1 F6')) for x in ['SHIFT','braces','UP/DOWN'])
assert 'One solution: x0 plus all state values' in tail(run('2 F6'))
assert 'UP/DOWN: page' in tail(run('2 F6 F6 F6 F4'))
assert 'UP/DOWN  EXE' in tail(run('2 F6 F6 F6 F5 F1'))
# A modal numeric function picker owns its help, then returns to logical EDIT.
assert EDIT not in tail(run('2 F3 LEFT OPTN'))
assert EDIT in tail(run('2 F3 LEFT OPTN EXIT'))
assert 'LEFT/RIGHT: ON/OFF toggle' in tail(run('2 F6 F6 F4'))
for prefix in ['4 9 F6 F6','4 9 F6 F6 F6 F4']:
    out=tail(run(prefix))
    assert 'TEXT 20 167 ' in out and not re.findall(r'TEXT \d+ 174 ',out)

graph='2 F6 F6 F6 '
out,rgb=run(graph,image=True);assert bar(out)==BASE
_,prev=run('2 F6',image=True)
assert pixel(rgb,327,203)==pixel(prev,7,203)==rgb565(0xf81f)
assert 'TEXT 14 9 Parameter\n' in tail(run(graph+'F6'))
assert bar(run(graph+'F1'))==['x=','NORMAL','FAST','FASTER','LEFT','RIGHT']
assert bar(run(graph+'F2'))==['IN','OUT','AUTO','ORIG','','']
assert bar(run(graph+'F5'))==['ROOT','MAX','MIN','Y-ICPT','ICPT','>']
for menu in ['F1','F2','F5']:
    out,rgb=run(graph+menu+' EXIT',image=True)
    assert bar(out)==BASE and pixel(rgb,327,203)==rgb565(0xf81f)

# Both color rows use the same framed box geometry, with their own palettes.
settings='1 4 F6 F6 F5 DOWN DOWN DOWN '
for color,moves in zip([0x8d5b,0xdc51,0x65d8,0xcc59,0xc549,0x9492],
                      ['', 'RIGHT','RIGHT RIGHT','DOWN','DOWN RIGHT','DOWN RIGHT RIGHT']):
    selected=settings+'F3 '+moves+' EXE '
    out,rgb=run(selected,image=True)
    assert pixel(rgb,334,148)==rgb565(color)
    assert pixel(rgb,332,146)==rgb565(0x198a) # shared UI_INK frame
    assert plot(run(selected+'F3 DOWN EXIT'))==plot(out)
    _,reset=run(selected+'F4',image=True);assert pixel(reset,334,148)==rgb565(0x8d5b)
with tempfile.TemporaryDirectory() as directory:
    run(settings+'F3 DOWN RIGHT EXE '+('EXIT '*5)+'F6 EXE',directory)
    _,rgb=run('F5 2 F1 F6 F6 F5',directory,image=True)
    assert pixel(rgb,334,148)==rgb565(0xc549)

# Actual renderer capability, including unsupported N-th1/SYS1, drives rows.
first=['1 1','1 2','1 3','1 4']
higher=['2','3 1 F6','3 2 F6','3 9 F6','4 1 F6','4 9 F6']
for entry in first+higher:
    params=entry+' F6 F6 '
    supported=entry in first
    count=7 if supported else 6
    labels=re.findall(r'TEXT 20 \d+ ([^\n]+)',tail(run(params)))
    assert labels==['Xrange min','Xrange max','Method','h','Step']+(['SF'] if supported else [])+['Max steps']
    assert plot(run(params+'DOWN '*20))==plot(run(params+'DOWN '*(count-1)))
    assert plot(run(params+'DOWN '*20+'UP '*20))==plot(run(params))
    for row in range(count):
        current=params+'DOWN '*row
        assert plot(run(current+'F5 EXIT'))==plot(run(current))
        if not supported:assert 'Slope-field' not in tail(run(current))
    step=params+'DOWN DOWN DOWN DOWN 2 EXE '
    expected='Slope-field columns' if supported else 'Max RK4 steps'
    assert expected in tail(run(step)) and EDIT not in tail(run(step))
    if not supported:
        last=step+'5 0 0 0 0 EXE '
        assert 'TEXT 144 145 50000\n' in tail(run(last))
        assert 'Max RK4 steps' in tail(run(last))
        assert bar(run(last+'EXE'))==BASE
        assert 'LEFT/RIGHT: ON/OFF toggle' in tail(run(params+'F5'))

sf20='1 4 F6 F6 DOWN DOWN DOWN DOWN DOWN 2 0 EXIT '
for entry in higher:
    in_higher=sf20+('EXIT '*4)+entry+' F6 F6 F2 '
    to_main='EXIT '*(3 if entry=='2' else 4)
    restored=in_higher+to_main+'1 4 F6 F6 '
    assert 'TEXT 144 145 20\n' in tail(run(restored))
    assert 'TEXT 144 145 12\n' in tail(run(restored+'F2'))
with tempfile.TemporaryDirectory() as directory:
    run(sf20+('EXIT '*4)+'2 F6 F6 F2 '+('EXIT '*3)+'F6 EXE',directory)
    loaded='F5 2 F1 F6 F6 '
    assert 'TEXT 20 145 Max steps\n' in tail(run(loaded,directory))
    assert 'TEXT 144 145 20\n' in tail(run(loaded+('EXIT '*3)+'1 4 F6 F6',directory))
print('UI polish: logical EDIT hints, context/overlay restoration, PREV pixels, six field swatches and mode-aware SF/INIT/SAVE retention passed.')
