#include "model.h"
#include <float.h>
#include <math.h>
#include <string.h>

/* Primary reference: SciPy v1.16.2 ivp.py accepted-step event bracketing.
   Unlike its dense interpolation, refinements here use target landing with
   the selected solver. Direction is explicitly increasing-x in both loops. */
static SolverReport report;
/* TRACE prepares transactionally; failed scratch work preserves visible marks. */
static EventMarkers staged_markers;
const SolverReport *solver_report(void){return &report;}
void solver_report_reset(void){memset(&report,0,sizeof(report));}
void solver_report_begin(const Document *d,CompiledModel *m)
{
    solver_report_reset();model_work_begin(m);report.valid=true;
    report.method=d->adaptive.method;report.dim=d->dim;report.nic=d->nic;
    report.xmin=d->solver.xmin;report.xmax=d->solver.xmax;report.h=d->solver.h;
    report.reltol=d->adaptive.reltol;report.abstol=d->adaptive.abstol;
    report.enabled=d->event.enabled;report.direction=d->event.direction;report.action=d->event.action;
    m->event_sink=&report.markers;
}
void solver_report_end(CompiledModel *m,OdeStatus status)
{
    report.work=m->work;report.status=status;report.hits=m->event_hits;
    report.evaluations=m->event_evals;report.unavailable=m->event_unavailable;m->event_sink=NULL;
}
void solver_report_stage(CompiledModel *m)
{memset(&staged_markers,0,sizeof(staged_markers));m->event_sink=&staged_markers;}
void solver_report_commit(const Document *d,CompiledModel *m,const OdeSettings *range,OdeStatus status)
{
    report.valid=true;report.method=d->adaptive.method;report.dim=d->dim;report.nic=d->nic;
    report.xmin=range->xmin;report.xmax=range->xmax;report.h=d->solver.h;
    report.reltol=d->adaptive.reltol;report.abstol=d->adaptive.abstol;
    report.enabled=d->event.enabled;report.direction=d->event.direction;report.action=d->event.action;
    report.markers=staged_markers;solver_report_end(m,status);
}
ExprScope model_event_scope(const Document *d)
{return (ExprScope){d->dim,d->kind==EQ_SECOND || d->kind==EQ_HIGHER,true,true};}
ExprError model_event_compile(const Document *d,CompiledModel *m)
{
    if(!d->event.enabled)return (ExprError){EXPR_OK,0};
    ExprError error=expr_compile(d->event.text,model_event_scope(d),&m->event_program);
    if(error.status!=EXPR_OK)return error;
    for(int f=0;f<d->nic;f++) {
        double value;error.status=expr_eval(&m->event_program,d->ic[f].x,d->ic[f].y,d->dim,&value);
        if(error.status==EXPR_OK && (!isfinite(value) || fabs(value)>1e100))error.status=EXPR_NONFINITE;
        if(error.status!=EXPR_OK)return error;
    }
    return error;
}
typedef struct {
    const Document *d;CompiledModel *m;const OdeSettings *range;
    OdeCancel cancel;void *cancel_ctx;int family;
    double initial_x,previous,last_root;const double *initial_y;
    bool have,armed,pending,triggered,initial_resolved,initial_match;
} EventPath;
static double x_tolerance(double a,double b)
{return 64*DBL_EPSILON*fmax(fmax(fabs(a),fabs(b)),fmax(fabs(a-b),DBL_MIN));}
static bool opposite(double a,double b)
{return (a<0 && b>0) || (a>0 && b<0);}
static OdeResult integrate_plain(const Document *d,CompiledModel *m,double x,const double *y,
    double target,const OdeSettings *range,OdeSample sample,void *sample_ctx,
    OdeCancel cancel,void *cancel_ctx,OdeAccepted accepted,void *accepted_ctx)
{
    if(d->adaptive.method==ODE_RK45)return ode_rk45_control(model_rhs,m,d->dim,x,y,target,
        range,&d->adaptive,sample ? model_output_spacing(d,range):0,sample,sample_ctx,cancel,cancel_ctx,
        &m->work,accepted,accepted_ctx);
    return ode_integrate_control(model_rhs,m,d->dim,x,y,target,range,sample,sample_ctx,cancel,cancel_ctx,
        &m->work,accepted,accepted_ctx);
}
static OdeStatus event_value(EventPath *p,double x,const double *y,double *value)
{
    if(p->cancel && p->cancel(p->cancel_ctx))return ODE_CANCELLED;
    if(p->m->event_evals>=EVENT_EVAL_BUDGET)return ODE_WORK_LIMIT;
    p->m->event_evals++;
    ExprStatus status=expr_eval(&p->m->event_program,x,y,p->d->dim,value);
    if(status!=EXPR_OK || !isfinite(*value) || fabs(*value)>1e100)return ODE_DOMAIN;
    return ODE_OK;
}
/* A directed STOP at an initial zero may need several accepted steps to learn
   the direction. Probe without publishing samples so a zero plateau cannot
   leave visible points beyond an eventual STOP at the original IC. */
static bool probe_sample(double x,const double *y,uint32_t step,void *context)
{(void)x;(void)y;(void)step;(void)context;return true;}
static OdeStatus initial_probe(double from,const double *old,double *at,double *next,void *context)
{
    (void)old;EventPath *p=context;if(from==*at)return ODE_OK;
    double value;OdeStatus status=event_value(p,*at,next,&value);
    if(status==ODE_CANCELLED || status==ODE_WORK_LIMIT)return status;
    if(status!=ODE_OK)return ODE_SAMPLE_STOP;
    if(value==0)return ODE_OK;
    int direction=(value>0)==(*at>from) ? EVENT_RISING:EVENT_FALLING;
    p->initial_match=p->d->event.direction==direction;
    return ODE_SAMPLE_STOP;
}
static void record_event(EventPath *p,double x,const double *y,int direction)
{
    p->triggered=true;p->last_root=x;
    if(x==p->d->ic[p->family].x) {
        unsigned bit=1u<<(unsigned)p->family;
        if(p->m->initial_events&bit)return;
        p->m->initial_events|=bit;
    }
    p->m->event_hits++;
    EventMarkers *out=p->m->event_sink;
    if(!out || out->count==EVENT_CAPACITY)return;
    EventMarker *point=&out->point[out->count++];point->x=x;point->family=p->family;point->direction=direction;
    memcpy(point->y,y,(unsigned)p->d->dim*sizeof(double));
}
/* Successful candidates satisfy residual/progress checks; a sign-changing
   discontinuity with non-shrinking residual is not silently labeled a root. */
static OdeStatus refine(EventPath *p,double reference,const double *old,double end,const double *next,
    double fa,double fb,EventMarker *root,bool *found)
{
    *found=false;
    if(fb==0){root->x=end;memcpy(root->y,next,(unsigned)p->d->dim*sizeof(double));*found=true;return ODE_OK;}
    double left=reference,right=end,scale=fmax(fabs(fa),fabs(fb));
    double xtol=x_tolerance(left,right);
    for(int iteration=0;iteration<EVENT_ITERATIONS;iteration++) {
        double fraction=fabs(fa)/(fabs(fa)+fabs(fb));
        if(fraction<.1 || fraction>.9 || iteration%3==2)fraction=.5;
        double at=left+(right-left)*fraction;
        if(at==left || at==right)break;
        OdeResult r=integrate_plain(p->d,p->m,reference,old,at,p->range,NULL,NULL,p->cancel,p->cancel_ctx,NULL,NULL);
        if(r.status!=ODE_OK)return r.status;
        double value;OdeStatus status=event_value(p,at,r.y,&value);if(status!=ODE_OK)return status;
        double residual=fabs(value)/scale;
        if(value==0 || (residual<=128*DBL_EPSILON && fabs(right-left)<=sqrt(DBL_EPSILON)*fmax(fabs(reference-end),fabs(at)))
            || (fabs(right-left)<=xtol && residual<=sqrt(DBL_EPSILON))) {
            root->x=at;memcpy(root->y,r.y,(unsigned)p->d->dim*sizeof(double));*found=true;return ODE_OK;
        }
        if(opposite(fa,value)){right=at;fb=value;}else {left=at;fa=value;}
    }
    return ODE_OK;
}
static OdeStatus accepted_event(double from,const double *old,double *at,double *next,void *context)
{
    EventPath *p=context;double value;OdeStatus status=event_value(p,*at,next,&value);
    if(status==ODE_CANCELLED || status==ODE_WORK_LIMIT)return status;
    if(status!=ODE_OK){p->have=false;p->pending=false;p->m->event_unavailable++;return ODE_OK;}
    if(from==*at) {
        p->have=true;p->previous=value;p->armed=value!=0;p->pending=value==0 && !p->initial_resolved;
        if(value==0 && p->d->event.direction==EVENT_ANY) {
            record_event(p,*at,next,EVENT_ANY);p->pending=false;
            return p->d->event.action==EVENT_STOP ? ODE_EVENT_STOP:ODE_OK;
        }
        return ODE_OK;
    }
    double before=p->previous;p->previous=value;
    if(!p->have){p->have=true;p->armed=value!=0;return ODE_OK;}
    int direction=(value>before)==(*at>from) ? EVENT_RISING:EVENT_FALLING;
    bool match=p->d->event.direction==EVENT_ANY || p->d->event.direction==direction;
    if(p->pending) {
        if(value==0)return ODE_OK;
        p->pending=false;p->armed=true;
        if(match) {
            record_event(p,p->initial_x,p->initial_y,direction);
            if(p->d->event.action==EVENT_STOP){*at=p->initial_x;memcpy(next,p->initial_y,(unsigned)p->d->dim*sizeof(double));return ODE_EVENT_STOP;}
        }
        return ODE_OK;
    }
    if(!p->armed){if(value!=0)p->armed=true;return ODE_OK;}
    if(!match || (!opposite(before,value) && value!=0))return ODE_OK;
    EventMarker root;bool found;
    status=refine(p,from,old,*at,next,before,value,&root,&found);
    if(status==ODE_CANCELLED || status==ODE_WORK_LIMIT)return status;
    if(status!=ODE_OK || !found){p->m->event_unavailable++;return ODE_OK;}
    if(p->triggered && fabs(root.x-p->last_root)<=x_tolerance(root.x,p->last_root))return ODE_OK;
    record_event(p,root.x,root.y,direction);p->armed=value!=0;
    if(p->d->event.action==EVENT_STOP) {
        *at=root.x;memcpy(next,root.y,(unsigned)p->d->dim*sizeof(double));return ODE_EVENT_STOP;
    }
    return ODE_OK;
}
OdeResult model_integrate(const Document *d,CompiledModel *m,double x,const double *y,
    double target,const OdeSettings *range,OdeSample sample,void *sample_ctx,OdeCancel cancel,void *cancel_ctx)
{
    if(!d->event.enabled)return integrate_plain(d,m,x,y,target,range,sample,sample_ctx,cancel,cancel_ctx,NULL,NULL);
    EventPath path={.d=d,.m=m,.range=range,.cancel=cancel,.cancel_ctx=cancel_ctx,
        .family=m->event_family,.initial_x=x,.initial_y=y};
    if(d->event.action==EVENT_STOP && d->event.direction!=EVENT_ANY) {
        double value;OdeStatus status=event_value(&path,x,y,&value);
        if(status==ODE_OK && value==0) {
            OdeResult probe=integrate_plain(d,m,x,y,target,range,sample ? probe_sample:NULL,NULL,
                cancel,cancel_ctx,initial_probe,&path);
            status=probe.status;path.initial_resolved=true;
        }
        if(path.initial_match || status==ODE_CANCELLED || status==ODE_WORK_LIMIT) {
            OdeResult r={.status=status,.x=x};memcpy(r.y,y,(unsigned)d->dim*sizeof(double));
            if(path.initial_match) {
                record_event(&path,x,y,d->event.direction);r.status=ODE_EVENT_STOP;
                if(sample && x>=range->xmin && x<=range->xmax && !sample(x,y,0,sample_ctx))r.status=ODE_SAMPLE_STOP;
            }
            return r;
        }
    }
    return integrate_plain(d,m,x,y,target,range,sample,sample_ctx,cancel,cancel_ctx,accepted_event,&path);
}
