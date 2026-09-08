#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Document d,expected;
static unsigned char ink[DWIDTH*DHEIGHT];
static void hints(void)
{
    const char *texts[]={"MENU: return to MAIN MENU, EXE: Enter",UI_EDIT_HINT,
        "LEFT/RIGHT: SEGMENT/ARROW toggle","RIGHT/F3: COLOR",
        "LEFT/RIGHT: ON/OFF toggle, F3: COLOR"};
    for(unsigned k=0;k<sizeof(texts)/sizeof(*texts);k++) {
        int width;dsize(texts[k],NULL,&width,NULL);assert(width<=UI_W-16);
        printf("Help width %d / %d: %s\n",width,UI_W-16,texts[k]);
        dclear(C_WHITE);ui_text(8,174,UI_MUTED,"%s",texts[k]);
        for(int i=0;i<DWIDTH*DHEIGHT;i++)ink[i]=gint_vram[i]!=C_WHITE;
        dclear(C_WHITE);ui_help(8,174,texts[k],k==0);
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
    ui_softkeys("INIT","ADV","V-WIN","INIT","SET","GRAPH");
    assert(gint_vram[203*DWIDTH+7]==UI_YELLOW && gint_vram[203*DWIDTH+71]==C_BLACK);
    assert(gint_vram[203*DWIDTH+199]==UI_YELLOW && gint_vram[203*DWIDTH+327]==C_RED);
    bool black=false,white=false;
    for(int y=206;y<217;y++)for(int x=11;x<65;x++)black|=gint_vram[y*DWIDTH+x]==C_BLACK;
    for(int y=206;y<217;y++)for(int x=75;x<129;x++)white|=gint_vram[y*DWIDTH+x]==C_WHITE;
    assert(black && white);
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
