"""Production keys/pixels: tile focus, fixed TRACE view, reset and warnings."""
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

app = str(Path(sys.argv[1]).resolve())


def run(keys, pixels=False):
    with tempfile.TemporaryDirectory() as folder:
        env = dict(os.environ, DIFFEQ_HOST_KEYS=keys, DIFFEQ_HOST_MAX_FRAMES='10000')
        if pixels:
            env['DIFFEQ_HOST_OUT'] = folder
        p = subprocess.run([app], cwd=folder, env=env, capture_output=True, text=True, timeout=30)
        assert p.returncode == 0 and 'SCRIPT COMPLETE' in p.stdout and 'runtime error:' not in p.stderr, (keys, p.stderr, p.stdout[-1800:])
        if pixels:
            return p.stdout, sorted(Path(folder).glob('*.ppm'))[-1].read_bytes().split(b'\n', 3)[3]
        return p.stdout


def tail(out):
    return next(s for s in reversed(out.split('\nKEY ')) if 'TEXT ' in s)


def solves(out):
    return int(re.findall(r'solves=(\d+)', out)[-1])


def point(out):
    f, x, y = re.findall(r'IC(\d+) x=([-+.\deE]+) [^=\n]+=([-+.\deE]+)', tail(out))[-1]
    return int(f), float(x), float(y)


def window(out):
    return [float(v) for v in re.findall(r'TEXT 144 (?:35|57|79|101|123|145|167) ([-+0-9.eE]+)\n', tail(out))]


def pixel(data, x, y):
    return data[(y*396+x)*3:(y*396+x)*3+3]


def rgb(c):
    return bytes([(c >> 11 & 31)*255//31, (c >> 5 & 63)*255//63, (c & 31)*255//31])


for subtype, count in [(False, 6), (True, 4)]:
    prefix = '1 ' if subtype else ''
    original, data = run(prefix, True)
    assert all(label in original for label in (['Separable', 'Linear', 'Bernoulli', 'Others'] if subtype else ['1st', '2nd', 'N-th', 'SYSTEM', 'RECALL', 'SAVE']))
    assert re.findall(r'TEXT \d+ 206 ([^\n]*)', original)[-6:] == ['', '', '', '', '', 'OPEN']
    for i in range(count):
        moves = 'DOWN '*(i//2)+'RIGHT '*(i%2)
        out, data = run(prefix+moves, True)
        x, y = 10+192*(i%2), 31+62*(i//2)
        assert pixel(data, x, y) == rgb((19 << 6) | 29)
        opened = run(prefix+moves+'F6')
        direct = run(prefix+str(i+1))
        assert re.findall(r'PLOT (\w+)', opened)[-1] == re.findall(r'PLOT (\w+)', direct)[-1]
        assert re.findall(r'PLOT (\w+)', run(prefix+moves+'EXE'))[-1] == re.findall(r'PLOT (\w+)', direct)[-1]
    # Same-column wrap and row-local wrap do not reorder items.
    assert run(prefix+'UP DOWN', True)[1] == run(prefix, True)[1]
    assert run(prefix+'RIGHT RIGHT', True)[1] == run(prefix, True)[1]
assert run('1 DOWN RIGHT EXIT', True)[1] == run('', True)[1]
assert run('EXIT R:EXIT', True)[1] == run('', True)[1]

# Two exact affine solutions; MAN solver range is wider than a custom viewport.
graph = ('1 4 1 F6 DOWN S:MUL 0 COMMA 5 S:DIV F6 '
         'NEG 4 EXE 4 EXE F3 NEG 1 EXE 1 EXE DOWN DOWN '
         'NEG 0 DOT 2 5 EXE 0 DOT 2 5 EXE F6 F6 ')
entry = window(run(graph+'F3'))
assert entry[0:2] == [-1, 1] and entry[4:6] == [-.25, .25]
trace = graph+'F1 '
base = run(trace)
assert point(base) == (1, 0, 0)
for speed, stride in [('F2', 1), ('F3', 2), ('F4', 3)]:
    for direction, sign in [('LEFT', -1), ('RIGHT', 1)]:
        out = run(trace+speed+' '+direction)
        assert abs(point(out)[1]-sign*stride*2/378) < 1e-7
    for endpoint, x in [('F5', -1), ('F6', 1)]:
        out = run(trace+speed+' '+endpoint)
        assert point(out) == (1, x, x) and solves(out) == solves(base)
        for keys in [endpoint, endpoint+' '+('R:LEFT ' if x < 0 else 'R:RIGHT ')*100]:
            view = window(run(trace+speed+' '+keys+' EXIT F3'))
            assert view[:4] == entry[:4] and view[6] == entry[6]
            assert abs(view[5]-view[4]-.5) < 1e-12 and view[4] < x < view[5]
            assert solves(run(trace+speed+' '+keys)) == solves(base)
reset, data = run(trace+'F4 F6 DOWN F1', True)
assert point(reset) == (1, 0, 0) and solves(reset) == solves(base)
for slot, color in [(0, 0xffe0), (1, 0xfc40), (2, 0x37e6), (3, 0x07ff)]:
    assert pixel(data, 6+64*slot+3, 204) == rgb(color)
assert pixel(data, 6+64*3+1, 203) == rgb(0)  # FASTER retained by INIT.
assert point(run(trace+'F4 F6 DOWN')) == (2, 1, 6)
assert window(run(trace+'F4 F6 DOWN F1 EXIT F3'))[:4] == entry[:4]
assert 'MAN\n' in tail(run(trace+'F6 F1 EXIT EXIT'))
assert solves(run(trace+'F4 '+'R:RIGHT '*400+'EXIT')) == solves(base)
assert 'Partial: Cancelled' not in run(trace+'F4 '+'R:RIGHT '*400+'EXIT')
assert window(run(trace+'EXIT RIGHT F3'))[0] > entry[0]
assert solves(run(trace+'EXIT RIGHT')) > solves(base)
assert window(run(trace+'EXIT RIGHT F6 F3')) == window(run(trace+'EXIT F3 F1 F6 F3'))

# Narrow PHASE horizontal slab: traversal stops before leaving it, even if a
# later oscillator state would reenter. Integration-time endpoints stay distinct.
phase = '4 2 F6 F6 F6 F6 F4 F2 F3 NEG 0 DOT 5 EXE 0 DOT 5 EXE F6 '
phase_entry = window(run(phase+'F3'))
out = run(phase+'F1 F4 F6')
t, y1, y2 = map(float, re.findall(r'x=([-+.\deE]+) y1=([-+.\deE]+) y2=([-+.\deE]+)', tail(out))[-1])
assert 1 < abs(t) < 3 and -.5 <= y1 <= .5
assert window(run(phase+'F1 F4 F6 EXIT F3'))[:4] == phase_entry[:4]
assert solves(out) == solves(run(phase+'F1'))

domain = '1 4 S:SQUARE 1 SUB XOT RIGHTP F6 F6 F6 '
for action in ['', 'F1 F6', 'F1 F6 DOWN', 'F5 F4']:
    out, data = run(domain+action, True)
    assert 'TEXT 13 8 END: Math domain / singularity\n' in tail(out)
    assert pixel(data, 11, 6) == rgb(0xffff)
    assert rgb(0xf800) in {pixel(data, x, y) for x in range(13, 280) for y in range(8, 19)}
    if action.startswith('F1'):
        assert point(out)[1] <= 1
    if action == 'F5 F4':
        assert 'Y-ICPT 1/1' in tail(out)
phase_warning = '4 2 F6 DOWN S:SQUARE 1 SUB XOT RIGHTP F6 F6 F6 F4 F2 F5 F2 '
out = tail(run(phase_warning))
assert 'TEXT 13 8 END: Math domain / singularity' in out
assert 'TEXT 12 24 N1' in out and 'TEXT 37 24 N2' in out
assert re.search(r'TEXT \d+ 8 PHASE\n', out)
print('Tile keys/pixels, frozen TIME/PHASE view, endpoints, Y follow, origin/curve/speed INIT, no-extension hold, ordinary pan and warning overlays passed.')
