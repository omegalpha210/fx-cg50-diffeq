#!/usr/bin/env python3
"""Render the actual UI sources through the host adapter; not an emulator."""
import os
from pathlib import Path
import subprocess
import tempfile
from PIL import Image, ImageDraw, ImageFont

root=Path(__file__).resolve().parents[1]
executable=root/'build-host/host_app'
output=root/'docs/ui-review'
output.mkdir(parents=True,exist_ok=True)
cases=[('main','Main',''),
    ('save-confirm','SAVE / explicit confirmation','2 EXIT 6'),
    ('equation','Equation / NEXT','1 4 NEG A:SUB EXE'),
    ('parameter','Parameters / GRAPH','1 4 F6 F6 DOWN DOWN RIGHT'),
    ('vwindow','V-Window','1 4 F6 F6 F3 NEG 7 EXE'),
    ('initial-conditions','First-order IC / list input','1 1 F6 DOWN S:MUL 0 COMMA 1 S:DIV EXE'),
    ('output','Output / selected state ON-OFF','2 F6 F6 F4 DOWN RIGHT'),
    ('graph','Full-size Graph / 2 IC colors','1 1 F6 DOWN S:MUL 0 COMMA 1 S:DIV EXE F6 F6'),
    ('gsolve','G-Solve ROOT result','2 F6 F6 F6 F5 F1 EXE'),
    ('table','Numerical Table','2 F6 F6 F6 F4'),
    ('zoom','Inline Zoom menu','2 F6 F6 F6 F2'),
    ('system9','Nine variables / formula above fields','4 9 F6 '+ 'DOWN '*8),
    ('ic9','Nine-state IC scroll','4 9 F6 F6 '+'DOWN '*9),
    ('color-chooser','Output / six-color chooser','2 F6 F6 F4 DOWN F3'),
    ('trace','TRACE / Xdot interpolation','2 F6 F6 F6 F1 RIGHT'),
    ('func','FUNC / softkeys only','1 4 LEFT F2'),
    ('var','VAR / second page','4 9 F6 LEFT F3 F6'),
    ('scalar-prompt','Y-CAL / scalar RUN','2 F6 F6 F6 F5 F6 F1 EXE 2 DOT 5'),
    ('graph-settings','Graph settings / INIT','2 F6 F6 F5')]
cases += [
    ('table-limit','Table / last valid row', '1 1 LEFT ACON F2 F2 XOT RIGHTP EXE F6 F6 F6 F4 F2'),
    ('table-scroll','Table / frozen x', '4 4 F6 F6 F6 F6 F4 RIGHT RIGHT'),
    ('preflight','Preflight / no calculation', '2 F6 F6 DOWN DOWN DOWN 0 DOT 0 0 0 1 EXE F6'),
    ('trace-follow','TRACE / fixed horizontal viewport', '2 EXE EXE EXE F1 '+'RIGHT '*160),
    ('zoom-pan','ZOOM / arrow pan keeps menu', '2 EXE EXE EXE F2 RIGHT')]
ux_start=len(cases)
field='1 4 F6 F6 F5 '
arrow='DOWN DOWN DOWN F3 '
cases += [
    ('func-select','SELECT / F2 blank','1 4'),
    ('func-exit','FUNC EXIT / incomplete draft retained','1 4 LEFT ACON F2 F2 F2 EXIT'),
    ('trace-fast','TRACE / FAST selected','2 EXE EXE EXE F1 F3 RIGHT'),
    ('trace-faster','TRACE / FASTER selected','2 EXE EXE EXE F1 F4 RIGHT'),
    ('field-settings','Graph Settings / appearance only',field+'DOWN DOWN'),
    ('field-palette','Field / separate six-color chooser',field+arrow+'RIGHT'),
    ('field-arrow-zero','Arrow / zero slope points right','1 4 0 EXE F6 F6 F5 '+arrow+'EXE EXIT F6'),
    ('field-arrow-up','Arrow / positive slope','1 4 1 EXE F6 F6 F5 '+arrow+'RIGHT EXE EXIT F6'),
    ('field-arrow-down','Arrow / negative slope','1 4 NEG 1 EXE F6 F6 F5 '+arrow+'RIGHT RIGHT EXE EXIT F6'),
    ('field-arrow-max','SF50 / arrows remain visible','1 4 F6 F6 DOWN DOWN DOWN DOWN DOWN 5 0 EXE F5 '+arrow+'DOWN RIGHT EXE EXIT F6'),
    ('output-first','Output / y only, no x row','1 4 F6 F6 F4'),
    ('output-enabled','Output / shared ON-OFF','2 F6 F6 F4 DOWN RIGHT')]
cases += [
    ('table-solutions','Table / manual example MID','1 1 F6 DOWN S:MUL 0 COMMA 1 S:DIV EXE F6 F6 F4'),
    ('table-top','Table / smallest x','1 1 F6 DOWN S:MUL 0 COMMA 1 S:DIV EXE F6 F6 F4 F1'),
    ('table-bottom','Table / largest x','1 1 F6 DOWN S:MUL 0 COMMA 1 S:DIV EXE F6 F6 F4 F2'),
    ('table-many','Table / IC columns, frozen x','1 1 F6 DOWN S:MUL 0 COMMA 1 COMMA 2 COMMA 3 COMMA 4 COMMA 5 S:DIV EXE F6 F6 F4 RIGHT RIGHT'),
    ('ic-second','2nd / one complete vector','2 F6'),
    ('ic-list-error','IC / malformed list','1 1 F6 DOWN S:MUL 0 COMMA S:DIV F6'),
    ('sf-parameter','Parameters / SF selected','1 4 F6 F6 DOWN DOWN DOWN DOWN DOWN')]
polish_start=len(cases)
cases += [
    ('polish-equation-select','Equation SELECT / contextual hint','1 4'),
    ('polish-equation-edit','Equation EDIT / common hint','1 4 LEFT'),
    ('polish-ic-edit','IC numeric EDIT / common hint','2 F6 DOWN 2'),
    ('polish-list-edit','IC list EDIT / common hint','1 1 F6 DOWN S:MUL 0 COMMA 1 S:DIV'),
    ('polish-vwindow-edit','V-Window EDIT / common hint','2 F6 F6 F3 DOWN DOWN 2'),
    ('polish-parameter-edit','Parameters EDIT / common hint','2 F6 F6 DOWN DOWN DOWN 0 DOT 0 5'),
    ('polish-sf-first','First-order / SF selected','1 4 F6 F6 DOWN DOWN DOWN DOWN DOWN'),
    ('polish-sf-second','2nd / Step then Max steps','2 F6 F6 DOWN DOWN DOWN DOWN 2 EXE'),
    ('polish-sf-nth','N-th9 / no SF row','3 9 F6 F6 F6'),
    ('polish-sf-system','SYS9 / no SF row','4 9 F6 F6 F6'),
    ('polish-field-swatch','Field / Pale Magenta swatch','1 4 F6 F6 F5 DOWN DOWN DOWN F3 DOWN EXE'),
    ('polish-prev','Graph / semantic INIT','2 F6 F6 F6'),
    ('polish-ic-full','SYS9 IC / seven rows clear of help','4 9 F6 F6'),
    ('polish-output-full','SYS9 OUTPUT / seven rows + one hint','4 9 F6 F6 F6 F4')]
trace_ic_start=len(cases)
ten='1 4 0 EXE F6 DOWN S:MUL 0 COMMA 1 COMMA 2 COMMA 3 COMMA 4 COMMA 5 COMMA 6 COMMA 7 COMMA 8 COMMA 9 S:DIV EXE '
cases += [
    ('trace-y-upper','TRACE / Y upper follow + RIGHT endpoint','1 4 1 EXE F6 F6 F6 F1 F6'),
    ('trace-y-lower','TRACE / Y lower follow + LEFT endpoint','1 4 1 EXE F6 F6 F6 F1 F5'),
    ('trace-speed-fast','TRACE / FAST selected','2 F6 F6 F6 F1 F3'),
    ('trace-speed-faster','TRACE / FASTER selected','2 F6 F6 F6 F1 F4'),
    ('settings-toggle','Graph Settings / arrows only','1 4 F6 F6 F5'),
    ('ic-ten','First-order / ten initial values',ten),
    ('table-ten','Table / tenth solution, frozen x',ten+'F6 F6 F4 '+'RIGHT '*10),
    ('ic-count-limit','IC / separate ten-value limit',ten+'LEFT ACON S:MUL '+ '0 COMMA '*10+'0 S:DIV F6'),
    ('ic-length-limit','IC / bounded 191-character limit','1 4 F6 DOWN '+'0 '*192+'EXE')]
assert len({name for name,_,_ in cases})==len(cases)
sheet=Image.new('RGB',(816,8+264*((len(cases)+1)//2)),'#e8eef5')
draw=ImageDraw.Draw(sheet)
font=ImageFont.load_default()
for i,(name,label,keys) in enumerate(cases):
    with tempfile.TemporaryDirectory(dir=root/'build-host') as directory:
        env=dict(os.environ,DIFFEQ_HOST_KEYS=keys,DIFFEQ_HOST_OUT=directory)
        subprocess.run([str(executable)],cwd=directory,env=env,check=True,
                       stdout=subprocess.DEVNULL,timeout=30)
        frame=sorted(Path(directory).glob('*.ppm'))[-1]
        im=Image.open(frame).convert('RGB')
        im.save(output/f'{name}.png')
    x=8+(i%2)*404;y=8+(i//2)*264
    draw.text((x,y),label,fill='#193857',font=font)
    sheet.paste(im,(x,y+25))
sheet.save(output/'host-overview.png')
for name,start in [('ux-field-overview',ux_start),('polish-overview',polish_start),('trace-ic-overview',trace_ic_start)]:
    sheet=Image.new('RGB',(816,8+264*((len(cases)-start+1)//2)),'#e8eef5')
    draw=ImageDraw.Draw(sheet)
    for i,(filename,label,_) in enumerate(cases[start:]):
        x=8+(i%2)*404;y=8+(i//2)*264
        draw.text((x,y),label,fill='#193857',font=font)
        sheet.paste(Image.open(output/f'{filename}.png'),(x,y+25))
    sheet.save(output/(name+'.png'))
review=['main','save-confirm','equation','initial-conditions','parameter','field-settings','output','vwindow']
sheet=Image.new('RGB',(816,8+264*4),'#e8eef5');draw=ImageDraw.Draw(sheet)
for i,name in enumerate(review):
    x=8+(i%2)*404;y=8+(i//2)*264
    draw.text((x,y),name,fill='#193857',font=font)
    sheet.paste(Image.open(output/(name+'.png')),(x,y+25))
sheet.save(output/'workflow-overview.png')
print(f'Saved {len(cases)} host framebuffer views to docs/ui-review/. HARDWARE RETEST REQUIRED.')
