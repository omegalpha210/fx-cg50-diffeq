#include "graph.h"
#include "trace.h"
#include "gsolve.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;
static bool cancelled(void *p){(void)p;return true;}
int main(void)
{
    model_defaults(&d,EQ_GENERAL,1);d.solver.sf=0;d.solver.h=.01;
    d.view.xmin=d.solver.xmin=-4;d.view.xmax=d.solver.xmax=4;
    for(int side=-1;side<=1;side+=2) {
        strcpy(d.text[0],side<0 ? "-100*y":"100*y");
        assert(model_compile(&d,&m).expression.status==EXPR_OK);
        GraphResult graph=graph_render(&d,&m,false);
        assert(graph.status==ODE_HAS_INVALID && graph.invalid==ODE_DIVERGED);
        assert(trace_prepare(&d,&m,0,0) && trace_has_invalid());
        TracePoint point;assert(trace_point_near(0,&point));
        for(int i=0;i<300;i++) {
            trace_move(point.x,side,&point);
            assert(isfinite(point.x) && isfinite(point.y[0]) && fabs(point.y[0])<=1e100);
        }
        GsolveResults r=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_XCAL,2,NULL,NULL);
        assert(r.status==ODE_OK && r.has_invalid && r.count==1);
        assert(fabs(r.point[0].x-side*log(2)/100)<.0002);
        GsolvePoint exact;
        assert(gsolve_ycal(&d,&m,(GsolveCurve){0,0},side*.01,&exact,NULL,NULL)==ODE_OK);
        assert(gsolve_ycal(&d,&m,(GsolveCurve){0,0},side*4,&exact,NULL,NULL)==ODE_DIVERGED);
        assert(gsolve_ycal(&d,&m,(GsolveCurve){0,0},0,&exact,NULL,NULL)==ODE_OK && exact.y==1);
        r=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_ROOT,0,cancelled,NULL);
        assert(r.status==ODE_CANCELLED);
    }
    /* The secondary IC can be unreachable at the primary IC, but reachable
       later in the same scan. A failed secondary lookup must not poison it. */
    model_defaults(&d,EQ_SYSTEM,2);d.nic=2;d.solver.sf=0;d.solver.h=.001;
    d.view.xmin=d.solver.xmin=-1;d.view.xmax=d.solver.xmax=4;
    strcpy(d.text[0],"1");strcpy(d.text[1],"-100*y2");
    d.ic[0].y[0]=d.ic[0].y[1]=0;
    d.ic[1].x=4;d.ic[1].y[0]=4;d.ic[1].y[1]=1;
    assert(model_compile(&d,&m).expression.status==EXPR_OK);
    GsolveResults intersections=gsolve_intersections(&d,&m,(GsolveCurve){0,0},
        (GsolveCurve){1,1},NULL,NULL);
    assert(intersections.status==ODE_OK && intersections.has_invalid && intersections.count==1);
    double intersection_x=intersections.point[0].x;
    assert(intersection_x>3.98 && intersection_x<4);
    assert(fabs(intersection_x-exp(100*(4-intersection_x)))<.0001);
    puts("Real RK4: guarded side retained, opposite direction usable, TRACE boundaries, X/Y-CAL and distinct cancellation passed.");
}
