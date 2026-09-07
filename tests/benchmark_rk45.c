#include "rk45.h"
#include <math.h>
#include <stdio.h>
#include <time.h>
typedef struct {int equation;unsigned rhs;} Case;
static OdeStatus rhs(double x,const double *y,double *dy,void *ctx)
{
    Case *c=ctx;c->rhs++;
    if(c->equation==0)dy[0]=y[0];
    else if(c->equation==1 || c->equation==4){dy[0]=y[1];dy[1]=-y[0];}
    else if(c->equation==2)dy[0]=sinh(x)*(y[0]*y[0]-1);
    else dy[0]=y[0]*y[0];
    return ODE_OK;
}
int main(void)
{
    const char *names[]={"exp","oscillator-2nd","bounded-sinh","singular-square","harmonic-SYS2"};
    for(int i=0;i<5;i++)for(int method=0;method<2;method++) {
        Case c={i,0};int n=i==1 || i==4 ? 2:1;
        double y[9]={i==2 ? 0:1},target=i==0 ? 5:(i==1 || i==4 ? 40:(i==2 ? 7:2));
        OdeSettings s={-target,target,.1,20000,1,0};OdeAdaptive a={ODE_RK45,1e-6,1e-9};
        OdeWork w={0};clock_t start=clock();
        OdeResult r=method ? ode_rk45_integrate(rhs,&c,n,0,y,target,&s,&a,0,NULL,NULL,NULL,NULL,&w):
            ode_integrate(rhs,&c,n,0,y,target,&s,NULL,NULL,NULL,NULL);
        double ms=1000.*(clock()-start)/CLOCKS_PER_SEC,error=NAN,amplitude=NAN,phase=NAN;
        if(i==0)error=fabs(r.y[0]/exp(r.x)-1);
        if(i==1 || i==4){error=fmax(fabs(r.y[0]-cos(r.x)),fabs(r.y[1]+sin(r.x)));amplitude=fabs(hypot(r.y[0],r.y[1])-1);phase=fabs(remainder(atan2(-r.y[1],r.y[0])-r.x,2*acos(-1)));}
        if(i==2)error=fabs(r.y[0]+tanh(cosh(r.x)-1));
        if(i==3 && r.x<1-1e-10)error=fabs(r.y[0]-1/(1-r.x));
        printf("%s %s h=0.1 rel=%g abs=%g x=%.17g error=%.8g amplitude=%.8g phase=%.8g accepted=%u rejected=%u rhs=%u hmin=%g hmax=%g status=%s host_ms=%.4f\n",
            names[i],method ? "RK45":"RK4",method ? a.reltol:0,method ? a.abstol:0,r.x,error,amplitude,phase,
            method ? w.accepted:r.steps,method ? w.rejected:0,c.rhs,method ? w.min_h:NAN,method ? w.max_h:NAN,ode_status_text(r.status),ms);
    }
    puts("error: exp relative, oscillators max state absolute, sinh absolute; nan means not applicable (including the exact pole). No output-grid landing/raster in this core benchmark. HARDWARE TEST REQUIRED.");
}
