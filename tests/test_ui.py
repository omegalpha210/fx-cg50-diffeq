"""Real app/UI with scripted keys, counters and pixel evidence; not OS emulation."""
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path
executable=str(Path(sys.argv[1]).resolve())
def run(keys,expected=(),absent=(),frames=False):
    with tempfile.TemporaryDirectory(dir='.') as directory:
        env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_MAX_FRAMES='5000')
        if frames: env['DIFFEQ_HOST_OUT']=str(Path(directory).resolve())
        p=subprocess.run([executable],cwd=directory,env=env,text=True,capture_output=True,timeout=30)
        assert p.returncode==0 and 'runtime error:' not in p.stderr,(p.stderr,p.stdout[-4000:])
        assert 'SCRIPT COMPLETE' in p.stdout
        for item in expected: assert item in p.stdout,(item,p.stdout[-4000:])
        for item in absent: assert item not in p.stdout,(item,p.stdout[-4000:])
        if frames: return p.stdout,[f.read_bytes() for f in sorted(Path(directory).glob('*.ppm'))]
        return p.stdout

def plot(out): return re.findall(r'^PLOT (\w+)',out,re.M)[-1]
def metrics(out):
    return [tuple(map(int,m)) for m in re.findall(
        r'METRICS solves=(\d+) searches=(\d+) reads=(\d+) writes=(\d+) opens=(\d+) closes=(\d+)',out)]

for keys,expected in [('A:ADD','x'),('ADD','+'),('A:SUB','y'),('SUB','-'),
                      ('NEG','-'),('A:NEG','-'),('XOT','x'),('S:SQUARE','sqrt('),
                      ('S:EXP ADD','pi+'),('A:ADD ADD A:SUB SUB NEG','x+y--')]:
    run('1 4 '+keys,['TEXT 144 57 '+expected+'\n'])
run('1 4 SIN F6',['Check equation','Syntax error'],['Initial Conditions'])
run('1 4 SIN F3 EXE',['Check equation','sin('],['View Window'])
run('1 4 LEFT ACON F2 F2',['sinh('],['Insert function'])
run('1 4 LEFT ACON F1 F1',[],['Insert variable'])
run('1 4 NEG A:SUB EXE',['-y'],['Initial Conditions'])
run('1 1 1 DOWN',['g(y)','TEXT 144 79 y^2-1'])
run('1 4 LEFT',['DIFF EQ / General 1st','NEXT','V-WIN'],['OUTPUT','SET'])

families=[('1 1','Separable',"y' = f(x) * g(y)"),
 ('1 2','Linear 1st',"y' + f(x)*y = g(x)"),
 ('1 3','Bernoulli',"y' + f(x)*y = g(x)*y^n"),
 ('1 4','General 1st',"y' = f(x,y)"),
 ('2','Linear 2nd',"y'' + f(x)*y' + g(x)*y = h(x)"),
 ('3 9 F6','N-th order','y^(9) = f(x,y,y1,...,y8)'),
 ('4 9 F6','1st order system',"yi' = fi(x,y1,...,y9), i=1..9")]
for entry,title,formula in families:
    out=run(entry+' F6 F6 F6 EXIT F1 F1',[title,formula,'Initial Conditions','Parameter','TRACE'],
        ['h=0.1','1/2   ','Calculating graph...'])
    assert out.rfind('DIFF EQ /')>out.rfind('Initial Conditions')>out.rfind('TEXT 14 9 Parameter')
    for stage in ['', ' F6', ' F6 F6']:
        prefix=entry+stage+' DOWN'
        baseline=run(prefix)
        for aux in (['F3 EXIT','F4 EXIT','F5 EXIT'] if stage==' F6 F6' else ['F3 EXIT']):
            assert plot(run(prefix+' '+aux))==plot(baseline),(entry,stage,aux)

run('4 9 F6 '+'DOWN '*8+'0 EXE F6 '+'DOWN '*9,["y9'",'Initial Conditions','y9'])
for invalid in ['0','1 0','NEG 2','1 DOT 5','LEFT ACON']:
    run('3 '+invalid+' F6',['Invalid dimension'],['DIFF EQ / Higher order'])
run('3 3 F6 5 EXE EXIT EXE F6',['TEXT 144 57 5\n'],['Change Equation Size'])
run('3 3 F6 5 EXE EXIT 2 F6 F5 EXIT',['Change Equation Size','Differential Equation'])
kept=run('3 3 F6 5 EXE EXIT 2 F6 F5 3 F6')
assert 'TEXT 144 57 5\n' in kept[kept.rfind('TEXT 14 9 DIFF EQ /'):]
run('3 3 F6 OPTN F1',['Convert to system','1st order system','y3'])
run('1',['OPEN'],['PG-','PG+','OK'])
run('2 F6 F4 DOWN 2 EXE F5',["y'0",'Initial Conditions'],['ADD','DROP','IC 2'])
run('2 F6 NEG 2 EXE',['Initial Conditions','x0','-2','y0'],['Expression editor'])
run('1 4 F6 F5 F6 F6',['TRACE'],['No initial conditions.'])
run('2 F6 F5',["y'0"],['No initial conditions.','ADD','DROP'])
run('1 4 NEG A:SUB F3 EXIT F6 DOWN 2 F6 F4 EXIT DOWN DOWN 0 DOT 0 5 F5 EXIT',
    ['-y','Initial Conditions','Output selection','Parameter','0.05'])
run('2 F6 SIN F1 EXE',['Invalid value','sin('])
# Repeated PREV/NEXT retains edited fields and manual parameters exactly.
edited='1 4 NEG A:SUB F6 DOWN 2 F6 NEG 3 EXE 3 EXE 0 DOT 0 5 EXE'
baseline=run(edited)
assert plot(run(edited+' F1 F1 F6 F6 '*30))==plot(baseline)
# Red emphasis is confined to F6; inspect RGB bytes from the host drawing.
_,images=run('2 F6 F6',frames=True)
rgb=images[-1].split(b'\n',3)[3]
def pixel(x,y): return rgb[(y*396+x)*3:(y*396+x)*3+3]
assert pixel(327,203)==bytes([255,0,0])
assert all(pixel(7+64*i,203)!=bytes([255,0,0]) for i in range(5))

prefix='1 4 F3 NEG 6 DOT 3 EXE 6 DOT 3 EXE EXIT F6 F6'
run(prefix,['TEXT 144 35 -6\n','TEXT 144 57 6\n'])
manual=prefix+' NEG 3 EXE 3 EXE'
out=run(manual+' F3 EXIT F1 F6',['TEXT 144 35 -6.3\n','TEXT 144 57 6.3\n'])
tail=out[out.rfind('TEXT 14 9 Parameter'):]
assert 'TEXT 144 35 -3\n' in tail and 'TEXT 144 57 3\n' in tail
out=run(manual+' F1 F1 F6 F6 F3 NEG 7 EXE EXIT')
tail=out[out.rfind('TEXT 14 9 Parameter'):]
assert 'TEXT 144 35 -3\n' in tail and 'TEXT 144 57 3\n' in tail
run('1 4 F3 0 DOT 1 EXE 0 DOT 2 EXE EXIT F6 F6',['TEXT 144 35 0.1\n','TEXT 144 57 0.2\n'])
run('1 4 F3 DOWN DOWN DOWN 0 DOT 0 5 EXE',['dot','0.05','12.6'])
run('1 4 F3 S:EXP EXE',['3.14159265'])
run('1 4 F3 A:ADD EXE',['Invalid value'])
run('1 4 F3 NEG 7 EXE EXIT EXIT EXIT 2 F3',['View Window','-7'])

graph='2 F6 F6 F6 '
run(graph+'F5 F1 EXE RIGHT',['Select IC1 y','ROOT 2/4'])
run(graph+'F5 F2 EXE',['MAX 1/1','Y=1'])
run(graph+'F5 F3 EXE RIGHT',['MIN 2/2'])
run(graph+'F5 F4 EXE',['Y-ICPT 1/1','X=0','Y=1'])
run(graph+'F5 F5 RIGHT',['ICPT 2/4'])
run('1 4 F6 F6 F6 F5 F5',['ICPT: Not available'])
run(graph+'F5 F6 F1 EXE 1 F6',['Y-CAL 1/1','X=1','Y=0.540302'])
run(graph+'F5 F6 F2 EXE 0 F6 RIGHT',['X-CAL 2/4'])
run(graph+'F5 RIGHT F2 EXE',['MAX 1/2'])
run(graph+'F1 DOWN RIGHT',["IC1 x=0 y'=0","IC1 x=0.03333333 y'=-0.03327778"])
run('1 4 NEG A:SUB F6 F6 F6 F1 F1 1 EXE',['IC1 x=1 y=0.3678798'])
hidden=run('2 F6 F6 F4 DOWN RIGHT EXIT F6 F1 DOWN',['IC1 x=0 y=1'])
assert all(" y'=" not in line for line in hidden.splitlines() if line.startswith('TEXT 14 188 IC1 x='))
run(graph+'F3 DOWN DOWN DOWN DOWN DOWN 2 0 EXIT UP 5 EXIT EXIT F5 F1 EXE',
    ['ROOT 1/4'],['ROOT: Not found'])
run('4 2 F6 F6 F6 F6 OPTN 1 F2 F3',['TRACE'])
for keys,expected in [(graph+'F1',"IC1 x=0 y=1"),(graph+'F5 F1','Select IC1 y')]:
    out,images=run(keys+' BLINK BLINK',expected=[expected],frames=True)
    assert images[-3]!=images[-2] and images[-3]==images[-1]
    run(keys+' DOWN BLINK',["y'"])
base=run(graph)
menu=run(graph+('F5 F6 F6 EXIT F2 EXIT '*100),absent=['Calculating graph...'])
assert metrics(base)[-1][0]>0 and metrics(menu)[-1]==metrics(base)[-1]
assert plot(menu)==plot(base)
after_graph=menu.index('KEY F5')
assert all(p==plot(base) for p in re.findall(r'^PLOT (\w+)',menu[after_graph:],re.M))
assert 'Drawing...' not in menu[after_graph:]
actual=run(graph+'F2 F1')
assert metrics(actual)[-1][0]>metrics(base)[-1][0] and plot(actual)!=plot(base)
run(graph+'F2 F3',['TRACE'])
run('2 F6 F6 F4 DOWN RIGHT LEFT',['ON','OFF','LEFT/RIGHT: ON/OFF toggle'])
run('4 2 F6 F6 F6 F6 F4 F2 F3 F5',
    ['Preparing table...','Table','MID','STAT data saved'],['DIR','Table / IC1'])
run(graph+'EXIT EXIT EXIT EXIT F5 1',['Recall','DIFF EQ / Linear 2nd'])
run('EXIT EXIT F6 EXE EXIT',['No session.','SCRIPT COMPLETE'])
print('UI workflows: seven families, return/drafts, 9 states, modifiers, range, G-Solve/TRACE, menu counters, Table/STAT passed.')
