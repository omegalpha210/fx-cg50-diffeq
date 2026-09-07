#include "model.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;
static CompiledModel m;
static void compile(void)
{ ModelError e=model_compile(&d,&m);assert(e.values==ODE_OK && e.expression.status==EXPR_OK); }
static OdeResult solve(double target)
{ compile(); return ode_integrate(model_rhs,&m,d.dim,d.ic[0].x,d.ic[0].y,target,&d.solver,NULL,NULL,NULL,NULL); }
int main(void)
{
    model_defaults(&d,EQ_GENERAL,1);
    assert(!d.solver_custom && d.solver.xmin==-6 && d.solver.xmax==6);
    d.view.xmin=-8.8;d.view.xmax=4.2;model_sync_solver_window(&d);
    assert(d.solver.xmin==-8 && d.solver.xmax==4);
    d.solver_custom=1;d.solver.xmin=-2;d.solver.xmax=3;
    d.view.xmin=-20;d.view.xmax=20;model_sync_solver_window(&d);
    assert(d.solver.xmin==-2 && d.solver.xmax==3);
    for(int k=EQ_SEPARABLE;k<=EQ_SYSTEM;k++) {
        model_defaults(&d,(EquationKind)k,4); d.solver.h=.01;
        OdeResult r=solve(1); assert(r.status==ODE_OK);
        if(k==EQ_SEPARABLE) assert(fabs(r.y[0]+tanh(1))<1e-9);
        if(k==EQ_LINEAR) assert(fabs(r.y[0]-(1-3*exp(-.5)))<1e-9);
        if(k==EQ_BERNOULLI) assert(fabs(r.y[0]-2/(1+exp(-2)))<1e-8);
        if(k==EQ_SECOND) assert(fabs(r.y[0]-cos(1))<1e-9);
    }
    model_defaults(&d,EQ_HIGHER,4);
    d.ic[0].y[0]=0;d.ic[0].y[1]=-2;d.ic[0].y[3]=3;
    OdeResult r=solve(2); assert(r.status==ODE_OK && fabs(r.y[0])<1e-12);
    r=solve(-2); assert(r.status==ODE_OK && fabs(r.y[0])<1e-12);
    model_defaults(&d,EQ_HIGHER,3);strcpy(d.text[0],"sin(x)-Y1-Y2");
    d.ic[0].y[0]=0;d.ic[0].y[1]=1;
    OdeResult before=solve(2);
    assert(model_convert_system(&d));
    assert(!strcmp(d.text[0],"y2") && !strcmp(d.text[1],"y3") && !strcmp(d.text[2],"sin(x)-y2-y3"));
    r=solve(2);for(int i=0;i<3;i++) assert(fabs(before.y[i]-r.y[i])<1e-14);
    for(int n=1;n<=9;n++) {
        model_defaults(&d,EQ_SYSTEM,n);d.solver.h=.01;
        for(int i=0;i<n;i++) {snprintf(d.text[i],EXPR_TEXT,"-y%d",i+1); d.ic[0].y[i]=i+1;}
        r=solve(1);assert(r.status==ODE_OK);
        for(int i=0;i<n;i++) assert(fabs(r.y[i]-(i+1)*exp(-1))<1e-8);
    }
    model_defaults(&d,EQ_HIGHER,9); memset(d.ic[0].y,0,sizeof(d.ic[0].y));d.ic[0].y[8]=40320;
    d.solver.h=.01; r=solve(1); printf("9th order polynomial y(1)=%.12g\n",r.y[0]); assert(r.status==ODE_OK && fabs(r.y[0]-1)<1e-6);
    model_defaults(&d,EQ_LINEAR,1);strcpy(d.text[0],"y");
    assert(model_compile(&d,&m).expression.status==EXPR_VARIABLE);
    model_defaults(&d,EQ_SEPARABLE,1);strcpy(d.text[1],"x");
    assert(model_compile(&d,&m).expression.status==EXPR_VARIABLE);
    model_defaults(&d,EQ_GENERAL,1); d.nic=0;assert(model_validate(&d)==ODE_OK);
    d.solver.sf=0;assert(model_validate(&d)!=ODE_OK);
    printf("All 7 modes, dimensions 1-9, IC conversion and PDF examples passed.\nDocument=%zu; compiled model=%zu bytes (host).\n",sizeof(d),sizeof(m));
    return 0;
}
