#include "table.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;
int main(void)
{
    model_defaults(&d,EQ_GENERAL,1);d.solver.xmin=-6;d.solver.xmax=6;
    for(int i=0;i<2;i++) {d.solver.h=i ? .001:.1;assert(model_preflight(&d,&d.solver).status==ODE_OK);}
    d.solver.h=.0001;assert(model_preflight(&d,&d.solver).status==ODE_STEP_LIMIT);
    d.solver.max_steps=60000;assert(model_preflight(&d,&d.solver).status==ODE_OK);
    d.solver.max_steps=20000;
    double bad[]={0,-.1,NAN,INFINITY};
    for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++){d.solver.h=bad[i];assert(model_preflight(&d,&d.solver).status==ODE_BAD_STEP);}
    d.solver.h=1e-300;assert(model_preflight(&d,&d.solver).status==ODE_STEP_LIMIT);
    d.solver.h=.0001;d.solver.xmin=0;d.solver.xmax=.01;
    assert(model_preflight(&d,&d.solver).status==ODE_OK);
    d.ic[0].x=10;assert(model_preflight(&d,&d.solver).status==ODE_STEP_LIMIT);
    model_defaults(&d,EQ_SYSTEM,9);d.solver.h=.001;
    assert(model_preflight(&d,&d.solver).status==ODE_OK);
    d.nic=9;for(int f=1;f<9;f++)d.ic[f]=d.ic[0];
    assert(model_preflight(&d,&d.solver).status==ODE_WORK_LIMIT);
    d.solver.h=.1;assert(model_preflight(&d,&d.solver).status==ODE_OK);
    model_defaults(&d,EQ_SEPARABLE,1);strcpy(d.text[0],"sinh(x)");strcpy(d.text[1],"y^2-1");d.ic[0].y[0]=0;
    assert(model_compile(&d,&m).expression.status==EXPR_OK);
    for(int step=1;step<=3;step+=2) {
        d.solver.step=step;TableIndex index;
        assert(table_index_build(&d,&m,&index,NULL,NULL)==ODE_OK);
        assert(index.low==ODE_DIVERGED && index.high==ODE_DIVERGED);
        for(int side=0;side<2;side++) {
            TablePage p;table_read_page(&d,&m,&index,side ? table_bottom(&index):0,&p,NULL,NULL);
            unsigned terminal=side ? p.count-1:0;double x=p.row[terminal][0];
            assert(p.result.status==ODE_OK && fabs(fabs(x)-4.4)<1e-12);
            assert(fabs(-tanh(cosh(x)-1))<=1);
            for(unsigned i=0;i<p.count;i++)assert((p.valid[i]&2) && isfinite(p.row[i][1]) && fabs(p.row[i][1])<=1e100);
        }
        TablePage p;table_read_page(&d,&m,&index,index.mid,&p,NULL,NULL);
        assert(p.row[3][0]==0 && p.row[3][1]==0);
    }
    /* Explicitly smaller fixed h is a user choice, never an automatic retry. */
    d.solver.h=.001;
    for(int dir=-1;dir<=1;dir+=2) {
        OdeResult r=model_trajectory(&d,&m,0,dir,NULL,NULL,NULL,NULL);
        assert(r.status==ODE_OK && fabs(r.y[0]+tanh(cosh(r.x)-1))<1e-8);
    }
    puts("Preflight range/IC/dimension bounds and bounded-solution Table regression passed.");
}
