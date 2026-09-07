#include "graph.h"
#include "ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
bool graph_family_enabled(const Document *d,int family)
{
    unsigned mask=d->graph_mask[family];
    if(!d->view.phase)return mask!=0;
    unsigned axes=(1u<<d->view.phase_x)|(1u<<d->view.phase_y);
    return (mask&axes)==axes;
}
void graph_segment(const ViewWindow *v,double x0,double y0,double x1,double y1,int color)
{
    if(!graph_clip(v,&x0,&y0,&x1,&y1)) return;
    int a,b,c,e;
    if(graph_point(v,x0,y0,&a,&b) && graph_point(v,x1,y1,&c,&e)) ui_line(a,b,c,e,color);
}
static void solution_segment(const ViewWindow *v,double x0,double y0,double x1,double y1,int color)
{
    if(!graph_clip(v,&x0,&y0,&x1,&y1))return;
    int a,b,c,e;
    if(!graph_point(v,x0,y0,&a,&b) || !graph_point(v,x1,y1,&c,&e))return;
    ui_line(a,b,c,e,color);
    if(abs(c-a)>=abs(e-b)) {
        int offset=(b<PLOT_BOTTOM && e<PLOT_BOTTOM) ? 1:-1;
        ui_line(a,b+offset,c,e+offset,color);
    } else {
        int offset=(a<PLOT_RIGHT && c<PLOT_RIGHT) ? 1:-1;
        ui_line(a+offset,b,c+offset,e,color);
    }
}
static double tick_step(double requested,double span)
{
    if(span/requested<=60) return requested;
    double base=pow(10,floor(log10(span/10)));
    if(!isfinite(base) || base<=0) return span/10;
    double t=span/10/base;
    return (t<=1 ? 1:(t<=2 ? 2:(t<=5 ? 5:10)))*base;
}
static void axes(const ViewWindow *v)
{
    ui_rect(PLOT_LEFT-1,PLOT_TOP-1,PLOT_RIGHT-PLOT_LEFT+3,PLOT_BOTTOM-PLOT_TOP+3,UI_LINE);
    ui_rect(PLOT_LEFT,PLOT_TOP,PLOT_RIGHT-PLOT_LEFT+1,PLOT_BOTTOM-PLOT_TOP+1,C_WHITE);
    double xs=tick_step(v->xscale,v->xmax-v->xmin),ys=tick_step(v->yscale,v->ymax-v->ymin);
    for(int i=0;i<61;i++) {
        double x=(ceil(v->xmin/xs)+i)*xs;
        if(x>v->xmax || !isfinite(x)) break;
        int px,py;if(!graph_point(v,x,v->ymin,&px,&py)) continue;
        if(v->grid) ui_line(px,PLOT_TOP,px,PLOT_BOTTOM,UI_PALE);
        if(v->ymin<=0 && v->ymax>=0 && graph_point(v,x,0,&px,&py))
            ui_line(px,py> PLOT_TOP+2 ? py-2:PLOT_TOP,px,py<PLOT_BOTTOM-2 ? py+2:PLOT_BOTTOM,UI_MUTED);
    }
    for(int i=0;i<61;i++) {
        double y=(ceil(v->ymin/ys)+i)*ys;
        if(y>v->ymax || !isfinite(y)) break;
        int px,py;if(!graph_point(v,v->xmin,y,&px,&py)) continue;
        if(v->grid) ui_line(PLOT_LEFT,py,PLOT_RIGHT,py,UI_PALE);
        if(v->xmin<=0 && v->xmax>=0 && graph_point(v,0,y,&px,&py))
            ui_line(px>PLOT_LEFT+2 ? px-2:PLOT_LEFT,py,px<PLOT_RIGHT-2 ? px+2:PLOT_RIGHT,py,UI_MUTED);
    }
    graph_segment(v,v->xmin,0,v->xmax,0,UI_MUTED);
    graph_segment(v,0,v->ymin,0,v->ymax,UI_MUTED);
    if(v->labels) {
        int px,py;
        if(v->ymin<=0 && v->ymax>=0 && graph_point(v,v->xmax,0,&px,&py))
            ui_text(PLOT_RIGHT-8,py>PLOT_BOTTOM-13 ? PLOT_BOTTOM-13:py+2,UI_MUTED,"x");
        if(v->xmin<=0 && v->xmax>=0 && graph_point(v,0,v->ymax,&px,&py))
            ui_text(px<PLOT_LEFT+3 ? PLOT_LEFT+3:px+3,PLOT_TOP+2,UI_MUTED,"y");
    }
}
static bool slope_field(Document *d,CompiledModel *m)
{
    if(d->kind>EQ_GENERAL || d->view.phase || d->solver.sf==0) return true;
    const ViewWindow *v=&d->view;int columns=d->solver.sf;
    int rows=(int)ceil(columns*(double)(PLOT_BOTTOM-PLOT_TOP)/(PLOT_RIGHT-PLOT_LEFT));
    if(rows<1) rows=1;
    double rx=v->xmax-v->xmin,ry=v->ymax-v->ymin;
    double length=fmin((PLOT_RIGHT-PLOT_LEFT)/(double)columns,(PLOT_BOTTOM-PLOT_TOP)/(double)rows)*.34;
    for(int col=0;col<columns;col++) {
        if(ui_cancel(NULL)) return false;
        for(int row=0;row<rows;row++) {
            double x=v->xmin+(col+.5)/columns*rx,y=v->ymin+(row+.5)/rows*ry,dy;
            if(model_rhs(x,&y,&dy,m)!=ODE_OK || !isfinite(dy)) continue;
            double a=(PLOT_RIGHT-PLOT_LEFT)/rx,b=-dy*(PLOT_BOTTOM-PLOT_TOP)/ry,norm=hypot(a,b);
            if(!isfinite(norm) || norm==0) continue;
            int px,py;if(!graph_point(v,x,y,&px,&py)) continue;
            int dx=(int)round(a/norm*length),sy=(int)round(b/norm*length);
            ui_line(px-dx,py-sy,px+dx,py+sy,UI_LINE);
        }
    }
    return true;
}
typedef struct {Document *d;int family,variable,stride,color;bool have;double x,y[9];} Curve;
static bool curve_point(double x,const double *y,uint32_t step,void *ctx)
{
    Curve *c=ctx;Document *d=c->d;
    if(!y){c->have=false;return true;}
    if(c->have && step%(unsigned)c->stride && x!=d->solver.xmin && x!=d->solver.xmax) return true;
    if(c->have) {
        if(d->view.phase) {
            if(graph_family_enabled(d,c->family)) solution_segment(&d->view,
                c->y[d->view.phase_x],c->y[d->view.phase_y],y[d->view.phase_x],y[d->view.phase_y],
                c->color<0 ? graph_palette_color(model_color(d,c->family,d->view.phase_y)):c->color);
        } else for(int j=0;j<d->dim;j++) if((d->graph_mask[c->family]&(1u<<j))
            && (c->variable<0 || c->variable==j))
            solution_segment(&d->view,c->x,c->y[j],x,y[j],
                c->color<0 ? graph_palette_color(model_color(d,c->family,j)):c->color);
    }
    c->x=x;memcpy(c->y,y,(unsigned)d->dim*sizeof(double));c->have=true;
    return true;
}
GraphResult graph_render(Document *d,CompiledModel *m,bool first)
{
    dclear(C_WHITE);
    axes(&d->view);
    GraphResult result={.status=ODE_OK,.failed_family=-1};
    if(!slope_field(d,m)) result.status=ODE_CANCELLED;
    for(int i=0;i<d->nic && result.status!=ODE_CANCELLED;i++) {
        if(!graph_family_enabled(d,i)) continue;
        for(int direction=-1;direction<=1;direction+=2) {
            Curve c={.d=d,.family=i,.variable=-1,.stride=first ? 1:d->solver.step,.color=-1};
            ModelPathResult r=model_path_branch(d,m,i,direction,&d->solver,curve_point,&c,ui_cancel,NULL);
            result.steps+=r.steps;
            if(r.invalid!=ODE_OK)result.invalid=r.invalid;
            if(r.status!=ODE_OK && (result.status==ODE_OK ||
                (result.status==ODE_HAS_INVALID && r.status!=ODE_HAS_INVALID))) {
                result.status=r.status;result.failed_family=i;
            }
            if(r.status==ODE_CANCELLED) {result.status=r.status;break;}
        }
    }
    if(result.status!=ODE_OK) {
        ui_rect(0,0,384,18,C_WHITE);
        if(result.status==ODE_HAS_INVALID)
            ui_text(7,4,C_RED,"ERROR: %s",ode_status_text(result.invalid));
        else if(result.failed_family>=0)
            ui_text(7,4,C_RED,"Partial: %s (IC %d)",ode_status_text(result.status),result.failed_family+1);
        else ui_text(7,4,C_RED,"Partial: %s",ode_status_text(result.status));
    }
    ui_softkeys("TRACE","ZOOM","V-WIN","TABLE","G-SLV","BACK");
    return result;
}
OdeStatus graph_highlight_curve(Document *d,CompiledModel *m,int family,int variable)
{
    if(!d || !m || family<0 || family>=d->nic || !graph_family_enabled(d,family)
        || (!d->view.phase && (variable<0 || variable>=d->dim
            || !(d->graph_mask[family]&(1u<<variable)))))
        return ODE_BAD_INPUT;
    OdeStatus status=ODE_OK;
    int base=graph_palette_color(model_color(d,family,d->view.phase ? d->view.phase_y:variable));
    for(int direction=-1;direction<=1;direction+=2) {
        Curve c={.d=d,.family=family,.variable=variable,
            .stride=d->solver.step,.color=graph_highlight_color(base,false)};
        ModelPathResult r=model_path_branch(d,m,family,direction,&d->solver,curve_point,&c,ui_cancel,NULL);
        if(r.status!=ODE_OK)status=r.status;
        if(r.status!=ODE_OK && r.status!=ODE_HAS_INVALID)break;
    }
    return status;
}
typedef struct {Document *d;int family;bool have;double xmin,xmax,ymin,ymax;} Bounds;
static void bounds_add(Bounds *b,double x,double y)
{
    if(!isfinite(x) || !isfinite(y)) return;
    if(x<b->d->view.xmin || x>b->d->view.xmax)return;
    if(!b->have){b->xmin=b->xmax=x;b->ymin=b->ymax=y;b->have=true;}
    if(x<b->xmin)b->xmin=x;
    if(x>b->xmax)b->xmax=x;
    if(y<b->ymin)b->ymin=y;
    if(y>b->ymax)b->ymax=y;
}
static bool bound_point(double x,const double *y,uint32_t step,void *ctx)
{
    (void)step;Bounds *b=ctx;
    if(b->d->view.phase) bounds_add(b,y[b->d->view.phase_x],y[b->d->view.phase_y]);
    else for(int i=0;i<b->d->dim;i++)if(b->d->graph_mask[b->family]&(1u<<i))bounds_add(b,x,y[i]);
    return true;
}
OdeStatus graph_auto_window(Document *d,CompiledModel *m)
{
    Bounds b={.d=d};OdeStatus status=ODE_OK;
    OdeSettings sampling=d->solver;
    if(!d->view.phase) {
        sampling.xmin=fmax(sampling.xmin,d->view.xmin);
        sampling.xmax=fmin(sampling.xmax,d->view.xmax);
        if(sampling.xmin>=sampling.xmax)return ODE_BAD_INPUT;
    }
    for(int i=0;i<d->nic;i++)if(graph_family_enabled(d,i)) {
        b.family=i;
        for(int dir=-1;dir<=1;dir+=2) {
            const InitialCondition *ic=&d->ic[i];
            if((dir>0 && ic->x>sampling.xmax) || (dir<0 && ic->x<sampling.xmin))continue;
            OdeResult r=ode_integrate(model_rhs,m,d->dim,ic->x,ic->y,
                dir>0 ? sampling.xmax:sampling.xmin,&sampling,bound_point,&b,ui_cancel,NULL);
            if(r.status==ODE_CANCELLED) return r.status;
            if(r.status!=ODE_OK) status=r.status;
        }
    }
    if(!b.have) return ODE_BAD_INPUT;
    /* Do not silently fit only a partial trajectory. */
    if(status!=ODE_OK) return status;
    double dy=(b.ymax-b.ymin)*.12;
    if(dy==0)dy=fmax(1,fabs(b.ymin)*.1);
    if(!isfinite(b.ymin-dy) || !isfinite(b.ymax+dy) || b.ymin-dy>=b.ymax+dy)return ODE_NONFINITE;
    d->view.ymin=b.ymin-dy;d->view.ymax=b.ymax+dy;
    d->view.yscale=tick_step(1,d->view.ymax-d->view.ymin);
    return ODE_OK;
}
