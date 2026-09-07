/* Analytic trusted sample provider, deliberately substituted at link time for
   sampling.c. It proves consumers handle valid/gap/valid without pretending
   RK4 can restart beyond a singularity. All validity/render/TRACE/search code
   below is the production implementation. */
#include "trace.h"
#include "gsolve.h"
#include "ui.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;static int profile;
static double value_at(double x){double value=(fabs(x)-2)*(fabs(x)-2)-1;
    return profile ? (profile==2 ? -value:value):x;}
OdeResult model_value_at(const Document *doc,CompiledModel *model,int family,double x,
    OdeCancel cancel,void *context)
{
    (void)doc;(void)model;
    if(cancel && cancel(context))return (OdeResult){.status=ODE_CANCELLED};
    if(fabs(x)<1-1e-10)return (OdeResult){.status=ODE_DIVERGED};
    return (OdeResult){.status=ODE_OK,.x=x,.y={value_at(x)+family*5}};
}
OdeResult model_trajectory_range(const Document *doc,CompiledModel *model,int family,int direction,
    const OdeSettings *range,OdeSample sample,void *context,OdeCancel cancel,void *cancel_ctx)
{
    (void)doc;(void)model;(void)family;(void)range;
    OdeResult r={.status=ODE_OK};if(direction<0)return r;
    for(unsigned step=0;step<=60;step++) {
        if(cancel && cancel(cancel_ctx)){r.status=ODE_CANCELLED;break;}
        r.x=((int)step-30)/10.0;r.steps=step;
        double y[9]={value_at(r.x)};
        if(fabs(r.x)<1-1e-10)y[0]=r.x<-.3 ? 1e101:(r.x<.3 ? NAN:INFINITY);
        if(sample && !sample(r.x,y,step,context)){r.status=ODE_SAMPLE_STOP;break;}
    }
    return r;
}
static unsigned points,segments;static bool connected;
static bool count(double x,const double *y,uint32_t step,void *ctx)
{
    (void)x;(void)step;(void)ctx;
    if(!y){connected=false;return true;}
    assert(isfinite(y[0]) && fabs(y[0])<=1e100);
    if(!connected)segments++;
    connected=true;points++;return true;
}
static uint32_t pixels(void)
{
    uint32_t h=0;for(int i=0;i<DWIDTH*DHEIGHT;i++)h=h*31+gint_vram[i];return h;
}
int main(void)
{
    model_defaults(&d,EQ_GENERAL,1);strcpy(d.text[0],"1");d.solver.sf=0;d.view.grid=0;
    d.solver.xmin=d.view.xmin=-3;d.solver.xmax=d.view.xmax=3;
    assert(model_compile(&d,&m).expression.status==EXPR_OK);
    ModelPathResult path=model_path_branch(&d,&m,0,1,&d.solver,count,NULL,NULL,NULL);
    assert(path.status==ODE_HAS_INVALID && path.segments==2 && segments==2 && points==42);
    assert(graph_render(&d,&m,false).status==ODE_HAS_INVALID);
    int x,y;assert(graph_point(&d.view,0,0,&x,&y));
    assert(gint_vram[(y+UI_Y)*DWIDTH+x+UI_X]!=graph_palette_color(2));
    uint32_t before=pixels();trace_overlay_begin();
    assert(trace_prepare(&d,&m,0,0));TracePoint point;
    assert(trace_point_near(-1,&point) && point.x<=-1+1e-10);
    assert(trace_move(point.x,1,&point) && point.x>=1-1e-10); /* explicit nearest-valid jump */
    trace_overlay_show(&d,&point,0,true);trace_overlay_restore();assert(pixels()==before);
    GsolveCurve curve={0,0};GsolvePoint p;
    for(int side=-1;side<=1;side+=2) {
        GsolveResults r=gsolve_search(&d,&m,curve,GSOLVE_XCAL,side*2.25,NULL,NULL);
        assert(r.status==ODE_OK && r.has_invalid && r.count==1 && fabs(r.point[0].x-side*2.25)<1e-8);
        assert(gsolve_ycal(&d,&m,curve,side*2,&p,NULL,NULL)==ODE_OK && p.y==side*2);
    }
    GsolveResults roots=gsolve_search(&d,&m,curve,GSOLVE_ROOT,0,NULL,NULL);
    assert(roots.status==ODE_OK && roots.has_invalid && !roots.count); /* no false crossing across gap */
    assert(gsolve_ycal(&d,&m,curve,0,&p,NULL,NULL)==ODE_DIVERGED);
    d.nic=2;d.ic[1]=d.ic[0];d.enabled=1;
    roots=gsolve_intersections(&d,&m,curve,(GsolveCurve){1,0},NULL,NULL);
    assert(roots.status==ODE_OK && roots.has_invalid && roots.count==0);
    d.nic=1;
    profile=1;strcpy(d.text[0],"2*x-4*x/abs(x)");assert(model_compile(&d,&m).expression.status==EXPR_OK);
    roots=gsolve_search(&d,&m,curve,GSOLVE_XCAL,-.75,NULL,NULL);
    assert(roots.status==ODE_OK && roots.count==4);
    const double expected[]={-2.5,-1.5,1.5,2.5};
    for(int i=0;i<4;i++)assert(fabs(roots.point[i].x-expected[i])<1e-8);
    roots=gsolve_search(&d,&m,curve,GSOLVE_MINIMUM,0,NULL,NULL);
    assert(roots.status==ODE_OK && roots.count==2);
    assert(fabs(roots.point[0].x+2)<1e-8 && fabs(roots.point[1].x-2)<1e-8);
    roots=gsolve_search(&d,&m,curve,GSOLVE_ROOT,0,NULL,NULL);
    assert(roots.status==ODE_OK && roots.count==4);
    profile=2;strcpy(d.text[0],"-2*x+4*x/abs(x)");assert(model_compile(&d,&m).expression.status==EXPR_OK);
    roots=gsolve_search(&d,&m,curve,GSOLVE_MAXIMUM,0,NULL,NULL);
    assert(roots.status==ODE_OK && roots.count==2);
    profile=1;strcpy(d.text[0],"2*x-4*x/abs(x)");assert(model_compile(&d,&m).expression.status==EXPR_OK);
    d.color[0][0]=3;graph_render(&d,&m,false);trace_overlay_begin();
    assert(trace_prepare(&d,&m,0,0));assert(trace_point_near(2,&point));
    assert(graph_point(&d.view,1.5,-.75,&x,&y));before=pixels();
    trace_overlay_show(&d,&point,0,true);
    assert(gint_vram[(y+UI_Y)*DWIDTH+x+UI_X]==C_BLUE);
    trace_overlay_restore();assert(before==pixels());
    graph_highlight_curve(&d,&m,0,0);assert(gint_vram[(y+UI_Y)*DWIDTH+x+UI_X]==C_BLUE);
    puts("Trusted segmented stream A-I: finite islands, no gap line/root, TRACE jump/restore, X/Y-CAL, sorted roots/extrema and black-blue passed.");
}
