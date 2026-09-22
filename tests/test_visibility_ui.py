"""Approved per-IC output and temporary G-Solve prompt contracts, actual handlers."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
BASE=['TRACE','ZOOM','V-WIN','TABLE','G-SLV','INIT']
PAGE=['Y-CAL','X-CAL','','','','<']
def run(keys):
    with tempfile.TemporaryDirectory() as directory:
        p=subprocess.run([app],cwd=directory,env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_MAX_FRAMES='2000'),capture_output=True,text=True,timeout=15)
        assert p.returncode==0 and 'SCRIPT COMPLETE' in p.stdout and 'runtime error:' not in p.stderr,(keys,p.stderr,p.stdout[-1000:])
        return p.stdout
def tail(s):return s[s.rfind('\nKEY '):]
def bar(s):return re.findall(r'TEXT \d+ 206 ([^\n]*)',s)[-6:]
def last(s,name):return re.findall(r'^'+name+r' (\w+)',s,re.M)[-1]
def work(s):return re.findall(r'METRICS solves=(\d+) searches=(\d+)',s)[-1]
scalar='1 4 1 EXE F6 DOWN 0 EXE F6 F6 '
five='1 4 0 EXE F6 DOWN S:MUL 1 COMMA 2 COMMA 3 COMMA 4 COMMA 5 S:DIV EXE F6 '
subset=five+'F4 RIGHT DOWN DOWN RIGHT DOWN RIGHT F6 F6 '
for graph in [scalar,subset]:
    page=graph+'F5 F6 ';before=run(page)
    for op in ['F1','F2']:
        prompt=page+op+(' EXE ' if graph==subset else ' ')
        opened=run(prompt)
        for draft in ['', 'NEG ', '1 DOT ', '1 EXP ', '2 DOT 5 ', 'EXE ', 'NEG EXE ']:
            state=run(prompt+draft)
            cancelled=run(prompt+draft+'EXIT R:EXIT R:EXIT')
            assert bar(cancelled)==PAGE and 'Invalid number' not in tail(cancelled)
            assert 'Partial: Cancelled' not in cancelled
            assert work(cancelled)==work(state)==work(opened)
            assert last(cancelled,'PLOT')==last(before,'PLOT')
            assert last(cancelled,'REPORT')==last(before,'REPORT')
            assert bar(run(prompt+draft+'EXIT R:EXIT EXIT'))==BASE
        invalid=run(prompt+'EXE');assert 'Invalid number' in tail(invalid)
        assert 'Invalid number' in tail(run(prompt+'NEG EXE'))
        # F6 no longer advertises or commits a temporary numerical prompt.
        uncommitted=run(prompt+'2 DOT 5 F6');assert work(uncommitted)==work(opened)
        assert bar(uncommitted)==['']*6
assert 'X=0.5' in tail(run(scalar+'F5 F6 F1 0 DOT 5 EXE'))
assert 'Y=0.5' in tail(run(scalar+'F5 F6 F2 0 DOT 5 EXE'))
# The sparse mapping is IC2 then IC5, never hidden IC1/3/4.
assert 'IC2 x=0 y=2' in tail(run(subset+'F1'))
assert 'IC5 x=0 y=5' in tail(run(subset+'F1 DOWN'))
assert 'IC2 x=0 y=2' in tail(run(subset+'F1 DOWN DOWN'))
assert 'Y=2' in tail(run(subset+'F5 F6 F1 EXE 0 EXE'))
assert 'Y=5' in tail(run(subset+'F5 F6 F1 DOWN EXE 0 EXE'))
assert 'ICPT: Not found' in tail(run(subset+'F5 F5'))
# A single remaining family skips selection; fewer than two cannot do ICPT.
one=five+'F4 RIGHT DOWN RIGHT DOWN RIGHT DOWN RIGHT F6 F6 '
assert 'SELECT GRAPH' not in run(one+'F5 F6 F1')
assert 'Y=5' in tail(run(one+'F5 F6 F1 0 EXE'))
assert 'ICPT: Not available' in tail(run(one+'F5 F5'))
all_off=five+'F4 '+('RIGHT DOWN '*5)+'F6 F6 '
assert 'No visible graph' in tail(run(all_off+'F1'))
assert bar(run(all_off+'F1 EXIT'))==BASE
assert 'ROOT: Not available' in tail(run(all_off+'F5 F1'))
assert 'ICPT: Not available' in tail(run(all_off+'F5 F5'))
assert re.findall(r'TEXT 16 49 ([^\n]+)',tail(run(all_off+'F4')))==['x']
# A visibility-only edit is an input preference, just like an existing color edit.
# The dimension-change guard must not silently reset it with untouched equations/ICs.
changed='1 4 F6 F6 F4 RIGHT F6 EXIT EXIT EXIT EXIT 3 F6 '
assert 'Replace equation and ICs with defaults?' in tail(run(changed))
assert 'Order (1-9)' in tail(run(changed+'EXIT'))
# Long output lists expose position without changing wrap, INIT, or EXE policy.
ten='1 4 F6 DOWN S:MUL '+(' COMMA '.join(' '.join(str(i)) for i in range(1,11)))+' S:DIV EXE F6 F4 '
for keys,position in [('',1),('UP',10),('UP DOWN',1),('DOWN '*7,8),('UP F1',1)]:
    result=tail(run(ten+keys))
    assert re.search(r'TEXT \d+ 9 '+str(position)+r' of 10',result)
assert not re.search(r'TEXT \d+ 9 \d+ of ',tail(run(five+'F4')))
print('Approved UI decisions: sparse IC mapping, all-OFF safety, single-curve skip, X/Y-CAL empty/partial/valid/error EXIT, exact page/report/pixel preservation, HOLD boundary and EXE-only validation PASS.')
