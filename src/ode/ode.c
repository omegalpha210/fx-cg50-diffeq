#include "ode.h"
#include <math.h>
#include <string.h>
#include <float.h>

OdeStatus ode_validate(const OdeSettings *s)
{
    if(!s || !isfinite(s->xmin) || !isfinite(s->xmax) || s->xmin>=s->xmax
        || !isfinite(s->xmax-s->xmin) || s->max_steps<1 || s->max_steps>100000
        || s->step<1 || s->step>10000 || s->sf<0 || s->sf>ODE_SF_MAX)
        return ODE_BAD_INPUT;
    if(!isfinite(s->h) || s->h<=0) return ODE_BAD_STEP;
    return ODE_OK;
}

typedef struct {OdeRhs rhs;void *ctx;OdeWork *work;int n;} CountedRhs;
static OdeStatus counted_rhs(double x,const double *y,double *dy,void *context)
{
    CountedRhs *c=context;
    if(c->work->rhs>=ODE_RHS_BUDGET || c->work->work>ODE_RHS_WORK_BUDGET-(unsigned)c->n)
        return ODE_WORK_LIMIT;
    c->work->rhs++;c->work->work+=(unsigned)c->n;
    return c->rhs(x,y,dy,c->ctx);
}
OdeResult ode_integrate_control(OdeRhs rhs, void *ctx, int n, double x0,
    const double *y0, double target, const OdeSettings *s,
    OdeSample sample, void *sample_ctx, OdeCancel cancel, void *cancel_ctx,
    OdeWork *work,OdeAccepted accepted,void *accepted_ctx)
{
    OdeResult r={.status=ODE_BAD_INPUT,.x=x0};
    if(!rhs || !y0 || n<1 || n>ODE_MAX_DIM || !isfinite(x0)
        || !isfinite(target)) return r;
    r.status=ode_validate(s);
    if(r.status!=ODE_OK) return r;
    for(int i=0; i<n; i++) {
        if(!isfinite(y0[i])) { r.status=ODE_NONFINITE; return r; }
        if(fabs(y0[i])>1e100) { r.status=ODE_DIVERGED; return r; }
        r.y[i]=y0[i];
    }
    if(cancel && cancel(cancel_ctx)) { r.status=ODE_CANCELLED; return r; }
    if(accepted) {
        r.status=accepted(x0,r.y,&r.x,r.y,accepted_ctx);
        if(r.status!=ODE_OK && r.status!=ODE_EVENT_STOP)return r;
    }
    if(sample && x0>=s->xmin && x0<=s->xmax
        && !sample(x0,r.y,0,sample_ctx)) {
        r.status=ODE_SAMPLE_STOP; return r;
    }
    if(r.status==ODE_EVENT_STOP)return r;
    CountedRhs counter={rhs,ctx,work,n};
    double direction=target>=x0 ? 1 : -1;
    while(direction*(target-r.x)>0) {
        if(cancel && cancel(cancel_ctx)) { r.status=ODE_CANCELLED; break; }
        if(r.steps>=s->max_steps) { r.status=ODE_STEP_LIMIT; break; }
        if(work && work->accepted+work->rejected>=ODE_ATTEMPT_BUDGET){r.status=ODE_WORK_LIMIT;break;}
        double remaining=fabs(target-r.x);
        if(r.x+direction*s->h==r.x) { r.status=ODE_BAD_STEP; break; }
        double rounding=8*DBL_EPSILON*fmax(fabs(target),fabs(r.x));
        bool last=remaining <= s->h+rounding;
        double h=direction*(last ? remaining : s->h);
        if(r.x+h==r.x) { r.status=ODE_BAD_STEP; break; }
        double next[ODE_MAX_DIM];
        r.status=ode_rk4(work ? counted_rhs:rhs,work ? &counter:ctx,n,r.x,r.y,h,next);
        if(r.status!=ODE_OK){if(work)work->rejected++;break;}
        if(work) {
            work->accepted++;
            if(!work->min_h || fabs(h)<work->min_h)work->min_h=fabs(h);
            work->max_h=fmax(work->max_h,fabs(h));
        }
        double at=last ? target:r.x+h;
        if(accepted) {
            r.status=accepted(r.x,r.y,&at,next,accepted_ctx);
            if(r.status!=ODE_OK && r.status!=ODE_EVENT_STOP)break;
        }
        r.x=at;
        memcpy(r.y,next,(unsigned)n*sizeof(double));
        r.steps++;
        if(sample && r.x>=s->xmin && r.x<=s->xmax
            && !sample(r.x,r.y,r.status==ODE_EVENT_STOP ? 0:r.steps,sample_ctx)) {
            r.status=ODE_SAMPLE_STOP; break;
        }
        if(r.status==ODE_EVENT_STOP)break;
    }
    return r;
}
OdeResult ode_integrate(OdeRhs rhs,void *ctx,int n,double x0,const double *y0,
    double target,const OdeSettings *s,OdeSample sample,void *sample_ctx,OdeCancel cancel,void *cancel_ctx)
{return ode_integrate_control(rhs,ctx,n,x0,y0,target,s,sample,sample_ctx,cancel,cancel_ctx,NULL,NULL,NULL);}

const char *ode_status_text(OdeStatus s)
{
    static const char *const text[]={"Complete","Cancelled","Page complete",
        "Invalid values/range","h too small for x","Step limit reached",
        "NaN or infinity","Magnitude > 1e100","Math domain / singularity","Storage I/O error",
        "Valid regions / gaps","Total calculation too large","Step underflow","Tolerance too small","Event"};
    return (unsigned)s<sizeof(text)/sizeof(text[0]) ? text[s] : "Unknown error";
}
bool ode_invalid_region(OdeStatus status)
{return status==ODE_NONFINITE || status==ODE_DIVERGED || status==ODE_DOMAIN || status==ODE_STEP_UNDERFLOW;}
OdeStatus ode_values_status(const double *values,int count)
{
    for(int i=0;i<count;i++) {
        if(!isfinite(values[i]))return ODE_NONFINITE;
        if(fabs(values[i])>1e100)return ODE_DIVERGED;
    }
    return ODE_OK;
}
