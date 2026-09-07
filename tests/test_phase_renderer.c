#include "phase_graph.h"
#include "trace.h"
#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
unsigned long host_rhs_calls(void);
static Document d;
static CompiledModel m;
static unsigned hash(void)
{
    unsigned h=0;
    for(int i=0;i<DWIDTH*DHEIGHT;i++)h=h*31+gint_vram[i];
    return h;
}
static bool cancel(void *ctx){unsigned *n=ctx;return ++*n>=12;}
int main(void)
{
    model_defaults(&d,EQ_SYSTEM,2);d.solver.h=.01;d.solver.xmin=-2;d.solver.xmax=2;
    assert(model_compile(&d,&m).values==ODE_OK);
    assert(graph_render(&d,&m,true).status==ODE_OK);
    assert(trace_cache_matches(&d));unsigned long rhs=host_rhs_calls();
    d.view.phase=1;d.phase_field=0;d.enabled=0;
    assert(trace_cache_phase_window(&d,&d.phase_view));
    assert(graph_render(&d,&m,false).status==ODE_OK);
    assert(host_rhs_calls()==rhs);unsigned curve=hash();
    d.phase_field=1;assert(graph_render(&d,&m,false).status==ODE_OK);
    assert(hash()!=curve && host_rhs_calls()==rhs);
    unsigned field_hash=hash();d.phase_nullclines=1;
    assert(graph_render(&d,&m,false).status==ODE_OK);
    assert(hash()!=field_hash && host_rhs_calls()==rhs);
    unsigned full=hash(),polls=0;
    assert(graph_phase_preflight(&d,&m,cancel,&polls)==ODE_CANCELLED);
    assert(hash()==full); /* dry analysis owns no framebuffer pixels */
    assert(graph_phase_search(&d,&m,NULL,NULL)==ODE_OK);
    assert(graph_phase_results()->count==1);
    assert(graph_phase_results()->root[0].type==PHASE_CENTER_NEUTRAL);
    graph_phase_markers(&d,0);assert(hash()!=full);
    PhaseResults saved=*graph_phase_results();polls=0;unsigned marked=hash();
    assert(graph_phase_search(&d,&m,cancel,&polls)==ODE_CANCELLED);
    assert(!memcmp(&saved,graph_phase_results(),sizeof(saved)) && hash()==marked);
    ViewWindow time=d.view;OdeSettings solver=d.solver;
    assert(graph_zoom(&d.phase_view,.67,.2,-.2));
    assert(graph_render(&d,&m,false).status==ODE_OK);
    assert(!memcmp(&time,&d.view,sizeof(time)) && !memcmp(&solver,&d.solver,sizeof(solver)));
    assert(host_rhs_calls()==rhs);
    assert(graph_auto_window(&d,&m)==ODE_OK && host_rhs_calls()==rhs);
    strcpy(d.text[0],"x+y2");assert(model_compile(&d,&m).values==ODE_OK);
    assert(!phase_autonomous(&m));assert(graph_phase_preflight(&d,&m,NULL,NULL)==ODE_OK);
    assert(graph_phase_search(&d,&m,NULL,NULL)==ODE_BAD_INPUT);
    assert(!memcmp(&saved,graph_phase_results(),sizeof(saved)));
    strcpy(d.text[0],"sqrt(-1)");assert(model_compile(&d,&m).values==ODE_OK);
    assert(graph_phase_preflight(&d,&m,NULL,NULL)==ODE_OK); /* invalid cells skipped */
    graph_phase_reset();assert(!graph_phase_results()->count);
    puts("Phase renderer: same-stream cache, output-independent projection, field/null layers, no-RK4 pan/AUTO, windows, reference x and transactional cancel passed.");
}
