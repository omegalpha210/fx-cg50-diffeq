"""Actual UI: TRACE jump/color/stride contracts, settings and ten-column navigation."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys,image=False):
    with tempfile.TemporaryDirectory() as directory:
        env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_MAX_FRAMES='10000')
        if image:env['DIFFEQ_HOST_OUT']=directory
        p=subprocess.run([app],cwd=directory,env=env,capture_output=True,text=True,timeout=30)
        assert p.returncode==0 and 'runtime error:' not in p.stderr,(p.stderr,p.stdout[-2000:])
        assert 'SCRIPT COMPLETE' in p.stdout
        if image:return p.stdout,sorted(Path(directory).glob('*.ppm'))[-1].read_bytes().split(b'\n',3)[3]
        return p.stdout
def tail(out):return next(t for t in reversed(out.split('\nKEY ')) if 'TEXT ' in t)
def bar(out):
    result=['']*6
    for x,label in re.findall(r'TEXT (\d+) 206 ([^\n]*)',tail(out)):
        result[(int(x)-6)//64]+=label
    return result
def point(out):
    f,x,y=re.findall(r'IC(\d+) x=([-+.\deE]+) [^=\n]+=([-+.\deE]+)',tail(out))[-1]
    return int(f),float(x),float(y)
def plot(out):return re.findall(r"^PLOT (\w+)",out,re.M)[-1]
def solves(out):return int(re.findall(r'solves=(\d+)',out)[-1])
def pixel(rgb,x,y):return rgb[(y*396+x)*3:(y*396+x)*3+3]
def rgb565(c):return bytes([(c>>11&31)*255//31,(c>>5&63)*255//63,(c&31)*255//31])
def keys(text):
    mapping={'{':'S:MUL','}':'S:DIV',',':'COMMA','.':'DOT','-':'NEG'}
    return ' '.join(mapping.get(c,c) for c in text)+' '
# Constant slopes give exact cached interpolation and test both Y directions.
ic='1 4 1 EXE F6 DOWN '+keys('{0,1}')+'EXE '
graph=ic+'F6 DOWN DOWN DOWN DOWN DOWN 0 EXE F6 '
trace=graph+'F1 DOWN '
for speed,multiple in [('F2',1),('F3',2),('F4',3)]:
    base=run(trace+speed)
    for jump,x in [('F5',-6),('F6',6)]:
        out,rgb=run(trace+speed+' '+jump,image=True)
        assert point(out)==(2,x,x+1) and solves(out)==solves(base)
        assert bar(out)==['x=','NORMAL','FAST','FASTER','LEFT','RIGHT']
        for slot,color in [(1,0xffe0),(2,0x37e6),(3,0x07ff)]:
            assert pixel(rgb,6+64*slot+3,204)==rgb565(color)
            # Text pixels in each label are black; the selection border is black.
            colors=[pixel(rgb,xx,yy) for xx in range(6+64*slot+4,6+64*slot+60) for yy in range(206,215)]
            assert rgb565(0) in colors and rgb565(0xffff) not in colors
            border=pixel(rgb,6+64*slot+1,203)
            assert border==rgb565(0 if slot==multiple else color)
        direction='LEFT' if x<0 else 'RIGHT'
        moved=run(trace+speed+' '+jump+' '+direction)
        assert abs(point(moved)[1]-(x+(1 if x>0 else -1)*multiple/30))<1e-6
        assert solves(moved)>solves(out)
        returned=run(trace+speed+' '+jump+' '+direction+' '+jump)
        assert point(returned)==(2,x,x+1) and solves(returned)==solves(moved)
        assert bar(run(trace+speed+' '+jump+' EXIT'))==['TRACE','ZOOM','V-WIN','TABLE','G-SLV','PREV']
# A FAST target crosses in the same input, with no forced endpoint pause.
cross=trace+'F1 5 DOT 9 5 EXE F4 RIGHT '
assert abs(point(run(cross))[1]-6.05)<1e-7
# Panned Y bounds keep their span; configured endpoints are still -6 and 6.
params=run(trace+'F6 EXIT EXIT')
assert 'TEXT 144 35 -6\n' in tail(params) and 'TEXT 144 57 6\n' in tail(params)
window=run(trace+'F6 EXIT F3')
ymin=float(re.findall(r'TEXT 144 123 ([^\n]+)',tail(window))[-1])
ymax=float(re.findall(r'TEXT 144 145 ([^\n]+)',tail(window))[-1])
assert ymin>0 and abs(ymax-ymin-6.2)<1e-9
for row in ['', 'DOWN ']:
    settings='1 4 F6 F6 F5 '+row
    before=run(settings)
    assert 'LEFT/RIGHT: ON/OFF toggle' in tail(before) and 'Density SF:' not in before
    assert bar(before)==['INIT','','','','','DONE']
    assert plot(run(settings+'F2 F4'))==plot(before)
    changed=run(settings+'RIGHT')
    assert 'Off' in tail(changed) and plot(run(settings+'RIGHT LEFT'))==plot(before)
assert bar(run('1 4 F6 F6 F5 DOWN DOWN'))==['','','','','','DONE']
assert bar(run('1 4 F6 F6 F5 DOWN DOWN DOWN'))==['INIT','','COLOR','','','DONE']
# Ten physical initial values, last family and frozen-x final columns.
ten='1 4 0 EXE F6 DOWN '+keys('{0,1,2,3,4,5,6,7,8,9}')+'EXE '
assert 'at most 10' in tail(run(ten))
assert point(run(ten+'F6 F6 F1 '+'DOWN '*9))==(10,0,9)
out=run(ten+'F6 F6 F4 '+'RIGHT '*20)
assert re.findall(r'TEXT \d+ 49 ([^\n]*)',tail(out))==['x','y9','y10']
for page in ['F1','F2','F3','DOWN','UP']:
    out=run(ten+'F6 F6 F4 '+'RIGHT '*20+page)
    assert re.findall(r'TEXT \d+ 49 ([^\n]*)',tail(out))==['x','y9','y10']
long_input='1 4 F6 DOWN '+('0 '*192)
assert 'Input too long; Max: 191 characters' in tail(run(long_input))
assert 'Input too long' in tail(run(long_input+'EXE')) and 'Max: 191 characters' in tail(run(long_input+'EXE'))
assert ': return to MAIN MENU' in tail(run(''))
print('TRACE colors/black text/selection, exact configured jumps, same-input crossing, Y pan, settings arrows, ten families/frozen columns and bounded length feedback passed.')
