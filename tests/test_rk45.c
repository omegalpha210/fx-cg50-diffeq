#include "rk45.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static OdeStatus exponential(double x,const double *y,double *dy,void *ctx)
{(void)x;int n=ctx ? *(int*)ctx:1;for(int i=0;i<n;i++)dy[i]=y[i];return ODE_OK;}
static OdeStatus oscillator(double x,const double *y,double *dy,void *ctx)
{(void)x;(void)ctx;dy[0]=y[1];dy[1]=-y[0];return ODE_OK;}
static OdeStatus square(double x,const double *y,double *dy,void *ctx)
{(void)x;(void)ctx;dy[0]=y[0]*y[0];return ODE_OK;}
static OdeStatus bounded(double x,const double *y,double *dy,void *ctx)
{(void)ctx;dy[0]=sinh(x)*(y[0]*y[0]-1);return ODE_OK;}
static OdeStatus polynomial(double x,const double *y,double *dy,void *ctx)
{(void)y;(void)ctx;dy[0]=4*x*x*x;return ODE_OK;}
static OdeStatus domain(double x,const double *y,double *dy,void *ctx)
{(void)y;(void)ctx;if(x>=.4)return ODE_DOMAIN;dy[0]=1;return ODE_OK;}
static bool cancel_at(void *ctx){int *n=ctx;return --*n==0;}
static bool cancel(void *ctx){int *n=ctx;return ++*n>20;}
typedef struct {unsigned count;double previous,max_error;} Samples;
static bool samples(double x,const double *y,uint32_t ordinal,void *ctx)
{
    Samples *s=ctx;assert(ordinal==s->count++);assert(x>=s->previous);
    s->previous=x;s->max_error=fmax(s->max_error,fabs(y[0]-exp(x))/exp(x));return true;
}
int main(void)
{
    OdeSettings s={-10,50,1,20000,1,0};OdeAdaptive a={ODE_RK45,1e-6,1e-9};double y[9]={1};
    OdeDopri trial;OdeWork work={0};
    assert(ode_dopri_step(exponential,NULL,1,0,y,.2,&trial,&work,NULL,NULL)==ODE_OK);
    double coarse=fabs(trial.next[0]-exp(.2)),ec=fabs(trial.error[0]);assert(work.rhs==7);
    assert(ode_dopri_step(exponential,NULL,1,0,y,.1,&trial,&work,NULL,NULL)==ODE_OK);
    assert(coarse/fabs(trial.next[0]-exp(.1))>50 && ec/fabs(trial.error[0])>28);
    work=(OdeWork){0};assert(ode_dopri_step(polynomial,NULL,1,0,y,.7,&trial,&work,NULL,NULL)==ODE_OK);
    assert(fabs(trial.next[0]-(1+pow(.7,4)))<2e-15 && fabs(trial.error[0])<2e-16);
    double errors[3];
    for(int i=0;i<3;i++) {
        a.reltol=pow(10,-3-2*i);a.abstol=a.reltol*.001;work=(OdeWork){0};
        OdeResult r=ode_rk45_integrate(exponential,NULL,1,0,y,5,&s,&a,0,NULL,NULL,NULL,NULL,&work);
        assert(r.status==ODE_OK && r.x==5 && work.rhs==7*r.steps);
        errors[i]=fabs(r.y[0]/exp(5)-1);assert(errors[i]<a.reltol*3);
        printf("exp tol %.0e err %.9g accepted %u rejected %u RHS %u h[%.6g,%.6g]\n",a.reltol,errors[i],work.accepted,work.rejected,work.rhs,work.min_h,work.max_h);
    }
    assert(errors[2]<errors[1]/10 && errors[1]<errors[0]);
    a=(OdeAdaptive){ODE_RK45,1e-8,1e-11};s.h=10;work=(OdeWork){0};
    OdeResult r=ode_rk45_integrate(oscillator,NULL,2,0,y,40,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_OK && work.rejected>0 && work.max_h>2*work.min_h);
    assert(fabs(r.y[0]-cos(40))<2e-7 && fabs(r.y[1]+sin(40))<2e-7);
    assert(fabs(hypot(r.y[0],r.y[1])-1)<2e-7);
    work=(OdeWork){0};r=ode_rk45_integrate(exponential,NULL,1,0,y,-3.17,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_OK && r.x==-3.17 && fabs(r.y[0]-exp(-3.17))<2e-9);
    work=(OdeWork){0};Samples points={0};s.h=.5;
    r=ode_rk45_integrate(exponential,NULL,1,0,y,1.03,&s,&a,.1,samples,&points,NULL,NULL,&work);
    assert(r.status==ODE_OK && r.x==1.03 && points.count==12 && points.max_error<1e-8);
    a.reltol=1e-300;assert(ode_adaptive_validate(&a)==ODE_TOLERANCE);
    a.reltol=NAN;assert(ode_adaptive_validate(&a)==ODE_BAD_INPUT);
    a.reltol=1e-8;a.abstol=0;assert(ode_adaptive_validate(&a)==ODE_BAD_INPUT);a.abstol=1e-11;
    work=(OdeWork){0};s.h=1e-300;r=ode_rk45_integrate(exponential,NULL,1,1,y,2,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_STEP_UNDERFLOW && r.x==1 && r.y[0]==1);
    s.h=.1;s.max_steps=1;work=(OdeWork){0};
    r=ode_rk45_integrate(exponential,NULL,1,0,y,5,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_STEP_LIMIT && r.steps==1);
    s.max_steps=20000;work=(OdeWork){.rhs=ODE_RHS_BUDGET};
    r=ode_rk45_integrate(exponential,NULL,1,0,y,5,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_WORK_LIMIT && r.x==0);
    work=(OdeWork){.accepted=ODE_ATTEMPT_BUDGET};
    r=ode_rk45_integrate(exponential,NULL,1,0,y,5,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_WORK_LIMIT && r.steps==0 && work.rhs==0);
    work=(OdeWork){.work=ODE_RHS_WORK_BUDGET-1};int dim=9;
    r=ode_rk45_integrate(exponential,&dim,dim,0,y,5,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_WORK_LIMIT && r.x==0 && work.rhs==0);
    /* Every stage cancellation and the post-trial commit check retain the IC. */
    for(int at=3;at<=10;at++) {
        int poll=at;work=(OdeWork){0};
        r=ode_rk45_integrate(exponential,NULL,1,0,y,5,&s,&a,0,NULL,NULL,cancel_at,&poll,&work);
        assert(r.status==ODE_CANCELLED && r.x==0 && r.y[0]==1 && work.accepted==0);
    }
    work=(OdeWork){0};double endpoint=nextafter(1,2);
    r=ode_rk45_integrate(exponential,NULL,1,1,y,endpoint,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_OK && r.x==endpoint && work.accepted==1);
    s.h=10;work=(OdeWork){0};s.max_steps=1;
    r=ode_rk45_integrate(oscillator,NULL,2,0,y,40,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_STEP_LIMIT && r.x==0 && r.y[0]==1 && work.rejected==1);
    s.max_steps=20000;work=(OdeWork){0};
    r=ode_rk45_integrate(domain,NULL,1,.5,y,1,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_DOMAIN && r.x==.5 && work.rhs==1 && work.rejected==1);
    work=(OdeWork){0};
    r=ode_rk45_integrate(domain,NULL,1,0,y,1,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_DOMAIN && r.x<.4 && fabs(r.y[0]-1-r.x)<1e-14 && work.rejected>1);
    s.h=.1;
    work=(OdeWork){0};int polls=0;
    r=ode_rk45_integrate(exponential,NULL,1,0,y,5,&s,&a,0,NULL,NULL,cancel,&polls,&work);
    assert(r.status==ODE_CANCELLED && r.x<5 && fabs(r.y[0]-exp(r.x))<1e-7);
    work=(OdeWork){0};r=ode_rk45_integrate(square,NULL,1,0,y,2,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_STEP_UNDERFLOW && r.x<=1.0000001 && isfinite(r.y[0]));
    printf("singularity x %.17g status %s A%u R%u RHS%u\n",r.x,ode_status_text(r.status),work.accepted,work.rejected,work.rhs);
    y[0]=0;work=(OdeWork){0};r=ode_rk45_integrate(bounded,NULL,1,0,y,7,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_OK && fabs(r.y[0]+tanh(cosh(7)-1))<1e-7);
    int n=9;for(int i=0;i<n;i++)y[i]=(i+1)*1e-4;work=(OdeWork){0};
    r=ode_rk45_integrate(exponential,&n,n,0,y,3,&s,&a,0,NULL,NULL,NULL,NULL,&work);
    assert(r.status==ODE_OK);for(int i=0;i<n;i++)assert(fabs(r.y[i]-y[i]*exp(3))<2e-9);
    puts("RK45 core: embedded order, tolerance trend, rejected trials, output landing, backward, nine states, singular/domain progress and budgets passed.");
}
