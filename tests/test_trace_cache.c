#include "trace.h"
#include "ui.h"
#include "initial.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;
void host_cancel_after(unsigned polls);
unsigned long host_rhs_calls(void);
unsigned host_clear_count(void);
static unsigned hash(void) {
    unsigned h=0;for(int i=0;i<DWIDTH*DHEIGHT;i++)h=h*31+gint_vram[i];return h;
}
static void prepare(int family,int variable)
{
    assert(model_compile(&d,&m).expression.status==EXPR_OK);
    graph_render(&d,&m,false);assert(trace_prepare(&d,&m,family,variable));
}
int main(void)
{
    model_defaults(&d,EQ_SECOND,2);d.solver.sf=0;prepare(0,0);
    unsigned original=hash();ui_trace_input(true);
    TracePoint point;assert(trace_point_near(0,&point));double dx=model_xdot(&d.view);
    TracePoint unchanged=point;
    assert(!trace_step(INFINITY,1,dx,&point) && !trace_step(0,1,INFINITY,&point));
    assert(!trace_step(1e300,1,1,&point) && !memcmp(&point,&unchanged,sizeof(point)));
    for(int i=0;i<50;i++)assert(trace_step(point.x,1,dx,&point));
    assert(fabs(point.x-50*dx)<1e-12 && fabs(point.y[0]-cos(point.x))<.002);
    trace_overlay_show(&d,&point,0,true);trace_overlay_restore();assert(hash()==original);
    OdeSettings solver=d.solver,extent=*trace_extent();
    unsigned long calls=host_rhs_calls();
    /* Pan beyond view edges and jump exactly to configured endpoints without work. */
    for(int side=-1;side<=1;side+=2) {
        assert(trace_navigate(&d,&m,side*5.9,false,&point)==ODE_OK && point.x==side*5.9);
        assert(trace_navigate(&d,&m,side*6.0,false,&point)==ODE_OK && point.x==side*6.0);
        assert(trace_navigate(&d,&m,side*6.0,true,&point)==ODE_OK && point.x==side*6.0);
        assert(host_rhs_calls()==calls && !memcmp(trace_extent(),&extent,sizeof(extent)));
        assert(!memcmp(&d.solver,&solver,sizeof(solver)));
    }
    ViewWindow before=d.view;original=hash();
    for(double target=-13;target<=13;target+=26) {
        assert(trace_navigate(&d,&m,target,false,&point)==ODE_OK && point.x==(target<0 ? -6:6));
        assert(host_rhs_calls()==calls && !memcmp(trace_extent(),&extent,sizeof(extent)));
        assert(!memcmp(&d.view,&before,sizeof(before)) && !memcmp(&d.solver,&solver,sizeof(solver)));
    }
    unsigned committed=hash();trace_overlay_show(&d,&point,0,true);trace_overlay_restore();assert(hash()==committed);
    assert(trace_select(&d,0,1) && trace_point_near(point.x,&point) && point.x==6);
    trace_follow(&d,&m,&point);assert(fabs(point.y[1]+sin(point.x))<.003);
    d.solver_custom=1;assert(trace_navigate(&d,&m,1e8,false,&point)==ODE_OK && point.x==6);
    assert(host_rhs_calls()==calls && !memcmp(&solver,&d.solver,sizeof(solver)));
    /* Actual preparation remains transactional/cancellable; navigation no longer
       invokes this path just because the requested x is outside the viewport. */
    committed=hash();before=d.view;extent=*trace_extent();unchanged=point;
    host_cancel_after(10);assert(!trace_prepare(&d,&m,0,0));
    assert(hash()==committed && !memcmp(trace_extent(),&extent,sizeof(extent)));
    assert(!memcmp(&before,&d.view,sizeof(before)) && !memcmp(&unchanged,&point,sizeof(point)));
    UiBlink blink={0};assert(ui_trace_key(&blink).key==KEY_EXIT);
    d.solver.h=1e-300;assert(!trace_prepare(&d,&m,0,0));assert(hash()==committed);
    /* Ten families share the same 258-point total. Every family is selectable. */
    model_defaults(&d,EQ_GENERAL,1);strcpy(d.text[0],"1");d.solver.sf=0;
    InitialValues values;assert(initial_values_parse("{0,1,2,3,4,5,6,7,8,9}",&values)==IC_LIST_OK);
    initial_values_apply(&d,&values);prepare(9,0);solver=d.solver;
    unsigned renders=host_clear_count();calls=host_rhs_calls();
    assert(trace_navigate(&d,&m,6,true,&point)==ODE_OK); /* Y only */
    assert(host_clear_count()==renders+1 && host_rhs_calls()==calls);
    assert(d.view.xmin==-6.3 && d.view.ymin>-3.1);
    for(int f=0;f<10;f++) {
        assert(trace_select(&d,f,0));assert(trace_navigate(&d,&m,6,true,&point)==ODE_OK);
        assert(point.x==6 && fabs(point.y[0]-(f+6))<1e-12);
        int x,y;assert(graph_point(&d.view,point.x,point.y[0],&x,&y));
    }
    assert(!memcmp(&solver,&d.solver,sizeof(solver)) && TRACE_POINTS==258);
    /* Retain the previous nine-state/nine-family backend stress, including
       a late selected family and disable/re-enable cache handling. */
    model_defaults(&d,EQ_SYSTEM,9);d.nic=9;d.solver.sf=0;d.enabled=511;
    for(int f=1;f<9;f++)d.ic[f]=d.ic[0];prepare(8,8);
    assert(trace_point_near(0,&point) && trace_step(0,1,model_xdot(&d.view),&point));
    assert(trace_navigate(&d,&m,5.1,false,&point)==ODE_OK);
    d.enabled=0;graph_render(&d,&m,false);assert(!trace_prepare(&d,&m,8,8));
    d.enabled=511;prepare(8,8);assert(trace_point_near(0,&point));
    /* Invalid targets/gaps never generate a viewport from invalid y values. */
    model_defaults(&d,EQ_GENERAL,1);strcpy(d.text[0],"sqrt(1-x)");d.solver.sf=0;prepare(0,0);
    assert(trace_point_near(0,&point));assert(trace_navigate(&d,&m,6,true,&point)==ODE_HAS_INVALID);
    assert(point.x<=1 && ode_values_status(point.y,1)==ODE_OK);
    before=d.view;point.y[0]=NAN;trace_follow(&d,&m,&point);
    assert(!memcmp(&before,&d.view,sizeof(before)));
    point.y[0]=1e101;trace_follow(&d,&m,&point);assert(!memcmp(&before,&d.view,sizeof(before)));
    puts("TRACE: exact endpoints/no prefetch, fixed bounds, transactional preparation cancel/preflight, configured range preservation, ten families, Y follow and invalid endpoints passed.");
    ui_trace_input(false);
}
