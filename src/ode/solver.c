#include "model.h"
#include <string.h>
#include <math.h>
void model_work_begin(CompiledModel *m){memset(&m->work,0,sizeof(m->work));}
double model_output_spacing(const Document *d,const OdeSettings *range)
{
    return d->adaptive.method==ODE_RK45 ? fmax(model_xdot(&d->view),(range->xmax-range->xmin)/1024):range->h;
}
OdeResult model_integrate(const Document *d,CompiledModel *m,double x,const double *y,
    double target,const OdeSettings *range,OdeSample sample,void *sample_ctx,OdeCancel cancel,void *cancel_ctx)
{
    if(d->adaptive.method==ODE_RK45)return ode_rk45_integrate(model_rhs,m,d->dim,x,y,target,
        range,&d->adaptive,sample ? model_output_spacing(d,range):0,sample,sample_ctx,cancel,cancel_ctx,&m->work);
    return ode_integrate(model_rhs,m,d->dim,x,y,target,range,sample,sample_ctx,cancel,cancel_ctx);
}
