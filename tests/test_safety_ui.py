"""Hardware feedback and solver safety through the production UI handlers."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys):
    with tempfile.TemporaryDirectory() as directory:
        p=subprocess.run([app],cwd=directory,env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,
            DIFFEQ_HOST_MAX_FRAMES='5000'),capture_output=True,text=True,timeout=30)
        assert p.returncode==0 and 'runtime error:' not in p.stderr,(p.stderr,p.stdout[-2000:])
        return p.stdout
def tail(out):return out[out.rfind('\nKEY '):]
def solves(out):return int(re.findall(r'solves=(\d+)',out)[-1])
def rows(out):return re.findall(r'TEXT 16 (?:69|86|103|120|137|154|171) ([^\n]+)',tail(out))
def bar(out):return re.findall(r'TEXT \d+ 206 ([^\n]*)',tail(out))[-6:]
# Actual requested Separable example, including repeated endpoint navigation.
sep='1 1 LEFT ACON F2 F2 XOT RIGHTP EXE F6 F6 F6 '
table=sep+'F4 '
for edge in ['F1','F2']:
    end=run(table+edge)
    assert 'END: Numerical limit' in tail(end) and 'Table end\n' not in end
    x=list(map(float,rows(end)))
    terminal=x[0] if edge=='F1' else x[-1]
    assert x and 4<abs(terminal)<5 and abs(x[0])>0
    assert rows(run(table+edge+' '+edge))==rows(end)
    assert rows(run(table+edge+' F3'))==rows(run(table))
    assert x==sorted(x)
normal=run('2 EXE EXE EXE F4 F2')
assert '| END' in tail(normal) and 'Numerical limit' not in tail(normal)
# Fixed x and only enabled states scroll.
system='4 4 F6 F6 F6 F6 F4 '
for moves,expected in [('',('y1','y2')),('RIGHT',('y2','y3')),('RIGHT RIGHT',('y3','y4'))]:
    out=tail(run(system+moves))
    assert 'TEXT 16 49 x\n' in out
    assert 'TEXT 141 49 '+expected[0]+'\n' in out and 'TEXT 266 49 '+expected[1]+'\n' in out
    assert 'Left/Right: columns' in out
assert 'Left/Right: columns' not in tail(run('2 EXE EXE EXE F4'))
# Re-entering after hiding states uses only the remaining columns.
out=tail(run(system+'RIGHT RIGHT EXIT EXIT F4 DOWN DOWN RIGHT DOWN RIGHT EXIT F6 F4'))
assert 'TEXT 16 49 x\n' in out and 'TEXT 141 49 y1\n' in out and 'TEXT 266 49 y2\n' in out
assert 'Left/Right: columns' not in out
# SELECT EXE completes at any field, and hidden VAR has no legacy popup fallback.
for entry in ['1 1','1 2','1 3','1 4','2','3 F6']:
    out=run(entry+' LEFT F1')
    assert 'Insert variable' not in out and 'VAR' not in bar(out)
for prefix in ['2','2 F6','2 F6 F6']:
    for move in ['', 'DOWN','DOWN DOWN']:
        assert re.findall(r'^PLOT (\w+)',run(prefix+' '+move+' EXE'),re.M)[-1]==re.findall(r'^PLOT (\w+)',run(prefix+' '+move+' F6'),re.M)[-1]
params='2 F6 F6 DOWN DOWN DOWN '
for h in ['0','NEG 1']:
    out=run(params+h+' EXE EXE')
    assert 'h must be finite and > 0.' in out and solves(out)==0
for h in ['0 DOT 0 0 0 1','1 EXP NEG 3 0 0']:
    out=run(params+h+' EXE F6 EXE')
    assert 'Too many steps; increase h / Max Steps' in out
    assert 'TEXT 14 9 Parameter\n' in tail(out) and solves(out)==0
assert solves(run(params+'0 DOT 0 0 1 EXE F6'))>0
assert solves(run('2 F6 F6 0 EXE 0 DOT 0 1 EXE DOWN 0 DOT 0 0 0 1 EXE F6'))>0
# TRACE follows both viewport edges; only actual beyond-cache moves integrate.
graph='2 EXE EXE EXE '
base=run(graph+'F1')
for direction in ['LEFT','RIGHT']:
    out=run(graph+'F1 '+(direction+' ')*310)
    xs=[float(x) for x in re.findall(r'IC1 x=([-+.\deE]+)',out)]
    assert abs(xs[-1])>10 and all((b-a)*(1 if direction=='RIGHT' else -1)>0 for a,b in zip(xs,xs[1:]))
    assert solves(out)>solves(base) and 'Partial: Cancelled' not in out
    # Switch/blink retain x and the auto-followed viewport.
    switched=run(graph+'F1 '+(direction+' ')*310+'DOWN BLINK BLINK')
    assert re.findall(r'IC1 x=([-+.\deE]+)',switched)[-1]==re.findall(r'IC1 x=([-+.\deE]+)',out)[-1]
manual='2 F6 F6 NEG 2 EXE 2 EXE DOWN 0 DOT 0 5 EXE F6 F1 '
exact=run(manual+'RIGHT '*310+'F1 1 0 EXE')
assert 'IC1 x=10 ' in tail(exact)
out=run(manual+'RIGHT '*310+'EXIT EXIT')
assert all(s in tail(out) for s in ['TEXT 144 35 -2\n','TEXT 144 57 2\n','TEXT 144 101 0.05\n'])
for direction in ['LEFT','RIGHT','UP','DOWN']:
    out=run(graph+'F2 '+direction)
    assert bar(out)==['IN','OUT','AUTO','ORIG','BOX','']
    assert solves(out)>solves(run(graph))
    assert bar(run(graph+'F2 '+direction+' EXIT'))==['TRACE','ZOOM','V-WIN','TABLE','G-SLV','INIT']
print('Table limit/TOP/BTM/MID/frozen x, form EXE/hidden VAR, preflight, TRACE auto-follow and ZOOM arrows passed.')
