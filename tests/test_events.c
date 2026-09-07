#include "model.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;
unsigned long host_rhs_calls(void);
static void setup(int method,const char *rhs,const char *event)
{
    model_defaults(&d,EQ_GENERAL,1);strcpy(d.text[0],rhs);d.ic[0].y[0]=1;
    d.adaptive.method=method;d.solver.xmin=-50;d.solver.xmax=50;d.solver.sf=0;
    d.event.enabled=1;strcpy(d.event.text,event);d.event.action=EVENT_STOP;
}
static void compile(void)
{ModelError e=model_compile(&d,&m);assert(e.values==ODE_OK && e.expression.status==EXPR_OK);solver_report_begin(&d,&m);}
static OdeResult run(double target)
{return model_value_at(&d,&m,0,target,NULL,NULL);}
static bool cancel(void *ctx){return ++*(unsigned*)ctx>25;}
static bool cancel_refinement(void *ctx){return m.work.rhs>*(unsigned*)ctx;}
static bool visited(double x,const double *y,uint32_t step,void *ctx)
{(void)y;(void)step;double *last=ctx;assert(x>=*last);*last=x;return true;}
int main(void)
{
    for(int method=ODE_RK4;method<=ODE_RK45;method++) {
        setup(method,"y","y-10");d.event.direction=EVENT_RISING;compile();
        unsigned long rhs_before=host_rhs_calls();
        OdeResult r=run(4);assert(r.status==ODE_EVENT_STOP && fabs(r.x-log(10))<3e-6 && fabs(r.y[0]-10)<1e-10);
        assert(m.work.rhs==host_rhs_calls()-rhs_before);
        assert(m.work.accepted+m.work.rejected<=ODE_ATTEMPT_BUDGET);
        assert(m.event_hits==1 && solver_report()->markers.count==1);
        printf("%s exponential root %.14g error %.6g accepted %u rejected %u RHS %u event evals %u\n",
            method ? "RK45":"RK4",r.x,fabs(r.x-log(10)),m.work.accepted,m.work.rejected,m.work.rhs,m.event_evals);
        solver_report_end(&m,r.status);assert(solver_report()->status==ODE_EVENT_STOP);
        d.event.direction=EVENT_FALLING;compile();r=run(4);assert(r.status==ODE_OK && m.event_hits==0);
        setup(method,"-y","y-.5");d.event.direction=EVENT_FALLING;compile();r=run(2);
        assert(r.status==ODE_EVENT_STOP && fabs(r.x-log(2))<8e-7);
        setup(method,"y","y-.5");d.event.direction=EVENT_RISING;compile();r=run(-2);
        assert(r.status==ODE_EVENT_STOP && fabs(r.x+log(2))<8e-7);
        d.event.direction=EVENT_FALLING;compile();assert(run(-2).status==ODE_OK && m.event_hits==0);
        setup(method,"y^2","y-10");compile();r=run(2);
        printf("square method%d x%.15g y%.15g status%s hits%u unavailable%u\n",method,r.x,r.y[0],ode_status_text(r.status),m.event_hits,m.event_unavailable);
        assert(r.status==ODE_EVENT_STOP && fabs(r.x-.9)<1e-3 && fabs(r.y[0]-10)<1e-10);
        if(method==ODE_RK4) {
            double coarse=fabs(r.x-.9);d.solver.h=.05;compile();r=run(2);
            assert(r.status==ODE_EVENT_STOP && fabs(r.x-.9)<coarse/8);
        }
        for(int direction=EVENT_ANY;direction<=EVENT_FALLING;direction++)for(int sign=-1;sign<=1;sign+=2) {
            setup(method,"1","y");d.ic[0].y[0]=0;d.event.direction=(uint8_t)direction;compile();r=run(sign);
            assert(r.status==(direction==EVENT_FALLING ? ODE_OK:ODE_EVENT_STOP));
            if(r.status==ODE_EVENT_STOP)assert(r.x==0 && r.y[0]==0 && m.event_hits==1);
        }
        setup(method,"1","x-1+abs(x-1)");d.event.direction=EVENT_RISING;compile();double last=0;
        r=model_trajectory(&d,&m,0,1,visited,&last,NULL,NULL);
        assert(r.status==ODE_EVENT_STOP && r.x==0 && last==0); /* No published zero-plateau points past STOP. */
        d.event.direction=EVENT_FALLING;compile();last=0;
        r=model_trajectory(&d,&m,0,1,visited,&last,NULL,NULL);
        assert(r.status==ODE_OK && last==d.solver.xmax && m.event_hits==0);
        setup(method,"cos(10*x)","sin(10*x)");d.event.action=EVENT_MARK;compile();r=run(40);
        assert(r.status==ODE_OK && m.event_hits>100 && solver_report()->markers.count==EVENT_CAPACITY);
        for(unsigned i=1;i<EVENT_CAPACITY;i++)assert(solver_report()->markers.point[i].x>solver_report()->markers.point[i-1].x);
        setup(method,"0","1/(x-.55)");d.event.action=EVENT_MARK;compile();r=run(2);
        assert(r.status==ODE_OK && m.event_hits==0 && m.event_unavailable>0);
        setup(method,"0","sqrt(1-x)-.5");d.event.action=EVENT_MARK;compile();r=run(2);
        assert(r.status==ODE_OK && m.event_hits==1 && m.event_unavailable>0);
        setup(method,"y","y-10");compile();unsigned polls=0;
        r=model_value_at(&d,&m,0,4,cancel,&polls);assert(r.status==ODE_CANCELLED && m.event_hits==0);
        setup(method,"y","y-10");d.nic=10;
        for(int f=0;f<10;f++)d.ic[f].y[0]=1+f*.1;
        compile();for(int f=0;f<10;f++) {
            r=model_value_at(&d,&m,f,4,NULL,NULL);
            assert(r.status==ODE_EVENT_STOP && fabs(r.x-log(10/d.ic[f].y[0]))<3e-6);
        }
        assert(m.event_hits==10 && solver_report()->markers.count==10);
        setup(method,"y","y-1.05");compile();unsigned first_rhs=method==ODE_RK45 ? 7:4;
        r=model_value_at(&d,&m,0,1,cancel_refinement,&first_rhs);
        assert(r.status==ODE_CANCELLED && r.x==0 && r.y[0]==1 && m.event_hits==0 && m.work.rhs>first_rhs);
        setup(method,"y","y-10");compile();m.event_evals=EVENT_EVAL_BUDGET;
        r=run(4);assert(r.status==ODE_WORK_LIMIT && r.x==0 && m.work.rhs==0);
        for(int direction=EVENT_RISING;direction<=EVENT_FALLING;direction++)for(int sign=-1;sign<=1;sign+=2) {
            model_defaults(&d,EQ_SECOND,2);d.adaptive.method=method;d.event.enabled=1;
            strcpy(d.event.text,"y");d.event.direction=(uint8_t)direction;
            d.solver.xmin=-40;d.solver.xmax=40;compile();r=run(sign*40);
            assert(r.status==ODE_OK && m.event_hits==(unsigned)((direction==EVENT_FALLING)==(sign>0) ? 7:6));
            for(unsigned i=0;i<solver_report()->markers.count;i++)assert(solver_report()->markers.point[i].direction==direction);
        }
        model_defaults(&d,EQ_SYSTEM,9);d.adaptive.method=method;d.event.enabled=1;d.event.action=EVENT_STOP;
        for(int i=0;i<9;i++){strcpy(d.text[i],"1");d.ic[0].y[i]=0;}
        strcpy(d.event.text,"y9-1");compile();r=run(2);
        assert(r.status==ODE_EVENT_STOP && fabs(r.x-1)<2e-12);
        for(int i=0;i<9;i++)assert(fabs(r.y[i]-1)<2e-12);
        model_defaults(&d,EQ_HIGHER,9);d.event.enabled=1;strcpy(d.event.text,"y8+x+y");compile();
        strcpy(d.event.text,"y9");assert(model_compile(&d,&m).expression.status==EXPR_VARIABLE);
        strcpy(d.event.text,"y8+x+y");d.adaptive.method=method;compile();OdeResult higher=run(.7);
        assert(model_convert_system(&d) && !strcmp(d.event.text,"y9+x+y1"));compile();r=run(.7);
        assert(r.status==higher.status && r.x==higher.x && !memcmp(r.y,higher.y,sizeof(r.y)));
        model_defaults(&d,EQ_HIGHER,9); /* Even OFF drafts convert atomically. */
        for(int i=0;i<EXPR_TEXT-2;i+=2){d.event.text[i]='y';d.event.text[i+1]='+';}
        d.event.text[EXPR_TEXT-2]='y';d.event.text[EXPR_TEXT-1]=0;
        Document previous=d;assert(!model_convert_system(&d) && !memcmp(&d,&previous,sizeof(d)));
        setup(method,"y","nonsense");d.event.enabled=0;compile();r=run(.7);
        assert(r.status==ODE_OK && m.event_evals==0 && m.event_hits==0 && m.work.rhs>0);
        d.event.enabled=1;assert(model_compile(&d,&m).expression.status!=EXPR_OK);
        strcpy(d.event.text,"log(-1)");assert(model_compile(&d,&m).expression.status==EXPR_DOMAIN);
        strcpy(d.event.text,"");assert(model_compile(&d,&m).expression.status!=EXPR_OK);
    }
    model_defaults(&d,EQ_SECOND,2);d.adaptive.method=ODE_RK45;d.event.enabled=1;
    strcpy(d.event.text,"y");d.solver.xmin=-40;d.solver.xmax=40;d.solver.h=10;compile();
    OdeResult r=run(40);assert(r.status==ODE_OK && m.work.rejected>0 && m.event_hits==13);
    for(unsigned i=0;i<solver_report()->markers.count;i++)
        assert(fabs(solver_report()->markers.point[i].x-((double)i+.5)*acos(-1))<6e-6);
    puts("Events: both methods, direction/backward, initial roots, bounded MARK, invalid gaps, STOP, ten IC, parser and cancellation passed.");
}
