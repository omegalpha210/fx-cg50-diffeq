#include "gsolve.h"
#include "table.h"
#include "trace.h"
#include "ui.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;
static void compile(void)
{ModelError e=model_compile(&d,&m);assert(e.values==ODE_OK && e.expression.status==EXPR_OK);}
static void near(double a,double b,double tolerance)
{if(fabs(a-b)>tolerance)fprintf(stderr,"%.17g expected %.17g\n",a,b);assert(fabs(a-b)<=tolerance);}
static void event_endpoints(void)
{
    for(int method=ODE_RK4;method<=ODE_RK45;method++) {
        model_defaults(&d,EQ_GENERAL,1);d.adaptive.method=method;d.solver.sf=0;
        strcpy(d.text[0],"y");d.event.enabled=1;d.event.action=EVENT_STOP;
        strcpy(d.event.text,"y-10");compile();
        GsolveResults r=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_XCAL,10,NULL,NULL);
        assert(r.status==ODE_OK && r.count==1);near(r.point[0].x,log(10),3e-6);
        near(r.point[0].y,10,1e-9);
        strcpy(d.event.text,"y-.1");compile();
        r=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_XCAL,.1,NULL,NULL);
        assert(r.status==ODE_OK && r.count==1);near(r.point[0].x,-log(10),3e-6);
        strcpy(d.text[0],"1");strcpy(d.event.text,"y");
        for(int sign=-1;sign<=1;sign+=2) {
            d.ic[0].y[0]=sign;compile();
            r=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_ROOT,0,NULL,NULL);
            assert(r.status==ODE_OK && r.count==1);near(r.point[0].x,-sign,1e-10);
        }
        model_defaults(&d,EQ_SECOND,2);d.adaptive.method=method;d.event.enabled=1;
        d.event.action=EVENT_STOP;strcpy(d.event.text,"y-y1");compile();
        r=gsolve_intersections(&d,&m,(GsolveCurve){0,0},(GsolveCurve){0,1},NULL,NULL);
        assert(r.status==ODE_OK && r.count==2);
        near(r.point[0].x,-acos(-1)/4,6e-6);near(r.point[1].x,3*acos(-1)/4,6e-6);
        /* Direction filtering can retain just the backward/forward endpoint. */
        for(int direction=EVENT_RISING;direction<=EVENT_FALLING;direction++) {
            d.event.direction=(uint8_t)direction;compile();
            r=gsolve_intersections(&d,&m,(GsolveCurve){0,0},(GsolveCurve){0,1},NULL,NULL);
            assert(r.status==ODE_OK && r.count==3);
            double first=direction==EVENT_RISING ? -acos(-1)/4:-5*acos(-1)/4;
            for(int i=0;i<3;i++)near(r.point[i].x,first+i*acos(-1),6e-6);
        }
        /* Independent vector IVPs exercise the cross-family path at a common
           STOP boundary without borrowing another family's state. */
        strcpy(d.text[0],"0");strcpy(d.text[1],"0");d.ic[0].y[1]=1;
        d.nic=2;d.event.direction=EVENT_ANY;strcpy(d.event.text,"y");
        d.ic[1]=d.ic[0];d.ic[1].y[0]=-1;d.ic[1].y[1]=-1;compile();
        r=gsolve_intersections(&d,&m,(GsolveCurve){0,0},(GsolveCurve){1,0},NULL,NULL);
        assert(r.status==ODE_OK && r.count==1);near(r.point[0].x,-1,1e-10);
        /* Primary y=x continues until .75; secondary y=1-x stops at .25.
           Their apparent x=.5 crossing is outside the common valid domain. */
        d.ic[0].y[0]=0;d.ic[1].y[0]=1;d.solver.h=.25;
        strcpy(d.event.text,"y-.75");compile();
        r=gsolve_intersections(&d,&m,(GsolveCurve){0,0},(GsolveCurve){1,0},NULL,NULL);
        assert(r.status==ODE_OK && r.count==0);
        /* No absolute epsilon may turn a tiny nonzero constant into a root.
           STOP of an unrelated Event is not a ROOT or target crossing either. */
        model_defaults(&d,EQ_GENERAL,1);d.adaptive.method=method;d.solver.sf=0;
        strcpy(d.text[0],"0");d.event.enabled=1;d.event.action=EVENT_STOP;
        strcpy(d.event.text,"x-1");d.ic[0].y[0]=1e-90;compile();
        r=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_ROOT,0,NULL,NULL);
        assert(r.status==ODE_OK && !r.count);
        strcpy(d.text[0],"1");d.ic[0].y[0]=2;compile();
        r=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_XCAL,4,NULL,NULL);
        assert(r.status==ODE_OK && !r.count);
        d.ic[0].y[0]=1e12;d.solver.h=.125;compile();
        r=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_XCAL,1e12+1.001,NULL,NULL);
        assert(r.status==ODE_OK && !r.count); /* No absolute-y epsilon. */
        d.ic[0].y[0]=0;d.solver.xmin=d.view.xmin=-1e9;d.solver.xmax=d.view.xmax=1e9;
        if(method==ODE_RK45) {
            compile();r=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_XCAL,1.5,NULL,NULL);
            assert(r.status==ODE_OK && !r.count); /* No whole-view tolerance. */
        }
        d.solver.xmin=d.view.xmin=-6;d.solver.xmax=d.view.xmax=6;
        strcpy(d.text[0],"2*(x-1.000001)");d.ic[0].y[0]=1.000001*1.000001;compile();
        r=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_ROOT,0,NULL,NULL);
        assert(r.status==ODE_OK && !r.count); /* No coarse secant allowance. */
        strcpy(d.text[0],"1");d.ic[0].y[0]=2;compile();
        /* Coincident distinct-family curves have a continuum of intersections;
           retain bounded valid representatives without inventing a uniqueness
           policy. Endpoint Y-CAL cannot evaluate past the stopping Event. */
        d.nic=2;d.ic[1]=d.ic[0];compile();
        r=gsolve_intersections(&d,&m,(GsolveCurve){0,0},(GsolveCurve){1,0},NULL,NULL);
        assert(r.status==ODE_OK && r.count>=1 && r.count<=GSOLVE_MAX_RESULTS);
        for(int i=0;i<r.count;i++)near(r.point[i].y,2+r.point[i].x,1e-10);
        GsolvePoint point;assert(gsolve_ycal(&d,&m,(GsolveCurve){1,0},1,&point,NULL,NULL)==ODE_OK);
        near(point.y,3,1e-12);
        GsolvePoint kept=point;
        assert(gsolve_ycal(&d,&m,(GsolveCurve){1,0},1.001,&point,NULL,NULL)==ODE_EVENT_STOP);
        assert(!memcmp(&kept,&point,sizeof(point)));
    }
}
static void families_and_table(void)
{
    const int counts[]={1,2,5,10};
    for(int method=ODE_RK4;method<=ODE_RK45;method++)for(unsigned n=0;n<4;n++) {
        model_defaults(&d,EQ_GENERAL,1);d.adaptive.method=method;d.solver.sf=0;
        strcpy(d.text[0],"1");d.nic=counts[n];d.solver.xmin=d.view.xmin=-2;
        d.solver.xmax=d.view.xmax=2;d.solver_custom=1;
        for(int f=0;f<d.nic;f++)d.ic[f].y[0]=f+1;
        compile();assert(graph_render(&d,&m,true).status==ODE_OK);
        SolverReport before=*solver_report();
        TableIndex index;TablePage page;assert(table_index_build(&d,&m,&index,NULL,NULL)==ODE_OK);
        assert(index.count==d.nic && index.solutions && gsolve_curve_count(&d)==d.nic);
        for(unsigned i=1;i<index.total;i++)assert(table_x_at(&index,i)>table_x_at(&index,i-1));
        const unsigned starts[]={0,index.mid,table_bottom(&index)};
        for(unsigned p=0;p<3;p++) {
            table_read_page(&d,&m,&index,starts[p],&page,NULL,NULL);assert(page.result.status==ODE_OK);
            for(unsigned row=0;row<page.count;row++) {
                assert(page.valid[row]==(1u<<(d.nic+1))-1);
                for(int f=0;f<d.nic;f++)near(page.row[row][f+1],f+1+page.row[row][0],1e-10);
            }
        }
        for(int f=0;f<d.nic;f++) {
            GsolveCurve curve;GsolvePoint point;assert(gsolve_curve_at(&d,f,&curve));
            assert(curve.family==f && curve.variable==0);
            assert(gsolve_ycal(&d,&m,curve,0,&point,NULL,NULL)==ODE_OK);near(point.y,f+1,1e-12);
        }
        GsolveCurve invalid;assert(!gsolve_curve_at(&d,d.nic,&invalid) && !gsolve_curve_at(&d,-1,&invalid));
        assert(gsolve_intersections(&d,&m,(GsolveCurve){0,0},(GsolveCurve){0,0},NULL,NULL).status==ODE_BAD_INPUT);
        if(d.nic>1)assert(gsolve_intersections(&d,&m,(GsolveCurve){0,0},(GsolveCurve){d.nic-1,0},NULL,NULL).count==0);
        assert(!memcmp(&before,solver_report(),sizeof(before)));
        d.ic_enabled=0;assert(gsolve_curve_count(&d)==0);
        assert(table_index_build(&d,&m,&index,NULL,NULL)==ODE_OK && index.count==0);
        table_read_page(&d,&m,&index,index.mid,&page,NULL,NULL);
        for(unsigned row=0;row<page.count;row++)assert(page.valid[row]==1);
        d.nic=0;assert(gsolve_curve_count(&d)==0);
        assert(table_index_build(&d,&m,&index,NULL,NULL)==ODE_BAD_INPUT);
    }
}
static void boundaries(void)
{
    for(int method=ODE_RK4;method<=ODE_RK45;method++) {
        model_defaults(&d,EQ_SYSTEM,9);d.adaptive.method=method;d.enabled=0x155;
        d.solver.xmin=d.view.xmin=-1;d.solver.xmax=d.view.xmax=1;
        for(int i=0;i<9;i++){snprintf(d.text[i],EXPR_TEXT,"%d",i+1);d.ic[0].y[i]=0;}
        compile();TableIndex index;TablePage page;
        assert(table_index_build(&d,&m,&index,NULL,NULL)==ODE_OK && index.count==5);
        table_read_page(&d,&m,&index,table_bottom(&index),&page,NULL,NULL);
        for(unsigned row=0;row<page.count;row++)for(int col=0;col<5;col++)
            near(page.row[row][col+1],(2*col+1)*page.row[row][0],1e-10);
        assert(page.row[page.count-1][0]==1 && page.valid[page.count-1]==63);
        assert(gsolve_curve_count(&d)==5);GsolveCurve curve;
        assert(gsolve_curve_at(&d,4,&curve) && curve.variable==8 && !gsolve_curve_at(&d,5,&curve));
        model_defaults(&d,EQ_GENERAL,1);d.adaptive.method=method;d.ic[0].y[0]=0;
        strcpy(d.text[0],"20*cos(20*x)");d.solver.h=.01;d.solver.sf=0;
        d.solver.xmin=d.view.xmin=-3;d.solver.xmax=d.view.xmax=3;compile();
        GsolveResults roots=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_ROOT,0,NULL,NULL);
        assert(roots.status==ODE_OK && roots.count==GSOLVE_MAX_RESULTS);
        for(int i=0;i<roots.count;i++) {
            assert(isfinite(roots.point[i].x) && isfinite(roots.point[i].y));
            assert(roots.point[i].x>=-3 && roots.point[i].x<=3);
            if(i)assert(roots.point[i].x>roots.point[i-1].x);
            near(sin(20*roots.point[i].x),0,2e-6);
        }
    }
}
int main(void)
{
    event_endpoints();families_and_table();boundaries();
    puts("Full consumer audit: both methods, terminal Event ROOT/XCAL/ICPT, tiny nonroot, IC1/2/5/10 Table/G-Solve, shared hidden outputs, SYS9 sparse columns and result capacity passed.");
    return 0;
}
