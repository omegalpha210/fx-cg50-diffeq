#include "model.h"
#include <math.h>
#include <stddef.h>
typedef struct {
    OdeSample sample;void *context;int dim;bool connected;
    ModelPathResult result;
} Validity;
static bool visit(double x,const double *y,uint32_t step,void *context)
{
    Validity *v=context;
    OdeStatus status=!y ? ODE_DOMAIN:(!isfinite(x) ? ODE_NONFINITE:ode_values_status(y,v->dim));
    if(status!=ODE_OK) {
        if(v->result.invalid==ODE_OK)v->result.invalid=status;
        v->connected=false;
        return !v->sample || v->sample(x,NULL,step,v->context);
    }
    if(!v->connected)v->result.segments++;
    v->connected=true;v->result.points++;
    return !v->sample || v->sample(x,y,step,v->context);
}
ModelPathResult model_path_branch(const Document *d,CompiledModel *m,int family,int direction,
    const OdeSettings *range,OdeSample sample,void *context,OdeCancel cancel,void *cancel_ctx)
{
    Validity v={.sample=sample,.context=context,.dim=d->dim};
    if(sample && !sample(0,NULL,0,context))return (ModelPathResult){.status=ODE_SAMPLE_STOP};
    OdeResult r=model_trajectory_range(d,m,family,direction,range,visit,&v,cancel,cancel_ctx);
    v.result.steps=r.steps;v.result.status=r.status;
    if(ode_invalid_region(r.status)) {
        v.result.invalid=r.status;
        if(sample)sample(r.x,NULL,r.steps,context);
    }
    if(v.result.invalid!=ODE_OK && (r.status==ODE_OK || ode_invalid_region(r.status)))
        v.result.status=ODE_HAS_INVALID;
    return v.result;
}
