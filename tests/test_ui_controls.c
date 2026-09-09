#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Document d,expected;
static unsigned char ink[DWIDTH*DHEIGHT];
unsigned host_text_glyphs(void);
static void hints(void)
{
    int empty;dnsize("EXE",0,NULL,&empty,NULL);assert(empty==-1); /* Actual gint, not the old host shortcut. */
    const char *texts[]={"MENU: return to MAIN MENU, EXE: Enter",UI_EDIT_HINT,
        "LEFT/RIGHT: SEGMENT/ARROW toggle","RIGHT/F3: COLOR",
        "LEFT/RIGHT: ON/OFF toggle, F3: COLOR"};
    for(unsigned k=0;k<sizeof(texts)/sizeof(*texts);k++) {
        int width;dsize(texts[k],NULL,&width,NULL);assert(width<=UI_W-16);
        printf("Help width %d / %d: %s\n",width,UI_W-16,texts[k]);
        dclear(C_WHITE);ui_text(8,174,UI_MUTED,"%s",texts[k]);
        for(int i=0;i<DWIDTH*DHEIGHT;i++)ink[i]=gint_vram[i]!=C_WHITE;
        dclear(C_WHITE);unsigned glyphs=host_text_glyphs();ui_help(8,174,texts[k],k==0);
        assert(host_text_glyphs()-glyphs==strlen(texts[k])); /* Every glyph painted ONCE. */
        unsigned red=0,blue=0;
        for(int i=0;i<DWIDTH*DHEIGHT;i++) {
            assert(ink[i]==(gint_vram[i]!=C_WHITE)); /* Exactly the same glyph geometry. */
            red+=gint_vram[i]==C_RED;blue+=gint_vram[i]==C_BLUE;
        }
        assert((red>0)==(k==0));assert((blue>0)==(k<2));
        if(k==0) {
            /* Main EXE begins at x219 with this checked-in production font. */
            int menu_width,key_width;dsize("MENU",NULL,&menu_width,NULL);dsize("EXE",NULL,&key_width,NULL);
            for(int y=178;y<189;y++)for(int x=14;x<319;x++) {
                unsigned color=gint_vram[y*DWIDTH+x];if(color==C_WHITE)continue;
                if(x<14+menu_width)assert(color==C_RED);
                else if(x>=219 && x<219+key_width)assert(color==C_BLUE);
                else assert(color==UI_MUTED);
            }
        }
    }
    dclear(C_WHITE);ui_help(8,174,"MENU: elsewhere; EXIT: back; EXE: Enter",false);
    for(int i=0;i<DWIDTH*DHEIGHT;i++)assert(gint_vram[i]!=C_RED);
    const char *current[]={"MENU: return to MAIN MENU","EXE: SELECT   EXIT: cancel",
        "Select IC10 y9 UP/DOWN EXE: SELECT","Curve A IC10 y9 UP/DOWN EXE: SELECT",
        "Curve A IC1 y(8) UP/DOWN EXE: SELECT"};
    for(unsigned i=0;i<sizeof(current)/sizeof(*current);i++) {
        int width;dsize(current[i],NULL,&width,NULL);
        printf("Current hint width %d: %s\n",width,current[i]);
        assert(width<=(i>=2 ? 276:UI_W-16));
        dclear(C_WHITE);ui_text(8,184,UI_MUTED,"%s",current[i]);
        for(int p=0;p<DWIDTH*DHEIGHT;p++)ink[p]=gint_vram[p]!=C_WHITE;
        dclear(C_WHITE);unsigned glyphs=host_text_glyphs();ui_help(8,184,current[i],i==0);
        assert(host_text_glyphs()-glyphs==strlen(current[i]));
        for(int p=0;p<DWIDTH*DHEIGHT;p++)assert(ink[p]==(gint_vram[p]!=C_WHITE));
    }
    /* Same leading EXE glyphs in EDIT/palette as beta.2 Main's correctly
       aligned nonleading token, using the normal primitive as the oracle. */
    dclear(C_WHITE);ui_text(8,184,C_BLUE,"EXE");
    for(int p=0;p<DWIDTH*DHEIGHT;p++)ink[p]=gint_vram[p]==C_BLUE;
    for(unsigned i=0;i<2;i++) {
        dclear(C_WHITE);ui_help(8,184,i ? current[1]:UI_EDIT_HINT,false);
        for(int p=0;p<DWIDTH*DHEIGHT;p++)assert(ink[p]==(gint_vram[p]==C_BLUE));
    }
    ui_softkeys("INIT","ADV","V-WIN","INIT","SET","GRAPH");
    assert(gint_vram[203*DWIDTH+7]==UI_YELLOW && gint_vram[203*DWIDTH+71]==C_BLACK);
    assert(gint_vram[203*DWIDTH+199]==UI_YELLOW && gint_vram[203*DWIDTH+327]==C_RED);
    bool black=false,white=false;
    for(int y=206;y<217;y++)for(int x=11;x<65;x++)black|=gint_vram[y*DWIDTH+x]==C_BLACK;
    for(int y=206;y<217;y++)for(int x=75;x<129;x++)white|=gint_vram[y*DWIDTH+x]==C_WHITE;
    assert(black && white);
    const char *semantic[]={"INIT","ADV","V-WIN","SET","NEXT","PREV","GRAPH","RUN"};
    const int color[]={UI_YELLOW,C_BLACK,C_RGB(31,17,0),UI_BRIGHT_GREEN,UI_CYAN,0xf81f,C_RED,C_RED};
    for(unsigned k=0;k<sizeof(color)/sizeof(*color);k++) {
        ui_softkeys(semantic[k],semantic[k],semantic[k],semantic[k],semantic[k],semantic[k]);
        for(int slot=0;slot<6;slot++)assert(gint_vram[203*DWIDTH+UI_X+1+slot*64]==color[k]);
    }
    for(int kind=EQ_SEPARABLE;kind<=EQ_SYSTEM;kind++) {
        char title[64];snprintf(title,sizeof(title),"DIFF EQ / %s",model_kind_name(kind));
        int width;dsize(title,NULL,&width,NULL);assert(8+width<UI_W-40);
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
