"""Production handlers: edit-only FUNC, nested EXIT, fixed TRACE strides, field settings and meaningful LIST."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys,directory=None):
    if directory is None:
        with tempfile.TemporaryDirectory() as directory:return run(keys,directory)
    p=subprocess.run([app],cwd=directory,env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,
        DIFFEQ_HOST_MAX_FRAMES='10000'),capture_output=True,text=True,timeout=30)
    assert p.returncode==0 and 'runtime error:' not in p.stderr,(p.stderr,p.stdout[-2000:])
    return p.stdout
def tail(out):return next(part for part in reversed(out.split('\nKEY ')) if 'TEXT ' in part)
def bar(out):
    keys=['']*6
    for x,label in re.findall(r'TEXT (\d+) 206 ([^\n]*)',tail(out)):
        keys[(int(x)-6)//64]+=label
    return keys
def plot(out):return re.findall(r'^PLOT (\w+)',out,re.M)[-1]
def solves(out):return int(re.findall(r'solves=(\d+)',out)[-1])
def point(out):return float(re.findall(r'IC\d x=([-+.\deE]+)',out)[-1])
for entry in ['1 1','1 2','1 3','1 4','2','3 9 F6','4 9 F6']:
    assert bar(run(entry))[1]=='' and plot(run(entry+' F2'))==plot(run(entry))
    for edit in ['LEFT','RIGHT','2','SIN']:
        base=entry+' '+edit
        assert bar(run(base))[1]=='FUNC'
        assert plot(run(base+' F2 EXIT'))==plot(run(base))
        assert plot(run(base+' F2 F6 F6 EXIT'))==plot(run(base))
        for key in ['EXE','LEFT','RIGHT','UP','DOWN']:
            assert plot(run(base+' F2 '+key))==plot(run(base+' '+key))
    invalid=entry+' LEFT ACON F2 F2' # sinh(
    out=run(invalid+' F2 EXIT R:EXIT R:EXIT')
    assert 'Check equation' not in out and 'sinh(' in tail(out)
    assert bar(out)[1]=='FUNC' and plot(out)==plot(run(invalid))
    assert 'Check equation' in run(invalid+' F2 EXIT EXIT')
for entry in ['3 9 F6','4 9 F6']:
    for edit in ['', ' LEFT', ' LEFT ACON SIN']:
        base=entry+edit
        assert plot(run(base+' F1 EXIT R:EXIT'))==plot(run(base))
        assert plot(run(base+' F1 EXE'))==plot(run(base+' EXE'))

# Fixed Xdot=.025, independently selected x=.6. Mode changes are only button state.
graph='2 F6 F6 F3 DOWN DOWN DOWN 0 DOT 0 2 5 EXE EXIT EXE '
trace=graph+'F1 F1 0 DOT 6 EXE '
base=run(trace)
for key,multiple in [('F2',1),('F3',2),('F4',3)]:
    changed=run(trace+key)
    assert point(changed)==.6 and plot(changed)==plot(base) and solves(changed)==solves(base)
    assert bar(changed)==['x=','NORMAL','FAST','FASTER','LEFT','RIGHT']
    for direction,sign in [('LEFT',-1),('RIGHT',1)]:
        out=run(trace+key+' '+direction)
        assert abs(point(out)-(.6+sign*multiple*.025))<1e-7 and solves(out)==solves(base)
assert abs(point(run(graph+'F1 F4 EXIT F1 RIGHT'))-.025)<1e-7
for speed in ['F3','F4']:
    out=run(graph+'F1 '+speed+' '+'R:RIGHT '*260+'DOWN BLINK BLINK EXIT')
    assert 'Partial: Cancelled' not in out and bar(out)==['TRACE','ZOOM','V-WIN','TABLE','G-SLV','PREV']
    assert 'y\'' in out
    before=run(trace+speed);blink=run(trace+speed+' '+'BLINK '*100)
    assert point(blink)==point(before) and plot(blink)==plot(before) and solves(blink)==solves(before)
limited='2 F6 F6 DOWN DOWN DOWN 0 DOT 0 0 1 EXE F6 F1 F4 '
out=run(limited+'RIGHT '*250)
assert 'TRACE: too many steps' in out and 'Partial: Cancelled' not in out
out=run('1 4 1 0 0 MUL A:SUB EXE F6 F6 F6 F1 F4 '+'RIGHT '*100)
assert 'TRACE: Numerical limit' in out and 'nan' not in tail(out).lower()

params='1 4 F6 F6 '
sf=params+'DOWN DOWN DOWN DOWN DOWN '
settings=params+'F5 '
assert 'TEXT 20 145 SF\n' in tail(run(params))
assert all(s in tail(run(settings)) for s in ['Slope Field','Style','Color','Arrow','Pale Blue'])
assert 'TEXT 20 79 Density' not in run(settings)
assert 'Private constants' not in run(settings+'EXIT OPTN')
for entry in ['2','3 1 F6','3 9 F6','4 1 F6','4 9 F6']:
    assert 'Slope Field' not in tail(run(entry+' F6 F6 F5'))
changed=sf+'2 4 EXIT F5 DOWN DOWN LEFT DOWN F3 RIGHT EXE '
assert all(s in tail(run(changed)) for s in ['Segment','Pale Red'])
for name,moves in zip(['Pale Blue','Pale Red','Pale Cyan','Pale Magenta','Pale Gold','Gray'],
                      ['', 'RIGHT', 'RIGHT RIGHT', 'DOWN', 'DOWN RIGHT', 'DOWN RIGHT RIGHT']):
    choice=settings+'DOWN DOWN DOWN F3 '+moves+' EXE '
    assert name in tail(run(choice))
    assert plot(run(choice+'F3 DOWN EXIT'))==plot(run(choice))
assert plot(run(changed+'F3 DOWN EXIT'))==plot(run(changed))
assert plot(run(changed+'F1'))==plot(run(settings))
# Direct return restores the SF selector; appearance INIT preserves SF=24.
assert plot(run(changed+'F1 EXIT'))==plot(run(sf+'2 4 EXIT'))
assert 'TEXT 144 145 24\n' in tail(run(changed+'EXIT'))
assert 'TEXT 144 145 12\n' in tail(run(changed+'EXIT F1'))
assert all(s in tail(run(changed+'EXIT F1 F5')) for s in ['Segment','Pale Red'])
assert all(s in tail(run(changed+'EXIT F4 F1 EXIT F5')) for s in ['Segment','Pale Red'])
for invalid in ['NEG 1','1 0 1','1 DOT 5']:
    assert 'Invalid ' in run(sf+invalid+' EXE')
for value in ['0','1','1 0 0']:
    assert 'Invalid ' not in run(sf+value+' EXE F5')
with tempfile.TemporaryDirectory() as directory:
    run(changed+'EXIT EXIT EXIT EXIT EXIT 6 EXE EXE',directory)
    restored=run('5 2 F1 F6 F6 F5',directory)
    assert all(s in tail(restored) for s in ['Segment','Pale Red'])
    assert 'TEXT 144 145 24\n' in tail(run('5 2 F1 F6 F6',directory))
# Mode changes preserve global appearance and the existing SF setting.
out=run(changed+'EXIT EXIT EXIT EXIT EXIT 2 EXIT 1 4 F6 F6 F5')
assert all(s in tail(out) for s in ['Segment','Pale Red'])

output='2 F6 F6 F4 '
assert bar(run(output))==['INIT','','COLOR','','','DONE']
assert 'TEXT 20 35 y\n' in tail(run(output))
assert all(s not in tail(run(output)) for s in ['CSV X','Fixed','TEXT 20 35 x\n','G L'])
assert plot(run(output+'F1 F2'))==plot(run(output))
assert 'OFF' in tail(run(output+'RIGHT')) and 'Curve color' not in run(output+'RIGHT')
assert plot(run(output+'RIGHT LEFT'))==plot(run(output))
# One ON/OFF bit controls every consumer; selected states remain in variable order.
for entry,labels in [('3 9 F6',["y", "y'", "y(8)"]),('4 9 F6',['y1','y2','y9'])]:
    base=entry+' F6 F6 F4 DOWN '+('RIGHT DOWN '*7)+'EXIT F6 F4 '
    table=tail(run(base))
    headers=re.findall(r'TEXT \d+ 49 ([^\n]*)',table)
    assert headers==['x',labels[0],labels[2]],headers
# All outputs OFF still yields an x-only Table and STAT file.
with tempfile.TemporaryDirectory() as directory:
    out=run(output+'RIGHT DOWN RIGHT EXIT F6 F4 F5',directory)
    assert 'STAT data saved' in tail(out)
    data=next(Path(directory).glob('DIFFSTAT*.csv')).read_text().splitlines()
    assert data[0]=="'x" and all(',' not in line for line in data)
    xs=list(map(float,data[1:]));assert xs==sorted(xs) and xs[0]<0<xs[-1]
print('UX: edit FUNC/EXIT/repeats, TRACE strides, direct SET, SF/appearance INIT, palette/persistence and unified OUTPUT passed.')
