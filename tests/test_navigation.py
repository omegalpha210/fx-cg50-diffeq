"""Stress the fixed parent graph and the real host UI dispatcher."""
import os
import subprocess
import sys
import tempfile
from pathlib import Path

helper=str(Path(sys.argv[1]).resolve())
app=str(Path(sys.argv[2]).resolve())

checked=subprocess.run([helper],text=True,capture_output=True,timeout=10)
if checked.returncode or "1000 iterative navigation cycles passed" not in checked.stdout:
    raise AssertionError(checked.stderr+checked.stdout)

def execute(keys,expect_session):
    with tempfile.TemporaryDirectory(dir=".") as directory:
        environment=dict(os.environ,DIFFEQ_HOST_KEYS=keys,
            DIFFEQ_HOST_MAX_FRAMES="10000")
        result=subprocess.run([app],cwd=directory,env=environment,
            stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,text=True,timeout=20)
        if result.returncode!=0 or "runtime error:" in result.stderr:
            raise AssertionError(result.stderr)
        slots=list(Path(directory).glob("DIFFEQ?.dat"))
        if expect_session:
            assert 1<=len(slots)<=2
        else:
            assert not slots

# Same-process screen loops exercise actual modal handlers and dispatcher state.
execute("F1 4 F6 F6 "+"F5 EXIT "*1000+"EXIT EXIT EXIT",False)
execute("F1 4 "+"F6 F6 F6 EXIT F1 F1 "*100+"EXIT EXIT EXIT",False)
execute(("F1 4 EXIT EXIT F2 EXIT F3 1 F6 EXIT EXIT F4 1 F6 EXIT EXIT ")*100+"EXIT",False)

print("Actual dispatcher: SET 1000, all stages/Graph 100, family round trip 100, no writes passed.")
