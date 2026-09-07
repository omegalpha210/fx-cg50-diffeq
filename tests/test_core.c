#include "ode.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static OdeStatus exponential(double x,const double *y,double *dy,void *ctx)
{ (void)x; dy[0]=*(double*)ctx*y[0]; return ODE_OK; }
static OdeStatus oscillator(double x,const double *y,double *dy,void *ctx)
{ (void)x; (void)ctx; dy[0]=y[1]; dy[1]=-y[0]; return ODE_OK; }
static OdeStatus logistic(double x,const double *y,double *dy,void *ctx)
{ (void)x; (void)ctx; dy[0]=y[0]*(1-y[0]); return ODE_OK; }
static OdeStatus bad(double x,const double *y,double *dy,void *ctx)
{ (void)x; (void)y; (void)ctx; dy[0]=NAN; return ODE_OK; }
static bool cancel(void *ctx) { (void)ctx; return true; }
static bool count(double x,const double *y,uint32_t step,void *ctx)
{ (void)x;(void)y;(void)step; (*(int*)ctx)++; return true; }

int main(void)
{
    OdeSettings s={.xmin=-8,.xmax=8,.h=.1,.max_steps=10000,.step=1,.sf=12};
    double y[]={1,0}, rate=1, errors[3];
    for(int i=0;i<3;i++) {
        OdeResult r=ode_integrate(exponential,&rate,1,0,y,1,&s,NULL,NULL,NULL,NULL);
        assert(r.status==ODE_OK && r.x==1);
        errors[i]=fabs(r.y[0]-exp(1));
        printf("exp(1), h=%.5f: %.15g, abs error=%.9g\n",s.h,r.y[0],errors[i]);
        s.h*=.5;
    }
    assert(errors[0]/errors[1]>14 && errors[1]/errors[2]>14);
    s.h=.01;
    OdeResult r=ode_integrate(exponential,&rate,1,0,y,-1,&s,NULL,NULL,NULL,NULL);
    assert(r.status==ODE_OK && fabs(r.y[0]-exp(-1))<1e-9);
    rate=-1;
    r=ode_integrate(exponential,&rate,1,0,y,1,&s,NULL,NULL,NULL,NULL);
    assert(r.status==ODE_OK && fabs(r.y[0]-exp(-1))<1e-9);
    r=ode_integrate(oscillator,NULL,2,0,y,6.283185307179586,&s,NULL,NULL,NULL,NULL);
    assert(r.status==ODE_OK && fabs(r.y[0]-1)<1e-8 && fabs(r.y[1])<1e-8);
    printf("oscillator after 2*pi: (%.12g, %.12g)\n",r.y[0],r.y[1]);
    y[0]=.2;
    r=ode_integrate(logistic,NULL,1,0,y,3,&s,NULL,NULL,NULL,NULL);
    assert(r.status==ODE_OK && fabs(r.y[0]-1/(1+4*exp(-3)))<1e-9);
    printf("logistic x=3: %.12g\n",r.y[0]);
    s.h=.1;
    r=ode_integrate(logistic,NULL,1,0,y,5,&s,NULL,NULL,NULL,NULL);
    assert(r.status==ODE_OK && r.steps==50 && r.x==5);
    s.h=.3; int samples=0;
    r=ode_integrate(logistic,NULL,1,0,y,1,&s,count,&samples,NULL,NULL);
    assert(r.status==ODE_OK && r.x==1 && r.steps==4 && samples==5);
    s.max_steps=2;
    r=ode_integrate(logistic,NULL,1,0,y,1,&s,NULL,NULL,NULL,NULL);
    assert(r.status==ODE_STEP_LIMIT && r.steps==2);
    s.max_steps=100;
    r=ode_integrate(bad,NULL,1,0,y,1,&s,NULL,NULL,NULL,NULL);
    assert(r.status==ODE_NONFINITE && r.steps==0 && r.y[0]==y[0]);
    r=ode_integrate(logistic,NULL,1,0,y,1,&s,NULL,NULL,cancel,NULL);
    assert(r.status==ODE_CANCELLED && r.steps==0);
    s.h=1e-15; assert(ode_validate(&s)==ODE_BAD_STEP);
    s.h=.1; s.xmax=s.xmin; assert(ode_validate(&s)==ODE_BAD_INPUT);
    puts("All solver checks passed.");
    return 0;
}
