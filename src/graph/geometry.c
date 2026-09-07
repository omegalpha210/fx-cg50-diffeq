#include "graph.h"
#include "ui.h"
#include <gint/display.h>
#include <math.h>
int graph_palette_color(unsigned color)
{
    /* RGB565, including the nearest representable #33ff33 (green=63). */
    static const uint16_t palette[]={0x001f,0xf800,0xf81f,0x0000,UI_CYAN,UI_BRIGHT_GREEN};
    return palette[color<6 ? color:2];
}
int graph_color(int family,int variable,int dimension)
{
    return graph_palette_color(model_default_color(family,variable,dimension));
}
int graph_highlight_color(int base,bool invert)
{
    if(base==C_BLACK)return C_BLUE;
    return invert ? base^0xffff:UI_INK;
}
static unsigned code(const ViewWindow *v,double x,double y)
{ return (x<v->xmin ? 1u:0u)|(x>v->xmax ? 2u:0u)|(y<v->ymin ? 4u:0u)|(y>v->ymax ? 8u:0u); }
bool graph_clip(const ViewWindow *v,double *x0,double *y0,double *x1,double *y1)
{
    if(!isfinite(*x0) || !isfinite(*y0) || !isfinite(*x1) || !isfinite(*y1)) return false;
    for(int tries=0;tries<12;tries++) {
        unsigned a=code(v,*x0,*y0),b=code(v,*x1,*y1);
        if(!(a|b)) return true;
        if(a&b) return false;
        unsigned c=a ? a:b;double x,y;
        if(c&12) {
            if(*y1==*y0) return false;
            y=c&8 ? v->ymax:v->ymin;
            x=*x0+(*x1-*x0)*((y-*y0)/(*y1-*y0));
        } else {
            if(*x1==*x0) return false;
            x=c&2 ? v->xmax:v->xmin;
            y=*y0+(*y1-*y0)*((x-*x0)/(*x1-*x0));
        }
        if(!isfinite(x) || !isfinite(y)) return false;
        if(c==a) {*x0=x;*y0=y;} else {*x1=x;*y1=y;}
    }
    return false;
}
bool graph_point(const ViewWindow *v,double x,double y,int *px,int *py)
{
    if(!isfinite(x) || !isfinite(y) || code(v,x,y)) return false;
    *px=PLOT_LEFT+(int)((x-v->xmin)/(v->xmax-v->xmin)*(PLOT_RIGHT-PLOT_LEFT)+.5);
    *py=PLOT_BOTTOM-(int)((y-v->ymin)/(v->ymax-v->ymin)*(PLOT_BOTTOM-PLOT_TOP)+.5);
    return true;
}
bool graph_zoom(ViewWindow *v,double factor,double dx,double dy)
{
    ViewWindow next=*v;
    double w=v->xmax-v->xmin,h=v->ymax-v->ymin;
    double cx=v->xmin+w*.5+dx*w,cy=v->ymin+h*.5+dy*h;
    next.xmin=cx-w*.5*factor;next.xmax=cx+w*.5*factor;
    next.ymin=cy-h*.5*factor;next.ymax=cy+h*.5*factor;
    if(!isfinite(next.xmin) || !isfinite(next.xmax) || !isfinite(next.ymin)
        || !isfinite(next.ymax) || next.xmin>=next.xmax || next.ymin>=next.ymax
        || !isfinite(next.xmax-next.xmin) || !isfinite(next.ymax-next.ymin)) return false;
    *v=next;return true;
}

/* Trigger at 10%, land at 30%: hysteresis avoids edge jitter. Translate both
   axes atomically; reject a pan if its span cannot be represented safely. */
static bool follow_axis(double low,double high,double value,double *a,double *b)
{
    double span=high-low;
    if(!isfinite(span) || span<=0)return false;
    *a=low;*b=high;
    if(value>high-.1*span){*b=value+.3*span;*a=*b-span;}
    else if(value<low+.1*span){*a=value-.3*span;*b=*a+span;}
    return isfinite(*a) && isfinite(*b) && *a<*b
        && fabs((*b-*a)-span)<=1e-12*span;
}
bool graph_follow_window(ViewWindow *v,double x,double y)
{
    if(v->phase || !isfinite(x) || !isfinite(y) || fabs(x)>1e100 || fabs(y)>1e100)return false;
    ViewWindow next=*v;
    if(!follow_axis(v->xmin,v->xmax,x,&next.xmin,&next.xmax)
        || !follow_axis(v->ymin,v->ymax,y,&next.ymin,&next.ymax))return false;
    bool changed=next.xmin!=v->xmin || next.xmax!=v->xmax
        || next.ymin!=v->ymin || next.ymax!=v->ymax;
    if(changed)*v=next;
    return changed;
}

int graph_field_color(unsigned color)
{
    /* gint C_RGB uses 0..31 channels, not RGB888. Opaque field-only presets. */
    static const int palette[]={C_RGB(17,21,27),C_RGB(27,17,17),C_RGB(12,23,24),
        C_RGB(25,17,25),C_RGB(24,21,9),C_RGB(18,18,18)};
    return palette[color<FIELD_COLORS ? color:0];
}
bool graph_field_direction(const ViewWindow *v,double slope,double *dx,double *dy)
{
    double rx=v->xmax-v->xmin,ry=v->ymax-v->ymin;
    if(!isfinite(slope) || !isfinite(rx) || !isfinite(ry) || rx<=0 || ry<=0)return false;
    if(slope==0){*dx=1;*dy=0;return true;}
    /* Normalize (W/rx, -slope*H/ry) without overflowing intermediate products.
       Mantissas stay small; exponent differences also cover subnormal spans. */
    int ex,ey,ef;
    double mx=frexp(rx,&ex),my=frexp(ry,&ey),mf=frexp(slope,&ef);
    double t=mf*mx/my*(PLOT_BOTTOM-PLOT_TOP)/(PLOT_RIGHT-PLOT_LEFT);
    int exponent=ef+ex-ey;
    if(exponent>=0) {
        double inverse=ldexp(1/fabs(t),-exponent),norm=hypot(inverse,1);
        *dx=inverse/norm;*dy=-copysign(1/norm,t);
    } else {
        t=ldexp(t,exponent);double norm=hypot(1,t);
        *dx=1/norm;*dy=-t/norm;
    }
    return true;
}
