#include "trace.h"
#include "ui.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;
void host_cancel_after(unsigned polls);
static unsigned hash(void) {
    unsigned h=0;for(int i=0;i<DWIDTH*DHEIGHT;i++)h=h*31+gint_vram[i];return h;
}
int main(void)
{
    model_defaults(&d,EQ_SECOND,2);d.solver.sf=0;
    assert(model_compile(&d,&m).expression.status==EXPR_OK);
    graph_render(&d,&m,false);unsigned original=hash();
    ui_trace_input(true);trace_overlay_begin();assert(trace_prepare(&d,&m,0,0));
    TracePoint point;assert(trace_point_near(0,&point));
    double dx=model_xdot(&d.view);
    TracePoint unchanged=point;
    assert(!trace_step(INFINITY,1,dx,&point) && !trace_step(0,1,INFINITY,&point));
    assert(!trace_step(1e300,1,1,&point) && !memcmp(&point,&unchanged,sizeof(point)));
    for(int i=0;i<50;i++)assert(trace_step(point.x,1,dx,&point));
    assert(fabs(point.x-50*dx)<1e-12 && fabs(point.y[0]-cos(point.x))<.002);
    trace_overlay_show(&d,&point,0,true);trace_overlay_restore();assert(hash()==original);
    ViewWindow before=d.view;OdeSettings solver=d.solver;
    assert(trace_point_near(5.1,&point));host_cancel_after(10);
    assert(trace_follow(&d,&m,point.x)==ODE_CANCELLED);
    assert(!memcmp(&d.view,&before,sizeof(before)) && !memcmp(&d.solver,&solver,sizeof(solver)));
    assert(hash()==original);UiBlink blink={0};assert(ui_trace_key(&blink).key==KEY_EXIT);
    TracePoint old;assert(trace_point_near(5.1,&old) && old.x==point.x);
    assert(trace_follow(&d,&m,point.x)==ODE_OK && d.view.xmin>before.xmin);
    assert(d.solver.h==solver.h && d.solver.xmin!=solver.xmin);
    unsigned committed=hash();trace_overlay_show(&d,&point,0,true);trace_overlay_restore();assert(hash()==committed);
    assert(trace_select(&d,0,1) && trace_point_near(point.x,&old) && old.x==point.x);
    /* A later in-cache pan must not invoke cancellation/integration at all. */
    host_cancel_after(1);assert(trace_follow(&d,&m,7.7)==ODE_OK);
    assert(d.view.xmin>before.xmin+3);host_cancel_after(0);
    /* Manual range persists even when runtime extent expands. */
    d.solver_custom=1;solver=d.solver;
    assert(trace_follow(&d,&m,13)==ODE_OK);
    assert(!memcmp(&solver,&d.solver,sizeof(solver)));
    before=d.view;committed=hash();
    d.solver.h=1e-300; /* invalidate proposed extent deliberately */
    /* Existing cache uses the settings captured at preparation; edits happen
       outside TRACE and therefore require a fresh prepare. */
    assert(!trace_prepare(&d,&m,0,0));assert(hash()==committed);
    d.solver=solver;
    model_defaults(&d,EQ_SYSTEM,9);d.nic=9;
    for(int f=1;f<9;f++){d.ic[f]=d.ic[0];d.enabled=511;}
    assert(model_compile(&d,&m).expression.status==EXPR_OK);
    graph_render(&d,&m,false);assert(trace_prepare(&d,&m,8,8));
    assert(trace_point_near(0,&point) && trace_step(0,1,model_xdot(&d.view),&point));
    assert(trace_follow(&d,&m,5.1)==ODE_OK);
    d.enabled=0;graph_render(&d,&m,false);assert(!trace_prepare(&d,&m,8,8));
    d.enabled=511;graph_render(&d,&m,false);assert(trace_prepare(&d,&m,8,8));
    assert(trace_point_near(0,&point));
    puts("TRACE: fractional Xdot interpolation, transactional cancel, cached redraw, curve switch, manual range preservation passed.");
    ui_trace_input(false);
}
