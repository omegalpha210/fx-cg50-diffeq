#!/usr/bin/env python3
"""Proposal mockups rendered with the same gint 8x9 atlas the host harness uses.
Logical UI coordinates (384x216) are offset by (6,4) inside a 396x224 frame,
exactly like include/ui.h. Colors are quantised to RGB565 like the device."""
import re, sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(sys.argv[1])
OUT = Path(sys.argv[2]); OUT.mkdir(parents=True, exist_ok=True)
CAP = ROOT / 'docs/captures'
BCAP = ROOT / 'build/captures'

src = (ROOT / 'tests/host/font_data.h').read_text()
WID = list(map(int, re.search(r'font_width\[95\]=\{([^}]*)\}', src).group(1).split(',')))
ROWS = [list(map(int, r.split(','))) for r in re.findall(r'\{([0-9,]+)\}', src.split('font_rows')[1])]
assert len(ROWS) == 95

def c5(r, g, b):  # C_RGB(r,g,b) 5-bit each -> RGB888 after RGB565 quantisation
    return (r * 255 // 31, (g * 2) * 255 // 63, b * 255 // 31)
def hx(v):  # raw RGB565
    return (((v >> 11) & 31) * 255 // 31, ((v >> 5) & 63) * 255 // 63, (v & 31) * 255 // 31)

WHITE, BLACK = (255, 255, 255), (0, 0, 0)
# --- current tokens (include/ui.h) ---
UI_BLUE = c5(3, 10, 24)
# --- proposed tokens ---
NAVY = c5(4, 8, 17)        # header / tabs base
ACCENT = c5(6, 16, 31)     # focus + header underline
INK = c5(3, 6, 10)
MUTED = c5(12, 14, 17)
LINE = c5(26, 27, 29)
SURF = c5(29, 30, 31)
SEL_BG = c5(25, 28, 31)
TAB = c5(5, 7, 11)
ERR = c5(26, 3, 3)
ERR_BG = c5(31, 28, 28)
OK_GREEN = c5(3, 22, 8)
SEM = {'INIT': hx(0xffe0), 'ADV': c5(20, 22, 26), 'V-WIN': c5(31, 17, 0), 'NORMAL': c5(31, 17, 0),
       'SET': hx(0x37e6), 'FAST': hx(0x37e6), 'NEXT': hx(0x07ff), 'FASTER': hx(0x07ff),
       'PREV': hx(0xf81f), 'YES': hx(0x37e6)}
PRIMARY = {'GRAPH': c5(26, 4, 4), 'RUN': c5(26, 4, 4)}

class Frame:
    def __init__(self, base=None):
        self.im = base.copy().convert('RGB') if base else Image.new('RGB', (396, 224), WHITE)
        self.px = self.im.load()
    def pix(self, x, y, c):
        x += 6; y += 4
        if 0 <= x < 396 and 0 <= y < 224: self.px[x, y] = c
    def rect(self, x, y, w, h, c):
        if w > 0 and h > 0:
            ImageDraw.Draw(self.im).rectangle((x + 6, y + 4, x + 6 + w - 1, y + 4 + h - 1), fill=c)
    def line(self, x1, y1, x2, y2, c):
        ImageDraw.Draw(self.im).line((x1 + 6, y1 + 4, x2 + 6, y2 + 4), fill=c)
    def box(self, x, y, w, h, c):
        self.line(x, y, x + w - 1, y, c); self.line(x, y + h - 1, x + w - 1, y + h - 1, c)
        self.line(x, y, x, y + h - 1, c); self.line(x + w - 1, y, x + w - 1, y + h - 1, c)
    def rrect(self, x, y, w, h, c, bg=WHITE, r=1, top_only=False):
        self.rect(x, y, w, h, c)
        corners = [(x, y), (x + w - 1, y)] + ([] if top_only else [(x, y + h - 1), (x + w - 1, y + h - 1)])
        for cx, cy in corners:
            self.pix(cx, cy, bg)
            if r > 1:
                dx = 1 if cx == x else -1; dy = 1 if cy == y else -1
                self.pix(cx + dx, cy, bg); self.pix(cx, cy + dy, bg)
    def rbox(self, x, y, w, h, c, fill=None, bg=WHITE):
        if fill: self.rect(x, y, w, h, fill)
        self.line(x + 1, y, x + w - 2, y, c); self.line(x + 1, y + h - 1, x + w - 2, y + h - 1, c)
        self.line(x, y + 1, x, y + h - 2, c); self.line(x + w - 1, y + 1, x + w - 1, y + h - 2, c)
        for cx, cy in [(x, y), (x + w - 1, y), (x, y + h - 1), (x + w - 1, y + h - 1)]: self.pix(cx, cy, bg)
    def text(self, x, y, c, s, bold=False):
        for ch in s:
            i = ord(ch) - 32
            if not 0 <= i < 95: continue
            for yy in range(11):
                bits = ROWS[i][yy]
                for xx in range(WID[i]):
                    if bits & (1 << xx):
                        self.pix(x + xx, y + yy, c)
                        if bold: self.pix(x + xx + 1, y + yy, c)
            x += WID[i] + 1 + (1 if bold else 0)
        return x
    def save(self, name):
        self.im.save(OUT / name); return self.im

def tw(s, bold=False):
    w = sum(WID[ord(ch) - 32] + 1 for ch in s if 32 <= ord(ch) < 127) - 1
    return w + (len(s) if bold else 0)

# ---------------------------------------------------------------- components
def header(f, title, step=None, pos=None):
    f.rect(0, 0, 384, 21, NAVY); f.rect(0, 21, 384, 2, ACCENT)
    f.text(8, 5, WHITE, title, bold=True)
    if step:
        sx = 376 - (3 * 12 + 2 * 3)
        for k in range(3):
            x = sx + k * 15
            if k + 1 < step: f.rect(x, 9, 12, 4, c5(14, 19, 27))
            elif k + 1 == step: f.rect(x, 8, 12, 6, WHITE)
            else: f.box(x, 9, 12, 4, c5(11, 15, 23))
        label = f'{step}/3'; f.text(sx - 6 - tw(label), 5, c5(22, 25, 30), label)
    if pos:
        f.text(376 - tw(pos), 5, c5(22, 25, 30), pos)

def tri(f, x, y, d, c, n=3):  # small arrow head: d in 'LRUD'
    for k in range(n):
        if d == 'L': f.line(x + k, y - k, x + k, y + k, c)
        if d == 'R': f.line(x + n - 1 - k, y - k, x + n - 1 - k, y + k, c)
        if d == 'U': f.line(x - k, y + k, x + k, y + k, c)
        if d == 'D': f.line(x - k, y + n - 1 - k, x + k, y + n - 1 - k, c)

def keycap(f, x, y, label, color=INK):
    """Rounded key-cap chip; label may be text or 'LR'/'UD' arrows."""
    if label in ('LR', 'UD'):
        w = 23; f.rbox(x, y - 2, w, 13, LINE, fill=SURF)
        if label == 'LR': tri(f, x + 4, y + 4, 'L', INK, 4); tri(f, x + 15, y + 4, 'R', INK, 4)
        else: tri(f, x + 7, y + 1, 'U', INK, 4); tri(f, x + 16, y + 3, 'D', INK, 4)
        return x + w
    w = tw(label) + 8; f.rbox(x, y - 2, w, 13, LINE, fill=SURF); f.text(x + 4, y, color, label)
    return x + w

def hint(f, parts, y=184, x=8):
    """parts: list of ('key', label[, color]) or ('txt', text)."""
    for p in parts:
        if p[0] == 'key': x = keycap(f, x, y, p[1], p[2] if len(p) > 2 else INK) + 4
        else: x = f.text(x, y, MUTED, p[1]) + 9

def softkeys(f, keys, selected=None):
    f.rect(0, 197, 384, 19, WHITE)
    for i, k in enumerate(keys):
        if not k: continue
        x = i * 64 + 1
        if k in PRIMARY:
            f.rrect(x, 199, 62, 17, PRIMARY[k], r=2, top_only=True)
            f.text(x + (62 - tw(k, True)) // 2, 203, WHITE, k, bold=True); continue
        sel = k == selected
        f.rrect(x, 199, 62, 17, SEM[k] if sel else TAB, r=2, top_only=True)
        if k == 'COLOR':
            for j, c in enumerate([hx(0xf800), hx(0xfc40), hx(0x37e6), hx(0x07ff), hx(0xf81f)]):
                f.rect(x + 3 + j * 11, 199, 12, 3, c)
        elif k in SEM and not sel:
            f.rect(x + 2, 199, 58, 3, SEM[k])
        fg = BLACK if sel else WHITE
        f.text(x + (62 - tw(k, sel)) // 2, 204, fg, k, bold=sel)

FIELD_LABEL_X, FIELD_VALUE_X = 14, 122
def field(f, y, label, value, state='normal', chip=None, err_col=None):
    """state: normal | selected | edit | error"""
    if state in ('selected', 'edit', 'error'):
        f.rrect(4, y - 4, 376, 21, ERR_BG if state == 'error' else SEL_BG)
        f.rect(4, y - 4, 3, 21, ERR if state == 'error' else ACCENT)
        f.text(FIELD_LABEL_X, y, INK, label, bold=True)
    else:
        f.text(FIELD_LABEL_X, y, MUTED, label)
        f.line(10, y + 17, 374, y + 17, LINE)
    if state in ('edit', 'error'):
        bc = ERR if state == 'error' else ACCENT
        f.rect(FIELD_VALUE_X - 6, y - 2, 254, 17, WHITE)
        f.box(FIELD_VALUE_X - 6, y - 2, 254, 17, bc); f.box(FIELD_VALUE_X - 5, y - 1, 252, 15, bc)
        end = f.text(FIELD_VALUE_X, y, INK, value)
        if err_col is not None:
            ex = FIELD_VALUE_X + (tw(value[:err_col]) + 1 if err_col else 0)
            f.rect(ex, y + 11, 6, 2, ERR); tri(f, ex + 3, y + 13, 'U', ERR)
        f.rect(end + 1, y - 1, 1, 13, bc)
    else:
        f.text(FIELD_VALUE_X, y, INK, value, bold=(state == 'selected'))
    if chip:
        w = tw(chip) + 8; x = 372 - w
        f.rbox(x, y - 2, w, 13, MUTED if state == 'normal' else ACCENT, fill=WHITE,
               bg=WHITE if state == 'normal' else SEL_BG)
        f.text(x + 4, y, MUTED if state == 'normal' else ACCENT, chip)

def banner(f, message, kind='error'):
    f.rect(0, 178, 384, 19, ERR_BG if kind == 'error' else SURF)
    f.rect(0, 178, 3, 19, ERR)
    cx, cy = 13, 187
    for dx in range(-5, 6):
        for dy in range(-5, 6):
            if dx * dx + dy * dy <= 25: f.pix(cx + dx, cy + dy, ERR)
    f.rect(cx, cy - 3, 1, 4, WHITE); f.rect(cx, cy + 2, 1, 1, WHITE)
    f.text(24, 182, ERR, message)

# ---------------------------------------------------------------- composites
TTF = '/System/Library/Fonts/AppleSDGothicNeo.ttc'
def font(sz):
    try: return ImageFont.truetype(TTF, sz)
    except Exception: return ImageFont.load_default()

def compare(name, before, after, cap_b='현재', cap_a='제안', scale=2):
    W, H = 396 * scale, 224 * scale
    pad, top = 16, 40
    sheet = Image.new('RGB', (W * 2 + pad * 3, H + top + pad), (236, 240, 246))
    d = ImageDraw.Draw(sheet); fn = font(22)
    for k, (im, cap) in enumerate([(before, cap_b), (after, cap_a)]):
        x = pad + k * (W + pad)
        d.text((x + 2, 8), cap, fill=(60, 66, 80) if k == 0 else (20, 70, 190), font=fn)
        sheet.paste((255, 255, 255), (x - 2, top - 2, x + W + 2, top + H + 2))
        d.rectangle((x - 2, top - 2, x + W + 1, top + H + 1), outline=(150, 160, 180))
        sheet.paste(im.convert('RGB').resize((W, H), Image.NEAREST), (x, top))
    sheet.save(OUT / name)

def gallery(name, items, scale=2, cols=2):
    W, H = 396 * scale, 224 * scale
    pad, top = 16, 40
    rows = (len(items) + cols - 1) // cols
    sheet = Image.new('RGB', (cols * W + (cols + 1) * pad, rows * (H + top) + pad), (236, 240, 246))
    d = ImageDraw.Draw(sheet); fn = font(22)
    for k, (im, cap) in enumerate(items):
        x = pad + (k % cols) * (W + pad); y = (k // cols) * (H + top)
        d.text((x + 2, y + 8), cap, fill=(20, 70, 190), font=fn)
        d.rectangle((x - 2, y + top - 2, x + W + 1, y + top + H + 1), outline=(150, 160, 180))
        sheet.paste(im.convert('RGB').resize((W, H), Image.NEAREST), (x, y + top))
    sheet.save(OUT / name)

def load(p): return Image.open(p).convert('RGB')

# ================================================================ 1. softkey bar
def softkey_strip():
    cur = load(CAP / 'solver-parameters.png').crop((0, 200, 396, 224))
    new = Frame(); softkeys(new, ['INIT', 'ADV', 'V-WIN', 'OUTPUT', 'SET', 'GRAPH'])
    tr_cur = load(CAP / 'graph-trace.png').crop((0, 200, 396, 224))
    tr = Frame(); softkeys(tr, ['INIT', 'NORMAL', 'FAST', 'FASTER', 'LEFT', 'RIGHT'], selected='NORMAL')
    eq_cur = load(CAP / 'equation-entry.png').crop((0, 200, 396, 224))
    eq = Frame(); softkeys(eq, ['INIT', '', '', '', '', 'NEXT'])
    s = 3; W = 396 * s; h = 24 * s; pad = 14; top = 34
    sheet = Image.new('RGB', (W * 2 + pad * 3, (h + top) * 3 + pad), (236, 240, 246))
    d = ImageDraw.Draw(sheet); fn = font(22)
    rows = [(cur, new.im.crop((0, 200, 396, 224)), 'Parameters'),
            (tr_cur, tr.im.crop((0, 200, 396, 224)), 'TRACE (NORMAL 선택됨)'),
            (eq_cur, eq.im.crop((0, 200, 396, 224)), 'Equation (빈 슬롯)')]
    for r, (a, b, cap) in enumerate(rows):
        y = r * (h + top)
        for k, im in enumerate((a, b)):
            x = pad + k * (W + pad)
            d.text((x, y + 6), ('현재 · ' if k == 0 else '제안 · ') + cap,
                   fill=(60, 66, 80) if k == 0 else (20, 70, 190), font=fn)
            sheet.paste(im.resize((W, h), Image.NEAREST), (x, y + top))
    sheet.save(OUT / '01-softkeys.png')

# ================================================================ 2. main menu
def main_menu(selected=0):
    base = load(CAP / 'tiles-main-first.png')
    f = Frame(); header(f, 'DIFF EQ')
    tints = [c5(24, 29, 31), c5(31, 27, 21), c5(25, 24, 31), c5(24, 31, 25)]
    badges = [c5(6, 14, 24), c5(24, 12, 2), c5(12, 9, 24), c5(4, 18, 8)]
    names = ['1st', '2nd', 'N-th', 'SYSTEM', 'RECALL', 'SAVE']
    for i in range(6):
        x, y = 4 + (i % 2) * 192, 27 + (i // 2) * 62
        w, h = 184, 58 if i < 4 else 25
        if i < 4:
            f.rrect(x, y, w, h, tints[i], r=2)
            ix, iy = x + (184 - 108) // 2, y + 4
            icon = base.crop((ix + 6, iy + 4, ix + 6 + 108, iy + 4 + 34))
            f.im.paste(icon, (ix + 6, iy + 4))
            f.text(x + (w - tw(names[i], True)) // 2, y + 44, BLACK, names[i], bold=True)
            bc = badges[i]
        else:
            f.rrect(x, y, w, h, SURF, r=2); f.box(x, y, w, h, LINE)
            for cx, cy in [(x, y), (x + w - 1, y), (x, y + h - 1), (x + w - 1, y + h - 1)]: f.pix(cx, cy, WHITE)
            lx = x + (w - tw(names[i], True)) // 2 + 8
            ix, iy = lx - 20, y + 7
            if i == 4:  # folder icon
                f.rect(ix, iy + 2, 14, 9, c5(30, 22, 6)); f.rect(ix, iy, 6, 3, c5(30, 22, 6))
                f.rect(ix + 1, iy + 4, 12, 1, c5(31, 27, 14))
            else:       # floppy icon
                f.rect(ix, iy, 12, 12, c5(6, 12, 22)); f.rect(ix + 3, iy, 6, 4, c5(26, 28, 31))
                f.rect(ix + 2, iy + 7, 8, 5, WHITE)
            f.text(lx, y + 8, INK, names[i], bold=True)
            bc = MUTED
        f.rrect(x + w - 21, y + 4, 16, 15, bc, bg=tints[i] if i < 4 else SURF, r=2)
        d = str(i + 1); f.text(x + w - 13 - tw(d) // 2, y + 6, WHITE, d, bold=False)
        if i == selected:
            f.box(x - 2, y - 2, w + 4, h + 4, ACCENT); f.box(x - 1, y - 1, w + 2, h + 2, ACCENT)
            f.box(x, y, w, h, WHITE)
    hint(f, [('key', 'MENU', ERR), ('txt', 'Main Menu'), ('key', '1-6'), ('txt', 'open directly')])
    softkeys(f, ['', '', '', '', '', 'OPEN'])
    return f

# ================================================================ 3. equation select / edit / error
def equation(state):
    f = Frame(); header(f, 'DIFF EQ / General 1st', step=1)
    f.rrect(4, 27, 376, 20, SURF); f.rect(4, 27, 3, 20, ACCENT)
    f.text(14, 31, MUTED, 'Form'); f.text(54, 31, INK, "y' = f(x,y)", bold=True)
    if state == 'select':
        field(f, 57, "y'", '1-y^2', 'selected')
        f.text(14, 92, MUTED, 'Examples'); x = 80
        for ex in ['1-y^2', 'sin(x)-y', 'x*y']:
            x = keycap(f, x, 92, ex) + 6
        hint(f, [('key', 'LR'), ('txt', 'edit'), ('key', 'EXE', ACCENT), ('txt', 'next step')])
        softkeys(f, ['INIT', '', '', '', '', 'NEXT'])
    elif state == 'edit':
        field(f, 57, "y'", 'sin(x)-y', 'edit')
        f.text(14, 92, MUTED, 'Editing  -  F2 FUNC inserts sinh, cosh, ...')
        hint(f, [('key', 'EXE', ACCENT), ('txt', 'commit / next'), ('key', 'EXIT'), ('txt', 'commit')])
        softkeys(f, ['INIT', 'FUNC', '', '', '', 'NEXT'])
    else:
        field(f, 57, "y'", 'sin(', 'error', err_col=4)
        banner(f, "y': Syntax error at character 5")
        softkeys(f, ['INIT', 'FUNC', '', '', '', 'NEXT'])
    return f

# ================================================================ 4. parameters
def parameters():
    f = Frame(); header(f, 'Parameter', step=3)
    rows = [('Xrange min', '-3', 'AUTO'), ('Xrange max', '3', 'AUTO'), ('Method', 'RK45', None),
            ('h0', '0.1', None), ('RelTol', '1e-06', None), ('AbsTol', '1e-09', None), ('SF', '12', None)]
    for r, (l, v, c) in enumerate(rows):
        y = 31 + r * 22
        if r == 2:
            field(f, y, l, '', 'selected')
            x = FIELD_VALUE_X
            tri(f, x, y + 4, 'L', ACCENT)
            for k, opt in enumerate(['RK4', 'RK45']):
                on = opt == 'RK45'; w = tw(opt, on) + 10; bx = x + 8 + k * 50
                if on: f.rrect(bx, y - 2, w, 14, ACCENT, bg=SEL_BG, r=2)
                f.text(bx + 5, y, WHITE if on else MUTED, opt, bold=on)
            tri(f, x + 8 + 100 + 4, y + 4, 'R', ACCENT)
        else:
            field(f, y, l, v, 'normal', chip=c)
    hint(f, [('key', 'LR'), ('txt', 'switch method')])
    softkeys(f, ['INIT', 'ADV', 'V-WIN', 'OUTPUT', 'SET', 'GRAPH'])
    return f

# ================================================================ 5. IC with list chips
def initial():
    f = Frame(); header(f, 'Initial Conditions', step=2)
    field(f, 31, 'x0', '0')
    field(f, 53, 'y0', '{0,0.5}', 'selected')
    f.text(14, 84, MUTED, 'Solutions'); x = 90
    for k, (v, c) in enumerate([('0', hx(0x07ff)), ('0.5', hx(0xf81f))]):
        w = tw(f'IC{k+1}  y0={v}') + 22
        f.rbox(x, 82, w, 13, LINE, fill=WHITE); f.rect(x + 4, 88, 10, 2, c)
        f.text(x + 18, 84, INK, f'IC{k+1}  y0={v}'); x += w + 6
    f.text(14, 100, MUTED, '2 of max 10 initial values')
    hint(f, [('key', 'LR'), ('txt', 'edit'), ('key', ','), ('txt', 'separate values')])
    softkeys(f, ['INIT', '', '', '', '', 'NEXT'])
    return f

# ================================================================ 6. graph trace readout
def trace():
    base = load(CAP / 'graph-trace.png')
    f = Frame(base)
    f.rect(0, 178, 384, 19, WHITE); f.line(0, 178, 383, 178, LINE)
    green = hx(0x37e6)
    f.rect(8, 186, 12, 3, green)
    x = f.text(24, 183, INK, 'IC1', bold=True) + 12
    for lab, val in [('x', '0.04761905'), ('y', '0.04746091')]:
        x = f.text(x, 183, MUTED, lab) + 5
        x = f.text(x, 183, INK, val) + 14
    softkeys(f, ['INIT', 'NORMAL', 'FAST', 'FASTER', 'LEFT', 'RIGHT'], selected='NORMAL')
    return f

def gsolve():
    base = load(CAP / 'graph-gsolve.png')
    f = Frame(base)
    f.rect(0, 178, 384, 19, WHITE); f.line(0, 178, 383, 178, LINE)
    w = tw('Y-ICPT', True) + 10; f.rrect(6, 181, w, 14, ACCENT, r=2); f.text(11, 183, WHITE, 'Y-ICPT', bold=True)
    x = 6 + w + 10
    for lab, val in [('X', '0'), ('Y', '0')]:
        x = f.text(x, 183, MUTED, lab) + 5; x = f.text(x, 183, INK, val, bold=True) + 14
    f.text(376 - tw('1 / 1'), 183, MUTED, '1 / 1')
    softkeys(f, ['', '', '', '', '', 'BACK'] if False else ['', '', '', '', '', ''])
    f.rrect(5 * 64 + 1, 199, 62, 17, TAB, r=2, top_only=True); f.text(5 * 64 + 1 + (62 - tw('BACK')) // 2, 204, WHITE, 'BACK')
    return f

# ================================================================ 7. table
TABLE = [('-0.3', '-0.29131232', '0.24426691'), ('-0.2', '-0.19737514', '0.33576048'),
         ('-0.1', '-0.099667911', '0.42132871'), ('0', '0', '0.5'),
         ('0.1', '0.099667911', '0.57120242'), ('0.2', '0.19737514', '0.63473431'),
         ('0.3', '0.29131232', '0.69070605')]
def table():
    f = Frame(); header(f, 'Table / initial solutions')
    cols = [(4, 96, 'x', None), (102, 134, 'y1', hx(0x07ff)), (238, 134, 'y2', hx(0xf81f))]
    f.rect(4, 27, 368, 17, SURF)
    for x, w, name, c in cols:
        f.text(x + w - 6 - tw(name, True), 30, INK, name, bold=True)
        f.rect(x, 43, w, 2, c if c else MUTED)
    for r, row in enumerate(TABLE):
        y = 47 + r * 18
        if r % 2: f.rect(4, y, 368, 18, SURF)
        if row[0] == '0': f.rect(4, y, 368, 18, SEL_BG); f.rect(4, y, 2, 18, ACCENT)
        for (x, w, _, _), v in zip(cols, row):
            f.text(x + w - 6 - tw(v), y + 4, INK, v)
    for x, w, _, _ in cols[1:]: f.line(x - 1, 27, x - 1, 172, LINE)
    # scrollbar: rows 28-34 of 61
    f.rect(376, 27, 4, 146, LINE)
    t0 = 27 + int(146 * 27 / 61); th = max(6, int(146 * 7 / 61)); f.rrect(376, t0, 4, th, NAVY, bg=LINE)
    hint(f, [('key', 'UD'), ('txt', 'page')], y=181)
    s = 'Rows 28-34 / 61'; f.text(372 - tw('Step 1') - 12 - tw(s), 181, INK, s)
    w = tw('Step 1') + 8; f.rbox(372 - w, 179, w, 13, LINE, fill=SURF); f.text(376 - w, 181, MUTED, 'Step 1')
    softkeys(f, ['TOP', 'BTM', 'MID', '', 'STAT', 'GRAPH'])
    for i, k in enumerate(['TOP', 'BTM', 'MID', '', 'STAT']):
        pass
    return f

# ================================================================ 8. busy bar
def busy():
    base = load(BCAP / 'audit/audit-drawing-busy.png')
    f = Frame(base)
    f.rect(0, 197, 384, 19, WHITE)
    f.rrect(1, 199, 382, 17, TAB, r=2, top_only=True)
    x = f.text(10, 204, WHITE, 'Drawing', bold=True) + 10
    f.rect(x, 206, 96, 4, c5(9, 12, 18))
    f.rect(x + 30, 206, 26, 4, ACCENT)
    for k in range(3): f.rect(x + 104 + k * 6, 207, 3, 3, ACCENT if k == 1 else c5(11, 14, 20))
    w = tw('EXIT') + 8; kx = 376 - w - tw('cancel') - 6
    f.rbox(kx, 202, w, 13, c5(14, 17, 23), fill=c5(8, 11, 17), bg=TAB); f.text(kx + 4, 204, WHITE, 'EXIT')
    f.text(kx + w + 6, 204, c5(22, 25, 30), 'cancel')
    return f

# ================================================================ 9. save modal
def save_modal():
    under = main_menu().im.copy()
    px = under.load(); dim = NAVY
    for y in range(224):
        for x in range(396):
            r, g, b = px[x, y]
            px[x, y] = ((r + dim[0] * 2) // 3, (g + dim[1] * 2) // 3, (b + dim[2] * 2) // 3)
    f = Frame(under)
    x, y, w, h = 62, 46, 260, 104
    f.rect(x + 3, y + 3, w, h, c5(2, 4, 8))
    f.rrect(x, y, w, h, WHITE, bg=c5(9, 12, 20), r=2)
    f.rrect(x, y, w, 22, NAVY, bg=c5(9, 12, 20), r=2, top_only=True); f.rect(x, y + 22, w, 2, ACCENT)
    f.text(x + 10, y + 6, WHITE, 'Save session', bold=True)
    ix, iy = x + 14, y + 36
    f.rect(ix, iy, 18, 18, c5(6, 12, 22)); f.rect(ix + 4, iy, 10, 6, c5(26, 28, 31)); f.rect(ix + 3, iy + 10, 12, 8, WHITE)
    f.text(x + 44, y + 36, INK, 'Save current session?', bold=True)
    f.text(x + 44, y + 52, MUTED, 'Restore later with RECALL.')
    hint(f, [('key', 'EXE', ACCENT), ('txt', 'Yes'), ('key', 'EXIT'), ('txt', 'No')], y=y + 82, x=x + 44)
    f.rect(0, 197, 384, 19, WHITE)
    softkeys(f, ['', '', '', '', 'NO', 'YES'])
    return f

# ================================================================ 10. solver info grouped
def solver_info():
    f = Frame(); header(f, 'Solver Info')
    def section(y, title):
        f.text(8, y, ACCENT, title, bold=True); f.line(8 + tw(title, True) + 6, y + 5, 376, y + 5, LINE)
    def kv(x, y, k, v):
        f.text(x, y, MUTED, k); f.text(x + 178 - 12 - tw(v, True), y, INK, v, bold=True)
    section(28, 'Tolerance')
    kv(8, 44, 'RelTol', '1e-06'); kv(196, 44, 'AbsTol', '1e-09')
    section(64, 'Steps')
    kv(8, 80, 'Accepted', '462'); kv(196, 80, 'Rejected', '0')
    kv(8, 96, 'Attempts', '462'); kv(196, 96, 'RHS evals', '3234')
    f.rect(8, 114, 368, 6, LINE); f.rect(8, 114, 368, 6, OK_GREEN)
    f.text(8, 124, MUTED, 'Acceptance 100%  (462 / 462)')
    section(144, 'Step size'); kv(8, 160, 'h min', '4.4408921e-16')
    hint(f, [('txt', 'Last trajectory run'), ('key', 'UD'), ('txt', 'scroll')])
    return f

# ---------------------------------------------------------------- run
softkey_strip()
m = main_menu()
compare('02-main-menu.png', load(CAP / 'tiles-main-first.png'), m.im)
eqs = equation('select')
compare('03-equation-select.png', load(CAP / 'equation-entry.png'), eqs.im)
eqe = equation('edit')
compare('04-equation-edit.png', load(BCAP / 'consistency/consistency-equation-edit.png'), eqe.im)
eqr = equation('error')
compare('05-input-error.png', load(BCAP / 'consistency/consistency-input-error.png'), eqr.im)
compare('06-initial-conditions.png', load(CAP / 'initial-conditions.png'), initial().im)
compare('07-parameters.png', load(CAP / 'solver-parameters.png'), parameters().im)
compare('08-trace-readout.png', load(CAP / 'graph-trace.png'), trace().im)
compare('09-gsolve-result.png', load(CAP / 'graph-gsolve.png'), gsolve().im)
compare('10-table.png', load(CAP / 'table-view.png'), table().im)
compare('11-busy.png', load(BCAP / 'audit/audit-drawing-busy.png'), busy().im)
compare('12-save-confirm.png', load(BCAP / 'consistency/consistency-save.png'), save_modal().im)
compare('13-solver-info.png', load(CAP / 'solver-diagnostics.png'), solver_info().im)
print('ok', sorted(p.name for p in OUT.iterdir()))
