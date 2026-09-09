"""Current production UI: Main/SAVE, ADV/INIT, SELECT cycles and workflow headers."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys,directory=None):
    if directory is None:
        with tempfile.TemporaryDirectory() as temp:return run(keys,temp)
    p=subprocess.run([app],cwd=directory,env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,
        DIFFEQ_HOST_MAX_FRAMES='5000'),capture_output=True,text=True,timeout=30)
    assert p.returncode==0 and 'SCRIPT COMPLETE' in p.stdout and 'runtime error:' not in p.stderr,(p.stderr,p.stdout[-2500:])
    return p.stdout
def tail(out):return next(p for p in reversed(out.split('\nKEY ')) if 'TEXT ' in p)
def plot(out):return re.findall(r'^PLOT (\w+)',out,re.M)[-1]
def bar(out):return re.findall(r'TEXT \d+ 206 ([^\n]*)',tail(out))[-6:]
def metrics(out):return tuple(map(int,re.findall(r'METRICS solves=(\d+) searches=(\d+) reads=(\d+) writes=(\d+) opens=(\d+) closes=(\d+)',out)[-1]))
def progress(out,n):
    found=re.findall(r'TEXT \d+ 9 ([123]/3)\n',tail(out));assert found==([f'{n}/3'] if n else []),tail(out)
# Six digits, EXE and F6 OPEN converge on the same current item handler.
assert bar(run(''))==['','','','','','OPEN']
assert plot(run('F1 F2 F3 F4 F5'))==plot(run(''))
for n in range(1,7):
    direct=run(str(n));selected='DOWN '*(n-1)
    assert plot(direct)==plot(run(selected+'EXE'))==plot(run(selected+'F6'))
    progress(direct,1 if n==2 else 0)
assert plot(run('UP DOWN'))==plot(run(''))
assert plot(run('UP'))==plot(run('DOWN '*5))
assert plot(run('5 EXIT'))==plot(run('DOWN '*4)) # Internal return keeps selector.
# Confirmation does no backend work; EXE/F6 save once, NO/EXIT never write.
prepared='2 EXIT '
for key in ['','F5','EXIT','R:EXE R:F6']:
    with tempfile.TemporaryDirectory() as directory:
        out=run(prepared+'6 '+key,directory)
        assert metrics(out)==(0,0,0,0,0,0) and not list(Path(directory).iterdir())
        if key in ['F5','EXIT']:assert plot(out)==plot(run('DOWN '*5))
        else:assert 'Save current session?' in tail(out)
confirmed=[]
for key in ['EXE','F6']:
    with tempfile.TemporaryDirectory() as directory:
        out=run(prepared+'6 '+key+' EXE R:EXE',directory)
        assert len(list(Path(directory).glob('DIFFEQ?.dat')))==1
        assert out.count('Saved equation, settings, ICs and last')==1
        confirmed.append(metrics(out));assert confirmed[-1][3]>0
assert confirmed[0]==confirmed[1]
with tempfile.TemporaryDirectory() as directory:
    (Path(directory)/'DIFFEQ0.dat').mkdir();(Path(directory)/'DIFFEQ1.dat').mkdir()
    out=run(prepared+'6 EXE EXE',directory)
    assert 'Save failed.' in out and plot(out)==plot(run('DOWN '*5))
# All main ODE modes, including long/nine-state titles, have exactly three stages.
families=[('1 1',2,1),('1 2',2,1),('1 3',3,1),('1 4',1,1),
          ('2',3,2),('3 9 F6',1,9),('4 9 F6',9,9)]
for entry,equations,dim in families:
    for suffix,n in [('',1),(' F6',2),(' F6 F6',3),(' F6 F6 EXIT',2),(' F6 F6 EXIT EXIT',1)]:
        out=run(entry+suffix);progress(out,n)
        if n<3:
            assert 'V-WIN' not in bar(out)
            assert plot(run(entry+suffix+' F3'))==plot(out)
            if n==2:assert plot(run(entry+suffix+' F1 F2 F3 F4 F5'))==plot(out)
    params=entry+' F6 F6 '
    assert bar(run(params))==['INIT','ADV','V-WIN','OUTPUT','SET','GRAPH']
    for side in ['F3','F4','F5','F2','F2 F1','F2 F2']:
        progress(run(params+side),0)
    assert plot(run(params+'F3 EXIT'))==plot(run(params))
    assert plot(run(params+'F2 EXIT'))==plot(run(params))
    assert metrics(run(params+('F2 F2 EXIT EXIT '*80)))==metrics(run(params))
    assert bar(run(params+'F2'))==['EVENT','INFO','','','','']
    assert plot(run(params+'F2 F3 F4 F5 F6'))==plot(run(params+'F2'))
    # SELECT only: cycles include visible rows, with paged high-dimensional lists.
    selectors=[(entry+' ',equations),(entry+' F6 ',dim+1),
        (params,7 if dim==1 and entry.startswith('1 ') else 6),
        (params+'DOWN DOWN RIGHT ',8 if entry.startswith('1 ') else 7),
        (params+'F3 ',7),(params+'F4 ',dim),
        (params+'F5 ',4 if entry.startswith('1 ') else 2),(params+'F2 F1 ',4)]
    for prefix,count in selectors:
        # RK45 prefix starts at Method row2; restore to the first visible row.
        if prefix.endswith('RIGHT '):prefix+='UP UP '
        assert plot(run(prefix+'UP'))==plot(run(prefix+'DOWN '*(count-1))),(prefix,count)
        assert plot(run(prefix+'UP DOWN'))==plot(run(prefix))
        assert plot(run(prefix+'DOWN '*count))==plot(run(prefix))
    progress(run(entry+' LEFT F2'),0) # Softkey-only FUNC hides workflow indicator.
    graph=params+'F6 '
    progress(run(graph),0);progress(run(graph+'F3'),0)
    assert 'TRACE' in bar(run(graph+'F3 EXIT'))
# EDIT UP at the first field retains old commit/clamp behavior; no cyclic cursor.
for prefix in ['2 ','2 F6 ','2 F6 F6 ','2 F6 F6 F3 ']:
    assert plot(run(prefix+'LEFT UP'))==plot(run(prefix+'LEFT EXIT'))
# Existing numerical/data navigation clamps or pans; it does not cycle list ends.
graph='2 F6 F6 F6 '
table=graph+'F4 '
assert plot(run(table+'F1 UP UP'))==plot(run(table+'F1'))
assert plot(run(table+'F2 DOWN DOWN'))==plot(run(table+'F2'))
assert plot(run(table+'LEFT '*10))==plot(run(table))
assert plot(run(graph+'UP'))!=plot(run(graph))
assert plot(run(graph+'UP DOWN'))==plot(run(graph))
# Context-specific Graph Settings exception: Style has no direct F1/F2 buttons.
style='1 4 F6 F6 F5 DOWN DOWN '
assert bar(run(style))==['INIT','','','','','DONE']
assert plot(run(style+'F1 F2'))==plot(run('1 4 F6 F6 F5'))
assert 'LEFT/RIGHT: SEGMENT/ARROW toggle' in tail(run(style))
assert 'Segment' in tail(run(style+'LEFT')) and 'Arrow' in tail(run(style+'LEFT RIGHT'))
assert 'RIGHT/F3: COLOR' in tail(run(style+'DOWN'))
assert 'Field color' in tail(run(style+'DOWN RIGHT'))
out=run('4 9 F6 F6 F6 F4 ')
assert 'LEFT/RIGHT: ON/OFF toggle, F3: COLOR' in tail(out)
assert 'ON/OFF: Graph, Table and CSV' not in out
print('Workflow polish: six Main entries, safe SAVE confirmation/failure/repeat, ADV no-work, all-mode progress and SELECT cycles, isolated data navigation passed.')
