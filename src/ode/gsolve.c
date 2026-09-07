#include "gsolve.h"
#include <math.h>
#include <string.h>

typedef struct {
    const Document *d;
    CompiledModel *m;
    GsolveCurve curve,other;
    GsolveMode mode;
    double target,xmin,xmax,x_tolerance;
    GsolveResults *result;
    OdeCancel cancel;
    void *cancel_ctx;
    bool intersection,have_previous,in_zero_run,other_ready;
    double previous_x,previous_value,other_x,other_y[ODE_MAX_DIM];
    OdeStatus failure,other_failure;
    int other_failure_direction;
} Search;

int gsolve_curve_count(const Document *d)
{
    if(!d || d->view.phase)return 0;
    int count=0;
    for(int family=0;family<d->nic;family++)
        for(int variable=0;variable<d->dim;variable++)
            if(d->enabled&(1u<<variable))count++;
    return count;
}
bool gsolve_curve_at(const Document *d,int ordinal,GsolveCurve *curve)
{
    if(!curve || ordinal<0)return false;
    for(int family=0;d && !d->view.phase && family<d->nic;family++)
        for(int variable=0;variable<d->dim;variable++)
            if(d->enabled&(1u<<variable)) {
                if(ordinal--==0){*curve=(GsolveCurve){family,variable};return true;}
            }
    return false;
}
static bool same_curve(GsolveCurve a,GsolveCurve b)
{ return a.family==b.family && a.variable==b.variable; }
static bool valid_curve(const Document *d,GsolveCurve curve)
{
    return d && !d->view.phase && curve.family>=0 && curve.family<d->nic
        && curve.variable>=0 && curve.variable<d->dim
        && (d->enabled&(1u<<curve.variable));
}
static bool search_domain(const Document *d,double *xmin,double *xmax)
{
    *xmin=fmax(d->solver.xmin,d->view.xmin);
    *xmax=fmin(d->solver.xmax,d->view.xmax);
    return isfinite(*xmin) && isfinite(*xmax) && *xmin<*xmax;
}
static OdeStatus curve_at(const Document *d,CompiledModel *m,GsolveCurve curve,double x,
    OdeResult *result,uint32_t *steps,OdeCancel cancel,void *cancel_ctx)
{
    *result=model_value_at(d,m,curve.family,x,cancel,cancel_ctx);
    if(steps)*steps+=result->steps;
    if(result->status==ODE_EVENT_STOP && result->x==x)return ODE_OK;
    return result->status;
}
static OdeStatus metric_at(Search *s,double x,double *value,GsolvePoint *point)
{
    OdeResult first;OdeStatus status=curve_at(s->d,s->m,s->curve,x,&first,
        &s->result->steps,s->cancel,s->cancel_ctx);
    if(status!=ODE_OK)return status;
    double metric=first.y[s->curve.variable]-s->target;
    double y=first.y[s->curve.variable];
    if(s->intersection) {
        OdeResult second;status=curve_at(s->d,s->m,s->other,x,&second,
            &s->result->steps,s->cancel,s->cancel_ctx);
        if(status!=ODE_OK)return status;
        metric=first.y[s->curve.variable]-second.y[s->other.variable];
        y=(first.y[s->curve.variable]+second.y[s->other.variable])*.5;
    } else if(s->mode==GSOLVE_MAXIMUM || s->mode==GSOLVE_MINIMUM) {
        double derivative[ODE_MAX_DIM];status=model_rhs(first.x,first.y,derivative,s->m);
        if(status!=ODE_OK)return status;
        metric=derivative[s->curve.variable];
    }
    if(!isfinite(metric) || !isfinite(y))return ODE_NONFINITE;
    if(value)*value=metric;
    if(point)*point=(GsolvePoint){first.x,y};
    return ODE_OK;
}
OdeStatus gsolve_ycal(const Document *d,CompiledModel *m,GsolveCurve curve,double x,
    GsolvePoint *point,OdeCancel cancel,void *cancel_ctx)
{
    double xmin,xmax;
    if(!m || !point || !valid_curve(d,curve) || !isfinite(x)
        || !search_domain(d,&xmin,&xmax) || x<xmin || x>xmax)return ODE_BAD_INPUT;
    model_work_begin(m);
    OdeResult result;OdeStatus status=curve_at(d,m,curve,x,&result,NULL,cancel,cancel_ctx);
    if(status==ODE_OK)*point=(GsolvePoint){result.x,result.y[curve.variable]};
    return status;
}
static void add_result(Search *s,GsolvePoint point)
{
    if(point.x<s->xmin || point.x>s->xmax || !isfinite(point.x) || !isfinite(point.y))return;
    GsolveResults *r=s->result;
    int at=0;
    while(at<r->count && r->point[at].x<point.x)at++;
    if((at<r->count && fabs(point.x-r->point[at].x)<=s->x_tolerance*4)
        || (at>0 && fabs(point.x-r->point[at-1].x)<=s->x_tolerance*4))return;
    if(at>=GSOLVE_MAX_RESULTS)return;
    if(r->count<GSOLVE_MAX_RESULTS)r->count++;
    for(int i=r->count-1;i>at;i--)r->point[i]=r->point[i-1];
    r->point[at]=point;
}
static bool wanted_change(const Search *s,double left,double right)
{
    if(!s->intersection && s->mode==GSOLVE_MAXIMUM)return left>0 && right<0;
    if(!s->intersection && s->mode==GSOLVE_MINIMUM)return left<0 && right>0;
    return (left<0 && right>0)||(left>0 && right<0);
}
static bool refine(Search *s,double left,double right,double left_value,GsolvePoint *point)
{
    for(int i=0;i<48 && right-left>s->x_tolerance;i++) {
        if(s->cancel && s->cancel(s->cancel_ctx)){s->failure=ODE_CANCELLED;return false;}
        double middle=left+(right-left)*.5,value;
        OdeStatus status=metric_at(s,middle,&value,NULL);
        if(status!=ODE_OK){s->failure=status;return false;}
        if(value==0){left=right=middle;break;}
        if((left_value<0 && value>0)||(left_value>0 && value<0))right=middle;
        else {left=middle;left_value=value;}
    }
    OdeStatus status=metric_at(s,left+(right-left)*.5,NULL,point);
    if(status!=ODE_OK){s->failure=status;return false;}
    return true;
}
static OdeStatus other_step(Search *s,double x)
{
    if(!s->intersection)return ODE_OK;
    if(s->other_failure!=ODE_OK) {
        if(s->other_failure_direction*(x-s->other_x)>0)return s->other_failure;
        /* We have reached the accepted side of the secondary IC's boundary.
           Query from its original IC; never seed from the failed RK4 state. */
        s->other_failure=ODE_OK;s->other_ready=false;
    }
    OdeResult result;
    s->m->event_family=s->other.family;
    if(!s->other_ready) {
        result=model_value_at(s->d,s->m,s->other.family,x,s->cancel,s->cancel_ctx);
    } else if(x==s->other_x)return ODE_OK;
    else result=model_integrate(s->d,s->m,s->other_x,s->other_y,x,
        &s->d->solver,NULL,NULL,s->cancel,s->cancel_ctx);
    s->result->steps+=result.steps;
    if(result.status==ODE_OK) {
        s->other_ready=true;s->other_x=result.x;
        memcpy(s->other_y,result.y,(unsigned)s->d->dim*sizeof(double));
    }
    if(ode_invalid_region(result.status) || result.status==ODE_EVENT_STOP) {
        s->other_failure=result.status;s->other_failure_direction=x>=result.x ? 1:-1;
        s->other_x=result.x;s->other_ready=false;
    }
    return result.status;
}
static bool scan_failure(Search *s,OdeStatus status)
{
    s->have_previous=false;s->in_zero_run=false;
    if(status==ODE_EVENT_STOP){s->failure=ODE_OK;return true;}
    if(ode_invalid_region(status)) {s->result->has_invalid=true;s->failure=ODE_OK;return true;}
    s->failure=status;return false;
}

static bool scan_point(double x,const double *y,uint32_t step,void *ctx)
{
    (void)step;Search *s=ctx;
    if(!y){s->have_previous=false;s->in_zero_run=false;s->other_ready=false;
        s->other_failure=ODE_OK;return true;}
    if(x<s->xmin || x>s->xmax)return true;
    double primary=y[s->curve.variable],value=primary-s->target,result_y=primary;
    if(s->intersection) {
        OdeStatus status=other_step(s,x);
        if(status!=ODE_OK)return scan_failure(s,status);
        double secondary=s->other_y[s->other.variable];
        value=primary-secondary;result_y=(primary+secondary)*.5;
    } else if(s->mode==GSOLVE_MAXIMUM || s->mode==GSOLVE_MINIMUM) {
        double derivative[ODE_MAX_DIM];OdeStatus status=model_rhs(x,y,derivative,s->m);
        if(status!=ODE_OK)return scan_failure(s,status);
        value=derivative[s->curve.variable];
    }
    if(!isfinite(value) || !isfinite(result_y))return scan_failure(s,ODE_NONFINITE);
    if(value==0 && !s->in_zero_run) {
        if(s->intersection || (s->mode!=GSOLVE_MAXIMUM && s->mode!=GSOLVE_MINIMUM))
            add_result(s,(GsolvePoint){x,result_y});
        else {
            /* A branch can start exactly at an extremum (notably the IC).
               Check both trusted sides; a stationary inflection is not an extremum. */
            double left,right,delta=(s->d->adaptive.method==ODE_RK45 ? model_output_spacing(s->d,&s->d->solver):s->d->solver.h)*.5;
            if(x-delta>=s->xmin && x+delta<=s->xmax) {
                OdeStatus a=metric_at(s,x-delta,&left,NULL),b=metric_at(s,x+delta,&right,NULL);
                if(a==ODE_OK && b==ODE_OK && wanted_change(s,left,right))
                    add_result(s,(GsolvePoint){x,result_y});
                else if(a!=ODE_OK || b!=ODE_OK) {
                    OdeStatus failure=a!=ODE_OK ? a:b;
                    if(!scan_failure(s,failure))return false;
                }
            }
        }
    }
    s->in_zero_run=value==0;
    bool forward=x>s->previous_x;
    if(s->have_previous && wanted_change(s,forward ? s->previous_value:value,
        forward ? value:s->previous_value)) {
        GsolvePoint point;
        if(!refine(s,forward ? s->previous_x:x,forward ? x:s->previous_x,
            forward ? s->previous_value:value,&point))return scan_failure(s,s->failure);
        add_result(s,point);
    }
    if(value!=0){s->have_previous=true;s->previous_x=x;s->previous_value=value;}
    return true;
}

static GsolveResults search(const Document *d,CompiledModel *m,GsolveCurve curve,
    GsolveCurve other,bool intersection,GsolveMode mode,double target,
    OdeCancel cancel,void *cancel_ctx)
{
    if(m)model_work_begin(m);
    GsolveResults result={.status=ODE_BAD_INPUT};double xmin,xmax;
    if(!m || !valid_curve(d,curve) || (intersection && (!valid_curve(d,other)
        || same_curve(curve,other))) || (!intersection && (mode<GSOLVE_ROOT
        || mode>GSOLVE_XCAL)) || !isfinite(target) || !search_domain(d,&xmin,&xmax))return result;
    Search s={.d=d,.m=m,.curve=curve,.other=other,.mode=mode,.target=target,
        .xmin=xmin,.xmax=xmax,.x_tolerance=fmax((xmax-xmin)*1e-10,1e-12),
        .result=&result,.cancel=cancel,.cancel_ctx=cancel_ctx,
        .intersection=intersection,.failure=ODE_OK};
    OdeSettings range=d->solver;range.xmin=xmin;range.xmax=xmax;
    result.status=ODE_OK;
    for(int direction=-1;direction<=1;direction+=2) {
        ModelPathResult scan=model_path_branch(d,m,curve.family,direction,&range,
            scan_point,&s,cancel,cancel_ctx);
        result.steps+=scan.steps;
        if(scan.status==ODE_HAS_INVALID)result.has_invalid=true;
        else if(scan.status!=ODE_OK && scan.status!=ODE_SAMPLE_STOP && scan.status!=ODE_EVENT_STOP)result.status=scan.status;
        if(s.failure!=ODE_OK)result.status=s.failure;
        if(result.status!=ODE_OK)break;
    }
    return result;
}
GsolveResults gsolve_search(const Document *d,CompiledModel *m,GsolveCurve curve,
    GsolveMode mode,double target,OdeCancel cancel,void *cancel_ctx)
{
    return search(d,m,curve,(GsolveCurve){0,0},false,mode,target,cancel,cancel_ctx);
}
GsolveResults gsolve_intersections(const Document *d,CompiledModel *m,GsolveCurve first,
    GsolveCurve second,OdeCancel cancel,void *cancel_ctx)
{
    return search(d,m,first,second,true,GSOLVE_ROOT,0,cancel,cancel_ctx);
}
