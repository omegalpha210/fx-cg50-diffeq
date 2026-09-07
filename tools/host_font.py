#!/usr/bin/env python3
from PIL import Image
from pathlib import Path
im=Image.open('tests/host/vendor/font8x9.png').convert('RGB')
glyphs=[];widths=[]
for i in range(95):
    col=i%(im.width//10);row=i//(im.width//10)
    g=im.crop((col*10+1,row*13+1,col*10+9,row*13+12))
    left,right=0,8
    blank=lambda x: all(g.getpixel((x,y))==(255,255,255) for y in range(11))
    while left+1<right and blank(left):left+=1
    while right-1>left and blank(right-1):right-=1
    widths.append(right-left)
    glyphs.append([sum((g.getpixel((x+left,y))==(0,0,0))<<x for x in range(right-left)) for y in range(11)])
text='/* Generated from the attributed upstream gint atlas. */\n'
text+='static const unsigned char font_width[95]={'+','.join(map(str,widths))+'};\n'
text+='static const unsigned char font_rows[95][11]={\n'+',\n'.join('{'+','.join(map(str,row))+'}' for row in glyphs)+'\n};\n'
Path('tests/host/font_data.h').write_text(text)
