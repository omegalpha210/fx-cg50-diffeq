#include "graph.h"
#include <gint/display.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
int main(void)
{
    assert(graph_color(0,0,3)==C_RGB(31,0,31));
    assert(graph_color(0,1,3)==0x07ff);
    assert(graph_color(0,2,3)==0x37e6);
    assert(graph_color(1,0,1)==0x07ff);
    const int palette[]={0x001f,0xf800,0xf81f,0,0x07ff,0x37e6};
    for(unsigned i=0;i<6;i++)assert(graph_palette_color(i)==palette[i]);
    assert(graph_palette_color(255)==palette[2]);
    ViewWindow v;model_window_defaults(&v);
    assert(v.grid==1 && v.labels==1 && fabs(model_xdot(&v)-(12.6/378.0))<1e-15);
    assert(model_set_xdot(&v,.05) && fabs(v.xmax-12.6)<1e-12);
    assert(!model_set_xdot(&v,0) && fabs(v.xmax-12.6)<1e-12);
    model_window_defaults(&v);
    double x0=-100,y0=0,x1=100,y1=0;
    assert(graph_clip(&v,&x0,&y0,&x1,&y1) && x0==v.xmin && x1==v.xmax);
    x0=-2;y0=100;x1=2;y1=100;assert(!graph_clip(&v,&x0,&y0,&x1,&y1));
    x0=-1e100;y0=0;x1=1e100;y1=0;
    assert(graph_clip(&v,&x0,&y0,&x1,&y1) && x0==v.xmin && x1==v.xmax);
    x0=0;y0=-1e100;x1=0;y1=1e100;
    assert(graph_clip(&v,&x0,&y0,&x1,&y1) && y0==v.ymin && y1==v.ymax);
    x0=NAN;assert(!graph_clip(&v,&x0,&y0,&x1,&y1));
    int x,y;assert(graph_point(&v,0,0,&x,&y));
    assert(x==(PLOT_LEFT+PLOT_RIGHT+1)/2 && y==(PLOT_TOP+PLOT_BOTTOM)/2);
    assert(!graph_point(&v,100,0,&x,&y));
    double span=v.xmax-v.xmin;
    assert(graph_zoom(&v,.5,0,0) && fabs(v.xmax-v.xmin-span*.5)<1e-12);
    ViewWindow before=v;assert(!graph_zoom(&v,0,0,0) && v.xmin==before.xmin);
    assert(graph_zoom(&v,1,.2,0) && v.xmin>before.xmin);
    puts("Graph clipping, extreme endpoints, coordinate mapping, pan/zoom passed.");
    return 0;
}
