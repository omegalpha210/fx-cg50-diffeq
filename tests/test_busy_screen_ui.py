"""Dedicated busy LCD screens and transactional UI cancellation, using actual UI."""
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

app = str(Path(sys.argv[1]).resolve())
BASE = ['TRACE', 'ZOOM', 'V-WIN', 'TABLE', 'G-SLV', 'INIT']
PARAMETERS = ['INIT', 'ADV', 'V-WIN', 'OUTPUT', 'SET', 'GRAPH']
TABLE = ['TOP', 'BTM', 'MID', '', 'STAT', 'GRAPH']
base = '2 F6 F6 F6 '


def run(keys, pixels=False):
    with tempfile.TemporaryDirectory() as folder:
        env = dict(os.environ, DIFFEQ_HOST_KEYS=keys,
                   DIFFEQ_HOST_TICK_LIMIT='128', DIFFEQ_HOST_MAX_FRAMES='1000')
        if pixels:
            env['DIFFEQ_HOST_OUT'] = folder
        result = subprocess.run([app], cwd=folder, env=env, capture_output=True,
                                text=True, timeout=30)
        assert result.returncode == 0 and 'SCRIPT COMPLETE' in result.stdout, (keys, result.stderr, result.stdout[-2000:])
        assert 'runtime error:' not in result.stderr
        frames = {}
        if pixels:
            frames = {int(p.stem): p.read_bytes().split(b'\n', 3)[3]
                      for p in Path(folder).glob('*.ppm')}
        return result.stdout, frames


def bar(out):
    return re.findall(r'TEXT \d+ 206 ([^\n]*)', out)[-6:]


def last(out, field):
    return re.findall(r'^' + field + r' (\w+)', out, re.M)[-1]


def solves(out):
    return int(re.findall(r'solves=(\d+)', out)[-1])


def values(out):
    # The final V-Window frame includes bounds, scales and Xdot.
    text = out.split('KEY F3')[-1]
    return re.findall(r'TEXT 144 (?:35|57|79|101|123|145|167) ([-+0-9.eE]+)\n', text)


def verify_busy(out, frames, label):
    drawing = label == 'Drawing...'
    anchor = 'TEXT 11 206 ' if drawing else 'TEXT 14 9 '
    phases = re.findall(r'^' + anchor + re.escape(label) + r' ([/\\|\-])$', out, re.M)
    assert phases[:4] == ['/', '-', '\\', '|'], phases
    pending = False
    count = 0
    previous = None
    for line in out.splitlines():
        if line.startswith(anchor + label):
            pending = True
        if line.startswith('FRAME '):
            rgb = frames[int(line.split()[1])]
            if pending:
                if drawing:
                    assert previous is not None
                    assert rgb[:202*396*3] == previous[:202*396*3]
                    assert rgb[220*396*3:] == previous[220*396*3:]
                    # Every former separator is now solid application blue.
                    for x in [69,133,197,261,325,389]:
                        assert rgb[(203*396+x)*3:(203*396+x)*3+3] == bytes([24,80,197])
                else:
                    assert rgb[44*396*3:] == b'\xff' * ((224-44)*396*3)
                count += 1
            pending = False
            previous = rgb
    assert count >= 4


prior, _ = run(base)
prior_plot, prior_report = last(prior, 'PLOT'), last(prior, 'REPORT')
table, table_pixels = run(base + 'TICKS:8 F4', True)
verify_busy(table, table_pixels, 'Preparing Table...')
assert bar(table) == TABLE and last(table, 'REPORT') == prior_report
assert 'Preparing Table...' not in table.split('KEY F4')[-1].split('FRAME')[-1]
draw, drawing_pixels = run('2 F6 F6 TICKS:8 F6', True)
verify_busy(draw, drawing_pixels, 'Drawing...')
assert bar(draw) == BASE and last(draw, 'PLOT') == prior_plot
for keys in [base + 'TICKS:8 CANCEL:12 F4 R:EXIT R:EXIT',
             base + 'F4 TICKS:8 CANCEL:12 DOWN R:EXIT R:EXIT',
             base + 'TICKS:8 CANCEL:12 RIGHT R:EXIT R:EXIT']:
    out, _ = run(keys)
    assert bar(out) == BASE and 'Partial: Cancelled' not in out
    assert last(out, 'REPORT') == prior_report and last(out, 'PLOT') == prior_plot
    # A new press exits exactly one level after stale HOLD has been ignored.
    exited, _ = run(keys + ' EXIT')
    assert bar(exited) == PARAMETERS
    window, _ = run(keys + ' F3')
    original_window, _ = run(base + 'F3')
    assert values(window) == values(original_window) and values(window)

initial, _ = run('2 F6 F6 TICKS:8 CANCEL:12 F6 R:EXIT R:EXIT')
assert 'Drawing...' in initial and 'Partial: Cancelled' not in initial
assert bar(initial) == PARAMETERS
params, _ = run('2 F6 F6')
assert last(initial, 'REPORT') == last(params, 'REPORT')
fast, _ = run('2 F6 F6 TICKS:0 F6')
assert 'TEXT 11 206 Drawing...' not in fast and bar(fast) == BASE
# Page buffers already contain all visible columns: column-only motion and an
# unchanged row position must reuse them without solver work or spinner flash.
for prefix, suffix in [(base + 'F4 ', 'F3'),
                       (base + 'F4 F2 ', 'F2'),
                       ('3 3 F6 F6 F6 F6 F4 ', 'RIGHT LEFT')]:
    before, _ = run(prefix)
    after, _ = run(prefix + 'TICKS:8 ' + suffix)
    assert solves(after) == solves(before) and bar(after) == TABLE
    assert 'TEXT 14 9 Preparing Table...' not in after
changed_page, _ = run(base + 'F4 TICKS:8 DOWN')
assert 'TEXT 14 9 Preparing Table...' in changed_page and bar(changed_page) == TABLE

# A completed noncancel numerical limit has a valid display prefix too. Its
# rollback must preserve that result; cached plotting can round a segment
# differently from the original full-resolution stream, so compare to the same
# committed cache repaint rather than pretending both rasterizations coincide.
partial = '2 F6 F6 DOWN DOWN RIGHT UP UP UP 1 0 EXE F6 '
completed, _ = run(partial)
cached, _ = run(partial + 'F3 F6')
cancelled, _ = run(partial + 'TICKS:8 CANCEL:12 RIGHT R:EXIT')
assert 'Partial: Step limit reached' in completed and bar(cancelled) == BASE
assert last(cancelled, 'REPORT') == last(completed, 'REPORT')
assert last(cancelled, 'PLOT') == last(cached, 'PLOT')
restored_window, _ = run(partial + 'TICKS:8 CANCEL:12 RIGHT R:EXIT F3')
entry_window, _ = run(partial + 'F3')
assert values(restored_window) == values(entry_window)

# A completed numerical-limit redraw owns its new geometry, report and display
# cache together. Only user cancellation rolls these back to the previous view.
panned_partial, _ = run(partial + 'RIGHT')
assert 'Phase:' not in panned_partial and 'Partial: Step limit reached' in panned_partial
panned_window, _ = run(partial + 'RIGHT F3')
assert values(panned_window) == ['-3.78', '8.82', '1', '0.0333333333', '-3.1', '3.1', '1']
resumed_partial, _ = run(partial + 'RIGHT F3 F6')
assert last(resumed_partial, 'REPORT') == last(panned_partial, 'REPORT')
assert solves(resumed_partial) == solves(panned_partial)
assert bar(resumed_partial) == BASE and 'Partial: Step limit reached' in resumed_partial

# A zero-step renderer preflight rejection leaves the previous transaction
# intact. A small RK4 window fits 10 steps; factory INIT would require 60.
bounded = '2 F6 F6 UP 1 0 EXE F3 NEG 1 DOT 5 EXE 1 DOT 5 EXE F6 F6 '
bounded_graph, _ = run(bounded)
rejected_init, _ = run(bounded + 'F6')
assert 'Graph: Step limit reached' in rejected_init and 'Phase:' not in rejected_init
assert last(rejected_init, 'REPORT') == last(bounded_graph, 'REPORT')
after_notice, _ = run(bounded + 'F6 EXE')
assert last(after_notice, 'PLOT') == last(bounded_graph, 'PLOT')
assert solves(after_notice) == solves(bounded_graph)
rejected_window, _ = run(bounded + 'F6 F3')
bounded_window, _ = run(bounded + 'F3')
assert values(rejected_window) == values(bounded_window)
invalid_initial, _ = run('2 F6 F6 UP 1 EXE F6')
assert bar(invalid_initial) == ['', '', '', '', '', 'EDIT']
assert 'Too many steps; increase h / Max Steps' in invalid_initial
assert solves(invalid_initial) == 0
print('Busy UI: dedicated LCD canvas, hidden softkeys, four phases, Table page reuse, index/page cancellation, initial Drawing rollback, normal/partial pan cache-window-report restoration, committed noncancel partial geometry and EXIT/HOLD PASS.')
