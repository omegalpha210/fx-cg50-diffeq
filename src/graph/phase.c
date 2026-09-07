#include "phase_graph.h"
#include "ui.h"
#include <math.h>
#include <string.h>
/* Only fixed points persist. No extra trajectory or framebuffer is allocated. */
static PhaseResults results;
void graph_phase_reset(void){memset(&results,0,sizeof(results));}
const PhaseResults *graph_phase_results(void){return &results;}
static bool active(const Document *d){return model_phase_supported(d) && d->view.phase;}
static void pixel_line(double x,double y,double u,double v,int color)
{
    ui_line((int)lround(fmax(PLOT_LEFT,fmin(PLOT_RIGHT,x))),
        (int)lround(fmax(PLOT_TOP,fmin(PLOT_BOTTOM,y))),
        (int)lround(fmax(PLOT_LEFT,fmin(PLOT_RIGHT,u))),
        (int)lround(fmax(PLOT_TOP,fmin(PLOT_BOTTOM,v))),color);
}
static OdeStatus field(const Document *d,const CompiledModel *m,bool draw,OdeCancel cancel,void *ctx)
{
    if(!d->phase_field)return ODE_OK;
    const ViewWindow *v=&d->phase_view;
    const int columns=PHASE_FIELD_COLUMNS,rows=11;
    double width=PLOT_RIGHT-PLOT_LEFT,height=PLOT_BOTTOM-PLOT_TOP;
    double length=fmin(width/columns,height/rows)*.32;
    int color=graph_field_color(0);
    for(int col=0;col<columns;col++) {
        if(cancel && cancel(ctx))return ODE_CANCELLED;
        for(int row=0;row<rows;row++) {
            double y[2]={v->xmin+(col+.5)/columns*(v->xmax-v->xmin),
                v->ymin+(row+.5)/rows*(v->ymax-v->ymin)},vector[2],unit[2];
            if(phase_vector(m,d->ic[0].x,y,vector)!=ODE_OK)continue;
            if(!draw)continue;
            double px=PLOT_LEFT+(col+.5)*width/columns,py=PLOT_BOTTOM-(row+.5)*height/rows;
            if(!phase_direction(v,vector,width,height,unit)) {
                if(vector[0]==0 && vector[1]==0)pixel_line(px,py,px+1,py,color);
                continue;
            }
            double dx=unit[0],dy=unit[1],tx=px+length*dx,ty=py+length*dy;
            pixel_line(px-length*dx,py-length*dy,tx,ty,color);
            pixel_line(tx,ty,tx-3*dx-2*dy,ty-3*dy+2*dx,color);
            pixel_line(tx,ty,tx-3*dx+2*dy,ty-3*dy-2*dx,color);
        }
    }
    return ODE_OK;
}
static bool contour(int component,const double a[2],const double b[2],void *ctx)
{
    const ViewWindow *v=ctx;
    graph_segment(v,a[0],a[1],b[0],b[1],graph_palette_color(component==0 ? 1:0));
    return true;
}
static bool discard_segment(int component,const double a[2],const double b[2],void *ctx)
{(void)component;(void)a;(void)b;(void)ctx;return true;}
OdeStatus graph_phase_preflight(const Document *d,const CompiledModel *m,OdeCancel cancel,void *ctx)
{
    if(!active(d))return ODE_OK;
    OdeStatus s=field(d,m,false,cancel,ctx);
    if(s==ODE_OK && d->phase_nullclines)s=phase_nullclines(m,d->ic[0].x,&d->phase_view,
        PHASE_CONTOUR_GRID,discard_segment,NULL,cancel,ctx);
    if(s==ODE_OK && cancel && cancel(ctx))s=ODE_CANCELLED;
    return s;
}
void graph_phase_layers(const Document *d,const CompiledModel *m)
{
    if(!active(d))return;
    /* Preflight owns cancellation. This deterministic bounded paint pass cannot
       abort midway and never consumes an EXIT intended for the next UI event. */
    field(d,m,true,NULL,NULL);
    if(d->phase_nullclines)phase_nullclines(m,d->ic[0].x,&d->phase_view,
        PHASE_CONTOUR_GRID,contour,(void*)&d->phase_view,NULL,NULL);
    if(d->phase_nullclines) {
        ui_text(6,4,graph_palette_color(1),"N1");ui_text(31,4,graph_palette_color(0),"N2");
    }
    if(!phase_autonomous(m) && (d->phase_field || d->phase_nullclines))
        ui_text(62,4,UI_MUTED,"Field at x=%.6g",d->ic[0].x);
}
void graph_phase_markers(const Document *d,int selected)
{
    if(!active(d))return;
    for(unsigned i=0;i<results.count;i++) {
        int x,y;if(!graph_point(&d->phase_view,results.root[i].y[0],results.root[i].y[1],&x,&y))continue;
        int r=(int)i==selected ? 6:4;
        pixel_line(x-r,y,x,y-r,C_BLACK);pixel_line(x,y-r,x+r,y,C_BLACK);
        pixel_line(x+r,y,x,y+r,C_BLACK);pixel_line(x,y+r,x-r,y,C_BLACK);
        pixel_line(x-1,y,x+1,y,C_WHITE);
    }
}
OdeStatus graph_phase_search(const Document *d,const CompiledModel *m,OdeCancel cancel,void *ctx)
{
    if(!active(d))return ODE_BAD_INPUT;
    return phase_equilibria(m,d->ic[0].x,&d->phase_view,&results,cancel,ctx);
}
