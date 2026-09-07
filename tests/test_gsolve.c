#include "gsolve.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static Document d;
static CompiledModel m;
static void prepare(const char *rhs,double y0)
{
    model_defaults(&d,EQ_GENERAL,1);
    strcpy(d.text[0],rhs);d.ic[0].y[0]=y0;d.solver.h=.025;
    d.solver.xmin=-3;d.solver.xmax=3;d.view.xmin=-2;d.view.xmax=2;
    d.view.ymin=-3;d.view.ymax=3;
    ModelError error=model_compile(&d,&m);
    assert(error.values==ODE_OK && error.expression.status==EXPR_OK);
}
static void near(double actual,double expected,double tolerance)
{ assert(fabs(actual-expected)<tolerance); }
static void same_points(GsolveResults a,GsolveResults b,double tolerance)
{
    assert(a.status==ODE_OK && b.status==ODE_OK && a.count==b.count);
    for(int i=0;i<a.count;i++) {
        near(a.point[i].x,b.point[i].x,tolerance);
        near(a.point[i].y,b.point[i].y,tolerance);
    }
}

int main(void)
{
    GsolveCurve curve;
    prepare("2*x",-1);
    assert(gsolve_curve_count(&d)==1 && gsolve_curve_at(&d,0,&curve));
    assert(curve.family==0 && curve.variable==0 && !gsolve_curve_at(&d,1,&curve));
    GsolveResults roots=gsolve_search(&d,&m,curve,GSOLVE_ROOT,0,NULL,NULL);
    assert(roots.status==ODE_OK && roots.count==2);
    near(roots.point[0].x,-1,1e-8);near(roots.point[1].x,1,1e-8);
    near(roots.point[0].y,0,1e-8);near(roots.point[1].y,0,1e-8);
    GsolveResults minimum=gsolve_search(&d,&m,curve,GSOLVE_MINIMUM,0,NULL,NULL);
    assert(minimum.status==ODE_OK && minimum.count==1);
    near(minimum.point[0].x,0,1e-8);near(minimum.point[0].y,-1,1e-8);
    GsolvePoint point;
    assert(gsolve_ycal(&d,&m,curve,.5,&point,NULL,NULL)==ODE_OK);
    near(point.x,.5,1e-15);near(point.y,-.75,1e-8);
    GsolveResults xcal=gsolve_search(&d,&m,curve,GSOLVE_XCAL,-.75,NULL,NULL);
    assert(xcal.status==ODE_OK && xcal.count==2);
    near(xcal.point[0].x,-.5,1e-8);near(xcal.point[1].x,.5,1e-8);
    d.view.ymin=5;d.view.ymax=20;
    same_points(roots,gsolve_search(&d,&m,curve,GSOLVE_ROOT,0,NULL,NULL),1e-12);
    same_points(minimum,gsolve_search(&d,&m,curve,GSOLVE_MINIMUM,0,NULL,NULL),1e-12);
    same_points(xcal,gsolve_search(&d,&m,curve,GSOLVE_XCAL,-.75,NULL,NULL),1e-12);

    prepare("-2*x",1);
    GsolveResults maximum=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_MAXIMUM,0,NULL,NULL);
    assert(maximum.status==ODE_OK && maximum.count==1);
    near(maximum.point[0].x,0,1e-8);near(maximum.point[0].y,1,1e-8);
    d.view.ymin=-5;d.view.ymax=-2;
    same_points(maximum,gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_MAXIMUM,0,NULL,NULL),1e-12);

    prepare("2*x",-10);d.view.ymin=-2;d.view.ymax=2;
    minimum=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_MINIMUM,0,NULL,NULL);
    assert(minimum.status==ODE_OK && minimum.count==1);
    near(minimum.point[0].x,0,1e-8);near(minimum.point[0].y,-10,1e-8);

    prepare("0",12);d.view.ymin=-1;d.view.ymax=1;
    assert(gsolve_ycal(&d,&m,(GsolveCurve){0,0},0,&point,NULL,NULL)==ODE_OK);
    near(point.y,12,1e-12);
    assert(gsolve_ycal(&d,&m,(GsolveCurve){0,0},1,&point,NULL,NULL)==ODE_OK);
    near(point.y,12,1e-12);

    prepare("1",8);d.view.ymin=-1;d.view.ymax=1;
    xcal=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_XCAL,10,NULL,NULL);
    assert(xcal.status==ODE_OK && xcal.count==1);near(xcal.point[0].x,2,1e-8);
    d.view.xmax=1.5;
    assert(gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_XCAL,10,NULL,NULL).count==0);

    prepare("0",0);
    roots=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_ROOT,0,NULL,NULL);
    assert(roots.status==ODE_OK && roots.count==1);

    d.nic=2;d.ic[1]=d.ic[0];d.graph_mask[0]=1;d.graph_mask[1]=1;
    assert(gsolve_curve_count(&d)==2);
    assert(gsolve_curve_at(&d,1,&curve) && curve.family==1 && curve.variable==0);
    d.view.phase=1;assert(gsolve_curve_count(&d)==0);
    d.view.phase=0;d.graph_mask[0]=0;assert(gsolve_curve_count(&d)==1);

    model_defaults(&d,EQ_SECOND,2);d.solver.h=.025;
    ModelError error=model_compile(&d,&m);
    assert(error.values==ODE_OK && error.expression.status==EXPR_OK);
    curve=(GsolveCurve){0,0};
    roots=gsolve_search(&d,&m,curve,GSOLVE_ROOT,0,NULL,NULL);
    assert(roots.status==ODE_OK && roots.count==4);
    near(roots.point[0].x,-1.5*3.14159265358979323846,2e-7);
    near(roots.point[1].x,-.5*3.14159265358979323846,2e-7);
    near(roots.point[2].x,.5*3.14159265358979323846,2e-7);
    near(roots.point[3].x,1.5*3.14159265358979323846,2e-7);
    maximum=gsolve_search(&d,&m,curve,GSOLVE_MAXIMUM,0,NULL,NULL);
    minimum=gsolve_search(&d,&m,curve,GSOLVE_MINIMUM,0,NULL,NULL);
    assert(maximum.count==1 && minimum.count==2);
    near(maximum.point[0].x,0,2e-7);near(maximum.point[0].y,1,2e-7);
    near(minimum.point[0].x,-3.14159265358979323846,2e-7);
    near(minimum.point[1].x,3.14159265358979323846,2e-7);

    GsolveResults intersections=gsolve_intersections(&d,&m,(GsolveCurve){0,0},
        (GsolveCurve){0,1},NULL,NULL);
    assert(intersections.status==ODE_OK && intersections.count==4);
    near(intersections.point[0].x,-1.25*3.14159265358979323846,2e-7);
    near(intersections.point[1].x,-.25*3.14159265358979323846,2e-7);
    near(intersections.point[2].x,.75*3.14159265358979323846,2e-7);
    near(intersections.point[3].x,1.75*3.14159265358979323846,2e-7);
    d.view.ymin=2;d.view.ymax=3;
    same_points(intersections,gsolve_intersections(&d,&m,(GsolveCurve){0,0},
        (GsolveCurve){0,1},NULL,NULL),1e-12);
    assert(!gsolve_intersections(&d,&m,(GsolveCurve){0,0},
        (GsolveCurve){0,0},NULL,NULL).count);

    puts("G-Solve X-domain/Y-invariance, ICPT, polynomial and harmonic checks passed.");
    return 0;
}
