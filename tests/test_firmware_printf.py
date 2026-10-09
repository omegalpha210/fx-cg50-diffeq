"""Firmware printf compatibility: every format in production source must be
accepted by fxlibc, the libc linked into the G3A.

Host tests run on the host libc, which accepts more than fxlibc does. fxlibc's
format parser (src/stdio/printf/print.c, parse_fmt) has no '*' width or
precision: "%.*g" leaves the int precision argument unconsumed, every later
argument shifts, and a following %s dereferences double bits as a pointer
(TLB miss on hardware; G-Solve results crashed this way). Such formats pass
every host test, so this static check is the only guard.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
# fxlibc formatters: c d i m n o p s u x X, plus e E f F g G once floating-point
# output is enabled (the add-in already relies on %g).
SUPPORTED = set('cdimnopsuxX' 'eEfFgG' '%')
SPEC = re.compile(r'%([-+ #0]*)(\*|\d+)?(?:\.(\*|\d*))?(hh|h|ll|l|q|j|z|Z|t|L)?([A-Za-z%])')
LITERAL = re.compile(r'"((?:[^"\\\n]|\\.)*)"')
COMMENT = re.compile(r'/\*.*?\*/|//[^\n]*', re.S)


def problems(text):
    found = []
    for literal in LITERAL.findall(COMMENT.sub('', text)):
        for spec in SPEC.finditer(literal):
            width, precision, conversion = spec.group(2), spec.group(3), spec.group(5)
            if width == '*' or precision == '*':
                found.append((literal, spec.group(0), "'*' width/precision"))
            elif conversion not in SUPPORTED:
                found.append((literal, spec.group(0), 'unsupported conversion'))
    return found


# The checker itself must catch the shipped defect and accept the fixed form.
assert problems('snprintf(t,n,"x=%.*g %s",8,x,s);')
assert problems('printf("%*d",4,n);')
assert not problems('snprintf(t,n,"x=%.8g %s=%lu %%",x,s,n); /* "%.*g" in a comment */')

failures = []
for path in sorted((ROOT / 'src').rglob('*.[ch]')) + sorted((ROOT / 'include').rglob('*.h')):
    for literal, spec, reason in problems(path.read_text()):
        failures.append(f'{path.relative_to(ROOT)}: "{literal}" uses {spec} ({reason})')
if failures:
    print('\n'.join(failures))
    sys.exit(1)
print('Firmware printf: all production formats are within the fxlibc subset (no * width/precision).')
