#include "trace.h"
#include "ui.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
static TraceSamples samples;
/* During extension overlays are restored, so their existing storage becomes
   the staging cache. No trajectory or framebuffer allocation is added. */
static union {
    struct {unsigned char bits[384*198/8];uint16_t rows[384*19];} overlay;
    TraceSamples staging;
} scratch;
_Static_assert(sizeof(TraceSamples)<=384*198/8+384*19*2,"TRACE staging exceeds overlay budget");
#define mask scratch.overlay.bits
#define footer scratch.overlay.rows
static bool inverted,pointer;
static int pointer_x,pointer_y;
static uint16_t highlight_xor;
static void mask_pixel(int x,int y)
{
    if(x<0 || x>=384 || y<0 || y>=198)return;
    unsigned p=(unsigned)(y*384+x);mask[p/8]|=(unsigned char)(1u<<(p%8));
}
static void mask_line(int a,int b,int c,int e)
{
    int dx=abs(c-a),sx=a<c ? 1:-1,dy=-abs(e-b),sy=b<e ? 1:-1,error=dx+dy;
    for(;;) {
        mask_pixel(a,b);if(a==c && b==e)break;
        int twice=2*error;if(twice>=dy){error+=dy;a+=sx;}if(twice<=dx){error+=dx;b+=sy;}
    }
}
static void mask_segment(const ViewWindow *v,double x0,double y0,double x1,double y1)
{
    int a,b,c,e;
    if(!graph_clip(v,&x0,&y0,&x1,&y1) || !graph_point(v,x0,y0,&a,&b)
        || !graph_point(v,x1,y1,&c,&e))return;
    mask_line(a,b,c,e);
    if(abs(c-a)>=abs(e-b)) {int offset=b<197 && e<197 ? 1:-1;mask_line(a,b+offset,c,e+offset);}
    else {int offset=a<383 && c<383 ? 1:-1;mask_line(a+offset,b,c+offset,e);}
}
typedef struct {
    const Document *d;TraceSamples *out;TraceBranch *branch;
    unsigned stride,capacity;double target;
    bool have,linked;TracePoint previous;
} Capture;
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
    if(!y) {
        if(c->have)retain(c,&c->previous);
        c->have=false;c->linked=false;return true;
    }
    TracePoint p={.x=x};memcpy(p.y,y,(unsigned)c->d->dim*sizeof(double));
    if(!c->have || step%c->stride==0 || x==c->target)retain(c,&p);
    c->previous=p;c->have=true;return true;
}
static OdeStatus prepare_range(const Document *d,CompiledModel *m,const OdeSettings *range,
    int family,int variable,TraceSamples *out)
{
    ModelWork plan=model_preflight(d,range);if(plan.status!=ODE_OK)return plan.status;
    memset(out,0,sizeof(*out));out->extent=*range;out->family=family;out->variable=variable;out->dim=d->dim;
    unsigned active=0,slot=0;
    for(int f=0;f<d->nic;f++)if(graph_family_enabled(d,f))active++;
    if(!active)return ODE_BAD_INPUT;
    unsigned capacity=TRACE_POINTS/(2*active);
    for(int f=0;f<d->nic;f++)if(graph_family_enabled(d,f))for(int side=0;side<2;side++) {
        TraceBranch *b=&out->branch[f][side];b->start=slot++*capacity;
        double target=side ? range->xmax:range->xmin;
        double steps=ceil(fabs(target-d->ic[f].x)/range->h);
        /* preflight bounded every executed path before any integer cast */
        unsigned stride=(unsigned)fmax(1,fmin(100000,ceil(steps/(capacity-1))));
        Capture c={.d=d,.out=out,.branch=b,.stride=stride,.capacity=capacity,.target=target};
        ModelPathResult r=model_path_branch(d,m,f,side ? 1:-1,range,capture,&c,ui_trace_cancel,NULL);
        if(c.have)retain(&c,&c.previous);
        if(r.status==ODE_HAS_INVALID){out->has_invalid=true;b->invalid=true;}
        else if(r.status!=ODE_OK)return r.status;
    }
    out->valid=out->branch[family][0].count+out->branch[family][1].count>0;
    return ODE_OK;
}
static void selected_mask(const Document *d)
{
    memset(mask,0,sizeof(mask));
    int base=graph_palette_color(model_color(d,samples.family,samples.variable));
    highlight_xor=(uint16_t)(base^graph_highlight_color(base,true));
    for(int side=0;side<2;side++) {
        TraceBranch *b=&samples.branch[samples.family][side];
        for(unsigned j=1;j<b->count;j++) {
            unsigned i=b->start+j;if(!samples.link[i])continue;
            TracePoint *p=&samples.point[i-1],*q=&samples.point[i];
            mask_segment(&d->view,d->view.phase ? p->y[d->view.phase_x]:p->x,p->y[samples.variable],
                d->view.phase ? q->y[d->view.phase_x]:q->x,q->y[samples.variable]);
        }
    }
    /* A decimated cache can differ from the full initial graph. Highlight only
       pixels of the selected base color, never XOR a synthetic line on white. */
    for(unsigned p=0;p<384*198;p++)if(gint_vram[(UI_Y+p/384)*DWIDTH+UI_X+p%384]!=(uint16_t)base)
        mask[p/8]&=(unsigned char)~(1u<<(p%8));
}

bool trace_prepare(Document *d,CompiledModel *m,int family,int variable)
{
    if(d->nic<1 || family<0 || family>=d->nic)return false;
    OdeStatus status=prepare_range(d,m,&d->solver,family,variable,&scratch.staging);
    if(status==ODE_OK)samples=scratch.staging;
    trace_overlay_begin();
    if(status==ODE_OK)selected_mask(d);else memset(mask,0,sizeof(mask));
    return status==ODE_OK && samples.valid;
}
const OdeSettings *trace_extent(void){return &samples.extent;}
bool trace_select(const Document *d,int family,int variable)
{
    samples.family=family;samples.variable=variable;
    samples.valid=samples.branch[family][0].count+samples.branch[family][1].count>0;
    selected_mask(d);return samples.valid;
}
bool trace_has_invalid(void)
{return samples.branch[samples.family][0].invalid || samples.branch[samples.family][1].invalid;}
bool trace_direction_invalid(int direction)
{return samples.branch[samples.family][direction>0 ? 1:0].invalid;}
static bool interpolate(double x,TracePoint *point)
{
    for(int side=0;side<2;side++) {
        TraceBranch *b=&samples.branch[samples.family][side];
        for(unsigned j=0;j<b->count;j++) {
            unsigned i=b->start+j;TracePoint *q=&samples.point[i];
            if(x==q->x){*point=*q;return true;}
            if(!j || !samples.link[i])continue;
            TracePoint *p=&samples.point[i-1];
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
    if(interpolate(x,point))return true;
    double best=INFINITY;
    for(int side=0;side<2;side++) {
        TraceBranch *b=&samples.branch[samples.family][side];
        for(unsigned j=0;j<b->count;j++) {
            TracePoint *p=&samples.point[b->start+j];double distance=fabs(p->x-x);
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
            TracePoint *p=&samples.point[b->start+j];double distance=direction*(p->x-x);
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
static void cached_render(Document *d,CompiledModel *m)
{
    graph_backdrop(d,m);
    for(int f=0;f<d->nic;f++)if(graph_family_enabled(d,f))for(int side=0;side<2;side++) {
        TraceBranch *b=&samples.branch[f][side];
        for(unsigned j=1;j<b->count;j++) {
            unsigned i=b->start+j;if(!samples.link[i])continue;
            TracePoint *p=&samples.point[i-1],*q=&samples.point[i];
            for(int k=0;k<d->dim;k++)if(d->enabled&(1u<<k))
                graph_solution_segment(&d->view,p->x,p->y[k],q->x,q->y[k],graph_palette_color(model_color(d,f,k)));
        }
    }
    if(samples.has_invalid)ui_text(7,4,C_RED,"ERROR: Numerical limit");
}
OdeStatus trace_follow(Document *d,CompiledModel *m,double x)
{
    if(d->view.phase)return ODE_OK;
    ViewWindow next=d->view;
    bool follow=graph_follow_window(&next,x);
    if(!follow && x>=samples.extent.xmin && x<=samples.extent.xmax)return ODE_OK;
    OdeSettings extent=samples.extent;
    if(next.xmin<extent.xmin || next.xmax>extent.xmax || x<extent.xmin || x>extent.xmax) {
        double margin=(next.xmax-next.xmin)*.5;
        if(next.xmin<extent.xmin || x<extent.xmin)extent.xmin=fmin(next.xmin,x)-margin;
        if(next.xmax>extent.xmax || x>extent.xmax)extent.xmax=fmax(next.xmax,x)+margin;
        /* Overlay pixels were restored before this operation. Only staging is
           mutated until all IVPs complete; cancellation retains cache + VRAM. */
        OdeStatus status=prepare_range(d,m,&extent,samples.family,samples.variable,&scratch.staging);
        if(status==ODE_OK)samples=scratch.staging;
        trace_overlay_begin();selected_mask(d);
        if(status!=ODE_OK)return status;
    }
    d->view=next;model_sync_solver_window(d); /* manual solver range is preserved */
    cached_render(d,m);trace_overlay_begin();selected_mask(d);
    return ODE_OK;
}
static void invert_mask(void)
{
    for(unsigned p=0;p<384*198;p++)if(mask[p/8]&(1u<<(p%8)))
        gint_vram[(UI_Y+p/384)*DWIDTH+UI_X+p%384]^=highlight_xor;
}
static void invert_pointer(void)
{
    for(int x=0;x<384;x++)gint_vram[(UI_Y+pointer_y)*DWIDTH+UI_X+x]^=0xffff;
    for(int y=0;y<198;y++)if(y!=pointer_y)gint_vram[(UI_Y+y)*DWIDTH+UI_X+pointer_x]^=0xffff;
}
void trace_overlay_begin(void)
{
    for(int y=0;y<19;y++)memcpy(footer+y*384,gint_vram+(UI_Y+179+y)*DWIDTH+UI_X,384*sizeof(uint16_t));
    inverted=false;pointer=false;
}
void trace_overlay_restore(void)
{
    if(inverted)invert_mask();
    if(pointer)invert_pointer();
    for(int y=0;y<19;y++)memcpy(gint_vram+(UI_Y+179+y)*DWIDTH+UI_X,footer+y*384,384*sizeof(uint16_t));
    inverted=false;pointer=false;
}
void trace_overlay_show(const Document *d,const TracePoint *point,int variable,bool highlight)
{
    if(highlight){invert_mask();inverted=true;}
    if(graph_point(&d->view,d->view.phase ? point->y[d->view.phase_x]:point->x,
        point->y[variable],&pointer_x,&pointer_y)) {invert_pointer();pointer=true;}
}
