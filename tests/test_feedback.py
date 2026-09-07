"""Hardware-feedback UI contracts through actual app handlers and pixels."""
import os,re,subprocess,sys,tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def run(keys, directory=None, image=False):
    if directory is None:
        with tempfile.TemporaryDirectory() as temporary:
            return run(keys,temporary,image)
    env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_MAX_FRAMES='10000')
    if image: env['DIFFEQ_HOST_OUT']=directory
    p=subprocess.run([app],cwd=directory,env=env,capture_output=True,text=True,timeout=30)
    assert p.returncode==0 and 'runtime error:' not in p.stderr,(p.stderr,p.stdout[-3000:])
    assert 'SCRIPT COMPLETE' in p.stdout
    if image:
        frames=sorted(Path(directory).glob('*.ppm'))
        rgb=frames[-1].read_bytes().split(b'\n',3)[3]
        return p.stdout,rgb
    return p.stdout
def plot(out):return re.findall(r'^PLOT (\w+)',out,re.M)[-1]
def tail(out):return out[out.rfind('\nKEY '):]
def pixel(rgb,x,y):return rgb[(y*396+x)*3:(y*396+x)*3+3]
def rgb565(value):return bytes([(value>>11&31)*255//31,(value>>5&63)*255//63,(value&31)*255//31])

# Ordinary fields: EDIT EXE advances once; EXIT commits/stays; last selected runs.
forms=[('F2','DIFF EQ / Linear 2nd',3),('F2 F6','Initial Conditions',3),
 ('F2 F6 F6','Parameter',6),('F2 F3','View Window',7),
 ('F2 F6 F6 F5 2','Private constants',26),('F3','Order (1-9)',1)]
for prefix,title,count in forms:
    assert plot(run(prefix+' EXE'))==plot(run(prefix+(' DOWN' if count>1 else ' F6')))
    if count>1:assert plot(run(prefix+' 2 EXE'))==plot(run(prefix+' 2 EXIT DOWN'))
    else:assert plot(run(prefix+' 2 EXE'))==plot(run(prefix+' 2 EXIT'))
    assert title in tail(run(prefix+' 2 EXIT'))
    assert 'TEXT 14 9 '+title+'\n' not in tail(run(prefix+' 2 EXIT EXIT'))
    last=prefix+' '+'DOWN '*(count-1)
    assert plot(run(last+'2 EXE'))==plot(run(last+'2 EXIT'))
    assert plot(run(last+'2 EXE EXE'))==plot(run(last+'2 EXIT F6'))
    assert plot(run(last+'EXE'))==plot(run(last+'F6'))
for prefix in ['F2','F2 F6','F2 F6 F6','F2 F3','F2 F6 F6 F5 2']:
    for accept in ['EXE','EXIT']:
        assert plot(run(prefix+' SIN '+accept+' EXE'))==plot(run(prefix+' SIN'))
for prefix,actions in [('F2',['F3','F6']),('F2 F6',['F1','F3','F4','F5','F6']),
 ('F2 F6 F6',['F1','F3','F4','F5','F6'])]:
    for action in actions:
        assert plot(run(prefix+' SIN '+action+' EXE'))==plot(run(prefix+' SIN'))
        assert plot(run(prefix+' 2 '+action))==plot(run(prefix+' 2 EXIT '+action))
# Both entry/menu page/exit and EXE no-op preserve the equation and cursor pixels.
for prefix in ['F2','F1 4 LEFT RIGHT','F3 9 F6 LEFT','F4 9 F6 DOWN RIGHT']:
    for menu in ['F1','F2']:
        assert plot(run(prefix+' '+menu+' EXE EXIT'))==plot(run(prefix))
        paging=' F6 F6 EXIT' if menu=='F2' or prefix.startswith(('F3','F4')) else ' F6'
        assert plot(run(prefix+' '+menu+paging))==plot(run(prefix)),(prefix,menu)
blank='F1 4 LEFT ACON '
for keys,token in [('F1','abs('),('F2','sinh('),('F3','cosh('),('F4','tanh('),
 ('F5','asinh('),('F6 F1','acosh('),('F6 F2','atanh(')]:
    out=run(blank+'F2 '+keys)
    assert 'TEXT 144 57 '+token+'\n' in tail(out)
    assert 'Insert function' not in out
for physical,token in [('S:SIN','asin('),('S:COS','acos('),('S:TAN','atan(')]:
    assert 'TEXT 144 57 '+token+'\n' in tail(run(blank+physical))
for entry,count in [('F3 9 F6',8),('F4 9 F6',9)]:
    for index in range(count):
        keys='F1 '+('F6 ' if index>=5 else '')+'F'+str(index%5+1)
        out=run(entry+' LEFT ACON '+keys)
        assert 'TEXT 144 57 y'+str(index+1)+'\n' in tail(out)
        assert 'Insert variable' not in out
# Main retains F-key inventory and has type-only numeric shortcuts.
for n in range(1,5):assert plot(run(str(n)))==plot(run('F'+str(n)))
assert plot(run('5 6'))==plot(run(''))
assert 'Recall / load session' in run('') and '...' not in run('').split('Recall / load session')[0][-10:]
# Current semantic stage colors and OUTPUT-specific styles.
for keys,colors in [('F2',[None,None,0xfc40,None,None,0x07ff]),
 ('F2 F6',[0xf81f,None,0xfc40,None,None,0x07ff]),
 ('F2 F6 F6',[0xf81f,None,0xfc40,None,0x37e6,0xf800])]:
    _,rgb=run(keys,image=True)
    for i,color in enumerate(colors):
        if color is not None:assert pixel(rgb,7+i*64,203)==rgb565(color)
    assert plot(run(keys+' DOWN F3 EXIT'))==plot(run(keys+' DOWN'))
output='F2 F6 F6 F4 '
assert plot(run(output+'EXE'))==plot(run(output+'DOWN'))
assert 'Curve color' not in run(output+'EXE EXE EXE')
assert 'Parameter' in tail(run(output+'DOWN DOWN EXE'))
modified=output+'DOWN DOWN F1 '
assert 'Output selection' in tail(run(modified+'EXE'))
assert 'Parameter' in tail(run(modified+'EXE EXE'))
_,rgb=run(output,image=True)
assert pixel(rgb,7,203)==rgb565(0x1a98) and pixel(rgb,135,203)==bytes([255,255,255])
region={pixel(rgb,x,y) for x in range(134,197) for y in range(205,217)}
for color in [0xf800,0xfc40,0x37e6,0x07ff,0xf81f]:assert rgb565(color) in region
assert 'Curve color' in tail(run(output+'DOWN RIGHT'))
assert 'Curve color' in tail(run(output+'DOWN F3'))
choices=['LEFT LEFT','LEFT','','DOWN LEFT LEFT','DOWN LEFT','DOWN']
for color,moves in zip([0x001f,0xf800,0xf81f,0,0x07ff,0x37e6],choices):
    chosen=output+'DOWN F3 '+moves+' EXE '
    _,rgb=run(chosen,image=True);assert pixel(rgb,114,76)==rgb565(color)
    assert plot(run(chosen+'F3 DOWN EXIT'))==plot(run(chosen))
    assert plot(run(chosen+'F1 F2 F1 F2'))==plot(run(chosen))
    assert plot(run(chosen+'EXIT F1 F1 F6 F6 F4 DOWN'))==plot(run(chosen))
soak=run(output+'DOWN '+('F3 DOWN EXIT F3 EXE '*100))
assert plot(soak)==plot(run(output+'DOWN'))
with tempfile.TemporaryDirectory() as directory:
    chosen=output+'DOWN F3 DOWN LEFT LEFT EXE '
    baseline=run(chosen,directory)
    run(chosen+'EXIT EXIT EXIT EXIT F6 EXE',directory)
    restored=run('F5 2 F1 F6 F6 F4 DOWN',directory)
    assert plot(restored)==plot(baseline)
    recalled=run(chosen+'EXIT F6 EXIT EXIT EXIT EXIT F5 1 F6 F6 F4 DOWN',directory)
    assert plot(recalled)==plot(baseline)
# INIT restores factory values + first selector; Output includes every IC color.
assert plot(run('F2 F3 DOWN DOWN 2 EXIT F1'))==plot(run('F2 F3'))
assert plot(run('F2 F6 F6 DOWN DOWN 2 EXIT F2'))==plot(run('F2 F6 F6'))
assert plot(run(output+'DOWN F1 F2 F3 DOWN EXE F4'))==plot(run(output))
settings='F2 F6 F6 F5 1 '
assert plot(run(settings+'LEFT DOWN LEFT F4'))==plot(run(settings))
assert 'DIFF EQ settings' in tail(run(settings+'DOWN EXE'))
assert plot(run('F2 F6 '+('F4 F5 '*100)))==plot(run('F2 F6'))
assert 'This mode needs at least one IC set.' in run('F2 F6 F5')
assert 'No initial conditions.' in run('F1 4 F6 F5')
# Resize confirmation policy stays intact under new EXE completion.
for entry in ['F3','F4']:
    first=entry+' 3 F6 '
    assert 'Change Equation Size' not in run(first+'EXIT 2 F6')
    populated=first+'5 EXIT EXIT '
    assert 'Change Equation Size' not in run(populated+'3 F6')
    for cancel in ['F5','EXIT']:
        kept=run(populated+'2 F6 '+cancel+' 3 F6')
        assert 'Change Equation Size' in kept and 'TEXT 144 57 5\n' in tail(kept)
    assert 'TEXT 144 57 5\n' not in tail(run(populated+'2 F6 F6'))
# Each zoom operation keeps its submenu; only EXIT restores base softkeys.
def bar(out):return [text for _,text in re.findall(r'TEXT (\d+) 206 ([^\n]*)',out)][-6:]
graph='F2 F6 F6 F6 '
for operation in ['F1','F2','F3','F4']:
    out=run(graph+'F2 '+operation)
    assert bar(out)==['IN','OUT','AUTO','ORIG','','']
    assert bar(run(graph+'F2 '+operation+' EXIT'))==['TRACE','ZOOM','V-WIN','TABLE','G-SLV','BACK']
assert plot(run(graph+'F2 '+('F1 F1 F2 F3 F4 '*100)+'EXIT'))==plot(run(graph))
assert plot(run(graph+'RIGHT UP F2 F4'))==plot(run(graph+'RIGHT UP F3 F1 EXIT'))
manual='F2 F6 F6 NEG 2 EXE 2 EXE 0 DOT 0 5 EXE F6 '
out=run(manual+'RIGHT F2 F4 EXIT EXIT')
assert all(s in tail(out) for s in ['TEXT 144 35 -2\n','TEXT 144 57 2\n','TEXT 144 79 0.05\n'])
assert plot(run(manual+'RIGHT F2 F4'))==plot(run(manual))
# Scalar prompts: empty draft, only RUN, EXE/F6 same path, blink has no solves.
for operation,result in [('F1','Y-CAL'),('F2','X-CAL')]:
    prompt=graph+'F5 F6 '+operation+' EXE '
    assert bar(run(prompt))==['','','','','','RUN']
    for submit in ['EXE','F6']:
        invalid=run(prompt+submit)
        assert 'Invalid number' in tail(invalid) and result+' 1/' not in invalid
        assert result+' 1/' in run(prompt+'1 '+submit)
        assert 'Invalid number' in tail(run(prompt+'NEG '+submit))
        assert 'Invalid number' in tail(run(prompt+'1 ACON '+submit))
    baseline=run(prompt+'1')
    blink=run(prompt+'1 '+'BLINK '*100)
    assert re.findall(r'solves=(\d+)',baseline)[-1]==re.findall(r'solves=(\d+)',blink)[-1]
    assert bar(run(prompt+'F1 F2 F3 F4 F5'))==['','','','','','RUN']
print('New feedback UI: EXE next/primary, EXIT, inline FUNC/VAR, Output/color/INIT, Main shortcuts, persistent Zoom and scalar RUN passed.')
