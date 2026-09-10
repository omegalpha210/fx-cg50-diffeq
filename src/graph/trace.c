#include "trace.h"
#include "ui.h"
#include "phase_graph.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
static TraceSamples samples;
/* During preparation overlays are restored, so their existing storage becomes
   the staging cache. No trajectory or framebuffer allocation is added. */
static union {
    struct {unsigned char bits[384*198/8];uint16_t rows[384*19];} overlay;
    TraceSamples staging;
    struct {uint16_t colors[384*198/8/2];uint16_t rows[384*19];} box;
} scratch;
_Static_assert(sizeof(TraceSamples)<=384*198/8+384*19*2,"TRACE staging exceeds overlay budget");
#define mask scratch.overlay.bits
#define footer scratch.overlay.rows
static bool inverted,mixed_highlight;
static int overlay_top=179;
static GraphPointPatch pointer;
static uint16_t highlight_xor;
static bool cache_ready;
static uint64_t cache_key,plot_key;
static bool plot_ready;
static TraceViewport viewport;
const TraceViewport *trace_viewport(void){return &viewport;}
static uint64_t hash_bytes(uint64_t hash,const void *data,size_t size)
{
    const unsigned char *bytes=data;
    for(size_t i=0;i<size;i++){hash^=bytes[i];hash*=UINT64_C(1099511628211);}
    return hash;
}
static uint64_t numerical_key(const Document *d)
{
    uint64_t hash=UINT64_C(14695981039346656037);
#define KEY(field) hash=hash_bytes(hash,&d->field,sizeof(d->field))
    KEY(event);KEY(adaptive.method);KEY(adaptive.reltol);KEY(adaptive.abstol);KEY(kind);KEY(dim);KEY(nic);KEY(text);KEY(power);KEY(ic);
    KEY(solver.xmin);KEY(solver.xmax);KEY(solver.h);KEY(solver.max_steps);KEY(solver.step);
#undef KEY
    return hash;
}
static uint64_t plot_identity(const Document *d)
{
    uint64_t hash=UINT64_C(14695981039346656037);
#define KEY(field) hash=hash_bytes(hash,&d->field,sizeof(d->field))
    KEY(event);KEY(adaptive);KEY(kind);KEY(dim);KEY(nic);KEY(text);KEY(power);KEY(ic);
    KEY(solver.h);KEY(solver.max_steps);KEY(solver.step);KEY(enabled);
#undef KEY
    return hash;
}
bool trace_plot_matches(const Document *d,double xmin,double xmax)
{
    return plot_ready && plot_key==plot_identity(d) && samples.extent.xmin<=xmin && samples.extent.xmax>=xmax;
}
bool trace_cache_matches(const Document *d)
{return d && model_phase_supported(d) && cache_ready && cache_key==numerical_key(d);}
void trace_cache_invalidate(void){cache_ready=false;plot_ready=false;}
GraphResult trace_cache_result(void){return samples.result;}
static void mask_pixel(int x,int y,int color)
{
    if(x<0 || x>=384 || y<0 || y>=198)return;
    if(gint_vram[(UI_Y+y)*DWIDTH+UI_X+x]!=(uint16_t)color)return;
    unsigned p=(unsigned)(y*384+x);mask[p/8]|=(unsigned char)(1u<<(p%8));
}
static void mask_line(int a,int b,int c,int e,int color)
{
    int dx=abs(c-a),sx=a<c ? 1:-1,dy=-abs(e-b),sy=b<e ? 1:-1,error=dx+dy;
    for(;;) {
        mask_pixel(a,b,color);if(a==c && b==e)break;
        int twice=2*error;if(twice>=dy){error+=dy;a+=sx;}if(twice<=dx){error+=dx;b+=sy;}
    }
}
static void mask_segment(const ViewWindow *v,double x0,double y0,double x1,double y1,int color)
{
    int a,b,c,e;
    if(!graph_clip(v,&x0,&y0,&x1,&y1) || !graph_point(v,x0,y0,&a,&b)
        || !graph_point(v,x1,y1,&c,&e))return;
    mask_line(a,b,c,e,color);
    if(abs(c-a)>=abs(e-b)) {int offset=b<197 && e<197 ? 1:-1;mask_line(a,b+offset,c,e+offset,color);}
    else {int offset=a<383 && c<383 ? 1:-1;mask_line(a+offset,b,c+offset,e,color);}
}
typedef struct {
    const Document *d;TraceSamples *out;TraceBranch *branch;
    unsigned stride,capacity;double target;
    bool have,linked;TracePoint previous;
} Capture;
static struct {
    Capture capture;uint64_t key;unsigned capacity,slot,finished,branches;int family;
    bool active,branch_active,failed;
} graph_capture;
static bool capture_family(const Document *d,int family)
{return model_phase_supported(d) || graph_family_enabled(d,family);}
static void phase_bound(TraceSamples *out,const double *y)
{
    if(ode_values_status(y,2)!=ODE_OK)return;
    for(int k=0;k<2;k++) {
        if(!out->phase_points || y[k]<out->phase_min[k])out->phase_min[k]=y[k];
        if(!out->phase_points || y[k]>out->phase_max[k])out->phase_max[k]=y[k];
    }
    out->phase_points++;
}
static void retain(Capture *c,const TracePoint *p)
{
    TraceBranch *b=c->branch;
    if(b->count && c->out->point[b->start+b->count-1].x==p->x)return;
    bool full=b->count>=c->capacity;
    unsigned i=b->start+(full ? c->capacity-1:b->count++);
    c->out->point[i]=*p;
    c->out->link[i]=(unsigned char)(c->linked && (!full || c->out->link[i]));
    c->linked=true;
}
static bool capture(double x,const double *y,uint32_t step,void *context)
{
    Capture *c=context;
    if(!y || !isfinite(x) || fabs(x)>1e100 || ode_values_status(y,c->d->dim)!=ODE_OK) {
        if(c->have)retain(c,&c->previous);
        c->have=false;c->linked=false;return true;
    }
    if(model_phase_supported(c->d))phase_bound(c->out,y);
    TracePoint p={.x=x};memcpy(p.y,y,(unsigned)c->d->dim*sizeof(double));
    if(!c->have || step%c->stride==0 || x==c->target)retain(c,&p);
    c->previous=p;c->have=true;return true;
}
static void record_result(TraceSamples *out,TraceBranch *branch,ModelPathResult result,int family)
{
    out->result.steps+=result.steps;
    if(result.invalid!=ODE_OK)out->result.invalid=result.invalid;
    if(result.status==ODE_HAS_INVALID){out->has_invalid=true;branch->invalid=true;}
    if(result.status==ODE_EVENT_STOP)branch->event=true;
    if(result.status!=ODE_OK && (out->result.status==ODE_OK || out->result.status==ODE_EVENT_STOP ||
        (out->result.status==ODE_HAS_INVALID && result.status!=ODE_HAS_INVALID && result.status!=ODE_EVENT_STOP))) {
        out->result.status=result.status;out->result.failed_family=family;
    }
}
bool trace_capture_begin(const Document *d)
{
    graph_capture.active=false;
    if(!d || d->nic<1 || (!model_phase_supported(d) && !d->enabled) ||
        model_preflight(d,&d->solver).status!=ODE_OK)return false;
    memset(&graph_capture,0,sizeof(graph_capture));
    memset(&scratch.staging,0,sizeof(scratch.staging));
    scratch.staging.extent=d->solver;scratch.staging.dim=d->dim;
    scratch.staging.result.failed_family=-1;
    graph_capture.capture.d=d;graph_capture.key=numerical_key(d);
    graph_capture.capacity=TRACE_POINTS/(2*(unsigned)d->nic);
    graph_capture.active=true;return true;
}
bool trace_capture_branch_begin(const Document *d,int family,int side)
{
    if(!graph_capture.active || graph_capture.branch_active || d!=graph_capture.capture.d ||
        family<0 || family>=d->nic || side<0 || side>1 || graph_capture.slot>=2*(unsigned)d->nic) {
        graph_capture.failed=true;return false;
    }
    TraceBranch *b=&scratch.staging.branch[family][side];
    unsigned bit=1u<<(2*family+side);
    if(graph_capture.branches&bit){graph_capture.failed=true;return false;}
    graph_capture.branches|=bit;graph_capture.family=family;
    b->start=graph_capture.slot++*graph_capture.capacity;
    double target=side ? d->solver.xmax:d->solver.xmin;
    double steps=ceil(fabs(target-d->ic[family].x)/model_output_spacing(d,&d->solver));
    unsigned stride=(unsigned)fmax(1,fmin(100000,ceil(steps/(graph_capture.capacity-1))));
    graph_capture.capture=(Capture){.d=d,.out=&scratch.staging,.branch=b,.stride=stride,
        .capacity=graph_capture.capacity,.target=target};
    graph_capture.branch_active=true;return true;
}
bool trace_capture_point(double x,const double *y,uint32_t step,void *unused)
{
    (void)unused;
    if(!graph_capture.active || !graph_capture.branch_active)return true;
    return capture(x,y,step,&graph_capture.capture);
}
void trace_capture_branch_end(ModelPathResult result)
{
    if(!graph_capture.active || !graph_capture.branch_active){graph_capture.failed=true;return;}
    Capture *c=&graph_capture.capture;
    if(c->have)retain(c,&c->previous);
    record_result(&scratch.staging,c->branch,result,graph_capture.family);
    if(result.status!=ODE_OK && result.status!=ODE_HAS_INVALID && result.status!=ODE_EVENT_STOP)graph_capture.failed=true;
    graph_capture.finished++;graph_capture.branch_active=false;
}
void trace_capture_end(bool success)
{
    if(graph_capture.active && success && !graph_capture.failed && !graph_capture.branch_active &&
        graph_capture.finished==2*(unsigned)graph_capture.capture.d->nic) {
        scratch.staging.valid=scratch.staging.branch[0][0].count+scratch.staging.branch[0][1].count>0;
        samples=scratch.staging;cache_key=graph_capture.key;cache_ready=true;
        plot_key=plot_identity(graph_capture.capture.d);plot_ready=true;
    }
    graph_capture.active=false;graph_capture.branch_active=false;
}
static OdeStatus prepare_range(const Document *d,CompiledModel *m,const OdeSettings *range,
    int family,int variable,TraceSamples *out,OdeCancel cancel,void *context,bool display)
{
    model_work_begin(m);
    ModelWork plan=model_preflight(d,range);if(plan.status!=ODE_OK)return plan.status;
    memset(out,0,sizeof(*out));out->extent=*range;out->family=family;out->variable=variable;out->dim=d->dim;
    out->result.failed_family=-1;
    unsigned active=0,slot=0;
    for(int f=0;f<d->nic;f++)if(capture_family(d,f))active++;
    if(!active)return ODE_BAD_INPUT;
    solver_report_stage(m);
    unsigned capacity=TRACE_POINTS/(2*active);
    for(int f=0;f<d->nic;f++)if(capture_family(d,f))for(int side=0;side<2;side++) {
        TraceBranch *b=&out->branch[f][side];b->start=slot++*capacity;
        double target=side ? range->xmax:range->xmin;
        double steps=ceil(fabs(target-d->ic[f].x)/model_output_spacing(d,range));
        /* preflight bounded every executed path before any integer cast */
        unsigned stride=(unsigned)fmax(1,fmin(100000,ceil(steps/(capacity-1))));
        Capture c={.d=d,.out=out,.branch=b,.stride=stride,.capacity=capacity,.target=target};
        ModelPathResult r=model_path_branch(d,m,f,side ? 1:-1,range,capture,&c,cancel,context);
        if(c.have)retain(&c,&c.previous);
        record_result(out,b,r,f);
        if(r.status==ODE_CANCELLED || (!display && r.status!=ODE_OK && r.status!=ODE_HAS_INVALID && r.status!=ODE_EVENT_STOP)){m->event_sink=NULL;return r.status;}
    }
    out->valid=out->branch[family][0].count+out->branch[family][1].count>0;
    if(!display)solver_report_commit(d,m,range,out->result.status==ODE_HAS_INVALID ? out->result.invalid:out->result.status);
    else m->event_sink=NULL;
    return ODE_OK;
}
static void selected_mask(const Document *d,int family,int selected_variable,bool append)
{
    if(!append)memset(mask,0,sizeof(mask));
    const ViewWindow *view=model_view_const(d);
    int variable=d->view.phase ? view->phase_y:selected_variable;
    int base=graph_palette_color(model_color(d,family,variable));
    highlight_xor=(uint16_t)(base^graph_highlight_color(base,true));
    for(int side=0;side<2;side++) {
        TraceBranch *b=&samples.branch[family][side];
        for(unsigned j=1;j<b->count;j++) {
            unsigned i=b->start+j;if(!samples.link[i])continue;
            TracePoint *p=&samples.point[i-1],*q=&samples.point[i];
            mask_segment(view,d->view.phase ? p->y[view->phase_x]:p->x,p->y[variable],
                d->view.phase ? q->y[view->phase_x]:q->x,q->y[variable],base);
        }
    }
    /* Only base-color pixels are masked at emission: cached approximations
       never introduce a synthetic line on white or disturb other colors. */
}

bool trace_prepare(Document *d,CompiledModel *m,int family,int variable)
{
    if(d->nic<1 || family<0 || family>=d->nic || variable<0 || variable>=d->dim)return false;
    const ViewWindow *v=model_view_const(d);
    viewport=(TraceViewport){v->xmin,v->xmax,v->xscale,model_xdot(&d->view),
        d->view.phase!=0,v->phase_x};
    if(trace_cache_matches(d)) {
        trace_overlay_begin();return trace_select(d,family,variable);
    }
    UiBusy busy;ui_busy_begin(&busy,"CALCULATING...",UI_BUSY_TRACE,ui_trace_cancel,NULL);
    OdeStatus status=prepare_range(d,m,&d->solver,family,variable,&scratch.staging,ui_busy_cancel,&busy,false);
    ui_busy_end(&busy);
    if(status==ODE_OK) {
        samples=scratch.staging;cache_ready=model_phase_supported(d);
        if(cache_ready)cache_key=numerical_key(d);
        plot_key=plot_identity(d);plot_ready=true;
    }
    trace_overlay_begin();
    if(status==ODE_OK)selected_mask(d,samples.family,samples.variable,false);else memset(mask,0,sizeof(mask));
    return status==ODE_OK && samples.valid;
}
OdeStatus graph_plot_prepare(Document *d,CompiledModel *m,OdeCancel cancel,void *context)
{
    if(trace_plot_matches(d,d->solver.xmin,d->solver.xmax))return ODE_OK;
    OdeStatus status=prepare_range(d,m,&d->solver,0,0,&scratch.staging,cancel,context,true);
    if(status==ODE_OK) {
        samples=scratch.staging;plot_key=plot_identity(d);plot_ready=true;
        /* Display prefixes are never promoted to a canonical SYS2 solve cache. */
        cache_ready=false;
    }
    return status;
}
/* At least1420 pixels remain after TraceSamples in the existing scratch union.
   This area is not used by staging; overlays are inactive during busy work. */
uint16_t *graph_busy_pixels(unsigned *capacity,bool staging)
{
    if(!staging){*capacity=sizeof(footer)/sizeof(uint16_t);return footer;}
    *capacity=(sizeof(scratch)-sizeof(TraceSamples))/sizeof(uint16_t);
    return (uint16_t *)((unsigned char *)&scratch+sizeof(TraceSamples));
}
_Static_assert(sizeof(scratch)-sizeof(TraceSamples)>=2800,"Busy patch exceeds inactive staging tail");
const OdeSettings *trace_extent(void){return &samples.extent;}
bool trace_select(const Document *d,int family,int variable)
{
    if(family<0 || family>=d->nic || variable<0 || variable>=d->dim)return false;
    samples.family=family;samples.variable=variable;
    samples.valid=samples.branch[family][0].count+samples.branch[family][1].count>0;
    selected_mask(d,samples.family,samples.variable,false);return samples.valid;
}
bool trace_has_invalid(void)
{return samples.branch[samples.family][0].invalid || samples.branch[samples.family][1].invalid;}
bool trace_direction_invalid(int direction)
{return samples.branch[samples.family][direction>0 ? 1:0].invalid;}
static bool in_view(const TracePoint *p)
{
    double x=viewport.phase ? p->y[viewport.axis]:p->x;
    return isfinite(x) && x>=viewport.xmin && x<=viewport.xmax;
}
/* PHASE never projects an offscreen sample onto a fake horizontal edge.
   Only a run of retained, linked, in-view states is traversable. */
static bool phase_sample(const TracePoint *p){return !viewport.phase || in_view(p);}
static bool interpolate(double x,TracePoint *point)
{
    for(int side=0;side<2;side++) {
        TraceBranch *b=&samples.branch[samples.family][side];
        for(unsigned j=0;j<b->count;j++) {
            unsigned i=b->start+j;TracePoint *q=&samples.point[i];
            if(x==q->x && in_view(q)){*point=*q;return true;}
            if(!j || !samples.link[i])continue;
            TracePoint *p=&samples.point[i-1];
            if(!phase_sample(p) || !phase_sample(q) || (!viewport.phase &&
                (x<viewport.xmin || x>viewport.xmax)))continue;
            if(x<fmin(p->x,q->x) || x>fmax(p->x,q->x) || p->x==q->x)continue;
            double t=(x-p->x)/(q->x-p->x);point->x=x;
            for(int k=0;k<samples.dim;k++)point->y[k]=(1-t)*p->y[k]+t*q->y[k];
            return ode_values_status(point->y,samples.dim)==ODE_OK;
        }
    }
    return false;
}
bool trace_point_near(double x,TracePoint *point)
{
    if(!samples.valid)return false;
    if(!isfinite(x))return false;
    if(!viewport.phase)x=fmax(viewport.xmin,fmin(viewport.xmax,x));
    if(interpolate(x,point))return true;
    double best=INFINITY;
    for(int side=0;side<2;side++) {
        TraceBranch *b=&samples.branch[samples.family][side];
        for(unsigned j=0;j<b->count;j++) {
            TracePoint *p=&samples.point[b->start+j];if(!in_view(p))continue;
            double distance=fabs(p->x-x);
            if(distance<best){best=distance;*point=*p;}
        }
    }
    return isfinite(best);
}
bool trace_move(double x,int direction,TracePoint *point)
{
    if(!samples.valid)return false;
    double best=INFINITY;
    for(int side=0;side<2;side++) {
        TraceBranch *b=&samples.branch[samples.family][side];
        for(unsigned j=0;j<b->count;j++) {
            TracePoint *p=&samples.point[b->start+j];if(!in_view(p))continue;
            double distance=direction*(p->x-x);
            if(distance>1e-12*fmax(1,fabs(x)) && distance<best){best=distance;*point=*p;}
        }
    }
    return isfinite(best);
}
bool trace_step(double x,int direction,double dx,TracePoint *point)
{
    if(!samples.valid || !isfinite(dx) || dx<=0)return false;
    double target=x+direction*dx;
    if(!isfinite(x) || !isfinite(target) || target==x)return false;
    if(interpolate(target,point))return true;
    return trace_move(x,direction,point);
}
void trace_cache_render(const Document *d)
{
    const ViewWindow *view=model_view_const(d);
    for(int f=0;f<d->nic;f++)if(graph_family_enabled(d,f))for(int side=0;side<2;side++) {
        TraceBranch *b=&samples.branch[f][side];
        for(unsigned j=1;j<b->count;j++) {
            unsigned i=b->start+j;if(!samples.link[i])continue;
            TracePoint *p=&samples.point[i-1],*q=&samples.point[i];
            if(d->view.phase) {
                graph_solution_segment(view,p->y[view->phase_x],p->y[view->phase_y],
                    q->y[view->phase_x],q->y[view->phase_y],
                    graph_palette_color(model_color(d,f,view->phase_y)));
            } else for(int k=0;k<d->dim;k++)if(d->enabled&(1u<<k))
                graph_solution_segment(view,p->x,p->y[k],q->x,q->y[k],graph_palette_color(model_color(d,f,k)));
        }
    }
}
bool trace_cache_phase_window(const Document *d,ViewWindow *window)
{
    if(!window || !trace_cache_matches(d) || samples.phase_points<2)return false;
    ViewWindow next=*model_view_const(d);
    double lower[2],upper[2],scale[2];
    for(int k=0;k<2;k++) {
        double span=samples.phase_max[k]-samples.phase_min[k];
        double margin=span>0 ? span*.12:fmax(1,fabs(samples.phase_min[k])*.1);
        lower[k]=samples.phase_min[k]-margin;upper[k]=samples.phase_max[k]+margin;
        if(!isfinite(lower[k]) || !isfinite(upper[k]) || lower[k]>=upper[k])return false;
        scale[k]=pow(10,floor(log10((upper[k]-lower[k])/6)));
        if(!isfinite(scale[k]) || scale[k]<=0)scale[k]=1;
    }
    next.xmin=lower[0];next.xmax=upper[0];next.xscale=scale[0];
    next.ymin=lower[1];next.ymax=upper[1];next.yscale=scale[1];
    next.phase=1;next.phase_x=0;next.phase_y=1;*window=next;return true;
}
static void time_bound(double y,double *minimum,double *maximum)
{
    if(!isfinite(y) || fabs(y)>1e100)return;
    if(y<*minimum)*minimum=y;
    if(y>*maximum)*maximum=y;
}
bool trace_cache_time_window(const Document *d,ViewWindow *window)
{
    if(!window || !trace_cache_matches(d))return false;
    ViewWindow next=d->view;
    double minimum=INFINITY,maximum=-INFINITY;
    for(int f=0;f<d->nic;f++)for(int side=0;side<2;side++) {
        const TraceBranch *branch=&samples.branch[f][side];
        for(unsigned j=0;j<branch->count;j++) {
            unsigned i=branch->start+j;const TracePoint *q=&samples.point[i];
            for(int k=0;k<d->dim;k++)if(d->enabled&(1u<<k)) {
                if(q->x>=next.xmin && q->x<=next.xmax)time_bound(q->y[k],&minimum,&maximum);
                if(!j || !samples.link[i])continue;
                const TracePoint *p=&samples.point[i-1];
                if(p->x==q->x)continue;
                for(int edge=0;edge<2;edge++) {
                    double x=edge ? next.xmax:next.xmin;
                    if(x<fmin(p->x,q->x) || x>fmax(p->x,q->x))continue;
                    double t=(x-p->x)/(q->x-p->x);
                    time_bound((1-t)*p->y[k]+t*q->y[k],&minimum,&maximum);
                }
            }
        }
    }
    if(!isfinite(minimum) || !isfinite(maximum))return false;
    double margin=maximum>minimum ? (maximum-minimum)*.12:fmax(1,fabs(minimum)*.1);
    next.ymin=minimum-margin;next.ymax=maximum+margin;
    if(!isfinite(next.ymin) || !isfinite(next.ymax) || next.ymin>=next.ymax)return false;
    next.yscale=pow(10,floor(log10((next.ymax-next.ymin)/6)));
    if(!isfinite(next.yscale) || next.yscale<=0)next.yscale=1;
    *window=next;return true;
}
static void cached_render(Document *d,CompiledModel *m)
{
    graph_backdrop(d,m);trace_cache_render(d);graph_event_markers(d);graph_phase_markers(d,-1);
    graph_labels(d,m);graph_status(samples.result);
}
static OdeStatus follow_point(Document *d,CompiledModel *m,const TracePoint *point,bool redraw)
{
    ViewWindow *view=model_view(d),before=*view,next=*view;
    double x=d->view.phase ? point->y[view->phase_x]:point->x;
    double y=point->y[d->view.phase ? view->phase_y:samples.variable];
    if(!isfinite(point->x) || fabs(point->x)>1e100 ||
        ode_values_status(point->y,samples.dim)!=ODE_OK || !in_view(point))return ODE_HAS_INVALID;
    if(graph_follow_y(&next,y)){*view=next;redraw=true;}
    int px,py;
    if(!graph_point(view,x,y,&px,&py)){*view=before;return ODE_NONFINITE;}
    /* Configured solver range remains independent of runtime cache and view. */
    if(redraw) {
        if(model_phase_supported(d) && d->view.phase) {
            OdeStatus status=graph_phase_preflight(d,m,ui_trace_cancel,NULL);
            if(status!=ODE_OK){*view=before;return status;}
        }
        cached_render(d,m);trace_overlay_begin();selected_mask(d,samples.family,samples.variable,false);
    }
    return ODE_OK;
}
void trace_follow(Document *d,CompiledModel *m,const TracePoint *point)
{(void)follow_point(d,m,point,false);}
/* Grow the connected component containing x from linked cache runs. Two
   branches can share their initial point. Three scans cover either branch order;
   no array, numerical query or extension transaction is needed. */
static bool visible_component(double x,double *low,double *high)
{
    *low=x;*high=x;bool found=false;
    for(int pass=0;pass<3;pass++)for(int side=0;side<2;side++) {
        const TraceBranch *b=&samples.branch[samples.family][side];
        for(unsigned j=0;j<b->count;) {
            unsigned begin=j++,end=begin;
            if(!phase_sample(&samples.point[b->start+begin]))continue;
            while(j<b->count && samples.link[b->start+j] &&
                phase_sample(&samples.point[b->start+j]))end=j++;
            double a=samples.point[b->start+begin].x,z=samples.point[b->start+end].x;
            double left=fmin(a,z),right=fmax(a,z);
            if(!viewport.phase){left=fmax(left,viewport.xmin);right=fmin(right,viewport.xmax);}
            if(left<=right && left<=*high && right>=*low) {
                *low=fmin(*low,left);*high=fmax(*high,right);found=true;
            }
        }
    }
    return found;
}
OdeStatus trace_navigate(Document *d,CompiledModel *m,double target,bool jump,TracePoint *point)
{
    (void)jump; /* Endpoints obey exactly the same connected, visible limits. */
    if(!samples.valid || !isfinite(target) || fabs(target)>1e100 || !in_view(point))return ODE_BAD_INPUT;
    double low,high;if(!visible_component(point->x,&low,&high))return ODE_HAS_INVALID;
    double limited=fmax(low,fmin(high,target));TracePoint next=*point;
    if(!interpolate(limited,&next))return ODE_HAS_INVALID;
    OdeStatus followed=follow_point(d,m,&next,false);
    if(followed!=ODE_OK)return followed;
    *point=next;
    if(limited!=target)for(int side=0;side<2;side++) {
        const TraceBranch *b=&samples.branch[samples.family][side];
        if(b->count && next.x==samples.point[b->start+b->count-1].x) {
            if(b->event)return ODE_EVENT_STOP;
            if(b->invalid)return ODE_HAS_INVALID;
        }
    }
    return ODE_OK;
}

static void invert_mask(void)
{
    for(unsigned p=0;p<384*198;p++)if(mask[p/8]&(1u<<(p%8))) {
        uint16_t *pixel=&gint_vram[(UI_Y+p/384)*DWIDTH+UI_X+p%384];
        *pixel^=mixed_highlight ? (*pixel==C_BLACK || *pixel==C_BLUE ? C_BLUE:0xffff):highlight_xor;
    }
}

void graph_overlay_begin(int top)
{
    overlay_top=top;mixed_highlight=false;
    for(int y=0;y<19;y++)memcpy(footer+y*384,gint_vram+(UI_Y+overlay_top+y)*DWIDTH+UI_X,384*sizeof(uint16_t));
    inverted=false;pointer.active=false;
}
void trace_overlay_begin(void){graph_overlay_begin(179);}
void graph_overlay_restore(void){trace_overlay_restore();}
void graph_overlay_point(int x,int y){graph_point_cross(x,y,&pointer);}
void graph_overlay_curves(const Document *d,int family,int variable,int other_family,int other_variable,bool highlighted)
{
    if(!highlighted)return;
    selected_mask(d,family,variable,false);
    if(other_family>=0)selected_mask(d,other_family,other_variable,true);
    mixed_highlight=true;invert_mask();inverted=true;
}
void graph_overlay_curve(const Document *d,int family,int variable,bool highlighted)
{graph_overlay_curves(d,family,variable,-1,0,highlighted);}

void trace_overlay_restore(void)
{
    graph_point_restore(&pointer);
    if(inverted)invert_mask();
    for(int y=0;y<19;y++)memcpy(gint_vram+(UI_Y+overlay_top+y)*DWIDTH+UI_X,footer+y*384,384*sizeof(uint16_t));
    inverted=false;pointer.active=false;
}
void trace_overlay_show(const Document *d,const TracePoint *point,int variable,bool highlight)
{
    if(highlight){invert_mask();inverted=true;}
    const ViewWindow *view=model_view_const(d);int x,y;
    if(graph_point(view,d->view.phase ? point->y[view->phase_x]:point->x,
        point->y[d->view.phase ? view->phase_y:variable],&x,&y))graph_point_cross(x,y,&pointer);
}

/* BOX borrows the inactive TRACE mask/footer. At most4512 color samples; no
   allocation or second framebuffer. A4px stipple keeps underlying curves visible. */
static struct {int left,right,top,bottom;bool shown;} box;
static bool box_pixel(int x,int y)
{
    return y>=35 && y<179 && (x==box.left || x==box.right || y==box.top || y==box.bottom
        || (x%4==0 && y%4==0));
}
void trace_box_restore(void)
{
    graph_point_restore(&pointer);
    unsigned i=0;
    if(box.shown)for(int y=box.top;y<=box.bottom;y++)for(int x=box.left;x<=box.right;x++)
        if(box_pixel(x,y))gint_vram[(UI_Y+y)*DWIDTH+UI_X+x]=scratch.box.colors[i++];
    box.shown=false;
    for(int y=0;y<19;y++)memcpy(gint_vram+(UI_Y+overlay_top+y)*DWIDTH+UI_X,footer+y*384,384*sizeof(uint16_t));
}
void trace_box_show(int x1,int y1,int x2,int y2,bool rectangle)
{
    box.left=x1<x2 ? x1:x2;box.right=x1>x2 ? x1:x2;
    box.top=y1<y2 ? y1:y2;box.bottom=y1>y2 ? y1:y2;box.shown=rectangle;
    unsigned i=0;
    if(rectangle)for(int y=box.top;y<=box.bottom;y++)for(int x=box.left;x<=box.right;x++) {
        if(!box_pixel(x,y))continue;
        unsigned p=(unsigned)((UI_Y+y)*DWIDTH+UI_X+x);
        scratch.box.colors[i++]=gint_vram[p];
        gint_vram[p]=x==box.left || x==box.right || y==box.top || y==box.bottom ? UI_BLUE:C_RGB(21,25,30);
    }
    graph_point_cross(x2,y2,&pointer);
}
