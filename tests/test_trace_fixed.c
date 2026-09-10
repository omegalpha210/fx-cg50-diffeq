#include "trace.h"
#include "ui.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;
unsigned long host_solves(void);
static TracePoint start(void)
{
    assert(model_compile(&d,&m).expression.status==EXPR_OK);
    assert(graph_render(&d,&m,true).status==ODE_OK);
    assert(trace_prepare(&d,&m,0,0));TracePoint p;
    assert(trace_point_near(d.ic[0].x,&p));return p;
}
static void fixed(ViewWindow before,const OdeSettings *solver,const OdeSettings *extent)
{
    const ViewWindow *v=model_view_const(&d);
    assert(v->xmin==before.xmin && v->xmax==before.xmax && v->xscale==before.xscale);
    assert(fabs((v->ymax-v->ymin)-(before.ymax-before.ymin))<1e-12);
    assert(v->yscale==before.yscale);
    assert(!memcmp(&d.solver,solver,sizeof(*solver)));
    assert(!memcmp(trace_extent(),extent,sizeof(*extent)));
}
static void scalar(void)
{
    for(int sign=-1;sign<=1;sign+=2)for(int method=ODE_RK4;method<=ODE_RK45;method++) {
        model_defaults(&d,EQ_GENERAL,1);strcpy(d.text[0],sign>0 ? "1":"-1");
        d.solver.sf=0;d.solver_custom=1;d.adaptive.method=method;d.ic[0].y[0]=0;
        d.view.xmin=-1;d.view.xmax=1;d.view.ymin=-.1;d.view.ymax=.1;
        TracePoint p=start();ViewWindow before=d.view;OdeSettings solver=d.solver,extent=*trace_extent();
        double xdot=model_xdot(&d.view);assert(trace_viewport()->xdot==xdot);
        unsigned long solves=host_solves();
        for(int direction=-1;direction<=1;direction+=2) {
            assert(trace_navigate(&d,&m,direction*6,true,&p)==ODE_OK);
            assert(p.x==direction && fabs(p.y[0]-sign*direction)<1e-10);
            int x,y;assert(graph_point(&d.view,p.x,p.y[0],&x,&y));
            assert(y>5 && y<175);
            for(int stride=1;stride<=3;stride++)for(int repeat=0;repeat<50;repeat++)
                assert(trace_navigate(&d,&m,p.x+direction*stride*xdot,false,&p)==ODE_OK && p.x==direction);
            fixed(before,&solver,&extent);assert(host_solves()==solves);
        }
        /* INIT's original point remains available without re-solving. */
        assert(trace_navigate(&d,&m,0,true,&p)==ODE_OK && p.x==0 && fabs(p.y[0])<1e-10);
        fixed(before,&solver,&extent);assert(host_solves()==solves);
        ViewWindow unchanged=d.view;p.y[0]=NAN;trace_follow(&d,&m,&p);
        assert(!memcmp(&unchanged,&d.view,sizeof(unchanged)));
        p.y[0]=1e101;trace_follow(&d,&m,&p);assert(!memcmp(&unchanged,&d.view,sizeof(unchanged)));
        p.y[0]=1e99;trace_follow(&d,&m,&p);assert(!memcmp(&unchanged,&d.view,sizeof(unchanged)));
    }
    model_defaults(&d,EQ_GENERAL,1);strcpy(d.text[0],"1");d.solver.sf=0;
    d.solver.xmin=-.375;d.solver.xmax=.875;d.solver_custom=1;
    d.view.xmin=-2;d.view.xmax=2;
    TracePoint p=start();unsigned long solves=host_solves();
    assert(trace_navigate(&d,&m,d.solver.xmax,true,&p)==ODE_OK && p.x==.875);
    assert(trace_navigate(&d,&m,d.solver.xmin,true,&p)==ODE_OK && p.x==-.375);
    assert(trace_navigate(&d,&m,-2,false,&p)==ODE_OK && p.x==-.375);
    assert(host_solves()==solves);
    /* Ordinary AUTO graph pan still expands and recomputes after TRACE. */
    d.solver_custom=0;assert(graph_zoom(&d.view,1,.2,0));model_sync_solver_window(&d);
    assert(graph_render(&d,&m,false).status==ODE_OK && host_solves()>solves);
    assert(d.solver.xmax==floor(d.view.xmax) && d.solver.xmax>.875 && d.view.xmax>2);
}
static void synthetic(bool phase)
{
    model_defaults(&d,EQ_SYSTEM,2);d.solver.xmin=0;d.solver.xmax=3;d.solver.sf=0;
    d.phase_field=d.phase_nullclines=0;
    assert(model_compile(&d,&m).expression.status==EXPR_OK);
    assert(trace_capture_begin(&d));
    for(int side=0;side<2;side++) {
        assert(trace_capture_branch_begin(&d,0,side));
        for(int i=0;i<=(side ? 3:0);i++) {
            double y[2]={i==3 ? 0:i,2*i};
            if(!phase && i==2)assert(trace_capture_point(1.5,NULL,0,NULL));
            assert(trace_capture_point(i,y,0,NULL));
        }
        trace_capture_branch_end((ModelPathResult){.status=phase ? ODE_OK:ODE_HAS_INVALID,
            .invalid=phase ? ODE_OK:ODE_DOMAIN});
    }
    trace_capture_end(true);d.view.phase=phase;
    if(phase){d.phase_view.xmin=-.5;d.phase_view.xmax=1.5;d.phase_view.ymin=-.2;d.phase_view.ymax=.2;}
    dclear(C_WHITE);trace_cache_render(&d);assert(trace_prepare(&d,&m,0,1));
    TracePoint p;assert(trace_point_near(0,&p));
    OdeSettings solver=d.solver,extent=*trace_extent();ViewWindow before=*model_view_const(&d);
    unsigned long solves=host_solves();
    /* Neither a disconnected valid sample nor an offscreen/reentering PHASE
       state can be reached by jumping over the first boundary. */
    assert(trace_navigate(&d,&m,3,true,&p)==ODE_OK && p.x==1 && p.y[0]==1 && p.y[1]==2);
    fixed(before,&solver,&extent);assert(host_solves()==solves);
    for(int i=0;i<20;i++)assert(trace_navigate(&d,&m,p.x+.25,false,&p)==ODE_OK && p.x==1);
    if(phase)assert(p.y[0]!=d.phase_view.xmax); /* No fake projection to y1=1.5. */
}
static void warnings(void)
{
    const OdeStatus cases[]={ODE_DOMAIN,ODE_NONFINITE,ODE_DIVERGED,ODE_STEP_UNDERFLOW,
        ODE_STEP_LIMIT,ODE_WORK_LIMIT,ODE_TOLERANCE,ODE_EVENT_STOP};
    for(unsigned i=0;i<sizeof(cases)/sizeof(*cases);i++) {
        dclear(UI_CYAN);
        GraphResult r={.status=i<3 ? ODE_HAS_INVALID:cases[i],.invalid=cases[i],.failed_family=-1};
        graph_status(r);
        assert(gint_vram[(UI_Y+2)*DWIDTH+UI_X+5]==C_WHITE);
        assert(gint_vram[(UI_Y+16)*DWIDTH+UI_X+5]==C_WHITE); /* Descender +2px. */
        assert(gint_vram[(UI_Y+2)*DWIDTH+UI_X+4]==UI_CYAN);
        assert(gint_vram[(UI_Y+2)*DWIDTH+UI_X+295]==UI_CYAN);
        assert(gint_vram[(UI_Y+18)*DWIDTH+UI_X+5]==UI_CYAN);
        unsigned red=0,ink=0;
        for(int y=4;y<17;y++)for(int x=7;x<291;x++) {
            unsigned color=gint_vram[(UI_Y+y)*DWIDTH+UI_X+x];
            red+=color==C_RED;ink+=color==UI_INK;
        }
        if(cases[i]==ODE_EVENT_STOP)assert(!red && ink);else assert(red);
    }
    model_defaults(&d,EQ_GENERAL,1);strcpy(d.text[0],"1");d.solver.sf=0;
    start();unsigned long solves=host_solves();
    graph_status((GraphResult){.status=ODE_HAS_INVALID,.invalid=ODE_DOMAIN});
    assert(graph_redraw_cached(&d,&m,d.solver.xmin,d.solver.xmax));
    assert(host_solves()==solves);
    for(int y=0;y<17;y++)for(int x=0;x<290;x++)assert(gint_vram[(UI_Y+y)*DWIDTH+UI_X+x]!=C_RED);
}
int main(void)
{
    scalar();synthetic(false);synthetic(true);warnings();
    puts("Fixed TRACE: visible connected bounds, Y-only follow, endpoints, zero extension solves, Phase exit/reentry, gap safety, normal Graph expansion, warning bounds passed.");
}
