#!/usr/bin/env python3
"""Draw our mathematical menu icons; fxSDK generate_g3a packages the PNGs.
No external images/fonts. Keep the bottom title strip free for the OS label.
"""
import math
from pathlib import Path
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1]
scale = 4
for selected in (False, True):
    bg = '#164981' if selected else '#ffffff'
    field = '#527baa' if selected else '#bfd1e1'
    axis = '#80a8cd' if selected else '#8ca2b8'
    curve = '#ffffff' if selected else '#164981'
    accent = '#52e0c7' if selected else '#069b8e'
    im = Image.new('RGB', (92*scale, 64*scale), bg)
    d = ImageDraw.Draw(im)
    def line(points, color, width=1):
        d.line([(round(x*scale), round(y*scale)) for x,y in points], fill=color,
               width=round(width*scale), joint='curve')
    def point(x,y):
        return 46 + 17*x, 25 - 9*y
    # Direction field of y'=1-y^2. Two translated tanh(x) solutions form a family.
    for x in (-2,-1.4,-.8,-.2,.4,1,1.6,2.2):
        for y in (-1.6,-.8,0,.8,1.6):
            slope = -(9/17)*(1-y*y)
            dx = 2.4/math.hypot(1,slope)
            px,py = point(x,y)
            line([(px-dx,py-dx*slope),(px+dx,py+dx*slope)],field,.85)
    line([(6,25),(85,25)],axis,.65)
    line([(46,4),(46,47)],axis,.65)
    for offset,color in ((-.65,curve),(.65,accent)):
        line([point(-2.3+i*4.6/180, math.tanh(-2.3+i*4.6/180+offset)) for i in range(181)],color,2.1)
    px,py=point(0,math.tanh(.65))
    d.ellipse(((px-2)*scale,(py-2)*scale,(px+2)*scale,(py+2)*scale),fill=accent)
    im.resize((92,64), Image.Resampling.LANCZOS).save(root/'assets'/f'icon-{"sel" if selected else "uns"}.png')
