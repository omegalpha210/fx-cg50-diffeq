#include "phase.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static Document d;
static CompiledModel m;
static unsigned maximum_evaluations;
static unsigned maximum_contour_evaluations;
static void prepare(const char *first,const char *second)
{
    model_defaults(&d,EQ_SYSTEM,2);
    strcpy(d.text[0],first);strcpy(d.text[1],second);
    d.view.xmin=-2;d.view.xmax=2;d.view.ymin=-2;d.view.ymax=2;
    ModelError error=model_compile(&d,&m);
    assert(error.values==ODE_OK && error.expression.status==EXPR_OK);
}
static void near(double a,double b,double tolerance)
{if(!(fabs(a-b)<=tolerance))fprintf(stderr,"near %.17g expected %.17g tol %.4g\n",a,b,tolerance);assert(fabs(a-b)<=tolerance);}
static PhaseResults search(void)
{
    PhaseResults results;
    OdeStatus s=phase_equilibria(&m,0,&d.view,&results,NULL,NULL);
    if(s!=ODE_OK)fprintf(stderr,"search status %d for %s / %s\n",s,d.text[0],d.text[1]);
    assert(s==ODE_OK && results.evaluations<=PHASE_EVAL_LIMIT);
    if(results.evaluations>maximum_evaluations)maximum_evaluations=results.evaluations;
    return results;
}
static PhaseRoot only(PhaseType type)
{
    PhaseResults r=search();
    if(r.count!=1 || r.root[0].type!=type)fprintf(stderr,"roots %u type %d expected %d for %s / %s\n",r.count,r.count ? (int)r.root[0].type:-1,type,d.text[0],d.text[1]);
    assert(r.count==1 && r.root[0].type==type);
    return r.root[0];
}
static bool cancel(void *ctx)
{unsigned *remaining=ctx;return (*remaining)--==0;}
static bool count_evaluations(void *ctx)
{unsigned *count=ctx;(*count)++;return false;}
typedef struct {unsigned n[2];int mode;} Contours;
static bool contour(int k,const double a[2],const double b[2],void *ctx)
{
    Contours *c=ctx;assert(k==0 || k==1);c->n[k]++;
    assert(ode_values_status(a,2)==ODE_OK && ode_values_status(b,2)==ODE_OK);
    if(c->mode==0) {near(a[1-k],0,1e-8);near(b[1-k],0,1e-8);}
    if(c->mode==1 && !k)assert(!"A pole must not produce a nullcline");
    if(c->mode==2 && !k) {
        near(a[1],-.75,1e-8);near(b[1],-.75,1e-8);
    }
    if(c->mode==3 && !k) {
        near(a[0]*a[0]+a[1]*a[1],1,1e-6);
        near(b[0]*b[0]+b[1]*b[1],1,1e-6);
    }
    return true;
}
static bool stop_segment(int k,const double a[2],const double b[2],void *ctx)
{(void)k;(void)a;(void)b;(void)ctx;return false;}
static void test_linear(void)
{
    prepare("y2","-y1");assert(phase_autonomous(&m));
    PhaseRoot r=only(PHASE_CENTER_NEUTRAL);
    near(r.y[0],0,1e-9);near(r.y[1],0,1e-9);
    near(r.jacobian[0],0,1e-9);near(r.jacobian[1],1,1e-9);
    near(r.jacobian[2],-1,1e-9);near(r.jacobian[3],0,1e-9);
    near(r.eigen_real[0],0,1e-9);near(fabs(r.eigen_imag[0]),1,1e-9);
    prepare("y1","-y2");r=only(PHASE_SADDLE);
    assert(r.eigen_real[0]*r.eigen_real[1]<0);
    prepare("-y1","-2*y2");r=only(PHASE_STABLE_NODE);
    near(r.eigen_real[0]+r.eigen_real[1],-3,1e-8);
    prepare("y1","2*y2");only(PHASE_UNSTABLE_NODE);
    prepare("-y1","-y2");only(PHASE_STABLE_NODE); /* repeated eigenvalue */
    prepare("-0.2*y1-y2","y1-0.2*y2");r=only(PHASE_STABLE_SPIRAL);
    near(r.eigen_real[0],-.2,1e-9);near(fabs(r.eigen_imag[0]),1,1e-9);
    prepare("0.2*y1-y2","y1+0.2*y2");only(PHASE_UNSTABLE_SPIRAL);
    prepare("y1^2","-y2");r=only(PHASE_INCONCLUSIVE);
    near(r.eigen_real[0]+r.eigen_real[1],-1,1e-9);
    near(r.eigen_real[0]*r.eigen_real[1],0,1e-9);
    prepare("-y1^3","-y2");only(PHASE_INCONCLUSIVE);
    prepare("abs(y1)","-y2");only(PHASE_UNAVAILABLE);
    prepare("sqrt(y1)","-y2");r=only(PHASE_UNAVAILABLE);
    near(r.y[0],0,1e-9);
}
static void test_nonlinear(void)
{
    prepare("y1*(1-y1-y2)","y2*(0.5-y1)");
    PhaseResults r=search();assert(r.count==3);
    const double expected[3][2]={{0,0},{.5,.5},{1,0}};
    for(unsigned i=0;i<3;i++) {
        near(r.root[i].y[0],expected[i][0],1e-7);
        near(r.root[i].y[1],expected[i][1],1e-7);
    }
    prepare("y1*(y1-1)*(y1+1)","y2*(y2-1)*(y2+1)");
    r=search();assert(r.count==9 && !r.truncated);
    for(unsigned i=0;i<r.count;i++)for(unsigned j=i+1;j<r.count;j++)
        assert(hypot(r.root[i].y[0]-r.root[j].y[0],r.root[i].y[1]-r.root[j].y[1])>.1);
    prepare("y1-0.12345","y2+0.2468");
    PhaseRoot root=only(PHASE_UNSTABLE_NODE);
    near(root.y[0],.12345,1e-8);near(root.y[1],-.2468,1e-8);
    prepare("1e-90*(y1-0.12345)","1e80*(y2+0.2468)");
    root=only(PHASE_INCONCLUSIVE);
    near(root.y[0],.12345,1e-8);near(root.y[1],-.2468,1e-8);
    prepare("1e-90*(y1-0.12345)","1e-90*(y2+0.2468)");
    root=only(PHASE_UNSTABLE_NODE);near(root.y[0],.12345,1e-8);
    prepare("y1-1000000.25","-2*(y2+1000000.125)");
    d.view.xmin=999998;d.view.xmax=1000002;d.view.ymin=-1000002;d.view.ymax=-999998;
    root=only(PHASE_SADDLE);
    near(root.y[0],1000000.25,1e-7);near(root.y[1],-1000000.125,1e-7);
    prepare("y1-1e-12","y2+2e-12");
    d.view.xmin=-4e-12;d.view.xmax=4e-12;d.view.ymin=-4e-12;d.view.ymax=4e-12;
    root=only(PHASE_UNSTABLE_NODE);near(root.y[0],1e-12,1e-19);near(root.y[1],-2e-12,1e-19);
    prepare("0","0");r=search();
    assert(r.count==PHASE_MAX_ROOTS && r.truncated);
    for(unsigned i=0;i<r.count;i++)assert(r.root[i].type==PHASE_INCONCLUSIVE);
}
static void test_invalid_and_cancellation(void)
{
    prepare("1/(y1-0.1)","y2");assert(search().count==0);
    prepare("1e-90","y2");assert(search().count==0);
    prepare("1e101","y2");PhaseResults r=search();assert(!r.count && r.has_invalid);
    prepare("ln(-abs(y1)-1)","y2");r=search();assert(!r.count && r.has_invalid);
    prepare("y2+x","-y1");assert(!phase_autonomous(&m));
    PhaseResults before;memset(&before,0xa5,sizeof(before));r=before;
    assert(phase_equilibria(&m,.75,&d.view,&r,NULL,NULL)==ODE_BAD_INPUT);
    assert(!memcmp(&before,&r,sizeof(r)));
    double y[2]={2,3},f[2];assert(phase_vector(&m,.75,y,f)==ODE_OK);
    near(f[0],3.75,1e-15);near(f[1],-2,1e-15);
    prepare("exp(y1)","y2");assert(phase_autonomous(&m));
    prepare("y2+0*x","-y1");assert(!phase_autonomous(&m)); /* bytecode use */
    prepare("y2+X","-y1");assert(!phase_autonomous(&m));
    prepare("y2","-y1");
    for(unsigned stop=0;stop<140;stop+=7) {
        unsigned remaining=stop;r=before;
        assert(phase_equilibria(&m,0,&d.view,&r,cancel,&remaining)==ODE_CANCELLED);
        assert(!memcmp(&before,&r,sizeof(r)));
    }
    d.view.xmax=d.view.xmin;
    assert(phase_equilibria(&m,0,&d.view,&r,NULL,NULL)==ODE_BAD_INPUT);
    assert(!memcmp(&before,&r,sizeof(r)));
    prepare("y2","-y1");
    double bad[2]={NAN,0},unchanged[2]={12,34};
    assert(phase_vector(&m,0,bad,unchanged)==ODE_BAD_INPUT);
    assert(unchanged[0]==12 && unchanged[1]==34);
}
static void test_contours(void)
{
    prepare("y2","-y1");
    for(unsigned grid=9;grid<=20;grid++) {
        Contours c={0};unsigned calls=0;
        assert(phase_nullclines(&m,0,&d.view,grid,contour,&c,count_evaluations,&calls)==ODE_OK);
        assert(c.n[0]>=grid && c.n[1]>=grid);
        unsigned dry_calls=0;
        assert(phase_nullclines(&m,0,&d.view,grid,NULL,NULL,count_evaluations,&dry_calls)==ODE_OK);
        assert(dry_calls==calls);
        if(calls>maximum_contour_evaluations)maximum_contour_evaluations=calls;
    }
    Contours c={0};unsigned stop=30;
    assert(phase_nullclines(&m,0,&d.view,12,contour,&c,cancel,&stop)==ODE_CANCELLED);
    assert(phase_nullclines(&m,0,&d.view,12,stop_segment,NULL,NULL,NULL)==ODE_SAMPLE_STOP);
    assert(phase_nullclines(&m,0,&d.view,21,contour,&c,NULL,NULL)==ODE_BAD_INPUT);
    prepare("1/(y1-0.1)","y2");c=(Contours){.mode=1};
    assert(phase_nullclines(&m,0,&d.view,12,contour,&c,NULL,NULL)==ODE_OK);
    assert(!c.n[0] && c.n[1]);
    prepare("(y1-0.1)/abs(y1-0.1)","y2");c=(Contours){.mode=1};
    assert(phase_nullclines(&m,0,&d.view,12,contour,&c,NULL,NULL)==ODE_OK);
    assert(!c.n[0] && c.n[1]);
    prepare("ln(-abs(y1)-1)","y2");c=(Contours){.mode=1};
    assert(phase_nullclines(&m,0,&d.view,12,contour,&c,NULL,NULL)==ODE_OK);
    assert(!c.n[0] && !c.n[1]);
    prepare("0","0");c=(Contours){0};
    assert(phase_nullclines(&m,0,&d.view,12,contour,&c,NULL,NULL)==ODE_OK);
    assert(!c.n[0] && !c.n[1]); /* zero everywhere is not a grid of curves */
    prepare("y2+x","-y1");c=(Contours){.mode=2};
    assert(phase_nullclines(&m,.75,&d.view,12,contour,&c,NULL,NULL)==ODE_OK);
    assert(c.n[0] && c.n[1]);
    prepare("y1^2+y2^2-1","y2");c=(Contours){.mode=3};
    assert(phase_nullclines(&m,0,&d.view,20,contour,&c,NULL,NULL)==ODE_OK);
    if(c.n[0]<=10 || c.n[1]<=10)fprintf(stderr,"circle contours %u / %u\n",c.n[0],c.n[1]);
    assert(c.n[0]>10 && c.n[1]>10);
    prepare("sin(101*y1+31*y2)","sin(37*y1-103*y2)");c=(Contours){.mode=4};
    unsigned calls=0;
    OdeStatus s=phase_nullclines(&m,0,&d.view,20,contour,&c,count_evaluations,&calls);
    assert((s==ODE_OK || s==ODE_WORK_LIMIT) && calls<=PHASE_EVAL_LIMIT+1);
    printf("Oscillatory contour stress: status=%d, evaluation/poll calls=%u.\n",s,calls);
}
static void test_direction(void)
{
    prepare("y2","-y1");double out[2],vector[2]={1,1};
    assert(phase_direction(&d.view,vector,200,100,out));
    near(out[0],2/sqrt(5),1e-12);near(out[1],-1/sqrt(5),1e-12);
    d.view.ymin=-1;d.view.ymax=1;
    assert(phase_direction(&d.view,vector,200,100,out));
    near(out[0],sqrt(.5),1e-12);near(out[1],-sqrt(.5),1e-12);
    vector[0]=1e100;vector[1]=1e100;
    assert(phase_direction(&d.view,vector,200,100,out));
    near(out[0],sqrt(.5),1e-12);
    d.view.xmin=-1e-300;d.view.xmax=1e-300;
    assert(phase_direction(&d.view,vector,200,100,out));near(out[0],1,1e-12);
    vector[0]=0;vector[1]=1;assert(phase_direction(&d.view,vector,200,100,out));
    near(out[0],0,1e-12);near(out[1],-1,1e-12);
    vector[1]=0;assert(!phase_direction(&d.view,vector,200,100,out));
    vector[0]=NAN;assert(!phase_direction(&d.view,vector,200,100,out));
}
int main(void)
{
    test_linear();test_nonlinear();test_invalid_and_cancellation();test_contours();test_direction();
    printf("Phase numeric checks passed: bounded roots=%d, result=%zu bytes, max known-system RHS evaluations=%u, max linear contour evaluations=%u.\n",
        PHASE_MAX_ROOTS,sizeof(PhaseResults),maximum_evaluations,maximum_contour_evaluations);
    return 0;
}
