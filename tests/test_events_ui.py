"""Production Event/SOLVE/Info UI and navigation; no calculator emulation."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys,directory=None):
    if directory is None:
        with tempfile.TemporaryDirectory() as temp:return run(keys,temp)
    p=subprocess.run([app],cwd=directory,env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,
        DIFFEQ_HOST_MAX_FRAMES='5000'),capture_output=True,text=True,timeout=30)
    assert p.returncode==0 and 'SCRIPT COMPLETE' in p.stdout and 'runtime error:' not in p.stderr,(p.stderr,p.stdout[-1500:])
    return p.stdout
def tail(out):return out[out.rfind('\nKEY '):]
def bar(out):return re.findall(r'TEXT \d+ 206 ([^\n]*)',tail(out))[-6:]
def metrics(out):return re.findall(r'METRICS ([^\n]+)',out)[-1]
params='1 4 A:SUB EXE F6 DOWN 1 EXE F6 '
assert bar(run(params))==['PREV','SOLVE','V-WIN','OUTPUT','SET','GRAPH']
assert bar(run(params+'F2'))==['EVENT','INFO','','INIT','','']
assert metrics(run(params))==metrics(run(params+('F2 F2 EXIT EXIT '*100)))
assert 'No solver run yet' in run(params+'F2 F2')
event=params+'F2 F1 '
assert bar(run(event))==['','','','','','DONE']
assert all(v in tail(run(event)) for v in ['Enabled','OFF','Direction','ANY','Action','MARK'])
# OFF with empty E and edit/toggle/menu visits do no numerical work or I/O.
assert metrics(run(event))==metrics(run(event+('RIGHT LEFT '*100)))
assert 'FUNC' in bar(run(event+'DOWN LEFT')) and 'VAR' not in bar(run(event+'DOWN LEFT'))
draft=event+'DOWN A:SUB SUB 1 0 '
assert tail(run(draft+'F2 EXIT')).count('y-10')>=1
assert 'Check Event E' in run(draft+'ADD EXE')
for entry in ['2','3 9 F6','4 9 F6']:
    screen=entry+' F6 F6 F2 F1 DOWN LEFT '
    assert bar(run(screen))[:2]==['VAR','FUNC']
    assert 'y1' in run(screen+'F1 F1 EXE')
invalid=event+'RIGHT F6 EXIT F6 '
assert 'Event E (including IC)' in run(invalid) and 'solves=0' in metrics(run(invalid))
# Event-only edits count as user input in the existing equation-size guard.
resize='3 3 F6 F6 F6 F2 F1 DOWN A:SUB EXE F6 EXIT EXIT EXIT EXIT 2 F6 '
assert 'Change Equation Size' in tail(run(resize))
assert 'y' in tail(run(resize+'F5 F6 F6 F6 F2 F1'))
# The same definition works under both methods; INFO preserves its snapshot.
for method in ['', 'DOWN DOWN RIGHT ']:
    base=params+method
    setting=base+'F2 F1 RIGHT DOWN A:SUB SUB 1 0 EXE RIGHT DOWN RIGHT '
    out=tail(run(setting));assert 'RISING' in out and 'STOP' in out
    graph=setting+'F6 EXIT F6 '
    assert 'END: Event' in tail(run(graph))
    assert 'END: Event' in tail(run(graph+'F4 F2'))
    assert 'END: Event' in tail(run(graph+'F1 F6 RIGHT RIGHT'))
    info=graph+'F6 F2 F2 '
    initial=run(info);assert 'Solver Info' in initial and 'EVENT' in tail(initial)
    assert metrics(initial)==metrics(run(info+('DOWN '*40)+('UP '*40)+('EXIT F2 '*50)))
    end=tail(run(info+'DOWN '*40));assert '1 / 32' in end and 'Stopped x' in end
    work=run(info+'DOWN '*7);assert 'RHS evals' in tail(work)
    if method:assert 'Rejected' in work and 'Attempts' in work
    else:assert 'Rejected' not in work and 'Steps' in work
    # Explicit SAVE/RCL; v10 config survives, runtime report does not.
    with tempfile.TemporaryDirectory() as directory:
        run(setting+'F6 EXIT '+('EXIT '*5)+'F6 EXE',directory)
        loaded=run('F5 2 F1 F6 F6 F2 F1',directory)
        assert all(x in tail(loaded) for x in ['y-10','RISING','STOP','ON'])
        assert 'No solver run yet' in run('F5 2 F1 F6 F6 F2 F2',directory)
# INIT moved into SOLVE and retains the chosen method.
reset=tail(run(params+'DOWN DOWN RIGHT DOWN 2 EXIT F2 F4'))
assert 'RK45' in reset and 'TEXT 144 101 0.1\n' in reset
print('Event UI: SOLVE/INIT, expression/FUNC/VAR, validation, STOP Graph/Table/TRACE, read-only INFO, SAVE/RCL and navigation stress passed.')
