/* Before/after numerical contract: no alternate solver implementation. */
#include "model.h"
#include "gsolve.h"
#include "phase.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Document d;
static CompiledModel m;
static unsigned samples,segments;
static double last_x,last_y[ODE_MAX_DIM];
static void compile_model(void)
{
    ModelError e=model_compile(&d,&m);
    assert(e.values==ODE_OK && e.expression.status==EXPR_OK);
    solver_report_begin(&d,&m);samples=segments=0;
}
static bool sample(double x,const double *y,uint32_t step,void *ctx)
{
    (void)step;(void)ctx;
    if(!y){segments++;return true;}
    samples++;last_x=x;memcpy(last_y,y,(size_t)d.dim*sizeof(double));
    /* Every solution sample is frozen, not merely the final endpoint. */
    printf("sample %u %a",samples,x);
    for(int j=0;j<d.dim;j++)printf(" %a",y[j]);
    puts("");return true;
}
static void result(const char *name,OdeResult r)
{
    printf("result %s %d %u %u %u %u %u %u %u %a",name,r.status,r.steps,
        m.work.accepted,m.work.rejected,m.work.rhs,m.work.work,m.event_hits,m.event_evals,r.x);
    for(int j=0;j<d.dim;j++)printf(" %a",r.y[j]);
    puts("");
    const EventMarkers *markers=&solver_report()->markers;
    for(unsigned i=0;i<markers->count;i++) {
        printf("event %u %d %a",i,markers->point[i].direction,markers->point[i].x);
        for(int j=0;j<d.dim;j++)printf(" %a",markers->point[i].y[j]);
        puts("");
    }
}
static void setup(EquationKind kind,int dim,int method)
{
    model_defaults(&d,kind,dim);d.adaptive.method=method;
    d.adaptive.reltol=1e-8;d.adaptive.abstol=1e-11;d.solver.h=method ? .2:.05;
    d.solver.xmin=-2;d.solver.xmax=2;d.solver.step=1;d.solver.sf=0;d.solver_custom=1;
}
static bool contour(int component,const double a[2],const double b[2],void *ctx)
{(void)ctx;printf("contour %d %a %a %a %a\n",component,a[0],a[1],b[0],b[1]);return true;}
int main(void)
{
    for(int method=ODE_RK4;method<=ODE_RK45;method++) {
        setup(EQ_GENERAL,1,method);strcpy(d.text[0],"y");compile_model();
        result("scalar",model_trajectory(&d,&m,0,1,sample,NULL,NULL,NULL));
        setup(EQ_HIGHER,9,method);strcpy(d.text[0],"y");
        for(int j=0;j<9;j++)d.ic[0].y[j]=1;
        compile_model();result("nth9",model_value_at(&d,&m,0,.75,NULL,NULL));
        setup(EQ_SYSTEM,9,method);
        for(int j=0;j<9;j++){snprintf(d.text[j],EXPR_TEXT,"-%d*y%d",j+1,j+1);d.ic[0].y[j]=j+1;}
        compile_model();result("system9",model_value_at(&d,&m,0,.75,NULL,NULL));
        for(int direction=EVENT_ANY;direction<=EVENT_FALLING;direction++)for(int action=EVENT_MARK;action<=EVENT_STOP;action++) {
            setup(EQ_GENERAL,1,method);strcpy(d.text[0],"1");d.ic[0].y[0]=-1;
            d.event.enabled=1;d.event.direction=(uint8_t)direction;d.event.action=(uint8_t)action;
            strcpy(d.event.text,"y");compile_model();
            printf("event-case %d %d %d\n",method,direction,action);
            result("event",model_value_at(&d,&m,0,2,NULL,NULL));
        }
        setup(EQ_GENERAL,1,method);strcpy(d.text[0],"sqrt(1-x)");compile_model();
        ModelPathResult path=model_path_branch(&d,&m,0,1,&d.solver,sample,NULL,NULL,NULL);
        printf("domain %d %d %u %u %u %u %u %u\n",path.status,path.invalid,path.steps,path.points,path.segments,samples,m.work.accepted,m.work.rejected);
        setup(EQ_SECOND,2,method);d.solver.xmin=-4;d.solver.xmax=4;compile_model();
        for(int mode=GSOLVE_ROOT;mode<=GSOLVE_XCAL;mode++) {
            GsolveResults found=gsolve_search(&d,&m,(GsolveCurve){0,0},(GsolveMode)mode,.5,NULL,NULL);
            printf("gsolve %d %d %d %d %u\n",method,mode,found.status,found.count,found.steps);
            for(int i=0;i<found.count;i++)printf("point %a %a\n",found.point[i].x,found.point[i].y);
        }
        GsolvePoint p;OdeStatus status=gsolve_ycal(&d,&m,(GsolveCurve){0,0},.3,&p,NULL,NULL);
        printf("ycal %d %d %a %a\n",method,status,p.x,p.y);
        GsolveResults found=gsolve_intersections(&d,&m,(GsolveCurve){0,0},(GsolveCurve){0,1},NULL,NULL);
        printf("icpt %d %d %d %u\n",method,found.status,found.count,found.steps);
        for(int i=0;i<found.count;i++)printf("point %a %a\n",found.point[i].x,found.point[i].y);
        setup(EQ_GENERAL,1,method);d.nic=3;d.ic_enabled=5;
        for(int i=0;i<3;i++){d.ic[i].y[0]=i+1;model_curve_color(&d,i,0,(unsigned)(i+1));}
        compile_model();for(int i=0;i<3;i++) {
            printf("output %d %d %u\n",i,model_curve_visible(&d,i,0),model_color(&d,i,0));
            result("multi-ic",model_value_at(&d,&m,i,.5,NULL,NULL));
        }
    }
    setup(EQ_SYSTEM,2,ODE_RK45);strcpy(d.text[0],"y2");strcpy(d.text[1],"-y1");compile_model();
    double state[2]={1,2},vector[2],direction[2];
    assert(phase_vector(&m,0,state,vector)==ODE_OK);
    assert(phase_direction(&d.view,vector,383,179,direction));
    printf("phase-vector %a %a %a %a\n",vector[0],vector[1],direction[0],direction[1]);
    printf("null-status %d\n",phase_nullclines(&m,0,&d.view,8,contour,NULL,NULL,NULL));
    PhaseResults equilibria;assert(phase_equilibria(&m,0,&d.view,&equilibria,NULL,NULL)==ODE_OK);
    printf("eqpt %u %u\n",equilibria.count,equilibria.evaluations);
    for(unsigned i=0;i<equilibria.count;i++) {
        PhaseRoot *p=&equilibria.root[i];printf("equilibrium %d %a %a %a %a %a\n",p->type,p->y[0],p->y[1],p->residual,p->eigen_real[0],p->eigen_imag[0]);
    }
    return 0;
}
