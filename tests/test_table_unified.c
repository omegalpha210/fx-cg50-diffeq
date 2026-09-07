#include "table.h"
#include "initial.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;static TableIndex table_index;static TablePage page;
static void prepare(void){assert(model_compile(&d,&m).expression.status==EXPR_OK);assert(table_index_build(&d,&m,&table_index,NULL,NULL)==ODE_OK);}
static void ordered(void)
{
    for(unsigned i=1;i<table_index.total;i++)assert(table_x_at(&table_index,i)>table_x_at(&table_index,i-1));
    for(unsigned i=0;i<table_index.total;i+=TABLE_ROWS) {
        table_read_page(&d,&m,&table_index,i,&page,NULL,NULL);assert(page.result.status==ODE_OK);
        for(unsigned r=0;r<page.count;r++) {
            assert(page.valid[r]==(1u<<(table_index.count+1))-1);
            assert(page.row[r][0]>=table_index.xmin && page.row[r][0]<=table_index.xmax);
        }
    }
}
int main(void)
{
    model_defaults(&d,EQ_SEPARABLE,1);d.solver.xmin=-5;d.solver.xmax=5;
    InitialValues v;assert(initial_values_parse("{0,1}",&v)==IC_LIST_OK);initial_values_apply(&d,&v);prepare();
    assert(table_index.count==2 && table_index.solutions && table_x_at(&table_index,table_index.mid+3)==0);ordered();
    table_read_page(&d,&m,&table_index,table_index.mid,&page,NULL,NULL);
    assert(page.row[3][0]==0 && page.row[3][1]==0 && page.row[3][2]==1);
    d.nic=1;strcpy(d.text[0],"sinh(x)");d.solver.xmin=-6;d.solver.xmax=6;d.solver.step=3;prepare();
    assert(ode_invalid_region(table_index.low) && ode_invalid_region(table_index.high));
    assert(fabs(table_index.xmin+4.4)<1e-12 && fabs(table_index.xmax-4.4)<1e-12);
    assert(table_x_at(&table_index,0)==table_index.xmin && table_x_at(&table_index,table_index.total-1)==table_index.xmax);
    ordered();unsigned center=table_index.mid;
    for(int i=0;i<20;i++) {
        table_read_page(&d,&m,&table_index,0,&page,NULL,NULL);
        table_read_page(&d,&m,&table_index,table_bottom(&table_index),&page,NULL,NULL);
        assert(!page.more);table_read_page(&d,&m,&table_index,table_index.mid,&page,NULL,NULL);assert(table_index.mid==center);
    }
    model_defaults(&d,EQ_GENERAL,1);strcpy(d.text[0],"0");
    const double cases[][5]={{0,-2,8,.1,1},{0,2,8,.1,1},{.05,-1,1,.1,2},
        {0,-1e-300,1e-300,1e-301,1},{0,-6,6,1e308,10000},{0,.35,.36,.1,3}};
    for(unsigned c=0;c<sizeof(cases)/sizeof(*cases);c++) {
        d.ic[0].x=cases[c][0];d.solver.xmin=cases[c][1];d.solver.xmax=cases[c][2];
        d.solver.h=cases[c][3];d.solver.step=(int)cases[c][4];prepare();ordered();
        double target=c<2 ? (d.solver.xmin+d.solver.xmax)/2:0;
        if(c<2)assert(fabs(table_x_at(&table_index,table_index.mid+3)-target)<1e-12);
        if(c==2)assert(table_x_at(&table_index,table_index.mid+3)==0);
    }
    strcpy(d.text[0],"1");d.ic[0].x=.05;d.solver.xmin=-1;d.solver.xmax=1;d.solver.h=.1;d.solver.step=2;
    prepare();table_read_page(&d,&m,&table_index,table_index.mid,&page,NULL,NULL);
    assert(page.row[3][0]==0 && fabs(page.row[3][1]-.95)<1e-14 && d.solver.h==.1 && d.solver.step==2);
    /* Different reachable lengths: shared x rows with explicit missing cells. */
    model_defaults(&d,EQ_SEPARABLE,1);assert(initial_values_parse("{2,1}",&v)==IC_LIST_OK);initial_values_apply(&d,&v);prepare();
    assert(ode_invalid_region(table_index.high));table_read_page(&d,&m,&table_index,table_bottom(&table_index),&page,NULL,NULL);
    assert((page.valid[6]&2)==0 && (page.valid[6]&4) && page.row[6][2]==1);
    /* Accumulated RK4 coordinates and direct grid arithmetic diverge slightly
       on long runs. Each family's accepted terminal point must remain usable. */
    d.solver.h=.0002;d.solver.max_steps=100000;prepare();
    OdeResult terminal=model_trajectory(&d,&m,0,1,NULL,NULL,NULL,NULL);
    assert(ode_invalid_region(terminal.status));
    unsigned terminal_row=table_nearest(&table_index,terminal.x);
    table_read_page(&d,&m,&table_index,terminal_row,&page,NULL,NULL);
    assert(page.valid[0]&2);
    assert(fabs(page.row[0][1]-terminal.y[0])<=1e-10*fabs(terminal.y[0]));
    model_defaults(&d,EQ_GENERAL,1);strcpy(d.text[0],"0");
    d.solver.h=.0001;d.solver.max_steps=100000;prepare();
    for(unsigned i=0;i<table_index.total;i++) {
        if(i)assert(table_x_at(&table_index,i)>table_x_at(&table_index,i-1));
    }
    for(int i=0;i<3;i++) {
        table_read_page(&d,&m,&table_index,i==0 ? 0:(i==1 ? table_index.mid:table_bottom(&table_index)),&page,NULL,NULL);
        for(unsigned r=0;r<page.count;r++)assert(page.valid[r]==3 && page.row[r][1]==1);
    }
    /* A relative-x epsilon would conflate many distinct states at a large x0. */
    model_defaults(&d,EQ_GENERAL,1);strcpy(d.text[0],"1");
    d.ic[0].x=1e16;d.solver.xmin=1e16;d.solver.xmax=1e16+100;d.solver.h=2;
    prepare();ordered();table_read_page(&d,&m,&table_index,0,&page,NULL,NULL);
    for(unsigned r=0;r<page.count;r++)assert(page.row[r][1]==1+2*r);
    d.solver.h=1.1;
    assert(table_index_build(&d,&m,&table_index,NULL,NULL)==ODE_BAD_STEP);
    puts("Unified Table: ascending grid/terminal merge, stable MID/TOP/BTM, asymmetric/zero/nearest anchors, multiple solution columns, tiny/large steps and missing valid cells passed.");
}
