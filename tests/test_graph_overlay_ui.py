"""Production overlay/navigation invariants, with bounded host clock/poll fixtures."""
import math,os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys,pixels=False):
 with tempfile.TemporaryDirectory() as folder:
  env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_MAX_FRAMES='2000',DIFFEQ_HOST_TICK_LIMIT='128')
  if pixels:env['DIFFEQ_HOST_OUT']=folder
  p=subprocess.run([app],cwd=folder,env=env,capture_output=True,text=True,timeout=30)
  assert p.returncode==0 and 'SCRIPT COMPLETE' in p.stdout and 'runtime error:' not in p.stderr,(keys,p.stderr,p.stdout[-2000:])
  if pixels:return p.stdout,sorted(Path(folder).glob('*.ppm'))[-1].read_bytes().split(b'\n',3)[3]
  return p.stdout
def tail(out):return next(s for s in reversed(out.split('\nKEY ')) if 'TEXT ' in s)
def last(out,word):return re.findall(r'^'+word+r' (\w+)',out,re.M)[-1]
def solves(out):return int(re.findall(r'solves=(\d+)',out)[-1])
def bar(out):return re.findall(r'TEXT \d+ 206 ([^\n]*)',out)[-6:]
def window(out):return [float(v) for v in re.findall(r'TEXT 144 (?:35|57|79|101|123|145|167) ([-+0-9.eE]+)\n',tail(out))]
def point(out):return tuple(map(float,re.findall(r'X=([-+.\deE]+)  Y=([-+.\deE]+)',out)[-1]))
base='2 F6 F6 F6 '
GS=['ROOT','MAX','MIN','Y-ICPT','ICPT','>']
BASE=['TRACE','ZOOM','V-WIN','TABLE','G-SLV','INIT']
ops=['F1','F2','F3','F4','F6 F1','F6 F2']
for op in ops:
 menu=base+'F5 ';prior=run(menu)
 selected=run(menu+op)
 assert 'TEXT 13 8 UP/DOWN: SELECT GRAPH, ' in tail(selected)
 assert 'TEXT 183 8 EXE\nTEXT 208 8 : SELECT' in tail(selected)
 assert solves(selected)==solves(prior) and last(selected,'REPORT')==last(prior,'REPORT')
 out=run(menu+op+' BLINK DOWN BLINK UP EXIT R:EXIT R:EXIT')
 expected=GS if 'F6' not in op else ['Y-CAL','X-CAL','','','','<']
 assert bar(out)==expected and last(out,'PLOT')==last(prior,'PLOT')
 assert solves(out)==solves(prior) and last(out,'REPORT')==last(prior,'REPORT')
 assert 'Partial: Cancelled' not in out
 assert bar(run(menu+op+' EXIT R:EXIT EXIT'))==BASE
# ICPT with >2 curves has two stages; both use the identical message/channel.
multi='3 3 F6 F6 F6 F6 F5 '
prior=run(multi)
for suffix in ['F5','F5 EXE']:
 selected=run(multi+suffix);assert 'TEXT 13 8 UP/DOWN: SELECT GRAPH, ' in tail(selected)
 out=run(multi+suffix+' EXIT R:EXIT')
 assert bar(out)==GS and last(out,'PLOT')==last(prior,'PLOT')
 assert solves(out)==solves(prior) and last(out,'REPORT')==last(prior,'REPORT')
# A single graph skips selection. Result EXIT closes one layer only.
scalar='1 4 1 F6 DOWN 0 F6 F6 '
assert 'SELECT GRAPH' not in run(scalar+'F5 F4')
for op in ['F1 EXE','F2 EXE','F3 EXE','F4 EXE','F5','F6 F1 EXE 0 EXE','F6 F2 EXE 0 EXE']:
 shown=run(base+'F5 '+op);assert 'X=' in shown or 'Not found' in shown
 out=run(base+'F5 '+op+' EXIT R:EXIT R:EXIT')
 expected=GS if not op.startswith('F6') else ['Y-CAL','X-CAL','','','','<']
 assert bar(out)==expected and 'Partial: Cancelled' not in out
 assert solves(out)==solves(shown) and last(out,'REPORT')==last(run(base),'REPORT')
 assert bar(run(base+'F5 '+op+' EXIT R:EXIT EXIT'))==BASE
# Temporary instructions hide a persistent warning and restore its exact pixels.
domain='1 4 S:SQUARE 1 SUB XOT RIGHTP F6 DOWN S:MUL 0 COMMA 1 S:DIV EXE F6 F6 '
prior=run(domain+'F5');out=run(domain+'F5 F3 EXIT')
assert last(out,'PLOT')==last(prior,'PLOT') and last(out,'REPORT')==last(prior,'REPORT')
assert 'END: Math domain / singularity' in prior and 'Partial: Cancelled' not in out
# Set a result point under the footer, then inspect the unchanged result and
# exact full9px marker above it. Cycling results cannot invoke the solver again.
params='2 F6 F6 '
small=params+'F3 NEG 6 EXE 6 EXE 1 EXE DOWN NEG 0 DOT 0 1 EXE 1 EXE 0 DOT 2 EXE F6 F6 '
entry=window(run(small+'F3'))
for op in ['F1 EXE','F2 EXE','F3 EXE','F5']:
 shown,rgb=run(small+'F5 '+op,True)
 x,y=point(shown)
 v=window(run(small+'F5 '+op+' EXIT EXIT F3'))
 assert v[:4]==entry[:4] and math.isclose(v[5]-v[4],entry[5]-entry[4],rel_tol=1e-8) and v[6]==entry[6]
 assert v[4:6]!=entry[4:6]
 px=6+int((x-v[0])/(v[1]-v[0])*383+.5);py=4+197-int((y-v[4])/(v[5]-v[4])*197+.5)
 assert 7<=py-4<=171 and rgb[((py+2)*396+px)*3:((py+2)*396+px)*3+3]==b'\0\0\0',(op,v,(px,py))
 cycled=run(small+'F5 '+op+' RIGHT RIGHT LEFT')
 assert solves(cycled)==solves(shown) and last(cycled,'REPORT')==last(shown,'REPORT')
 v2=window(run(small+'F5 '+op+' RIGHT RIGHT LEFT EXIT EXIT F3'))
 assert v2[:4]==entry[:4] and math.isclose(v2[5]-v2[4],entry[5]-entry[4],rel_tol=1e-8)
# Already visible markers do not move the window at all.
assert window(run(base+'F5 F1 EXE EXIT EXIT F3'))==window(run(base+'F3'))
# Shared factory reset after pan/zoom/BOX, including custom entry and MAN ranges.
for p in [base,small,'4 2 F6 F6 F6 F6 F4 F2 ']:
 expected=window(run(p+'F3 F1 F6 F3'))
 for change in ['RIGHT UP ','F2 F1 EXIT ','F2 F5 LEFT UP EXE '+'RIGHT '*8+'DOWN '*8+'EXE ']:
  assert window(run(p+change+'F6 F3'))==expected
# Delayed busy lifecycle: clock fixtures advance per poll, cap frames, and are
# reset immediately before the relevant operation; no physical timing claim.
assert 'CALCULATING...' not in run(base+'TICKS:0 F1')
for suffix,label in [('F1','CALCULATING...'),('F4','Preparing Table...')]:
 out=run(base+'TICKS:8 '+suffix)
 phases=re.findall(re.escape(label)+r' ([/\\|\-])',out)
 assert phases[:4]==['/','-','\\','|'],(suffix,phases)
 assert label not in tail(out).split('FRAME')[-1]
 assert 'IC1 x=' in tail(out) if suffix=='F1' else 'STAT' in tail(out)
 prior=run(base)
 cancelled=run(base+'TICKS:8 CANCEL:12 '+suffix)
 assert label in cancelled and bar(cancelled)==BASE
 assert 'Partial: Cancelled' not in cancelled and last(cancelled,'PLOT')==last(prior,'PLOT')
 assert last(cancelled,'REPORT')==last(prior,'REPORT')
slow='2 F6 F6 DOWN DOWN RIGHT F6 '
prior=run(slow)
for op in ['F1 EXE','F2 EXE','F3 EXE','F5']:
 cancelled=run(slow+'TICKS:8 CANCEL:12 F5 '+op)
 assert 'CALCULATING...' in cancelled and bar(cancelled)==GS
 assert 'Partial: Cancelled' not in cancelled and last(cancelled,'PLOT')==last(prior,'PLOT')
 assert last(cancelled,'REPORT')==last(prior,'REPORT')
# Genuine main-trajectory cancellation still reports Partial: Cancelled.
out=run('2 F6 F6 TICKS:8 CANCEL:12 F6')
assert 'Drawing...' in out and 'Partial: Cancelled' in out
out=run('2 F6 F6 TICKS:8 F6');assert 'Drawing...' in out and bar(out)==BASE
assert 'Drawing...' not in run('2 F6 F6 TICKS:0 F6')
print('Graph overlay UI: all seven G-Solve routes, exact cancel restore/report, one EXIT, Y-only result visibility, shared INIT and delayed/cancel-safe busy PASS.')
