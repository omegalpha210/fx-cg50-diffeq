"""Native UI conventions, real framebuffer geometry and no-work navigation."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys,images=False,directory=None):
    if directory is None:
        with tempfile.TemporaryDirectory() as temp:return run(keys,images,temp)
    env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_MAX_FRAMES='10000')
    if images:env['DIFFEQ_HOST_OUT']=directory
    p=subprocess.run([app],cwd=directory,env=env,capture_output=True,text=True,timeout=30)
    assert p.returncode==0 and 'SCRIPT COMPLETE' in p.stdout and 'runtime error:' not in p.stderr,(keys,p.stderr,p.stdout[-2200:])
    if images:
        raw=sorted(Path(directory).glob('*.ppm'))[-1].read_bytes().split(b'\n',3)[3]
        assert len(raw)==396*224*3
        return p.stdout,raw
    return p.stdout
def tail(out):return next(p for p in reversed(out.split('\nKEY ')) if 'TEXT ' in p)
def bar(out):
    keys=['']*6;previous=5
    for x,text in reversed(re.findall(r'TEXT (\d+) 206 ([^\n]*)',tail(out))):
        slot=(int(x)-6)//64
        if slot>previous:break
        if 0<=slot<6:keys[slot]=text+keys[slot];previous=slot
    return keys
def metrics(out):return re.findall(r'METRICS [^\n]*',out)[-1]
def plot(out):return re.findall(r'^PLOT (\w+)',out,re.M)[-1]
def rgb(c):return bytes([(c>>11&31)*255//31,(c>>5&63)*255//63,(c&31)*255//31])
def pixel(data,x,y):return data[(y*396+x)*3:(y*396+x)*3+3]
main,data=run('',True)
assert 'EXE' not in tail(main) and 'TEXT 14 188 MENU\n' in main
assert 'TEXT 47 188 : return to MAIN MENU\n' in main
colors={pixel(data,x,y) for y in range(184,201) for x in range(6,390)}
assert rgb(0xf800) in colors and rgb(0x001f) not in colors
# Shared tile borders, clear horizontal/vertical gutters, same six shortcuts.
assert all(pixel(data,x,152)==rgb(0xffff) for x in range(6,390))
assert plot(run('DOWN DOWN DOWN'))==plot(run(''))
for number in range(1,7):
    moves='DOWN '*((number-1)//2)+'RIGHT '*((number-1)%2)
    assert plot(run(str(number)))==plot(run(moves+'F6'))==plot(run(moves+'EXE'))
for entry in ['1 1','1 2','1 3','1 4','2','3 9 F6','4 9 F6']:
    assert 'EXE' not in tail(run(entry))
    assert 'EXE' not in tail(run(entry+' F6'))
    edit=tail(run(entry+' LEFT'))
    assert 'TEXT 14 188 EXE\n' in edit and ': commit / next   EXIT: commit' in edit
_,edit=run('1 4 LEFT',True)
assert all(pixel(edit,144,y)==rgb(0xffff) for y in range(56,70)) # Value-column caret.
_,selected=run('1 4',True)
assert any(pixel(selected,144,y)!=rgb(0xffff) for y in range(56,70))
error=run('1 4 SIN F6')
assert 'Syntax error @5' in tail(error) and 'Check equation' not in error
assert 'DIFF EQ / General 1st' in re.findall(r'FRAME \d+ ([^\n]*)',error)[-1]
assert bar(error)==['INIT','FUNC','','','','NEXT']
assert 'sin(' in tail(run('1 4 SIN F6'))
params='2 F6 F6 '
assert tail(run(params)).count('AUTO\n')==2 and 'TEXT 20 101 h\n' in tail(run(params))
adaptive=params+'DOWN DOWN RIGHT '
assert 'TEXT 20 101 h0\n' in tail(run(adaptive))
manual=params+'NEG 3 EXE '
assert tail(run(manual)).count('MAN\n')==2 and 'TEXT 144 35 -3\n' in tail(run(manual))
assert tail(run(manual+'F1')).count('AUTO\n')==2
assert tail(run(manual+'F3 F1 EXIT')).count('MAN\n')==2
assert 'TEXT 144 35 -3\n' in tail(run(manual+'F3 F1 EXIT'))
assert metrics(run(params))==metrics(run(params+('F2 F2 EXIT EXIT '*50)))
assert tail(run(adaptive+'F1')).count('AUTO\n')==2 and 'h0\n' in tail(run(adaptive+'F1'))
output=params+'F4 '
on,on_pixels=run(output,True);off,off_pixels=run(output+'RIGHT',True)
assert 'ON\n' in tail(on) and 'OFF\n' in tail(off)
for pixels in [on_pixels,off_pixels]:
    assert all(pixel(pixels,x,40)==rgb(0xf81f) for x in range(335,363))
    assert all(pixel(pixels,x,38)==rgb(0xffff) for x in range(335,363))
palette=tail(run(output+'F3'))
assert 'TEXT 90 169 EXE\n' in palette and ': SELECT   EXIT: cancel' in palette
# Every critical replacement has the same safe confirmation grammar.
for path in ['6','5 2','3 3 F6 5 EXIT EXIT 2 F6','3 3 F6 OPTN']:
    out=run(path)
    assert bar(out)==['','','','','NO','YES'],(path,tail(out))
    assert plot(run(path+' R:EXE R:F6 F1'))==plot(out)
    assert metrics(run(path+' F5'))==metrics(out)==metrics(run(path+' EXIT'))
graph=params+'F6 '
system='4 2 F6 F6 F6 F6 '
event_params='1 4 A:SUB EXE F6 DOWN 1 EXE F6 '
event=event_params+'F2 F1 RIGHT DOWN A:SUB SUB 1 0 EXE DOWN RIGHT F6 EXIT F6 '
phase_event='4 2 F6 F6 F6 F2 F1 RIGHT DOWN A:SUB 1 EXE F6 EXIT F6 F4 F2 '
for path,label in [(system,'TIME'),(system+'F4 F2 ','PHASE'),
                   (event,'EVT'),(phase_event,'PHASE EVT')]:
    out=run(path)
    assert re.search(r'TEXT \d+ 8 '+label+r'\n',tail(out))
    assert not re.search(r'TEXT \d+ 9 [123]/3',tail(out))
    assert 'EVT' not in tail(out) if 'EVT' not in label else 'EVT' in tail(out)
    traced=run(path+'F1')
    assert label in tail(traced)
    blink=run(path+'F1 '+'BLINK '*20)
    assert plot(blink)==plot(traced) and metrics(blink)==metrics(traced)
assert 'TEXT 13 8 END: Event\n' in tail(run(event))
assert 'EVT' in tail(run(event+'F5 F4'))
assert 'END: Event' in tail(run(event+'F4 F2'))
assert 'Linearized:' in tail(run(phase_event+'F5 F2 F3'))
# Result at the bottom plot boundary: its pointer cannot cut through footer text.
bottom='1 4 0 EXE F6 F6 F3 DOWN DOWN DOWN DOWN 0 EXE F6 F6 F5 F4 '
out,pixels=run(bottom,True)
assert 'Y-ICPT 1/1' in tail(out)
assert all(pixel(pixels,x,200)==rgb(0xffff) for x in range(6,390))
inventory=[('', ['', '', '', '', '', 'OPEN']),('1',['','','','','','OPEN']),
    ('2',['INIT','','','','','NEXT']),('2 F6',['INIT','','','','','NEXT']),
    (params,['INIT','ADV','V-WIN','OUTPUT','SET','GRAPH']),
    (params+'F2',['EVENT','INFO','','','','']),
    (params+'F2 F1',['','','','','','DONE']),
    (params+'F2 F2',['','','','','','']),
    (params+'F3',['INIT','','','','','DONE']),
    (output,['INIT','','COLOR','','','DONE']),
    ('1 4 F6 F6 F5 DOWN DOWN',['INIT','','','','','DONE']),
    (graph,['TRACE','ZOOM','V-WIN','TABLE','G-SLV','INIT']),
    (system+'F4',['TIME','PHASE','TABLE','','','']),
    (graph+'F2',['IN','OUT','AUTO','ORIG','BOX','']),
    (graph+'F1',['INIT','NORMAL','FAST','FASTER','LEFT','RIGHT']),
    (graph+'F5',['ROOT','MAX','MIN','Y-ICPT','ICPT','>']),
    (system+'F4 F2 F5',['FIELD','NULL','EQPT','INFO','','']),
    (graph+'F4',['TOP','BTM','MID','','STAT','GRAPH'])]
for path,expected in inventory:assert bar(run(path))==expected,(path,bar(run(path)),expected)
print('UI consistency: hint/caret/error geometry, AUTO/MAN/h0, divider, line previews, unified confirmations, view/EVT labels, blink/no-work/layering and F-key inventory passed.')
