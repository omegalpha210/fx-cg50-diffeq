#!/usr/bin/env python3
"""Reproduce the attributed host-only gint font table."""
import argparse
from pathlib import Path

from PIL import Image

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--check', action='store_true',
                    help='verify the checked-in table without writing it')
args = parser.parse_args()
im = Image.open(root / 'tests/host/vendor/font8x9.png').convert('RGB')
glyphs, widths = [], []
for i in range(95):
    col, row = i % (im.width // 10), i // (im.width // 10)
    glyph = im.crop((col * 10 + 1, row * 13 + 1, col * 10 + 9, row * 13 + 12))
    left, right = 0, 8

    def blank(x):
        return all(glyph.getpixel((x, y)) == (255, 255, 255) for y in range(11))

    while left + 1 < right and blank(left):
        left += 1
    while right - 1 > left and blank(right - 1):
        right -= 1
    widths.append(right - left)
    glyphs.append([
        sum((glyph.getpixel((x + left, y)) == (0, 0, 0)) << x
            for x in range(right - left))
        for y in range(11)
    ])

text = '/* Generated from the attributed upstream gint atlas. */\n'
text += 'static const unsigned char font_width[95]={' + ','.join(map(str, widths)) + '};\n'
text += 'static const unsigned char font_rows[95][11]={\n'
text += ',\n'.join('{' + ','.join(map(str, row)) + '}' for row in glyphs) + '\n};\n'
target = root / 'tests/host/font_data.h'
if args.check:
    if target.read_text() != text:
        parser.exit(1, 'Host font table differs; regenerate with tools/host_font.py\n')
else:
    target.write_text(text)
