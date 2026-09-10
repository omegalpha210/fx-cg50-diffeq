#include "graph.h"
#include "trace.h"
#include "ui.h"
#include <gint/rtc.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
extern void host_cancel_after(unsigned);
extern unsigned host_clear_count(void);
static uint16_t original[DWIDTH*DHEIGHT];
static Document d,expected,defaults;
static CompiledModel model;
static void pattern(void)
{
    for(unsigned i=0;i<DWIDTH*DHEIGHT;i++)gint_vram[i]=(uint16_t)(i*97u+41u);
    memcpy(original,gint_vram,sizeof(original));
}
int main(void)
{
    for(int kind=EQ_SEPARABLE;kind<=EQ_SYSTEM;kind++)for(int dim=1;dim<=9;dim++) {
        model_defaults(&defaults,kind,dim);d=defaults;
        d.solver.h=.03;d.solver_custom=1;d.event.enabled=1;strcpy(d.event.text,"x-1");
        d.enabled=1;d.field_color=4;d.view.xmax=19;d.power=3;d.ic[0].x=8;
        strcpy(d.text[0],"unfinished(");expected=d;
        memcpy(expected.text,defaults.text,sizeof(d.text));expected.power=defaults.power;
        model_equation_defaults(&d);assert(!memcmp(&d,&expected,sizeof(d)));
        expected=d;memcpy(expected.ic,defaults.ic,sizeof(d.ic));expected.nic=defaults.nic;
        model_initial_defaults(&d);assert(!memcmp(&d,&expected,sizeof(d)));
    }
    model_defaults(&d,EQ_SYSTEM,2);d.solver_custom=1;d.phase_ready=1;
    expected=d;
    graph_zoom(&d.view,.5,1,1);ui_vwindow_reset(&d);assert(!memcmp(&d,&expected,sizeof(d)));
    d.view.phase=1;expected=d;graph_zoom(&d.phase_view,.5,-1,-1);ui_vwindow_reset(&d);
    assert(!memcmp(&d,&expected,sizeof(d)) && d.view.phase);
    ViewWindow v=d.view,a=v,b=v;
    assert(graph_box_window(&a,100,150,300,40) && graph_box_window(&b,300,40,100,150));
    assert(!memcmp(&a,&b,sizeof(a)));
    assert(fabs(a.xmin-(v.xmin+(v.xmax-v.xmin)*100/383.0))<1e-12);
    assert(fabs(a.ymin-(v.ymax-(v.ymax-v.ymin)*150/197.0))<1e-12);
    b=a;assert(!graph_box_window(&a,100,100,104,130) && !memcmp(&a,&b,sizeof(a)));
    assert(!graph_box_window(&a,-1,0,100,100) && !memcmp(&a,&b,sizeof(a)));
    const int points[][2]={{192,99},{0,0},{383,197},{192,20},{20,184}};
    for(unsigned i=0;i<sizeof(points)/sizeof(*points);i++) {
        pattern();GraphPointPatch patch={0};int x=points[i][0],y=points[i][1];
        graph_point_cross(x,y,&patch);
        assert(gint_vram[(UI_Y+y)*DWIDTH+UI_X+x]==C_WHITE);
        if(y>=4)assert(gint_vram[(UI_Y+y-4)*DWIDTH+UI_X+x]==C_BLACK);
        graph_point_restore(&patch);assert(!memcmp(original,gint_vram,sizeof(original)));
    }
    const int boxes[][4]={{0,0,383,197},{383,197,0,0},{192,99,192,99},{12,20,300,190}};
    for(unsigned i=0;i<sizeof(boxes)/sizeof(*boxes);i++) {
        pattern();trace_overlay_begin();
        for(int j=0;j<50;j++) {
            trace_box_show(boxes[i][0],boxes[i][1],boxes[i][2],boxes[i][3],true);
            /* Reserved status/footer region is untouched except the9px point. */
            for(int x=6;x<390;x++)assert(gint_vram[(UI_Y+34)*DWIDTH+x]==original[(UI_Y+34)*DWIDTH+x]);
            trace_box_restore();assert(!memcmp(original,gint_vram,sizeof(original)));
        }
    }
    model_defaults(&d,EQ_GENERAL,1);d.solver.sf=0;
    assert(model_compile(&d,&model).expression.status==EXPR_OK);
    graph_render(&d,&model,true);assert(trace_plot_matches(&d,-6,6));
    graph_zoom(&d.view,.5,1,1);
    ui_vwindow_reset(&d);assert(graph_redraw_cached(&d,&model,-6,6));
    strcpy(d.text[0],"x");assert(!trace_plot_matches(&d,-6,6));
    model_defaults(&d,EQ_GENERAL,1);d.solver.sf=0;d.event.enabled=1;strcpy(d.event.text,"x-2");
    assert(model_compile(&d,&model).expression.status==EXPR_OK);
    graph_render(&d,&model,true);
    d.solver.xmin=-1;d.solver.xmax=1;graph_render(&d,&model,false);
    ui_vwindow_reset(&d);model_sync_solver_window(&d);
    assert(!graph_redraw_cached(&d,&model,-6,6)); /* Event report coverage guard. */
    graph_render(&d,&model,false);assert(solver_report()->markers.count>0);
    UiBusy busy;host_tick_step(0);ui_busy_start(&busy);assert(!busy.visible);ui_busy_end(&busy);
    host_tick_step(8);ui_busy_start(&busy);
    assert(!ui_busy_cancel(&busy) && !busy.visible);
    assert(!ui_busy_cancel(&busy) && !busy.visible);
    assert(!ui_busy_cancel(&busy) && busy.visible);
    ui_busy_end(&busy);assert(!busy.visible);
    host_cancel_after(1);ui_busy_start(&busy);assert(ui_busy_cancel(&busy) && !busy.visible);
    ui_busy_end(&busy);host_tick_step(0);
    puts("Interaction: isolated defaults, view restore, BOX bounds/reverse/cancel pixels,9px marker restoration, cache identity and delayed/cancel-first busy lifecycle passed.");
}
