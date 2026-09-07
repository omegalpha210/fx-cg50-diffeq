#include "table.h"
#include <string.h>
typedef struct {const Document *d;unsigned skip,seen;TablePage *page;} Collector;
static bool collect(double x,const double *y,uint32_t step,void *ctx)
{
    Collector *c=ctx;
    if(step%(unsigned)c->d->solver.step && x!=c->d->solver.xmin && x!=c->d->solver.xmax) return true;
    if(c->seen++<c->skip) return true;
    unsigned row=c->page->count++;
    c->page->step[row]=step;
    c->page->row[row][0]=x;
    memcpy(&c->page->row[row][1],y,(unsigned)c->d->dim*sizeof(double));
    return c->page->count<TABLE_ROWS+1;
}
typedef struct {const Document *d;unsigned count;} Counter;
static bool count_point(double x,const double *y,uint32_t step,void *ctx)
{
    (void)y;Counter *c=ctx;
    if(step%(unsigned)c->d->solver.step && x!=c->d->solver.xmin && x!=c->d->solver.xmax)return true;
    c->count++;return true;
}
unsigned table_last_page(const Document *d,CompiledModel *m,int family,int direction,
    OdeStatus *status,OdeCancel cancel,void *cancel_ctx)
{
    Counter counter={.d=d};
    OdeResult result=model_trajectory(d,m,family,direction,count_point,&counter,cancel,cancel_ctx);
    if(status)*status=result.status;
    return counter.count ? ((counter.count-1)/TABLE_ROWS)*TABLE_ROWS:0;
}
void table_page(const Document *d,CompiledModel *m,int family,int direction,unsigned start,
    TablePage *page,OdeCancel cancel,void *cancel_ctx)
{
    memset(page,0,sizeof(*page));
    Collector c={.d=d,.skip=start,.page=page};
    page->result=model_trajectory(d,m,family,direction,collect,&c,cancel,cancel_ctx);
    page->more=page->count>TABLE_ROWS;
    if(page->more)page->count=TABLE_ROWS;
}
