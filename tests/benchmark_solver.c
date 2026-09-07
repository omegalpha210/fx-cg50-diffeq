/* Host CPU measurements, not fx-CG50 performance predictions. */
#include "model.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
static Document d;static CompiledModel m;
static void measure(int dim,int families,double h,double end)
{
    model_defaults(&d,EQ_SYSTEM,dim);d.nic=families;
    for(int f=1;f<families;f++)d.ic[f]=d.ic[0];
    d.solver.h=h;d.solver.xmin=-end;d.solver.xmax=end;
    ModelWork plan=model_preflight(&d,&d.solver);
    if(plan.status!=ODE_OK) {
        printf("dim=%d ICs=%d h=%g X=+-%g: rejected before RK4 (%s)\n",dim,families,h,end,ode_status_text(plan.status));return;
    }
    assert(model_compile(&d,&m).expression.status==EXPR_OK);
    clock_t start=clock();unsigned steps=0;
    for(int repetition=0;repetition<5;repetition++)for(int f=0;f<families;f++)for(int dir=-1;dir<=1;dir+=2) {
        OdeResult r=model_trajectory(&d,&m,f,dir,NULL,NULL,NULL,NULL);
        assert(r.status==ODE_OK);steps+=r.steps;
    }
    double ms=1000.0*(double)(clock()-start)/CLOCKS_PER_SEC/5;
    printf("dim=%d ICs=%d h=%g X=+-%g: planned=%u state-steps=%u actual=%u mean host CPU=%.3f ms (5 runs)\n",
        dim,families,h,end,plan.steps,plan.work,steps/5,ms);
}
int main(void)
{
    puts("Clang host/UBSan; no target latency claim. Per-path Max Steps=20000, total steps=200000, state-steps=600000.");
    measure(1,1,.1,6);measure(1,1,.001,6);measure(9,1,.001,6);
    measure(1,9,.001,6);measure(9,9,.001,6);measure(9,9,.1,6);
    measure(1,1,.0001,.01);measure(1,1,.0001,6);
}
