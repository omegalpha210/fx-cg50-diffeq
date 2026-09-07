#include "table.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {double min,max;bool have;} Span;
static bool scan(double x,const double *y,uint32_t step,void *context)
{
    (void)y;(void)step;Span *s=context;
    if(!s->have){s->min=s->max=x;s->have=true;}
    s->min=fmin(s->min,x);s->max=fmax(s->max,x);return true;
}
static bool same_sample(const Document *d,double x,double y)
{
    /* Direct grid arithmetic differs from repeated RK4 additions. Match only
       a tiny fraction of h (and never more than that fraction of the range).
       Copy an accepted state at that point; do not extrapolate a failed step. */
    double tolerance=1e-7*fmin(d->solver.h,d->solver.xmax-d->solver.xmin);
    return x==y || fabs(x-y)<=tolerance;
}
static void extra_add(TableIndex *index,double x)
{
    unsigned at=0;
    while(at<index->extras && index->extra[at]<x)at++;
    if((at<index->extras && index->extra[at]==x) || (at && index->extra[at-1]==x))return;
    /* Common x0: two endpoints per IC, common window starts and optional zero. */
    if(index->extras==TABLE_EXTRA){index->status=ODE_BAD_INPUT;return;}
    memmove(index->extra+at+1,index->extra+at,(index->extras-at)*sizeof(double));
    index->extra[at]=x;index->extras++;
}
static unsigned grid_before(const TableIndex *index,double x)
{
    if(index->last<index->first)return 0;
    double count=ceil((x-index->origin)/index->spacing)-index->first;
    unsigned total=(unsigned)(index->last-index->first+1);
    return count<=0 ? 0:(count>=total ? total:(unsigned)count);
}
double table_x_at(const TableIndex *index,unsigned row)
{
    for(unsigned i=0;i<index->extras;i++) {
        unsigned before=grid_before(index,index->extra[i]);
        if(row==before)return index->extra[i];
        if(row<before)break;
        row--;
    }
    double offset=(index->first+(double)row)*index->spacing,x=index->origin+offset;
    if(index->xmin<=0 && index->xmax>=0
        && fabs(x)<=32*DBL_EPSILON*fmax(fabs(index->origin),fabs(offset)))x=0;
    return fmax(index->xmin,fmin(index->xmax,x));
}
unsigned table_bottom(const TableIndex *index)
{return index->total>TABLE_ROWS ? index->total-TABLE_ROWS:0;}
unsigned table_nearest(const TableIndex *index,double x)
{
    unsigned lo=0,hi=index->total;
    while(lo<hi){unsigned mid=lo+(hi-lo)/2;if(table_x_at(index,mid)<x)lo=mid+1;else hi=mid;}
    if(lo==index->total)return lo ? lo-1:0;
    if(lo && fabs(table_x_at(index,lo-1)-x)<=fabs(table_x_at(index,lo)-x))lo--;
    return lo;
}
OdeStatus table_index_build(const Document *d,CompiledModel *m,TableIndex *index,
    OdeCancel cancel,void *cancel_ctx)
{
    memset(index,0,sizeof(*index));index->last=-1;
    ModelWork work=model_preflight(d,&d->solver);
    if(work.status!=ODE_OK)return work.status;
    if(!d->nic)return ODE_BAD_INPUT;
    index->origin=d->ic[0].x;index->spacing=d->solver.h*d->solver.step;
    index->solutions=model_field_supported(d);bool have=false;
    if(index->solutions) {
        if(d->enabled&1u)for(int f=0;f<d->nic;f++)index->columns[index->count++]=f;
    } else for(int v=0;v<d->dim;v++)if(d->enabled&(1u<<v))index->columns[index->count++]=v;
    int families=index->solutions ? d->nic:1;
    for(int f=0;f<families;f++) {
        if(d->ic[f].x!=index->origin)return ODE_BAD_INPUT;
        for(int side=0;side<2;side++) {
            Span span={0};OdeResult r=model_trajectory(d,m,f,side ? 1:-1,scan,&span,cancel,cancel_ctx);
            if(r.status!=ODE_OK && !ode_invalid_region(r.status))return r.status;
            if(ode_invalid_region(r.status)){if(side)index->high=r.status;else index->low=r.status;}
            if(!span.have)continue;
            if(!have){index->xmin=span.min;index->xmax=span.max;have=true;}
            index->xmin=fmin(index->xmin,span.min);index->xmax=fmax(index->xmax,span.max);
            extra_add(index,span.min);extra_add(index,span.max);
        }
    }
    if(!have || index->status!=ODE_OK)return ODE_BAD_INPUT;
    double midpoint=index->xmin+(index->xmax-index->xmin)/2;
    /* Zero is natural in the middle half. The existing adjacent-point
       interpolation can supply that row without changing the RK4 grid. */
    bool zero=index->xmin<=0 && index->xmax>=0 && fabs(midpoint)<=(index->xmax-index->xmin)/4;
    if(zero)extra_add(index,0);
    if(isfinite(index->spacing)) {
        double resolution=fmax(nextafter(index->xmin,INFINITY)-index->xmin,
            index->xmax-nextafter(index->xmax,-INFINITY));
        if(index->spacing<resolution)return ODE_BAD_STEP; /* no duplicate rounded x rows */
        /* RK4 accumulates x while the index computes grid coordinates directly.
           Snap only sub-step rounding at endpoints, never change h. */
        double tolerance=1e-7/d->solver.step;
        double first=ceil((index->xmin-index->origin)/index->spacing-tolerance);
        double last=floor((index->xmax-index->origin)/index->spacing+tolerance);
        if(!isfinite(first) || !isfinite(last) || first<-100001 || last>100001)return ODE_BAD_STEP;
        index->first=(int)first;index->last=(int)last;
        unsigned n=0;
        for(unsigned i=0;i<index->extras;i++) {
            double q=(index->extra[i]-index->origin)/index->spacing,nearest=round(q);
            if(nearest>=first && nearest<=last
                && same_sample(d,index->extra[i],index->origin+nearest*index->spacing))continue;
            index->extra[n++]=index->extra[i];
        }
        index->extras=n;
    }
    index->total=(index->last>=index->first ? (unsigned)(index->last-index->first+1):0)+index->extras;
    double anchor=zero ? 0:midpoint;
    unsigned row=table_nearest(index,anchor);
    index->mid=row>TABLE_ROWS/2 ? row-TABLE_ROWS/2:0;
    if(index->mid>table_bottom(index))index->mid=table_bottom(index);
    return ODE_OK;
}
void table_column_label(const Document *d,const TableIndex *index,int column,char *out,unsigned size)
{
    if(column<0){snprintf(out,size,"x");return;}
    int item=index->columns[column];
    if(index->solutions && d->nic>1)snprintf(out,size,"y%d",item+1);
    else model_variable_label(d,index->solutions ? 0:item,out,size);
}
typedef struct {
    const Document *d;const TableIndex *index;TablePage *page;
    int family,direction;unsigned rows;bool have;double x,y[ODE_MAX_DIM];
} PageCollector;
static bool fill(double x,const double *y,uint32_t step,void *context)
{
    (void)step;PageCollector *c=context;
    for(unsigned row=0;row<c->rows;row++) {
        double target=c->page->row[row][0],lo=fmin(c->x,x),hi=fmax(c->x,x);
        bool exact=same_sample(c->d,target,x);
        if(!exact && (!c->have || target<lo || target>hi))continue;
        double fraction=exact || x==c->x ? 1:(target-c->x)/(x-c->x);
        for(int column=0;column<c->index->count;column++) {
            int item=c->index->columns[column];
            if(c->index->solutions && item!=c->family)continue;
            int state=c->index->solutions ? 0:item;
            double value=exact ? y[state]:c->y[state]+fraction*(y[state]-c->y[state]);
            if(isfinite(value) && fabs(value)<=1e100) {
                c->page->row[row][column+1]=value;c->page->valid[row]|=(uint16_t)(1u<<(column+1));
            }
        }
    }
    c->have=true;c->x=x;memcpy(c->y,y,(unsigned)c->d->dim*sizeof(double));
    double edge=c->page->row[c->direction>0 ? c->rows-1:0][0];
    return c->direction*(edge-x)>0;
}
void table_read_page(const Document *d,CompiledModel *m,const TableIndex *index,unsigned start,
    TablePage *page,OdeCancel cancel,void *cancel_ctx)
{
    memset(page,0,sizeof(*page));
    if(start>=index->total){page->result.status=ODE_BAD_INPUT;return;}
    unsigned rows=index->total-start;if(rows>TABLE_ROWS+1)rows=TABLE_ROWS+1;
    for(unsigned row=0;row<rows;row++){page->row[row][0]=table_x_at(index,start+row);page->valid[row]=1;}
    page->count=rows>TABLE_ROWS ? TABLE_ROWS:rows;page->more=rows>TABLE_ROWS;
    int families=index->solutions ? d->nic:1;
    for(int f=0;f<families;f++)for(int side=0;side<2;side++) {
        PageCollector c={.d=d,.index=index,.page=page,.family=f,.direction=side ? 1:-1,.rows=rows};
        OdeResult result=model_trajectory(d,m,f,c.direction,fill,&c,cancel,cancel_ctx);
        if(result.status!=ODE_OK && result.status!=ODE_SAMPLE_STOP && !ode_invalid_region(result.status)) {
            page->result=result;return;
        }
    }
    page->result.status=ODE_OK;
}
