"""RK45 Parameters and consumers through the actual iterative UI handlers."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys,directory=None):
    if directory is None:
        with tempfile.TemporaryDirectory() as temp:return run(keys,temp)
    p=subprocess.run([app],cwd=directory,env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,
        DIFFEQ_HOST_MAX_FRAMES='5000'),capture_output=True,text=True,timeout=30)
    assert p.returncode==0 and 'SCRIPT COMPLETE' in p.stdout and 'runtime error:' not in p.stderr,(p.stderr,p.stdout[-2000:])
    return p.stdout
def tail(out):return out[out.rfind('\nKEY '):]
def bar(out):return re.findall(r'TEXT \d+ 206 ([^\n]*)',tail(out))[-6:]
def solves(out):return int(re.findall(r'solves=(\d+)',out)[-1])
for entry,scalar in [('1 4',True),('2',False),('3 9 F6',False),('4 9 F6',False)]:
    params=entry+' F6 F6 '
    old=run(params);assert 'TEXT 144 79 RK4\n' in tail(old)
    adaptive=params+'DOWN DOWN RIGHT '
    out=run(adaptive)
    assert solves(out)==0 and 'TEXT 144 79 RK45\n' in tail(out)
    labels=re.findall(r'TEXT 20 \d+ ([^\n]+)',tail(out))
    assert labels==['Xrange min','Xrange max','Method','Initial h','RelTol','AbsTol']+(['SF'] if scalar else ['Max steps'])
    assert 'TEXT 144 123 1e-06\n' in tail(out) and 'TEXT 144 145 1e-09\n' in tail(out)
    returned=tail(run(adaptive+'LEFT'))
    assert 'TEXT 144 79 RK4\n' in returned and 'Step\n' in returned
    edited=adaptive+'DOWN DOWN 1 EXP NEG 7 EXE 1 EXP NEG 1 0 EXIT '
    back=tail(run(edited+'UP UP UP LEFT RIGHT'))
    assert 'TEXT 144 123 1e-07\n' in back and 'TEXT 144 145 1e-10\n' in back
    initialized=tail(run(edited+'F1'))
    assert 'TEXT 144 79 RK45\n' in initialized and 'TEXT 144 123 1e-06\n' in initialized
    last=tail(run(adaptive+'UP UP UP'))
    assert 'Max steps' in last and 'Accepted + rejected attempts per path' in last
    if scalar:assert 'TEXT 20 145 SF\n' in last and 'TEXT 20 167 Max steps\n' in last
    else:assert 'SF\n' not in last
    assert solves(run(adaptive+('LEFT RIGHT '*100)))==0
params='2 F6 F6 DOWN DOWN RIGHT '
for invalid in ['0','NEG 1','1 EXP NEG 3 0 0']:
    out=run(params+'DOWN DOWN '+invalid+' EXE')
    assert 'Invalid parameter' in out and solves(out)==0
    if '3 0 0' in invalid:assert 'Tolerance too small' in out
for invalid in ['0','NEG 1']:
    assert 'Invalid parameter' in run(params+'DOWN DOWN DOWN '+invalid+' EXE')
graph=params+'F6 '
assert solves(run(params+'EXE'))==solves(run(graph))
failed=run('2 F6 1 EXE F6 DOWN DOWN RIGHT DOWN 1 EXP NEG 3 0 0 EXE F6 EXE LEFT')
assert 'Step underflow' in failed and 'EXE: commit / next   EXIT: commit' in tail(failed)
base=run(graph+'F1')
assert 'IC1 x=0 y=1' in base and solves(base)>0
moved=run(graph+'F1 F4 '+('R:RIGHT '*70)+'EXIT')
assert bar(moved)==['TRACE','ZOOM','V-WIN','TABLE','G-SLV','PREV']
exact=tail(run(graph+'F1 F1 0 DOT 7 3 3 EXE'))
x,y=map(float,re.findall(r'IC1 x=([\d.eE+-]+) y=([\d.eE+-]+)',exact)[-1])
import math
assert abs(x-.733)<1e-12 and abs(y-math.cos(.733))<2e-6
assert 'ROOT 1/' in run(graph+'F5 F1 EXE')
assert 'Y-CAL 1/1' in run(graph+'F5 F6 F1 EXE 0 DOT 7 3 3 F6')
assert 'END' in tail(run(graph+'F4 F2'))
with tempfile.TemporaryDirectory() as directory:
    out=run(graph+'F4 F5',directory)
    assert 'STAT data saved' in out and list(Path(directory).glob('DIFFSTAT*.csv'))
with tempfile.TemporaryDirectory() as directory:
    run(params+'DOWN DOWN 1 EXP NEG 7 EXE EXIT EXIT EXIT EXIT 6 EXE EXE',directory)
    out=tail(run('5 2 F1 F6 F6',directory))
    assert 'TEXT 144 79 RK45\n' in out and 'TEXT 144 123 1e-07\n' in out
phase='4 2 F6 F6 F6 DOWN DOWN RIGHT F6 F4 F2 '
out=run(phase+'F5 F2 F3')
assert 'Linearized: Center / Neutral candidate' in out
assert 'y1=' in run(phase+'F1 RIGHT RIGHT EXIT')
assert bar(run(phase+'F4 F1 F4 F3'))==['TOP','BTM','MID','','STAT','GRAPH']
print('RK45 UI: Method/tolerance editing, dynamic rows/INIT, preference retention, 100 toggles, TRACE, G-Solve, Table/STAT, Phase and SAVE/RCL passed.')
