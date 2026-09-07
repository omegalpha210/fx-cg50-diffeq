#include "trace.h"
#include "ui.h"
#include "phase_graph.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
unsigned long host_rhs_calls(void);
void host_cancel_after(unsigned polls);
static Document d;
static CompiledModel m;
static unsigned hash(void)
{
    unsigned h=0;
    for(int i=0;i<DWIDTH*DHEIGHT;i++)h=h*31+gint_vram[i];
    return h;
}
static void synthetic_capture(bool fail)
{
    assert(trace_capture_begin(&d));
    for(int side=0;side<2;side++) {
        assert(trace_capture_branch_begin(&d,0,side));
        for(unsigned j=0;j<=200;j++) {
            double x=(side ? 1:-1)*j*.01;
            double y[2]={cos(x),sin(x)};
            if(j==1){y[0]=123;y[1]=-77;}
            trace_capture_point(x,y,j,NULL);
        }
        trace_capture_branch_end((ModelPathResult){.status=fail ? ODE_CANCELLED:ODE_OK,.steps=200});
    }
    trace_capture_end(!fail);
}
static void segmented_capture(void)
{
    assert(trace_capture_begin(&d));
    for(int side=0;side<2;side++) {
        assert(trace_capture_branch_begin(&d,0,side));
        double y[2]={-1,-1};trace_capture_point(0,y,0,NULL);
        y[1]=1;trace_capture_point(side ? 1:-1,y,100,NULL);
        if(!side) {
            trace_capture_point(-1.25,NULL,125,NULL);
            y[0]=1;y[1]=-1;trace_capture_point(-1.5,y,150,NULL);
            y[1]=1;trace_capture_point(-2,y,200,NULL);
        }
        trace_capture_branch_end((ModelPathResult){.status=side ? ODE_OK:ODE_HAS_INVALID,
            .invalid=side ? ODE_OK:ODE_DOMAIN,.steps=200});
    }
    trace_capture_end(true);
}
int main(void)
{
    model_defaults(&d,EQ_SYSTEM,2);d.solver.sf=0;d.solver.h=.01;
    d.solver.xmin=-2;d.solver.xmax=2;d.solver_custom=1;
    assert(model_compile(&d,&m).expression.status==EXPR_OK);
    assert(trace_capture_begin(&d));
    for(int side=0;side<2;side++) {
        assert(trace_capture_branch_begin(&d,0,side));
        ModelPathResult r=model_path_branch(&d,&m,0,side ? 1:-1,&d.solver,
            trace_capture_point,NULL,NULL,NULL);
        assert(r.status==ODE_OK);trace_capture_branch_end(r);
    }
    trace_capture_end(true);assert(trace_cache_matches(&d));
    unsigned long calls=host_rhs_calls();
    assert(trace_prepare(&d,&m,0,1));assert(host_rhs_calls()==calls);
    TracePoint p;assert(trace_point_near(.5,&p));
    assert(fabs(p.y[0]-cos(.5))<.001 && fabs(p.y[1]+sin(.5))<.001);
    ViewWindow phase;assert(trace_cache_phase_window(&d,&phase));
    d.view.phase=1;d.phase_view=phase;
    d.phase_view.xmin-=10;d.view.xmin-=10;d.enabled=0;d.color[0][0]=0;d.solver.sf=50;
    assert(trace_cache_matches(&d));assert(trace_prepare(&d,&m,0,1));
    assert(host_rhs_calls()==calls);
    d.solver.step++;assert(!trace_cache_matches(&d));d.solver.step--;
    d.solver.h=.02;assert(!trace_cache_matches(&d));d.solver.h=.01;
    d.view.phase=0;d.enabled=3;d.view.xmin=.101;d.view.xmax=.102;
    ViewWindow time;assert(trace_cache_time_window(&d,&time));
    assert(time.xmin==.101 && time.xmax==.102 && time.ymin<-.1 && time.ymax>.99);
    unsigned before=hash();double original=d.ic[0].y[0];d.ic[0].y[0]=42;
    synthetic_capture(true);d.ic[0].y[0]=original;
    assert(trace_cache_matches(&d) && hash()==before);
    assert(trace_point_near(.5,&p) && fabs(p.y[0]-cos(.5))<.001);
    synthetic_capture(false);assert(trace_cache_phase_window(&d,&phase));
    assert(phase.xmax>123 && phase.ymin<-77);
    assert(trace_prepare(&d,&m,0,0));assert(trace_point_near(.01,&p));
    assert(p.y[0]<2); /* The extrema came from the original stream, not retained samples. */
    assert(trace_cache_result().steps==400);
    assert(host_rhs_calls()==calls);
    /* Projection needs both states even when TIME outputs are hidden. */
    segmented_capture();d.enabled=0;d.view.phase=0;dclear(C_WHITE);before=hash();
    trace_cache_render(&d);assert(hash()==before);
    d.view.phase=1;model_phase_window_defaults(&d.phase_view);
    trace_cache_render(&d);assert(hash()!=before);
    int px,py;assert(graph_point(&d.phase_view,0,0,&px,&py));
    assert(gint_vram[(UI_Y+py)*DWIDTH+UI_X+px]==C_WHITE); /* no line across gap */
    assert(graph_point(&d.phase_view,-1,0,&px,&py));
    assert(gint_vram[(UI_Y+py)*DWIDTH+UI_X+px]==graph_palette_color(model_color(&d,0,1)));
    assert(trace_prepare(&d,&m,0,1));assert(trace_point_near(-1.25,&p));
    assert(p.x==-1 || p.x==-1.5); /* nearest endpoint, never synthetic interpolation */
    ViewWindow before_phase=d.phase_view,before_time=d.view;OdeSettings solver=d.solver;
    p.x=NAN;p.y[0]=100;p.y[1]=100;trace_follow(&d,&m,&p);
    assert(!memcmp(&before_phase,&d.phase_view,sizeof(before_phase)));
    assert(!memcmp(&before_time,&d.view,sizeof(before_time)));
    assert(!memcmp(&solver,&d.solver,sizeof(solver)));
    /* Offscreen cache redraws and window fits perform no numerical work. */
    d.phase_view.xmin=20;d.phase_view.xmax=21;dclear(C_WHITE);before=hash();
    trace_cache_render(&d);assert(hash()==before);
    assert(trace_cache_phase_window(&d,&phase));
    assert(phase.xmin<-1 && phase.xmax>1 && host_rhs_calls()==calls);
    assert(trace_cache_result().status==ODE_HAS_INVALID);
    /* Following in PHASE preflights expensive layers before committing either
       the window or cursor, and retains the EXIT for the TRACE event loop. */
    d.phase_field=1;d.phase_nullclines=1;ui_trace_input(true);
    assert(trace_point_near(-1,&p));TracePoint before_point=p;
    before_phase=d.phase_view;before_time=d.view;solver=d.solver;
    OdeSettings extent=*trace_extent();before=hash();
    host_cancel_after(2);
    assert(trace_navigate(&d,&m,-2,true,&p)==ODE_CANCELLED);
    assert(!memcmp(&p,&before_point,sizeof(p)) && hash()==before);
    assert(!memcmp(&d.phase_view,&before_phase,sizeof(before_phase)));
    assert(!memcmp(&d.view,&before_time,sizeof(before_time)));
    assert(!memcmp(&d.solver,&solver,sizeof(solver)));
    assert(!memcmp(trace_extent(),&extent,sizeof(extent)));
    UiBlink blink={.timer=-1};assert(ui_trace_key(&blink).key==KEY_EXIT);
    assert(trace_navigate(&d,&m,-2,true,&p)==ODE_OK && p.x==-2);
    assert(!memcmp(&d.view,&before_time,sizeof(before_time)));
    assert(!memcmp(&d.solver,&solver,sizeof(solver)) && host_rhs_calls()==calls);
    assert(graph_point(&d.phase_view,p.y[0],p.y[1],&px,&py));
    ui_trace_input(false);
    /* Cached TRACE redraw keeps the equilibrium marker above the curve layer. */
    d.phase_field=0;d.phase_nullclines=0;
    d.phase_view.xmin=d.phase_view.ymin=-1.3;
    d.phase_view.xmax=d.phase_view.ymax=1.2;
    assert(graph_phase_search(&d,&m,NULL,NULL)==ODE_OK);
    assert(graph_phase_results()->count==1);
    assert(trace_navigate(&d,&m,-2,true,&p)==ODE_OK);
    assert(graph_point(&d.phase_view,0,0,&px,&py));
    assert(gint_vram[(UI_Y+py)*DWIDTH+UI_X+px+4]==C_BLACK);
    assert(host_rhs_calls()==calls);graph_phase_reset();
    puts("Phase cache: same RK4 stream, projection/hidden outputs, no gap lines, zero-work reuse, identity, full-stream extrema, narrow TIME bounds and cancelled commit passed.");
}
