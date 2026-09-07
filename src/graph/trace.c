#include "trace.h"
#include "ui.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
static TraceSamples samples;
static unsigned char mask[384*198/8];
static uint16_t footer[384*19];
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
    Document *d;TraceBranch *branch;unsigned stride;int variable;
    double target;bool have;TracePoint previous;
} Capture;
static bool capture(double x,const double *y,uint32_t step,void *context)
{
    Capture *c=context;Document *d=c->d;
    bool first=!c->have;
    if(!y){
        TraceBranch *b=c->branch;
        if(c->have && (d->view.phase || (c->previous.x>=d->view.xmin && c->previous.x<=d->view.xmax))
            && (!b->count || b->point[b->count-1].x!=c->previous.x)) {
            unsigned i=b->count<TRACE_BRANCH_POINTS ? b->count++:TRACE_BRANCH_POINTS-1;
            b->point[i]=c->previous;
        }
        c->have=false;return true;
    }
    if(!c->have || step%(unsigned)d->solver.step==0 || x==c->target) {
        if(c->have)mask_segment(&d->view,d->view.phase ? c->previous.y[d->view.phase_x]:c->previous.x,
            c->previous.y[c->variable],d->view.phase ? y[d->view.phase_x]:x,y[c->variable]);
        c->previous.x=x;memcpy(c->previous.y,y,(unsigned)d->dim*sizeof(double));c->have=true;
    }
    TraceBranch *b=c->branch;
    if((first || !b->count || step%c->stride==0 || x==c->target)
        && (d->view.phase || (x>=d->view.xmin && x<=d->view.xmax))) {
        unsigned i=b->count<TRACE_BRANCH_POINTS ? b->count++:TRACE_BRANCH_POINTS-1;
        b->point[i].x=x;memcpy(b->point[i].y,y,(unsigned)d->dim*sizeof(double));
    }
    return true;
}
bool trace_prepare(Document *d,CompiledModel *m,int family,int variable)
{
    samples.valid=false;samples.has_invalid=false;memset(mask,0,sizeof(mask));
    int base=graph_palette_color(model_color(d,family,variable));
    highlight_xor=(uint16_t)(base^graph_highlight_color(base,true));
    for(int side=0;side<2;side++) {
        samples.branch[side].count=0;
        double target=side ? d->solver.xmax:d->solver.xmin;
        double steps=ceil(fabs(target-d->ic[family].x)/d->solver.h);
        unsigned stride=(unsigned)fmax(1,fmin(100000,ceil(steps/(TRACE_BRANCH_POINTS-1))));
        /* Keep Step's exact grid when downsampling a very long trajectory. */
        stride=((stride+(unsigned)d->solver.step-1)/(unsigned)d->solver.step)*(unsigned)d->solver.step;
        Capture c={.d=d,.branch=&samples.branch[side],.stride=stride,.variable=variable,.target=target};
        ModelPathResult result=model_path_branch(d,m,family,side ? 1:-1,&d->solver,capture,&c,ui_trace_cancel,NULL);
        if(result.status==ODE_HAS_INVALID)samples.has_invalid=true;
        else if(result.status!=ODE_OK)return false;
    }
    samples.family=family;samples.variable=variable;
    samples.valid=samples.branch[0].count+samples.branch[1].count>0;
    return samples.valid;
}
bool trace_has_invalid(void){return samples.has_invalid;}
bool trace_point_near(double x,TracePoint *point)
{
    if(!samples.valid)return false;
    double best=INFINITY;
    for(int s=0;s<2;s++)for(unsigned i=0;i<samples.branch[s].count;i++) {
        TracePoint *p=&samples.branch[s].point[i];double distance=fabs(p->x-x);
        if(distance<best){best=distance;*point=*p;}
    }
    return isfinite(best);
}
bool trace_move(double x,int direction,TracePoint *point)
{
    if(!samples.valid)return false;
    double best=INFINITY;
    for(int s=0;s<2;s++)for(unsigned i=0;i<samples.branch[s].count;i++) {
        TracePoint *p=&samples.branch[s].point[i];double distance=direction*(p->x-x);
        if(distance>1e-12*fmax(1,fabs(x)) && distance<best){best=distance;*point=*p;}
    }
    return isfinite(best);
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
