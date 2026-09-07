#include "rk45.h"
#include <float.h>
#include <math.h>
#include <string.h>
/* Dormand-Prince 5(4), cross-checked against SciPy v1.16.2 RK45 and
   Boost.Odeint boost-1.89.0 runge_kutta_dopri5. Mathematical coefficients only;
   independent implementation. Full provenance: docs/RK45_NUMERICS.md. */
static const double c[7]={0,1./5,3./10,4./5,8./9,1,1};
static const double tableau[7][6]={
    {0}, {1./5}, {3./40,9./40}, {44./45,-56./15,32./9},
    {19372./6561,-25360./2187,64448./6561,-212./729},
    {9017./3168,-355./33,46732./5247,49./176,-5103./18656},
    {35./384,0,500./1113,125./192,-2187./6784,11./84}
};
/* b5-b4, accumulated directly to avoid subtracting two nearly equal y values. */
static const double error_weight[7]={71./57600,0,-71./16695,71./1920,
    -17253./339200,22./525,-1./40};
void ode_adaptive_defaults(OdeAdaptive *a)
{*a=(OdeAdaptive){ODE_RK4,1e-6,1e-9};}
OdeStatus ode_adaptive_validate(const OdeAdaptive *a)
{
    if(!a || (a->method!=ODE_RK4 && a->method!=ODE_RK45))return ODE_BAD_INPUT;
    if(!isfinite(a->reltol) || !isfinite(a->abstol) || a->reltol<=0 || a->abstol<=0)
        return ODE_BAD_INPUT;
    if(a->reltol<100*DBL_EPSILON || a->abstol<DBL_MIN)return ODE_TOLERANCE;
    return ODE_OK;
}
OdeStatus ode_dopri_step(OdeRhs rhs,void *ctx,int n,double x,const double *y,
    double h,OdeDopri *t,OdeWork *w,OdeCancel cancel,void *cancel_ctx)
{
    if(!rhs || !y || !t || !w || n<1 || n>ODE_MAX_DIM || !isfinite(x))return ODE_BAD_INPUT;
    if(!isfinite(h) || !h || x+h==x)return ODE_STEP_UNDERFLOW;
    OdeStatus status=ode_values_status(y,n);if(status!=ODE_OK)return status;
    for(int stage=0;stage<7;stage++) {
        t->stage=stage;
        if(cancel && cancel(cancel_ctx))return ODE_CANCELLED;
        if(w->rhs>=ODE_RHS_BUDGET || w->work>ODE_RHS_WORK_BUDGET-(unsigned)n)return ODE_WORK_LIMIT;
        for(int i=0;i<n;i++) {
            double sum=0;for(int j=0;j<stage;j++)sum+=tableau[stage][j]*t->k[j][i];
            t->next[i]=y[i]+h*sum;
        }
        status=ode_values_status(t->next,n);if(status!=ODE_OK)return status;
        double at=x+c[stage]*h;if(!isfinite(at))return ODE_NONFINITE;
        w->rhs++;w->work+=(unsigned)n;
        status=rhs(at,t->next,t->k[stage],ctx);if(status!=ODE_OK)return status;
        status=ode_values_status(t->k[stage],n);if(status!=ODE_OK)return status;
    }
    for(int i=0;i<n;i++) {
        double sum=0;for(int j=0;j<7;j++)sum+=error_weight[j]*t->k[j][i];
        t->error[i]=h*sum;
        if(!isfinite(t->error[i]))return ODE_NONFINITE;
    }
    return ODE_OK;
}
static double error_norm(const OdeAdaptive *a,int n,const double *old,const OdeDopri *t)
{
    double norm=0;
    for(int i=0;i<n;i++) {
        double magnitude=fmax(fabs(old[i]),fabs(t->next[i]));
        /* Divide the scale terms first if their sum/product would overflow. */
        double scale=a->abstol+a->reltol*magnitude;
        double e=isfinite(scale) ? fabs(t->error[i])/scale:
            (fabs(t->error[i])/a->reltol)/(magnitude+a->abstol/a->reltol);
        if(!isfinite(e))return INFINITY;
        norm=fmax(norm,e);
    }
    return norm;
}
OdeResult ode_rk45_control(OdeRhs rhs,void *ctx,int n,double x0,const double *y0,
    double target,const OdeSettings *s,const OdeAdaptive *a,double spacing,
    OdeSample sample,void *sample_ctx,OdeCancel cancel,void *cancel_ctx,OdeWork *work,
    OdeAccepted accepted,void *accepted_ctx)
{
    OdeResult r={.status=ODE_BAD_INPUT,.x=x0};OdeWork local={0};OdeWork *w=work ? work:&local;
    if(!rhs || !y0 || n<1 || n>ODE_MAX_DIM || !isfinite(x0) || !isfinite(target)
        || !isfinite(target-x0) || !isfinite(spacing) || spacing<0)return r;
    r.status=ode_validate(s);if(r.status!=ODE_OK)return r;
    r.status=ode_adaptive_validate(a);if(r.status!=ODE_OK)return r;
    r.status=ode_values_status(y0,n);if(r.status!=ODE_OK)return r;
    memcpy(r.y,y0,(unsigned)n*sizeof(double));
    if(cancel && cancel(cancel_ctx)){r.status=ODE_CANCELLED;return r;}
    if(accepted) {
        r.status=accepted(x0,r.y,&r.x,r.y,accepted_ctx);
        if(r.status!=ODE_OK && r.status!=ODE_EVENT_STOP)return r;
    }
    if(sample && x0>=s->xmin && x0<=s->xmax && !sample(x0,r.y,0,sample_ctx))
        {r.status=ODE_SAMPLE_STOP;return r;}
    if(r.status==ODE_EVENT_STOP)return r;
    double direction=target>=x0 ? 1:-1,span=fabs(target-x0);
    if(!span)return r;
    double max_h=fmin(span,fmax(span/8,10*fabs(nextafter(x0,target)-x0)));
    double h=fmin(s->h,max_h),output=target,last_output=x0;uint32_t ordinal=0;
    if(sample && spacing>0 && spacing<span)output=x0+direction*spacing;
    if(output==x0){r.status=ODE_STEP_UNDERFLOW;return r;}
    OdeDopri trial;bool rejected=false;OdeStatus invalid=ODE_OK;
    while(direction*(target-r.x)>0) {
        if(cancel && cancel(cancel_ctx)){r.status=ODE_CANCELLED;break;}
        if(r.steps>=s->max_steps){r.status=ODE_STEP_LIMIT;break;}
        if(w->accepted+w->rejected>=ODE_ATTEMPT_BUDGET){r.status=ODE_WORK_LIMIT;break;}
        double remaining=fabs(output-r.x);
        double minimum=10*fabs(nextafter(r.x,target)-r.x);
        double step=fmin(h,remaining);
        if(step<minimum && step<remaining) {r.status=invalid!=ODE_OK ? invalid:ODE_STEP_UNDERFLOW;break;}
        bool landing=step==remaining;double end=landing ? output:r.x+direction*step;
        double signed_h=end-r.x;step=fabs(signed_h);
        if(!step || !isfinite(end)){r.status=invalid!=ODE_OK ? invalid:ODE_STEP_UNDERFLOW;break;}
        r.steps++;
        OdeStatus status=ode_dopri_step(rhs,ctx,n,r.x,r.y,signed_h,&trial,w,cancel,cancel_ctx);
        if(status==ODE_CANCELLED || status==ODE_WORK_LIMIT){w->rejected++;r.status=status;break;}
        double error=status==ODE_OK ? error_norm(a,n,r.y,&trial):INFINITY;
        if(status!=ODE_OK && !ode_invalid_region(status)){w->rejected++;r.status=status;break;}
        if(status==ODE_OK && error<=1) {
            if(cancel && cancel(cancel_ctx)){w->rejected++;r.status=ODE_CANCELLED;break;}
            w->accepted++;
            if(!w->min_h || step<w->min_h)w->min_h=step;
            if(step>w->max_h)w->max_h=step;
            if(accepted) {
                r.status=accepted(r.x,r.y,&end,trial.next,accepted_ctx);
                if(r.status!=ODE_OK && r.status!=ODE_EVENT_STOP)break;
            }
            memcpy(r.y,trial.next,(unsigned)n*sizeof(double));r.x=end;
            if(r.status==ODE_EVENT_STOP)break;
            double factor=error==0 ? 5:fmin(5,fmax(.2,.9*pow(error,-.2)));
            if(rejected)factor=fmin(1,factor);
            h=fmin(max_h,step*factor);rejected=false;invalid=ODE_OK;
            if(landing) {
                ordinal++;last_output=r.x;
                if(sample && r.x>=s->xmin && r.x<=s->xmax && !sample(r.x,r.y,ordinal,sample_ctx))
                    {r.status=ODE_SAMPLE_STOP;break;}
                if(r.x==target)break;
                double next=x0+direction*(double)(ordinal+1)*spacing;
                output=direction*(target-next)<=0 ? target:next;
                if(direction*(output-r.x)<=0){r.status=ODE_STEP_UNDERFLOW;break;}
            }
        } else {
            w->rejected++;rejected=true;invalid=status;
            /* A bad RHS at the unchanged starting state cannot be repaired by h. */
            if(status!=ODE_OK && trial.stage==0){r.status=status;break;}
            h=step*(status!=ODE_OK || !isfinite(error) ? .2:fmax(.2,fmin(.9,.9*pow(error,-.2))));
            if(h<minimum || r.x+direction*h==r.x) {r.status=status!=ODE_OK ? status:ODE_STEP_UNDERFLOW;break;}
        }
    }
    if(sample && r.x!=last_output && r.x>=s->xmin && r.x<=s->xmax
        && r.status!=ODE_CANCELLED && r.status!=ODE_SAMPLE_STOP)sample(r.x,r.y,ordinal+1,sample_ctx);
    return r;
}
OdeResult ode_rk45_integrate(OdeRhs rhs,void *ctx,int n,double x0,const double *y0,
    double target,const OdeSettings *s,const OdeAdaptive *a,double spacing,
    OdeSample sample,void *sample_ctx,OdeCancel cancel,void *cancel_ctx,OdeWork *work)
{return ode_rk45_control(rhs,ctx,n,x0,y0,target,s,a,spacing,sample,sample_ctx,cancel,cancel_ctx,work,NULL,NULL);}
