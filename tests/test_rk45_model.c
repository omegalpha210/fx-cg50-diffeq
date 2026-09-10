#include "trace.h"
#include "phase_graph.h"
#include "gsolve.h"
#include "table.h"
#include "ui.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;
void host_cancel_after(unsigned polls);
static void setup(EquationKind kind,int dim)
{
    model_defaults(&d,kind,dim);d.adaptive.method=ODE_RK45;
    d.adaptive.reltol=1e-8;d.adaptive.abstol=1e-11;d.solver.h=.3;
    d.solver.xmin=-3;d.solver.xmax=3;d.solver_custom=1;d.solver.sf=0;
}
static void compile(void){assert(model_compile(&d,&m).values==ODE_OK);}
int main(void)
{
    for(int kind=EQ_SEPARABLE;kind<=EQ_SYSTEM;kind++) {
        setup((EquationKind)kind,kind<=EQ_GENERAL ? 1:(kind==EQ_HIGHER || kind==EQ_SYSTEM ? 9:2));
        compile();OdeResult r=model_value_at(&d,&m,0,.735,NULL,NULL);
        assert(r.status==ODE_OK && r.x==.735 && m.work.rhs>0);
        assert(ode_values_status(r.y,d.dim)==ODE_OK);
    }
    setup(EQ_SECOND,2);compile();OdeResult r=model_value_at(&d,&m,0,2.173,NULL,NULL);
    assert(r.status==ODE_OK && fabs(r.y[0]-cos(2.173))<2e-8);
    GsolvePoint point;assert(gsolve_ycal(&d,&m,(GsolveCurve){0,0},.733,&point,NULL,NULL)==ODE_OK);
    assert(fabs(point.y-cos(.733))<2e-8);
    GsolveResults g=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_ROOT,0,NULL,NULL);
    assert(g.status==ODE_OK && g.count==2 && fabs(g.point[0].x+acos(-1)/2)<2e-8);
    g=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_MAXIMUM,0,NULL,NULL);
    assert(g.status==ODE_OK && g.count==1 && fabs(g.point[0].x)<1e-8);
    g=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_XCAL,.5,NULL,NULL);
    assert(g.status==ODE_OK && g.count==2 && fabs(g.point[0].x+acos(.5))<2e-8);
    g=gsolve_intersections(&d,&m,(GsolveCurve){0,0},(GsolveCurve){0,1},NULL,NULL);
    assert(g.status==ODE_OK && g.count==2);
    assert(gsolve_ycal(&d,&m,(GsolveCurve){0,0},0,&point,NULL,NULL)==ODE_OK && point.y==1);
    d.ic[0].y[0]=-1;compile();
    g=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_MINIMUM,0,NULL,NULL);
    assert(g.status==ODE_OK && g.count==1 && fabs(g.point[0].x)<1e-8 && fabs(g.point[0].y+1)<1e-9);
    d.ic[0].y[0]=1;compile();
    TableIndex index;TablePage page;
    assert(table_index_build(&d,&m,&index,NULL,NULL)==ODE_OK);
    double spacing=index.spacing;d.solver.h=.7;d.solver.step=29;
    assert(table_index_build(&d,&m,&index,NULL,NULL)==ODE_OK && index.spacing==spacing);
    unsigned rows=index.total;d.solver.h=1e-5;d.solver.step=10000;
    assert(table_index_build(&d,&m,&index,NULL,NULL)==ODE_OK && index.total==rows);
    d.solver.h=.3;
    table_read_page(&d,&m,&index,index.mid,&page,NULL,NULL);assert(page.result.status==ODE_OK);
    for(unsigned row=0;row<page.count;row++)assert(fabs(page.row[row][1]-cos(page.row[row][0]))<2e-8);
    m.work.rhs=ODE_RHS_BUDGET;
    table_read_page_budgeted(&d,&m,&index,index.mid,&page,NULL,NULL);
    assert(page.result.status==ODE_WORK_LIMIT);
    assert(graph_render(&d,&m,true).status==ODE_OK && trace_prepare(&d,&m,0,0));
    ui_trace_input(true);TracePoint cursor;assert(trace_point_near(0,&cursor));
    OdeSettings extent=*trace_extent(),solver=d.solver;ViewWindow view=d.view;
    unsigned calls=m.work.rhs;
    assert(trace_navigate(&d,&m,3.25,false,&cursor)==ODE_OK && cursor.x==3);
    assert(m.work.rhs==calls && !memcmp(trace_extent(),&extent,sizeof(extent)));
    assert(!memcmp(&d.view,&view,sizeof(view)) && fabs(cursor.y[0]-cos(3))<2e-8);
    assert(!memcmp(&d.solver,&solver,sizeof(solver)));
    /* RK45 cache preparation still rolls back on cancellation. */
    TracePoint before=cursor;trace_cache_invalidate();host_cancel_after(10);
    assert(!trace_prepare(&d,&m,0,0));
    assert(!memcmp(&cursor,&before,sizeof(cursor)) && !memcmp(&d.view,&view,sizeof(view)));
    UiBlink blink={0};assert(ui_trace_key(&blink).key==KEY_EXIT);
    ui_trace_input(false);
    setup(EQ_GENERAL,1);strcpy(d.text[0],"y");d.nic=10;
    for(int i=0;i<10;i++)d.ic[i].y[0]=(i+1)*.1;
    compile();GraphResult result=graph_render(&d,&m,true);assert(result.status==ODE_OK);
    assert(m.work.rhs<=ODE_RHS_BUDGET && m.work.work<=ODE_RHS_WORK_BUDGET);
    assert(table_index_build(&d,&m,&index,NULL,NULL)==ODE_OK && index.count==10);
    table_read_page(&d,&m,&index,index.mid,&page,NULL,NULL);
    for(unsigned row=0;row<page.count;row++)for(int col=0;col<10;col++)
        assert(fabs(page.row[row][col+1]-(col+1)*.1*exp(page.row[row][0]))<2e-8);
    setup(EQ_SYSTEM,2);compile();assert(graph_render(&d,&m,true).status==ODE_OK);
    assert(trace_cache_matches(&d));d.view.phase=1;d.phase_field=0;
    assert(trace_cache_phase_window(&d,&d.phase_view));
    assert(graph_render(&d,&m,false).status==ODE_OK && m.work.rhs==0);
    assert(trace_prepare(&d,&m,0,1));TracePoint p;assert(trace_point_near(.7,&p));
    assert(fabs(p.y[0]-cos(.7))<2e-3 && fabs(p.y[1]+sin(.7))<2e-3); /* display-only */
    assert(graph_phase_search(&d,&m,NULL,NULL)==ODE_OK && graph_phase_results()->count==1);
    d.adaptive.method=ODE_RK4;assert(!trace_cache_matches(&d));d.adaptive.method=ODE_RK45;
    d.adaptive.reltol*=2;assert(!trace_cache_matches(&d));
    setup(EQ_GENERAL,1);strcpy(d.text[0],"y^2");d.ic[0].y[0]=1;compile();
    assert(table_index_build(&d,&m,&index,NULL,NULL)==ODE_OK && index.high==ODE_STEP_UNDERFLOW);
    assert(index.xmax<1.000001);
    table_read_page(&d,&m,&index,table_bottom(&index),&page,NULL,NULL);assert(page.result.status==ODE_OK);
    setup(EQ_GENERAL,1);strcpy(d.text[0],"y");d.ic[0].y[0]=1;compile();
    m.work.work=ODE_RHS_WORK_BUDGET;
    r=model_value_at(&d,&m,0,1,NULL,NULL);assert(r.status==ODE_WORK_LIMIT && r.x==0);
    puts("RK45 integration: all modes/9D, 10 IC, exact Table/G-Solve coordinates, shared budgets, Phase/cache and singular terminal passed.");
}
