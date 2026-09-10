"""Actual app Output rows reflect shared visibility and independent IC colors."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys):
    with tempfile.TemporaryDirectory() as directory:
        env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_MAX_FRAMES='1000')
        result=subprocess.run([app],cwd=directory,env=env,capture_output=True,text=True,timeout=10)
        assert result.returncode==0 and 'runtime error:' not in result.stderr,(result.stderr,result.stdout[-2000:])
        assert 'SCRIPT COMPLETE' in result.stdout
        return result.stdout[result.stdout.rfind('\nKEY '):]
def rows(out):return re.findall(r'^TEXT 20 \d+ ([^\n]*)',out,re.M)
def states(out):return re.findall(r'^TEXT 144 \d+ ([^\n]*)',out,re.M)
def plot(out):return re.findall(r'^PLOT (\w+)',out,re.M)[-1]
def entry(count):
    values=' COMMA '.join(' '.join(str(i)) for i in range(1,count+1))
    if count>1:values='S:MUL '+values+' S:DIV'
    return '1 4 F6 DOWN '+values+' EXE F6 F4 '
for count in (1,2,5,10):
    prefix=entry(count);base=run(prefix)
    if count==1:
        assert rows(base)==['y'] and states(base)==['ON']
        assert 'ON/OFF toggle, F3: COLOR' in base
        continue
    assert rows(base)==['y (all ICs)']+[f'IC{i} y' for i in range(1,min(count,6)+1)]
    assert states(base)==['ON']+['']*min(count,6)
    assert 'LEFT/RIGHT: ON/OFF for all ICs' in base
    assert plot(run(prefix+'F3'))==plot(base) # No color action on the visibility row.
    seen=set()
    for family in range(count):
        selected=prefix+'DOWN '*(family+1);out=run(selected)
        assert f'IC{family+1} y' in rows(out)
        seen.update(label for label in rows(out) if label.startswith('IC'))
        assert 'F3: COLOR' in out and 'ON/OFF' not in out
        assert plot(run(selected+'LEFT RIGHT'))==plot(out)
        assert plot(run(selected+'F3 LEFT UP EXIT'))==plot(out) # Cancel preserves preview and focus.
        assert max(map(int,re.findall(r'^TEXT 20 (\d+)',out,re.M)))<=167
    assert seen=={f'IC{i} y' for i in range(1,count+1)}
    assert plot(run(prefix+'UP'))==plot(run(prefix+'DOWN '*count))
    assert plot(run(prefix+'UP DOWN'))==plot(base)
    off=run(prefix+'RIGHT');assert states(off)[0]=='OFF'
    assert plot(run(prefix+'RIGHT F1'))==plot(base)
for prefix,last in [('2 F6 F6 F4 ',"y'"),('3 9 F6 F6 F6 F4 ','y(8)'),('4 9 F6 F6 F6 F4 ','y9')]:
    out=run(prefix+'UP')
    assert rows(out)[-1]==last and 'ON/OFF toggle, F3: COLOR' in out
    assert all(not name.startswith('IC') for name in rows(out))
print('Output instances UI: truthful 1/2/5/10 color rows, shared y visibility, ten-row reachability/wrap, palette cancellation, INIT and higher/system components passed.')
