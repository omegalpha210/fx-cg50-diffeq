"""No integration on TRACE movement/blink; plot is transactional on exit."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys):
 with tempfile.TemporaryDirectory() as directory:
  p=subprocess.run([app],cwd=directory,env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,
   DIFFEQ_HOST_MAX_FRAMES='5000'),capture_output=True,text=True,timeout=30)
  assert p.returncode==0 and 'runtime error:' not in p.stderr,(p.stderr,p.stdout[-2000:])
  assert 'Partial: Cancelled' not in p.stdout
  return p.stdout
def lastplot(out): return re.findall(r'^PLOT (\w+)',out,re.M)[-1]
def solves(out): return int(re.findall(r'solves=(\d+)',out)[-1])
graph='2 F6 F6 F6 '
base=run(graph)
enter=run(graph+'F1 ')
for direction in ['LEFT','RIGHT']:
 out=run(graph+'F1 '+direction+' '+('R:'+direction+' ')*50+'EXIT')
 assert solves(out)==solves(enter) and lastplot(out)==lastplot(base)
 tail=out[out.rfind('KEY EXIT'):]
 assert 'TRACE' in tail and 'Drawing...' not in tail
blink=run(graph+'F1 '+'BLINK '*100+'EXIT')
assert solves(blink)==solves(enter) and lastplot(blink)==lastplot(base)
switch=run(graph+'F1 '+'DOWN UP '*100+'EXIT')
assert lastplot(switch)==lastplot(base)
soak=run(graph+('F1 LEFT RIGHT EXIT '*100))
assert lastplot(soak)==lastplot(base)
assert 'CURVE-' not in soak and 'CURVE+' not in soak and 'x=' in soak
# Graph auto range keeps extending beyond the original domain; trace does not pan.
pan=run(graph+'RIGHT '*8+'OPTN 6')
assert 'X: 13.86 to 26.46' in pan
assert solves(pan)>solves(base)
print('TRACE: 50 held repeats both ways, 100 blink/switch/entry cycles; zero move solves, exact restored plot, pan extension passed.')
