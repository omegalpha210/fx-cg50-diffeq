"""Real key/renderer contracts for BOX, reset, drafts and nonfatal graph status."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys,ticks=None):
 with tempfile.TemporaryDirectory() as directory:
  env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_MAX_FRAMES='10000')
  if ticks is not None:env.update(DIFFEQ_HOST_TICK_STEP=str(ticks),DIFFEQ_HOST_TICK_LIMIT='64')
  p=subprocess.run([app],cwd=directory,env=env,capture_output=True,text=True,timeout=30)
  assert p.returncode==0 and 'SCRIPT COMPLETE' in p.stdout and 'runtime error:' not in p.stderr,(keys,p.stderr,p.stdout[-2000:])
  return p.stdout
def tail(out):return next(p for p in reversed(out.split('\nKEY ')) if 'TEXT ' in p)
def plot(out):return re.findall(r'^PLOT (\w+)',out,re.M)[-1]
def solves(out):return int(re.findall(r'solves=(\d+)',out)[-1])
def window(out):
 return [float(v) for v in re.findall(r'TEXT 144 (?:35|57|79|101|123|145|167) ([-+0-9.eE]+)\n',tail(out))]
plain='1 4 F6 F6 F6 '
sys='4 2 F6 F6 F6 F6 '
phase=sys+'F4 F2 '
for p in [plain,'2 F6 F6 F6 ','3 3 F6 F6 F6 F6 ','4 3 F6 F6 F6 F6 ']:
 assert not re.search(r'TEXT \d+ 8 (?:TIME|PHASE)\n',tail(run(p)))
 assert 'INIT\n' in tail(run(p))
assert re.search(r'TEXT \d+ 8 TIME\n',tail(run(sys)))
assert re.search(r'TEXT \d+ 8 PHASE\n',tail(run(phase)))
event='1 4 F6 F6 F2 F1 RIGHT DOWN XOT F6 EXIT F6 '
assert re.search(r'TEXT \d+ 8 EVT\n',tail(run(event)))
# F1 remains active on every selectable Settings row, including Style/Color.
settings='1 4 F6 F6 F5 '
for n in range(4):
 assert 'INIT\n' in tail(run(settings+'DOWN '*n))
 assert plot(run(settings+'DOWN '*n+'LEFT F1'))==plot(run(settings))
# Drafts are allowed on edit exit/row move; only the primary NEXT blocks.
for p in ['1 4','2','3 9 F6','4 9 F6']:
 assert 'Syntax error' not in run(p+' SIN EXE EXIT')
 bad=run(p+' SIN F6')
 assert 'Syntax error' in tail(bad) and 'INIT\n' in tail(bad) and '1/3' in tail(bad)
 assert 'Initial Conditions' not in bad
 assert plot(run(p+' SIN F6 F1'))==plot(run(p))
 assert 'VAR\n' not in tail(run(p))
 assert ('VAR\n' in tail(run(p+' LEFT')))==p.startswith(('3','4'))
 if p.startswith(('3','4')):
  assert plot(run(p+' LEFT F3 EXIT'))==plot(run(p+' LEFT'))
for p in ['1 4 F6','2 F6','3 9 F6 F6','4 9 F6 F6']:
 assert 'Syntax error' not in run(p+' LEFT ACON EXIT EXIT')
 bad=run(p+' LEFT ACON F6')
 assert 'Syntax error' in tail(bad) and '2/3' in tail(bad) and 'INIT\n' in tail(bad)
 assert 'Parameter\n' not in bad
 assert plot(run(p+' LEFT ACON F6 F1'))==plot(run(p))
 # Blank draft survives going back to Equation and forward to IC.
 back=run(p+' LEFT ACON EXIT EXIT F6')
 assert 'Initial Conditions' in tail(back) and 'TEXT 144 35 \n' in tail(back)
# First invalid field, not the currently selected last field, receives focus.
bad=tail(run('2 SIN EXE DOWN F6'))
assert 'f(x): Syntax error' in bad and 'TEXT 144 57 sin(' in bad
ic=tail(run('2 F6 SIN EXE DOWN F6'))
assert 'Syntax error' in ic and 'TEXT 144 35 sin(' in ic
# Stage drafts never leak into a numerical calculation; fixing/INIT makes it usable.
assert solves(run('2 F6 SIN EXE F6'))==0
assert solves(run('2 F6 SIN EXE F6 F1 F6 F6'))>0
# BOX cancellation is byte-exact for graph pixels and performs no ODE solve.
for p in [plain,phase,event]:
 zoom=p+'F2 ';base=run(zoom)
 for suffix in ['F5 EXIT','F5 LEFT UP EXIT','F5 EXE '+('RIGHT '*20)+('DOWN '*10)+'EXIT']:
  out=run(zoom+suffix);assert plot(out)==plot(base) and solves(out)==solves(base),(p,suffix)
 assert 'Point 1' in tail(run(zoom+'F5 R:EXE'))
 assert 'Point 2' in tail(run(zoom+'F5 EXE'))
 assert 'BOX TOO SMALL' in tail(run(zoom+'F5 EXE EXE'))
 assert window(run(zoom+'F5 EXE EXE EXIT EXIT F3'))==window(run(p+'F3'))
# Reverse corners produce the identical V-window; neither h nor Xdot controls pixels.
box='F2 F5 '+('LEFT '*20)+('UP '*10)+'EXE '+('RIGHT '*40)+('DOWN '*20)+'EXE '
reverse='F2 F5 '+('RIGHT '*20)+('DOWN '*10)+'EXE '+('LEFT '*40)+('UP '*20)+'EXE '
assert window(run(plain+box+'F3'))==window(run(plain+reverse+'F3'))
assert window(run(plain+box+'F3'))!=window(run(plain+'F3'))
for p in [plain,phase]:
 zoomed=run(p+box);reset=run(p+box+'F6')
 assert solves(reset)==solves(zoomed)
 assert window(run(p+box+'F6 F3'))==window(run(p+'F3'))
 assert window(run(p+box+'F2 F4 EXIT F3'))==window(run(p+'F2 F4 EXIT F3'))
assert 'PHASE\n' in tail(run(phase+box+'F6'))
# Manual solver bounds remain manual after BOX and Graph INIT; custom entry != ORIG.
manual='1 4 F6 F6 NEG 4 EXE 4 EXE F6 '
assert 'MAN\n' in tail(run(manual+box+'F6 EXIT'))
assert 'TEXT 144 35 -4\n' in tail(run(manual+box+'F6 EXIT'))
custom='1 4 F6 F6 F3 NEG 5 EXE 5 EXE F6 F6 '
assert window(run(custom+box+'F6 F3'))==window(run(custom+'F3'))
assert window(run(custom+box+'F2 F4 EXIT F3'))!=window(run(custom+'F3'))
# Deterministic clock drives the same production busy callback; teardown on results.
for p,op in [('2 F6 F6 F6 ','F5'),('2 F6 F6 DOWN DOWN RIGHT F6 ','F5'),(sys,'F5')]:
 out=run(p+'F5 '+op,ticks=8)
 assert 'CALCULATING...' in out and 'CALCULATING...' not in re.split(r'^FRAME .*$',out,flags=re.M)[-2]
assert 'CALCULATING...' not in run('1 4 1 F6 DOWN 0 F6 F6 F5 F1',ticks=0)
not_found=run('1 4 0 F6 F6 F6 F5 F1',ticks=8)
assert 'CALCULATING...' in not_found and 'Not found' in tail(not_found)
assert 'CALCULATING...' not in re.split(r'^FRAME .*$',not_found,flags=re.M)[-2]
# Existing partial domains stay nonfatal: valid plots, TRACE and G-Solve are usable.
for expression in ['S:SQUARE 1 SUB XOT RIGHTP','1 DIV LEFTP XOT SUB 1 RIGHTP','LEFT ACON F2 F2 A:SUB RIGHTP']:
 p='1 4 '+expression+' F6 F6 '+('DOWN DOWN DOWN 0 DOT 2 5 EXE F6 ' if expression.startswith('1 DIV') else 'F6 ')
 out=run(p);assert 'END:' in tail(out) or 'Partial:' in tail(out),(p,tail(out))
 assert 'IC1 x=' in tail(run(p+'F1'))
 out=run(p+'F5 F4');assert 'Y-ICPT' in tail(out) and 'X=0' in tail(out)
print('Interaction UI: conditional labels, all-row INIT, draft/NEXT/focus, VAR, BOX/cancel/reverse/windows, Graph INIT/ORIG, delayed busy and valid-domain operations passed.')
