#include "graph.h"
#include "trace.h"
#include "ui.h"
#include <gint/rtc.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
extern unsigned long host_solves(void);
extern unsigned host_text_glyphs(void);
extern void host_cancel_after(unsigned);
static Document d,expected;
static CompiledModel m;
static SolverReport report;
static uint16_t original[DWIDTH*DHEIGHT];
static void copy_pixels(void){memcpy(original,gint_vram,sizeof(original));}
static void same_pixels(void){assert(!memcmp(original,gint_vram,sizeof(original)));}
static void visibility(void)
{
    const double values[]={-1e6,-10,-4.9,-4.5,0,4.9,10,1e6};
    for(unsigned i=0;i<sizeof(values)/sizeof(*values);i++) {
        ViewWindow v;model_window_defaults(&v);v.ymin=-5;v.ymax=5;ViewWindow before=v;
        int x,y;bool visible=graph_point(&v,0,values[i],&x,&y) && y>=7 && y<=171;
        bool moved=graph_result_visible_y(&v,0,values[i]);assert(moved!=visible);
        assert(v.xmin==before.xmin && v.xmax==before.xmax && v.xscale==before.xscale);
        assert(v.yscale==before.yscale && fabs((v.ymax-v.ymin)-10)<1e-10);
        assert(graph_point(&v,0,values[i],&x,&y) && y>=7 && y<=171);
        if(visible)assert(!memcmp(&v,&before,sizeof(v)));
        before=v;assert(!graph_result_visible_y(&v,0,values[i]) && !memcmp(&v,&before,sizeof(v)));
    }
    ViewWindow v;model_window_defaults(&v);ViewWindow before=v;
    const double bad[]={NAN,INFINITY,-INFINITY,1e101,1e99};
    for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++)assert(!graph_result_visible_y(&v,0,bad[i]) && !memcmp(&v,&before,sizeof(v)));
    assert(!graph_result_visible_y(&v,1000,1) && !memcmp(&v,&before,sizeof(v)));
}
static void reset(void)
{
    for(int phase=0;phase<2;phase++)for(int manual=0;manual<2;manual++) {
        model_defaults(&d,EQ_SYSTEM,2);d.view.phase=phase;d.phase_ready=1;
        d.solver_custom=manual;d.solver.xmin=-2;d.solver.xmax=3;d.solver.h=.031;
        d.adaptive.method=ODE_RK45;d.event.enabled=1;strcpy(d.event.text,"x-1");
        ViewWindow *v=model_view(&d);v->grid=0;v->labels=0;v->xmin=-17;v->xmax=29;v->ymin=-11;v->ymax=4;
        v->xscale=2;v->yscale=.3;expected=d;ViewWindow *e=model_view(&expected);
        ViewWindow factory;if(phase)model_phase_window_defaults(&factory);else model_window_defaults(&factory);
        e->xmin=factory.xmin;e->xmax=factory.xmax;e->xscale=factory.xscale;
        e->ymin=factory.ymin;e->ymax=factory.ymax;e->yscale=factory.yscale;
        model_sync_solver_window(&expected);ui_vwindow_reset(&d);
        assert(!memcmp(&d,&expected,sizeof(d)));
    }
}
static void overlays(void)
{
    model_defaults(&d,EQ_SECOND,2);assert(model_compile(&d,&m).expression.status==EXPR_OK);
    graph_render(&d,&m,true);report=*solver_report();expected=d;copy_pixels();
    unsigned long solves=host_solves();
    for(int selected=0;selected<2;selected++) {
        graph_overlay_begin(0);
        for(int i=0;i<8;i++) {
            graph_overlay_restore();graph_overlay_curve(&d,0,selected,i%2);
            unsigned glyphs=host_text_glyphs();graph_message("UP/DOWN: SELECT GRAPH, EXE: SELECT",GRAPH_INSTRUCTION);
            assert(host_text_glyphs()-glyphs==strlen("UP/DOWN: SELECT GRAPH, EXE: SELECT"));
        }
        graph_overlay_restore();same_pixels();
    }
    assert(host_solves()==solves && !memcmp(&report,solver_report(),sizeof(report)) && !memcmp(&d,&expected,sizeof(d)));
    graph_overlay_begin(GRAPH_RESULT_TOP);graph_overlay_curves(&d,0,0,0,1,true);
    graph_overlay_point(100,171);ui_rect(0,179,384,19,C_WHITE);ui_text(7,184,UI_BLUE,"X=0 Y=1 ROOT");
    graph_overlay_restore();same_pixels();
    /* Fallback display preparation after a replaced cache is transactional and
       leaves the canonical trajectory report and existing framebuffer intact. */
    trace_cache_invalidate();host_cancel_after(1);
    assert(graph_plot_prepare(&d,&m,ui_cancel,NULL)==ODE_CANCELLED);same_pixels();
    assert(!memcmp(&report,solver_report(),sizeof(report)));
    assert(graph_plot_prepare(&d,&m,ui_cancel,NULL)==ODE_OK);same_pixels();
    assert(!memcmp(&report,solver_report(),sizeof(report)));
    dclear(UI_CYAN);int width;dsize("UP/DOWN: SELECT GRAPH, EXE: SELECT",NULL,&width,NULL);assert(width<=284);
    graph_message("UP/DOWN: SELECT GRAPH, EXE: SELECT",GRAPH_INSTRUCTION);
    unsigned blue=0;for(int y=8;y<19;y++)for(int x=183;x<207;x++)blue+=gint_vram[y*DWIDTH+x]==C_BLUE;
    assert(blue && gint_vram[6*DWIDTH+11]==C_WHITE && gint_vram[6*DWIDTH+310]==UI_CYAN);
    printf("Selection prompt width: %d px\n",width);
}
static void busy(void)
{
    const char *labels[]={"CALCULATING...","CALCULATING...","Preparing Table...","Drawing..."};
    for(unsigned area=0;area<4;area++) {
        for(unsigned i=0;i<DWIDTH*DHEIGHT;i++)gint_vram[i]=(uint16_t)(i*31u+7u);copy_pixels();
        UiBusy b;host_tick_step(0);ui_busy_begin(&b,labels[area],area,ui_cancel,NULL);
        assert(!ui_busy_cancel(&b) && !b.visible);ui_busy_end(&b);same_pixels();
        host_tick_step(8);ui_busy_begin(&b,labels[area],area,ui_cancel,NULL);
        assert(!ui_busy_cancel(&b) && !b.visible);assert(!ui_busy_cancel(&b) && !b.visible);
        for(int n=0;n<9;n++){assert(!ui_busy_cancel(&b) && b.visible);same_pixels();}
        assert(b.frame==5);ui_busy_end(&b);same_pixels();
        host_cancel_after(1);ui_busy_begin(&b,labels[area],area,ui_cancel,NULL);
        assert(ui_busy_cancel(&b) && !b.visible);ui_busy_end(&b);same_pixels();
    }
    host_tick_step(0);
}
int main(void)
{
    visibility();reset();overlays();busy();
    puts("Overlay/reset/visibility: exact restoration, report preservation, fixed X, minimal Y, shared defaults and four busy lifecycles PASS.");
}
