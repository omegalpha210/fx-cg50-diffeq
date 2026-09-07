#include "model.h"
#include <string.h>
OdeResult model_value_at(const Document *d,CompiledModel *m,int family,double x,
    OdeCancel cancel,void *cancel_ctx)
{
    if(family<0 || family>=d->nic)return (OdeResult){.status=ODE_BAD_INPUT};
    const InitialCondition *ic=&d->ic[family];
    return model_integrate(d,m,ic->x,ic->y,x,&d->solver,NULL,NULL,cancel,cancel_ctx);
}
OdeResult model_trajectory(const Document *d,CompiledModel *m,int family,int direction,
    OdeSample sample,void *sample_ctx,OdeCancel cancel,void *cancel_ctx)
{
    return model_trajectory_range(d,m,family,direction,&d->solver,sample,sample_ctx,cancel,cancel_ctx);
}
OdeResult model_trajectory_range(const Document *d,CompiledModel *m,int family,int direction,
    const OdeSettings *range,OdeSample sample,void *sample_ctx,OdeCancel cancel,void *cancel_ctx)
{
    OdeResult r={.status=ODE_BAD_INPUT};
    if(family<0 || family>=d->nic || (direction!=-1 && direction!=1)) return r;
    const InitialCondition *ic=&d->ic[family];
    if((direction>0 && ic->x>range->xmax) || (direction<0 && ic->x<range->xmin)) {
        r.status=ODE_OK;r.x=ic->x;memcpy(r.y,ic->y,sizeof(r.y));return r;
    }
    return model_integrate(d,m,ic->x,ic->y,
        direction>0 ? range->xmax:range->xmin,range,sample,sample_ctx,cancel,cancel_ctx);
}
