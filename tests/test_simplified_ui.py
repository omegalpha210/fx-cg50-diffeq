"""Physical list input, shared outputs and unified Table through production UI."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys,directory=None):
    if directory is None:
        with tempfile.TemporaryDirectory() as directory:return run(keys,directory)
    p=subprocess.run([app],cwd=directory,env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,
        DIFFEQ_HOST_MAX_FRAMES='10000'),capture_output=True,text=True,timeout=30)
    assert p.returncode==0 and 'runtime error:' not in p.stderr,(p.stderr,p.stdout[-3000:])
    assert 'SCRIPT COMPLETE' in p.stdout
    return p.stdout
def tail(out):return next(p for p in reversed(out.split('\nKEY ')) if 'TEXT ' in p)
def plot(out):return re.findall(r'^PLOT (\w+)',out,re.M)[-1]
def rows(out):return list(map(float,re.findall(r'TEXT 16 (?:69|86|103|120|137|154|171) ([^\n]+)',tail(out))))
def headers(out):return re.findall(r'TEXT \d+ 49 ([^\n]*)',tail(out))
def keys(text):
    mapping={'{':'S:MUL','}':'S:DIV',',':'COMMA','-':'NEG','.':'DOT','/':'DIV','+':'ADD',
             '(':'LEFTP',')':'RIGHTP'}
    return ' '.join(mapping.get(c,c) for c in text)+' '
def ic(text):return '1 1 F6 DOWN '+keys(text)+'EXE '
for value,canonical in [('0','0'),('{1}','1'),('{0,1}','{0,1}'),('{0,1,-1}','{0,1,-1}'),
                        ('{0,0}','{0,0}'),('{1/2,1+1}','{0.5,2}')]:
    entered=ic(value)
    assert 'TEXT 144 57 '+canonical+'\n' in tail(run(entered))
    assert plot(run(entered+'F4 F5'))==plot(run(entered))
    assert plot(run(entered+'F3 EXIT'))==plot(run(entered))
    assert plot(run(entered+'F6 F1'))==plot(run(entered))
for value in ['{}','{0,}','{,1}','{0,,1}','{0,1','{0,{1}}','0,1','{1/0}','{0,1,2,3,4,5,6,7,8,9}']:
    out=run('1 1 F6 DOWN '+keys(value)+'F6')
    assert 'Initial values' in tail(out) and 'TEXT 14 9 Parameter\n' not in out
    if value.endswith('8,9}'):assert 'Too many initial values' in out
    # Dismiss, clear only the draft, and retry. A rejected list never advances.
    fixed=run('1 1 F6 DOWN '+keys(value)+'F6 EXE ACON '+keys('{0,1}')+'EXE F6')
    assert 'TEXT 14 9 Parameter\n' in tail(fixed)
# All nine values enter the existing total-work preflight (216,000 > 200,000).
many=ic('{0,1,2,3,4,5,6,7,8}')
out=run(many+'F6 DOWN DOWN 0 DOT 0 0 0 5 EXE F6')
assert 'Total calculation too large.' in out
assert re.findall(r'solves=(\d+)',out)[-1]=='0'
# Manual separable example: common x0, two distinct columns, exact initial states.
manual=ic('{0,1}')+'F6 NEG 5 EXE 5 EXE F6 F4 '
out=run(manual);assert headers(out)==['x','y1','y2']
xs=rows(out);assert len(xs)==7 and xs[3]==0 and xs==sorted(xs)
assert 'TEXT 141 120 0\n' in tail(out) and 'TEXT 266 120 1\n' in tail(out)
assert rows(run(manual+'UP'))[-1]<xs[0] and rows(run(manual+'DOWN'))[0]>xs[-1]
for path in ['F1 F3','F2 F3','F1 F3 F2 F3 '*20]:assert rows(run(manual+path))==xs
assert rows(run(manual+'F1'))[0]==-5 and rows(run(manual+'F2'))[-1]==5
assert 'DIR' not in tail(out) and 'MID' in tail(out)
with tempfile.TemporaryDirectory() as directory:
    saved=run(manual+'F5',directory);assert 'STAT data saved' in tail(saved)
    data=next(Path(directory).glob('DIFFSTAT*.csv')).read_text().splitlines()
    assert data[0]=="'x,'y1,'y2"
    triples=[list(map(float,line.split(','))) for line in data[1:]]
    assert [p[0] for p in triples]==sorted(p[0] for p in triples)
    assert next(p for p in triples if p[0]==0)==[0,0,1]
    assert all(p[2]==1 for p in triples)
# x0 changes all first-order solutions; TRACE switches IC at the same x0.
out=run(ic('{0,1,-1}')+'UP 1 EXE F6 F6 F1 DOWN DOWN')
assert 'IC3 x=1 y=-1' in tail(out)
# Multiple IC columns scroll horizontally; page jumps preserve column position.
table=many+'F6 F6 F4 RIGHT RIGHT RIGHT '
assert headers(run(table))==['x','y4','y5']
for key in ['F1','F2','F3','UP','DOWN']:
    assert headers(run(table+key))==['x','y4','y5']
assert 'Left/Right: columns' in tail(run(table))
assert 'Left/Right: columns' not in tail(run(manual))
# A first-order OUTPUT is still one dependent row despite nine ICs.
out=run(many+'F6 F4')
assert re.findall(r'TEXT 20 \d+ ([^\n]+)',tail(out))==['y']
out=run(many+'F6 F4 RIGHT EXIT F6 F1')
assert 'IC1 x=' not in out and plot(out)==plot(run(many+'F6 F4 RIGHT EXIT F6'))
# Higher modes have one complete state vector, never ADD/DROP or IC switching.
for entry,n,last in [('2',2,"y'0"),('3 9 F6',9,'y(8)0'),('4 9 F6',9,'y9_0')]:
    prefix=entry+' F6 '
    out=run(prefix+'DOWN '*n)
    assert last in tail(out) and all(s not in tail(out) for s in ['ADD','DROP','IC 1'])
    assert plot(run(prefix+'F4 F5'))==plot(run(prefix))
    # Every required initial state can be edited; scalar braces are not vectors.
    edited=prefix+'0 EXE '+('1 EXE '*n)
    assert last in tail(run(edited))
    assert 'TRACE' in run(edited+'F6 F6')
    bad=run(prefix+'DOWN '+keys('{0,1}')+'F6')
    assert 'Invalid value' in tail(bad) and 'Parameter' not in bad
# Save and restore the list, SF, colors, and the common output bit.
with tempfile.TemporaryDirectory() as directory:
    prefix=ic('{0,1,-1}')+'F6 DOWN DOWN DOWN DOWN 2 4 EXIT F4 RIGHT EXIT '
    run(prefix+'EXIT EXIT EXIT EXIT F6 EXE',directory)
    assert '{0,1,-1}' in tail(run('F5 2 F1 F6',directory))
    assert 'TEXT 144 123 24\n' in tail(run('F5 2 F1 F6 F6',directory))
    assert 'OFF' in tail(run('F5 2 F1 F6 F6 F4',directory))
print('List IC physical input/errors/workload, manual two curves, all-state ICs, unified Table navigation/CSV and SAVE passed.')
