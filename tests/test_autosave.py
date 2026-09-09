"""Explicit SAVE, true cold starts, RCL and no automatic writes."""
import os
import subprocess
import sys
import tempfile
from pathlib import Path
app=str(Path(sys.argv[1]).resolve())
def execute(keys,directory):
    p=subprocess.run([app],cwd=directory,env=dict(os.environ,DIFFEQ_HOST_KEYS=keys),
        text=True,capture_output=True,timeout=20)
    assert p.returncode==0 and 'SCRIPT COMPLETE' in p.stdout,(p.stderr,p.stdout[-3000:])
    return p.stdout
with tempfile.TemporaryDirectory(dir='.') as directory:
    execute('6 EXE EXE',directory)
    assert not list(Path(directory).glob('DIFFEQ?.dat'))
    execute('1 4 NEG A:SUB F6 F6 F6 EXIT EXIT EXIT EXIT EXIT EXIT',directory)
    assert not list(Path(directory).glob('DIFFEQ?.dat'))
    execute('1 4 NEG A:SUB F6 F6 F6 EXIT EXIT EXIT EXIT EXIT 6 EXE EXE',directory)
    slots={p.name:p.read_bytes() for p in Path(directory).glob('DIFFEQ?.dat')}
    assert slots
    cold=execute('1 4',directory)
    assert 'TEXT 144 57 sin(x)-y\n' in cold and 'TEXT 144 57 -y\n' not in cold
    assert 'reads=0 writes=0 opens=0 closes=0' in cold
    restored=execute('5 2 F6',directory)
    assert 'TEXT 144 57 -y\n' in restored
    assert slots=={p.name:p.read_bytes() for p in Path(directory).glob('DIFFEQ?.dat')}
    recalled=execute('5 2 F6 EXIT 5 1',directory)
    assert 'TEXT 144 57 -y\n' in recalled
print('Explicit session policy: no edit/EXIT writes, SAVE, cold defaults, RCL, retained slots passed.')
