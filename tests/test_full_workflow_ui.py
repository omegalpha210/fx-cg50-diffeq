"""Representative A-K workflows and gaps in maximum-row/nested-state coverage."""
import math,os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
BASE=['TRACE','ZOOM','V-WIN','TABLE','G-SLV','INIT']
def run(keys,directory=None):
    if directory is None:
        with tempfile.TemporaryDirectory() as temp:return run(keys,temp)
    p=subprocess.run([app],cwd=directory,env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,
        DIFFEQ_HOST_MAX_FRAMES='5000'),capture_output=True,text=True,timeout=30)
    assert p.returncode==0 and 'runtime error:' not in p.stderr,(keys,p.stderr,p.stdout[-3000:])
    assert 'SCRIPT COMPLETE' in p.stdout
    return p.stdout
def tail(out):return next(p for p in reversed(out.split('\nKEY ')) if 'TEXT ' in p)
def bar(out):
    keys=['']*6;previous=5
    for x,text in reversed(re.findall(r'TEXT (\d+) 206 ([^\n]*)',tail(out))):
        slot=(int(x)-6)//64
        if slot>previous:break
        if 0<=slot<6:keys[slot]=text+keys[slot];previous=slot
    return keys
def plot(out):return re.findall(r'^PLOT (\w+)',out,re.M)[-1]
def labels(out):return re.findall(r'^TEXT 20 \d+ ([^\n]*)',tail(out),re.M)
def headers(out):return re.findall(r'^TEXT \d+ 49 ([^\n]*)',tail(out),re.M)
def metrics(out):return tuple(map(int,re.findall(r'METRICS solves=(\d+) searches=(\d+) reads=(\d+) writes=(\d+) opens=(\d+) closes=(\d+)',out)[-1]))
def value_keys(value):
    mapping={'{':'S:MUL','}':'S:DIV',',':'COMMA','-':'NEG','.':'DOT'}
    return ' '.join(mapping.get(c,c) for c in str(value))+' '
def general(count=1,adaptive=False):
    values='1' if count==1 else '{'+','.join(str(i) for i in range(1,count+1))+'}'
    return '1 4 0 EXE F6 DOWN '+value_keys(values)+'EXE F6 '+('DOWN DOWN RIGHT ' if adaptive else '')
def vector(kind,adaptive=False):
    equation=f'{kind} 9 F6 '+('0 EXE '*9 if kind==4 else '')
    ic=equation+'F6 0 EXE '+''.join(value_keys(i)+'EXE ' for i in range(1,10))
    return ic+'F6 NEG 1 EXE 1 EXE '+('RIGHT ' if adaptive else '')

# A/B/C: all first-order consumers accept scalar and independent families.
for count,adaptive in [(1,False),(5,True),(10,False)]:
    params=general(count,adaptive);graph=params+'F6 '
    assert bar(run(graph))==BASE
    trace=run(graph+'F1 '+'DOWN '*(count-1))
    assert f'IC{count} x=0 y={count}' in tail(trace)
    assert bar(run(graph+'F1 EXIT R:EXIT'))==BASE
    table=run(graph+'F4 '+'RIGHT '*max(0,count-2))
    assert headers(table)==(['x','y'] if count==1 else ['x',f'y{count-1}',f'y{count}'])
    assert bar(run(graph+'F4 F6'))==BASE
    selection=run(graph+'F5 F1')
    assert ('UP/DOWN: SELECT GRAPH,' in tail(selection))==(count>1)
    assert bar(run(params+'F4 UP F3 EXIT EXIT'))==['INIT','ADV','V-WIN','OUTPUT','SET','GRAPH']
    if count>1:assert labels(run(params+'F4 UP'))[-1]==f'IC{count} y'

# D: second-order RK45 with both derivative outputs and direct two-curve ICPT.
second='2 F6 F6 DOWN DOWN RIGHT F6 '
cross=run(second+'F5 F5')
assert 'ICPT 1/' in tail(cross) and 'UP/DOWN: SELECT GRAPH,' not in cross
assert headers(run(second+'F4'))==['x','y',"y'"]

# E/F: all nine required ICs survive actual key entry; final field stays visible.
for kind,adaptive,last,display in [(3,False,'y(8)','y(8)'),(4,True,'y9','y9')]:
    params=vector(kind,adaptive);graph=params+'F6 '
    assert bar(run(graph))==BASE
    ic=run(params+'EXIT UP')
    assert labels(ic)[-1]==('y(8)0' if kind==3 else 'y9_0')
    assert 'TEXT 144 79 9\n' in tail(ic)
    out=run(params+'F4 UP');assert labels(out)[-1]==last
    assert max(map(int,re.findall(r'^TEXT 20 (\d+)',tail(out),re.M)))<=167
    traced=run(graph+'F1 '+'DOWN '*8)
    assert f'{display}=9' in tail(traced)
    assert headers(run(graph+'F4 '+'RIGHT '*7))==(['x','y(7)','y(8)'] if kind==3 else ['x','y8','y9'])
    # Every field supplied, and no hidden SF row enters maximum-mode navigation.
    assert 'SF' not in labels(run(params)) and ('TEXT 144 79 RK45\n' in tail(run(params)))==adaptive

# G: SYS2 projection/analysis and one fresh EXIT per nested layer.
phase='4 2 F6 F6 F6 F6 F4 F2 '
analysis=run(phase+'F5 F1 F2 F3')
assert 'Linearized: Center / Neutral candidate' in tail(analysis)
assert 'N1' in tail(analysis) and 'N2' in tail(analysis)
assert 'Equilibrium / INFO' in tail(run(phase+'F5 F3 F4'))
assert bar(run(phase+'F4 EXIT R:EXIT'))==['TRACE','ZOOM','V-WIN','VIEW','ANLYS','INIT']

# H/I: same E expression for MARK and STOP, followed by Table and INFO.
event_params='1 4 A:SUB EXE F6 DOWN 1 EXE F6 DOWN DOWN RIGHT '
event=event_params+'F2 F1 RIGHT DOWN A:SUB SUB 2 EXE '
for stop in [False,True]:
    graph=event+('DOWN RIGHT ' if stop else '')+'F6 EXIT F6 '
    out=run(graph);assert 'EVT' in tail(out)
    assert ('END: Event' in tail(out))==stop
    assert ('END: Event' in tail(run(graph+'F4 F2')))==stop
    assert 'Solver Info' in tail(run(graph+'EXIT F2 F2'))

# J: restricted RHS produces nonfatal status while valid-domain consumers remain.
domain='1 4 S:SQUARE 1 SUB XOT RIGHTP F6 DOWN 0 EXE F6 F6 '
assert 'END: Math domain / singularity' in tail(run(domain))
assert 'IC1 x=0 y=0' in tail(run(domain+'F1'))
assert 'Y-ICPT 1/1' in tail(run(domain+'F5 F4'))
assert 'END: Numerical limit' in tail(run(domain+'F4 F2'))

# K: explicit save, new process with no implicit load, then confirmed saved recall.
with tempfile.TemporaryDirectory() as directory:
    params=general(5,True)+'F4 DOWN F3 LEFT LEFT UP EXE EXIT '
    run(params+'EXIT EXIT EXIT EXIT 6 EXE EXE',directory)
    assert len(list(Path(directory).glob('DIFFEQ*.dat')))==1
    cold=run('',directory);assert metrics(cold)[2:]==(0,0,0,0)
    loaded=run('5 2 F6 F6 F6',directory)
    assert 'RK45' in tail(loaded)
    assert '{1,2,3,4,5}' in tail(run('5 2 F6 F6',directory))
    assert labels(run('5 2 F6 F6 F6 F4 UP',directory))[-1]=='IC5 y'
    assert 'No solver run yet' in tail(run('5 2 F6 F6 F6 F2 F2',directory))

# EXIT HOLD cannot cross a second boundary in any menu/edit/confirm class.
params='2 F6 F6 ';graph=params+'F6 '
nested=['2 LEFT F2 ','4 9 F6 LEFT F3 ',params+'F2 ',params+'F2 F1 ',params+'F2 F2 ',
    params+'F2 F1 DOWN LEFT F2 ',params+'F4 F3 ',params+'F3 LEFT F2 ',
    graph+'F2 ',graph+'F2 F5 ',graph+'F5 F1 ',phase+'F4 ',phase+'F5 F3 F4 ',
    '6 ','5 2 ','3 3 F6 5 EXIT EXIT 2 F6 ','3 3 F6 OPTN ']
for prefix in nested:
    once=run(prefix+'EXIT ')
    assert plot(run(prefix+'EXIT R:EXIT R:EXIT'))==plot(once),prefix
    twice=run(prefix+'EXIT R:EXIT EXIT ')
    assert plot(twice)==plot(run(prefix+'EXIT EXIT ')),prefix

# Required drafts: every family retains incomplete input until NEXT, then points
# at its offending field. The last SYS9/IC field exercises second-page validation.
for entry in ['1 1','1 2','1 3','1 4','2','3 1 F6','3 9 F6','4 1 F6','4 9 F6']:
    bad=run(entry+' LEFT ACON EXIT F6')
    assert 'Syntax error' in tail(bad) and 'INIT' in bar(bad)
    assert 'Initial Conditions' not in tail(bad)
    reset=run(entry+' LEFT ACON EXIT F6 F1 F6')
    assert 'Initial Conditions' in tail(reset)
for entry in ['3 9 F6 F6 ','4 9 F6 F6 ']:
    bad=run(entry+'UP LEFT ACON EXIT F6')
    assert 'Initial Conditions' in tail(bad) and 'Syntax error' in bad
    assert labels(bad)[-1] in ['y(8)0','y9_0']
    assert 'Parameter' in tail(run(entry+'UP LEFT ACON EXIT F6 F1 F6'))

# Regression for the audited mismatch: RK45 output ignores hidden RK4 Step.
table=run('2 F6 F6 DOWN DOWN DOWN DOWN 7 EXIT UP UP RIGHT F6 F4')
assert 'Step 7' not in tail(table) and 'dx 0.0333333' in tail(table)
assert 'Step 7' in tail(run('2 F6 F6 DOWN DOWN DOWN DOWN 7 EXIT F6 F4'))
# A cancelled Drawing attempt never becomes the last committed calculation.
recall='1 4 1 EXE F6 F6 F6 EXIT EXIT EXIT 2 EXE F6 F6 CANCEL:1 F6 EXIT EXIT EXIT EXIT 5 1 '
assert 'TEXT 144 57 1\n' in tail(run(recall))
assert 'No previous calculation' in tail(run('1 4 2 EXE F6 F6 CANCEL:1 F6 EXIT EXIT EXIT EXIT 5 1'))
# A later successful calculation replaces the snapshot; auxiliary Graph return
# and user window edits cannot accidentally publish a different equation.
successful='1 4 1 EXE F6 F6 F6 EXIT EXIT EXIT 2 EXE F6 F6 F6 EXIT EXIT EXIT EXIT EXIT 5 1 '
assert 'TEXT 144 57 2\n' in tail(run(successful))
print('Full workflow A-K, nine-state required inputs/columns, dynamic rows, nested EXIT/HOLD and truthful RK45 Table spacing passed.')
