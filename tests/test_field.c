#include "graph.h"
#include "trace.h"
#include "ui.h"
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;
unsigned long host_rhs_calls(void);unsigned long host_solves(void);
void host_cancel_after(unsigned polls);
static unsigned pixel(int x,int y){return gint_vram[(y+UI_Y)*DWIDTH+x+UI_X];}
static unsigned hash(void){unsigned h=0;for(int i=0;i<DWIDTH*DHEIGHT;i++)h=31*h+gint_vram[i];return h;}
static unsigned color_count(unsigned color) {
    unsigned n=0;for(int y=0;y<=PLOT_BOTTOM;y++)for(int x=0;x<=PLOT_RIGHT;x++)n+=pixel(x,y)==color;return n;
}
int main(void)
{
    model_defaults(&d,EQ_GENERAL,1);d.view.grid=d.view.labels=0;d.nic=0;
    assert(d.solver.sf==12);
    for(int sf=0;sf<=50;sf++) {
        d.solver.sf=sf;assert(ode_validate(&d.solver)==ODE_OK);
    }
    d.solver.sf=51;assert(ode_validate(&d.solver)==ODE_BAD_INPUT);
    d.solver.sf=-1;assert(ode_validate(&d.solver)==ODE_BAD_INPUT);
    const char *expressions[]={"0","1","-1","x-y"};
    for(int k=0;k<4;k++) {
        strcpy(d.text[0],expressions[k]);d.solver.sf=1;
        assert(model_compile(&d,&m).expression.status==EXPR_OK);
        double ux,uy;assert(graph_field_direction(&d.view,k==3 ? 0:k==2 ? -1:k,&ux,&uy));
        if(k<3){assert(ux>0);assert(k==0 ? uy==0:(k==1 ? uy<0:uy>0));}
        d.field_style=FIELD_SEGMENT;graph_render(&d,&m,false);
        unsigned segment[200],count=0,color=(unsigned)graph_field_color(d.field_color);
        for(int y=0;y<=PLOT_BOTTOM;y++)for(int x=0;x<=PLOT_RIGHT;x++)if(pixel(x,y)==color) {
            assert(count<200);segment[count++]=(unsigned)(y*384+x);
        }
        assert(count>0);
        d.field_style=FIELD_ARROW;unsigned long solves=host_solves();
        assert(graph_render(&d,&m,false).status==ODE_OK && host_solves()==solves);
        unsigned extra=0;
        for(int y=0;y<=PLOT_BOTTOM;y++)for(int x=0;x<=PLOT_RIGHT;x++)if(pixel(x,y)==color) {
            unsigned p=(unsigned)(y*384+x),i=0;while(i<count && segment[i]!=p)i++;
            if(i==count){extra++;if(k<3)assert((x-191.5)*ux+(y-98.5)*uy>0);}
        }
        assert(extra>0); /* Arrow visibly points forward, including y'=0. */
    }
    ViewWindow v=d.view;v.xmin=0;v.xmax=100;v.ymin=0;v.ymax=1;
    double x,y;assert(graph_field_direction(&v,1,&x,&y));
    assert(fabs(y/x+100.*197/383)<1e-12);
    assert(graph_field_direction(&v,DBL_MAX,&x,&y) && isfinite(x) && y==-1);
    v.xmax=1e-300;v.ymax=1e300;
    assert(graph_field_direction(&v,DBL_MAX,&x,&y) && isfinite(x) && isfinite(y));
    assert(!graph_field_direction(&v,NAN,&x,&y) && !graph_field_direction(&v,INFINITY,&x,&y));
    strcpy(d.text[0],"0");assert(model_compile(&d,&m).expression.status==EXPR_OK);
    d.solver.sf=12;
    unsigned long rhs=host_rhs_calls();graph_render(&d,&m,false);assert(host_rhs_calls()-rhs==84);
    unsigned baseline=hash();d.nic=3;
    for(int f=0;f<3;f++){d.ic[f]=d.ic[0];d.ic_enabled=0;}
    rhs=host_rhs_calls();graph_render(&d,&m,false);unsigned long integrated=host_rhs_calls()-rhs-84;
    assert(integrated>0 && hash()==baseline); /* OFF still integrates each IC. */
    for(unsigned c=0;c<FIELD_COLORS;c++) {
        d.field_color=(uint8_t)c;graph_render(&d,&m,false);assert(color_count((unsigned)graph_field_color(c))>0);
        assert(graph_field_color(c)!=0xffff && graph_field_color(c)!=0x37e6);
    }
    /* Final renderer guard is independent of validated entry/storage paths. */
    const int density[]={0,1,12,50,51,100,INT_MAX,-1,INT_MIN};
    const unsigned expected[]={0,1,84,1300,1300,1300,1300,0,0};
    unsigned maximum=0;
    for(unsigned i=0;i<sizeof(density)/sizeof(density[0]);i++) {
        d.solver.sf=density[i];rhs=host_rhs_calls();graph_backdrop(&d,&m);
        assert(host_rhs_calls()-rhs==expected[i] && d.solver.sf==density[i]);
        if(density[i]==50)maximum=hash();
        if(density[i]>50)assert(hash()==maximum);
    }
    d.solver.sf=50;graph_backdrop(&d,&m);
    assert(color_count((unsigned)graph_field_color(d.field_color))>0);
    for(int yy=0;yy<DHEIGHT;yy++)for(int xx=0;xx<DWIDTH;xx++)if(xx<UI_X || xx>UI_X+383 || yy<UI_Y || yy>UI_Y+197)
        assert(gint_vram[yy*DWIDTH+xx]!=(unsigned)graph_field_color(d.field_color));
    d.solver.sf=0;rhs=host_rhs_calls();graph_render(&d,&m,false);assert(host_rhs_calls()-rhs==integrated);
    d.solver.sf=12;strcpy(d.text[0],"sqrt(y)");d.nic=1;d.ic[0].y[0]=0;d.ic_enabled=MODEL_IC_MASK;
    assert(model_compile(&d,&m).expression.status==EXPR_OK);
    GraphResult result=graph_render(&d,&m,false);assert(result.status==ODE_OK && result.invalid==ODE_OK);
    unsigned before=hash();trace_overlay_begin();assert(trace_prepare(&d,&m,0,0));
    TracePoint p;assert(trace_point_near(0,&p));trace_overlay_show(&d,&p,0,true);trace_overlay_restore();assert(hash()==before);
    host_cancel_after(1);assert(graph_render(&d,&m,false).status==ODE_CANCELLED);host_cancel_after(0);
    for(int kind=EQ_SECOND;kind<=EQ_SYSTEM;kind++) {
        model_defaults(&d,(EquationKind)kind,1);d.enabled=0;d.field_style=FIELD_ARROW;
        assert(!model_field_supported(&d) && model_compile(&d,&m).expression.status==EXPR_OK);
        rhs=host_rhs_calls();graph_render(&d,&m,false);assert(host_rhs_calls()==rhs);
    }
    puts("Slope field: streaming/direction/zero/anisotropy/extreme finite/0..50/defensive clamp/single-pass/palette/domain/cancel/overlay/unsupported modes passed.");
}
