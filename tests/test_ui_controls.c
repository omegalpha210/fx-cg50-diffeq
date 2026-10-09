#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Document d,expected;
static unsigned char ink[DWIDTH*DHEIGHT];
unsigned host_text_glyphs(void);
static unsigned count_color(int color)
{unsigned n=0;for(int i=0;i<DWIDTH*DHEIGHT;i++)n+=gint_vram[i]==color;return n;}
static void hints(void)
{
    int empty;dnsize("EXE",0,NULL,&empty,NULL);assert(empty==-1); /* Actual gint, not the old host shortcut. */
    /* Key names before ':' become key caps (arrow keys as pixel arrows, no
       glyphs); every remaining glyph is painted exactly ONCE. */
    const char *texts[]={"MENU: return to MAIN MENU, EXE: Enter",UI_EDIT_HINT,
        "LEFT/RIGHT: SEGMENT/ARROW toggle","RIGHT/F3: COLOR",
        "LEFT/RIGHT: ON/OFF toggle, F3: COLOR"};
    const unsigned painted[]={33,35,20,7,22};
    for(unsigned k=0;k<sizeof(texts)/sizeof(*texts);k++) {
        int width=ui_help_width(texts[k]);assert(width>0 && width<=UI_W-16);
        printf("Help width %d / %d: %s\n",width,UI_W-16,texts[k]);
        dclear(C_WHITE);unsigned glyphs=host_text_glyphs();ui_help(8,174,texts[k],k==0);
        assert(host_text_glyphs()-glyphs==painted[k]);
        assert((count_color(C_RED)>0)==(k==0));assert((count_color(C_BLUE)>0)==(k<2));
        assert(count_color(UI_KEY_EDGE)>0 && count_color(UI_SURFACE)>0 && count_color(UI_MUTED)>0);
        /* Nothing is painted outside the measured width or the 13px cap band. */
        for(int y=0;y<DHEIGHT;y++)for(int x=0;x<DWIDTH;x++)if(gint_vram[y*DWIDTH+x]!=C_WHITE)
            assert(x>=UI_X+8 && x<UI_X+8+width && y>=UI_Y+172 && y<UI_Y+185);
    }
    dclear(C_WHITE);ui_help(8,174,"MENU: elsewhere; EXIT: back; EXE: Enter",false);
    assert(!count_color(C_RED));
    const char *current[]={"MENU: return to MAIN MENU","EXE: SELECT   EXIT: cancel",
        "Select IC10 y9 UP/DOWN EXE: SELECT","Curve A IC10 y9 UP/DOWN EXE: SELECT",
        "Curve A IC1 y(8) UP/DOWN EXE: SELECT"};
    const unsigned current_painted[]={23,22,32,33,34};
    for(unsigned i=0;i<sizeof(current)/sizeof(*current);i++) {
        int width=ui_help_width(current[i]);
        printf("Current hint width %d: %s\n",width,current[i]);
        assert(width<=(i>=2 ? 276:UI_W-16));
        dclear(C_WHITE);unsigned glyphs=host_text_glyphs();ui_help(8,184,current[i],i==0);
        assert(host_text_glyphs()-glyphs==current_painted[i]);
    }
    /* A leading EXE cap holds the same blue glyphs as plain text 4px inside. */
    dclear(C_WHITE);ui_text(12,184,C_BLUE,"EXE");
    for(int p=0;p<DWIDTH*DHEIGHT;p++)ink[p]=gint_vram[p]==C_BLUE;
    for(unsigned i=0;i<2;i++) {
        dclear(C_WHITE);ui_help(8,184,i ? current[1]:UI_EDIT_HINT,false);
        for(int p=0;p<DWIDTH*DHEIGHT;p++)assert(ink[p]==(gint_vram[p]==C_BLUE));
    }
    /* Full-width tabs: 66px physical slots; semantic color is a 3px top band,
       GRAPH/RUN alone fill the tab; labels are white on the dark tab. */
    ui_softkeys("INIT","ADV","V-WIN","INIT","SET","GRAPH");
    assert(gint_vram[203*DWIDTH+10]==UI_YELLOW && gint_vram[203*DWIDTH+76]==C_RGB(20,22,26));
    assert(gint_vram[203*DWIDTH+208]==UI_YELLOW && gint_vram[203*DWIDTH+340]==C_RGB(26,4,4));
    assert(gint_vram[207*DWIDTH+10]==UI_TAB && gint_vram[215*DWIDTH+340]==C_RGB(26,4,4));
    bool white=false;
    for(int y=210;y<221;y++)for(int x=2;x<66;x++)white|=gint_vram[y*DWIDTH+x]==C_WHITE;
    assert(white && gint_vram[203*DWIDTH+0]==C_WHITE && gint_vram[203*DWIDTH+65]==C_WHITE);
    const char *semantic[]={"INIT","ADV","V-WIN","SET","NEXT","PREV","GRAPH","RUN"};
    const int color[]={UI_YELLOW,C_RGB(20,22,26),C_RGB(31,17,0),UI_BRIGHT_GREEN,UI_CYAN,0xf81f,C_RGB(26,4,4),C_RGB(26,4,4)};
    for(unsigned k=0;k<sizeof(color)/sizeof(*color);k++) {
        ui_softkeys(semantic[k],semantic[k],semantic[k],semantic[k],semantic[k],semantic[k]);
        for(int slot=0;slot<6;slot++)assert(gint_vram[203*DWIDTH+slot*66+10]==color[k]);
    }
    ui_softkeys("","","","","","");
    for(int y=UI_Y+198;y<DHEIGHT;y++)for(int x=0;x<DWIDTH;x++)assert(gint_vram[y*DWIDTH+x]==C_WHITE);
    for(int kind=EQ_SEPARABLE;kind<=EQ_SYSTEM;kind++) {
        char title[64];snprintf(title,sizeof(title),"DIFF EQ / %s",model_kind_name(kind));
        int width;dsize(title,NULL,&width,NULL);assert(8+width<UI_W-92); /* stage indicator area */
    }
    for(int count=1;count<=10;count++) {
        int selected=0;assert(ui_select_move(KEY_UP,&selected,count) && selected==count-1);
        assert(ui_select_move(KEY_DOWN,&selected,count) && selected==0);
        assert(!ui_select_move(KEY_LEFT,&selected,count) && selected==0);
    }
}
static void configured(int method)
{
    model_defaults(&d,method ? EQ_SYSTEM:EQ_GENERAL,method ? 2:1);
    d.adaptive=(OdeAdaptive){method,1e-5,1e-8};d.solver.h=.25;d.solver.step=7;
    d.solver.sf=24;d.solver.max_steps=321;d.solver_custom=1;
    d.solver.xmin=-2;d.solver.xmax=2;d.view.xmin=-8.4;d.view.xmax=9.7;
    d.view.grid=d.view.labels=0;d.enabled=0;d.field_color=4;d.field_style=FIELD_SEGMENT;
    d.event.enabled=1;strcpy(d.event.text,"y-10");d.event.direction=EVENT_RISING;d.event.action=EVENT_STOP;
    expected=d;
}
int main(void)
{
    hints();
    assert(!setenv("DIFFEQ_HOST_KEYS","F1 F6 F1 F6 F1 F6 F1 F6 F1 F6 F1 F6 F1 F6 F1 F6 F1 F6",1));
    for(int method=ODE_RK4;method<=ODE_RK45;method++) {
        configured(method);UiStageState state={0};
        expected.solver=(OdeSettings){-8,9,.1,20000,method ? 7:1,method ? 24:12};
        expected.solver_custom=0;
        if(method){expected.adaptive.reltol=1e-6;expected.adaptive.abstol=1e-9;}
        assert(ui_parameters(&d,&state)==UI_STAGE_NEXT && !memcmp(&d,&expected,sizeof(d)));
        configured(method);
        expected.view.xmin=-6.3;expected.view.xmax=6.3;
        ui_vwindow(&d);assert(!memcmp(&d,&expected,sizeof(d))); /* Manual range and appearance survive. */
        configured(method);expected.view.grid=expected.view.labels=1;
        expected.field_style=FIELD_ARROW;expected.field_color=0;
        ui_graph_settings(&d);assert(!memcmp(&d,&expected,sizeof(d)));
        configured(method);model_output_defaults(&expected);
        ui_output(&d);assert(!memcmp(&d,&expected,sizeof(d)));
    }
    configured(ODE_RK45);d.view.phase=1;d.phase_view.xmin=-8;d.phase_view.xmax=9;
    d.phase_view.grid=d.phase_view.labels=0;expected=d;
    expected.phase_view.xmin=-3.1;expected.phase_view.xmax=3.1;expected.phase_ready=1;
    ui_vwindow(&d);assert(!memcmp(&d,&expected,sizeof(d))); /* Independent Phase geometry only. */
    puts("UI controls: real-font hint bounds/glyph positions/colors, title clearance, selector cycles and isolated INIT scopes passed.");
}
