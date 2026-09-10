"""Exercise the production SYS2 phase UI with the scripted host key source."""
import math
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path


app = str(Path(sys.argv[1]).resolve())
time_graph = "4 2 F6 F6 F6 F6 "
phase_graph = time_graph + "F4 F2 "
time_bar = ["TRACE", "ZOOM", "V-WIN", "VIEW", "G-SLV", "INIT"]
phase_bar = ["TRACE", "ZOOM", "V-WIN", "VIEW", "ANLYS", "INIT"]
view_bar = ["TIME", "PHASE", "TABLE", "", "", ""]
analysis_bar = ["FIELD", "NULL", "EQPT", "INFO", "", ""]


def run(keys, timeout=30):
    with tempfile.TemporaryDirectory(dir=".") as directory:
        environment = dict(
            os.environ,
            DIFFEQ_HOST_KEYS=keys,
            DIFFEQ_HOST_MAX_FRAMES="10000",
        )
        result = subprocess.run(
            [app],
            cwd=directory,
            env=environment,
            capture_output=True,
            text=True,
            timeout=timeout,
        )
        assert result.returncode == 0, (result.stderr, result.stdout[-3000:])
        assert "runtime error:" not in result.stderr, result.stderr
        assert "SCRIPT COMPLETE" in result.stdout, result.stdout[-3000:]
        return result.stdout


def tail(output):
    return next(frame for frame in reversed(output.split("\nKEY ")) if "TEXT " in frame)


def bar(output):
    return re.findall(r"TEXT \d+ 206 ([^\n]*)", tail(output))[-6:]


def last_plot(output):
    return re.findall(r"^PLOT (\w+)", output, re.MULTILINE)[-1]


def last_metrics(output):
    return tuple(
        map(
            int,
            re.findall(
                r"METRICS solves=(\d+) searches=(\d+) reads=(\d+) "
                r"writes=(\d+) opens=(\d+) closes=(\d+)",
                output,
            )[-1],
        )
    )


def phase_points(output):
    return [
        tuple(map(float, point))
        for point in re.findall(
            r"TEXT \d+ 188 x=([-+.\deE]+) y1=([-+.\deE]+) y2=([-+.\deE]+)",
            output,
        )
    ]


# VIEW exposes all three destinations and returns to the selected projection.
assert bar(run(time_graph)) == time_bar
assert bar(run(time_graph + "F4")) == view_bar
phase = run(phase_graph)
assert bar(phase) == phase_bar
assert re.search(r"TEXT \d+ \d+ y1\n", tail(phase))
assert re.search(r"TEXT \d+ \d+ y2\n", tail(phase))
assert bar(run(phase_graph + "F4 F1")) == time_bar

table = run(phase_graph + "F4 F3")
table_frame = tail(table)
assert "TEXT 14 9 Table\n" in table_frame
assert re.findall(r"TEXT \d+ 49 ([^\n]+)", table_frame) == ["x", "y1", "y2"]
assert bar(run(phase_graph + "F4 F3 F6")) == phase_bar

# TIME and PHASE retain separate, explicitly edited V-Window X geometry.
windows = (
    time_graph
    + "F3 NEG 8 EXE 9 EXE F6 "
    + "F4 F2 F3 NEG 2 EXE 4 EXE F6 "
)
time_window = tail(run(windows + "F4 F1 F3"))
assert "TEXT 14 9 View Window\n" in time_window
assert "TEXT 144 35 -8\n" in time_window
assert "TEXT 144 57 9\n" in time_window
phase_window = tail(run(windows + "F3"))
assert "TEXT 14 9 Phase View Window\n" in phase_window
assert "TEXT 144 35 -2\n" in phase_window
assert "TEXT 144 57 4\n" in phase_window
phase_again = tail(run(windows + "F4 F1 F3 F6 F4 F2 F3"))
assert "TEXT 14 9 Phase View Window\n" in phase_again
assert "TEXT 144 35 -2\n" in phase_again
assert "TEXT 144 57 4\n" in phase_again

# Phase TRACE reports one combined (x,y1,y2) family. INIT and held
# repeats use the captured trajectory without another numerical solve.
trace_start = run(phase_graph + "F1")
assert bar(trace_start) == ["INIT", "NORMAL", "FAST", "FASTER", "LEFT", "RIGHT"]
assert phase_points(trace_start)[-1] == (0.0, 1.0, 0.0)
direct = phase_points(run(phase_graph + "F1 " + "RIGHT " * 30))[-1]
assert abs(direct[0] - 1.0) < 1e-12
assert abs(direct[1] - math.cos(1.0)) < 2e-4
assert abs(direct[2] + math.sin(1.0)) < 2e-4

repeated = run(phase_graph + "F1 RIGHT " + "R:RIGHT " * 12 + "DOWN")
points = phase_points(repeated)
assert len(points) >= 14
assert all(points[i][0] < points[i + 1][0] for i in range(len(points) - 2))
assert points[-1] == points[-2]  # DOWN cannot select y1/y2 as separate curves.
assert last_metrics(repeated) == last_metrics(trace_start)
exited = run(phase_graph + "F1 RIGHT " + "R:RIGHT " * 12 + "DOWN EXIT")
assert bar(exited) == phase_bar
assert "Drawing..." not in exited[exited.rfind("KEY EXIT") :]

# FIELD and NULL are independent persistent toggles. Compare frames while the
# same ANALYSIS softkey menu is visible so differences come from graph layers.
field_only = run(phase_graph + "F5")
neither = run(phase_graph + "F5 F1")
field_and_null = run(phase_graph + "F5 F2")
null_only = run(phase_graph + "F5 F1 F2")
assert all(bar(output) == analysis_bar for output in [field_only, neither, field_and_null, null_only])
assert len({last_plot(output) for output in [field_only, neither, field_and_null, null_only]}) == 4
assert "TEXT 12 24 N1\n" not in tail(neither)
assert "TEXT 12 24 N1\n" in tail(field_and_null)
assert "TEXT 37 24 N2\n" in tail(field_and_null)
restored = run(phase_graph + "F5 F1 F2 F1 F2")
assert last_plot(restored) == last_plot(field_only)
retained = run(phase_graph + "F5 F1 F2 EXIT F4 F1 F4 F2 F5")
assert last_plot(retained) == last_plot(null_only)

# EQPT finds the oscillator origin; INFO provides both contextual help and the
# selected equilibrium's Jacobian/eigenvalue detail.
help_frame = tail(run(phase_graph + "F5 F4"))
assert "TEXT 14 9 Phase analysis\n" in help_frame
assert "Autonomous 2D systems only." in help_frame
equilibrium = tail(run(phase_graph + "F5 F3"))
assert re.search(r"EQPT 1/1.*y1=.*y2=", equilibrium)
assert "Linearized: Center / Neutral candidate" in equilibrium
info = tail(run(phase_graph + "F5 F3 F4"))
assert "TEXT 14 9 Equilibrium / INFO\n" in info
assert all(label in info for label in ["Linearized:", "J:", "L1:", "L2:"])
assert bar(run(phase_graph + "F5 F3 F4 EXE")) == analysis_bar

# Time-dependent systems may show a field frozen at x0, but equilibrium search
# is deliberately limited to autonomous systems.
nonautonomous_phase = "4 2 F6 XOT EXE F6 F6 F6 F4 F2 "
nonautonomous = tail(run(nonautonomous_phase))
assert "Field at x=0" in nonautonomous
blocked = tail(run(nonautonomous_phase + "F5 F3"))
assert "TEXT 14 9 EQPT\n" in blocked
assert "Autonomous systems only." in blocked

# Exercise the real iterative graph dispatcher through 120 projection changes.
soak = run(phase_graph + "F4 F1 F4 F2 " * 60, timeout=60)
assert bar(soak) == phase_bar
assert last_plot(soak) == last_plot(phase)
assert last_metrics(soak) == last_metrics(phase)

print(
    "Phase UI: TIME/PHASE/TABLE, independent windows, combined TRACE, "
    "FIELD/NULL, EQPT/INFO, autonomous policy and 120 mode transitions passed."
)

# Parameter INIT remains the TIME-window solver default even after Phase use.
parameter_init = tail(run(phase_graph + "EXIT F1"))
assert "TEXT 144 35 -6\n" in parameter_init and "TEXT 144 57 6\n" in parameter_init
