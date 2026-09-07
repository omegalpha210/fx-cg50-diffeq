#include "ode.h"
#include <math.h>
#include <string.h>

static OdeStatus bounded(const double *v, int n)
{
    for(int i=0; i<n; i++) {
        if(!isfinite(v[i])) return ODE_NONFINITE;
        if(fabs(v[i]) > 1e100) return ODE_DIVERGED;
    }
    return ODE_OK;
}

OdeStatus ode_rk4(OdeRhs rhs, void *ctx, int n, double x,
    const double *y, double h, double *next)
{
    if(!rhs || !y || !next || n<1 || n>ODE_MAX_DIM || !isfinite(x))
        return ODE_BAD_INPUT;
    if(!isfinite(h) || h==0 || x+h==x) return ODE_BAD_STEP;
    double k[4][ODE_MAX_DIM], temp[ODE_MAX_DIM], result[ODE_MAX_DIM];
    OdeStatus status=bounded(y,n);
    if(status!=ODE_OK) return status;
    for(int stage=0; stage<4; stage++) {
        double offset=stage==3 ? h : (stage==0 ? 0 : h*0.5);
        for(int i=0; i<n; i++)
            temp[i]=y[i]+(stage ? offset*k[stage-1][i] : 0);
        status=bounded(temp,n);
        if(status!=ODE_OK) return status;
        if(!isfinite(x+offset)) return ODE_NONFINITE;
        status=rhs(x+offset,temp,k[stage],ctx);
        if(status!=ODE_OK) return status;
        status=bounded(k[stage],n);
        if(status!=ODE_OK) return status;
    }
    for(int i=0; i<n; i++)
        result[i]=y[i]+h*(k[0][i]+2*k[1][i]+2*k[2][i]+k[3][i])/6;
    status=bounded(result,n);
    if(status==ODE_OK) memcpy(next,result,(unsigned)n*sizeof(double));
    return status;
}
