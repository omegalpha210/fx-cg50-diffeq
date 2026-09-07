#include "graph.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
    Document d;CompiledModel m;
    model_defaults(&d,EQ_SYSTEM,2);
    strcpy(d.text[0],"1");strcpy(d.text[1],"1000");
    d.ic[0].y[0]=10;d.ic[0].y[1]=100;
    d.enabled=1;d.solver_custom=1;
    d.solver.xmin=-3;d.solver.xmax=3;
    d.view.xmin=-1;d.view.xmax=1;d.view.ymin=-.1;d.view.ymax=.1;
    assert(model_compile(&d,&m).values==ODE_OK);
    assert(graph_auto_window(&d,&m)==ODE_OK);
    assert(d.view.xmin==-1 && d.view.xmax==1);
    assert(d.view.ymin<9.1 && d.view.ymin>8);
    assert(d.view.ymax>10.9 && d.view.ymax<12);
    assert(d.solver.xmin==-3 && d.solver.xmax==3 && d.solver_custom);
    strcpy(d.text[0],"0");assert(model_compile(&d,&m).values==ODE_OK);
    assert(graph_auto_window(&d,&m)==ODE_OK && d.view.ymin<10 && d.view.ymax>10);
    ViewWindow before=d.view;
    d.enabled=0;assert(graph_auto_window(&d,&m)!=ODE_OK);
    assert(!memcmp(&before,&d.view,sizeof(before)));
    d.enabled=1;d.view.xmin=20;d.view.xmax=21;before=d.view;
    assert(graph_auto_window(&d,&m)!=ODE_OK && !memcmp(&before,&d.view,sizeof(before)));
    /* Failure beyond the current X window must not spoil an in-window fit. */
    d.view.xmin=-1;d.view.xmax=1;
    strcpy(d.text[0],"sqrt(2-x)");assert(model_compile(&d,&m).values==ODE_OK);
    assert(graph_auto_window(&d,&m)==ODE_OK);
    /* SYS2 phase is independent of TIME Output flags and has its own bounds. */
    d.view.phase=1;d.view.phase_x=0;d.view.phase_y=1;
    assert(graph_family_enabled(&d,0));
    d.enabled=3;assert(graph_family_enabled(&d,0));
    strcpy(d.text[0],"0");strcpy(d.text[1],"0");
    d.view.xmin=9;d.view.xmax=11;d.ic[0].y[1]=20;
    assert(model_compile(&d,&m).values==ODE_OK);
    d.phase_field=0;before=d.view;
    assert(graph_render(&d,&m,true).status==ODE_OK);
    assert(graph_auto_window(&d,&m)==ODE_OK);
    assert(!memcmp(&before,&d.view,sizeof(before)));
    assert(d.phase_view.xmin<10 && d.phase_view.xmax>10 && d.phase_view.ymin<20 && d.phase_view.ymax>20);
    d.view.xmin=-1;d.view.xmax=1;strcpy(d.text[0],"sqrt(-1)");
    assert(model_compile(&d,&m).values==ODE_OK);before=d.view;
    assert(graph_auto_window(&d,&m)!=ODE_OK && !memcmp(&before,&d.view,sizeof(before)));
    puts("AUTO Y: selected finite samples, fixed X/manual range, constants, empty and domain failure passed.");
    return 0;
}
