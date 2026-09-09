#include "graph.h"
#include "trace.h"
#include "table.h"
#include "gsolve.h"
#include "ui.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;
void host_cancel_after(unsigned polls);
unsigned long host_rhs_calls(void);
static void compile(void){ModelError e=model_compile(&d,&m);assert(e.values==ODE_OK && e.expression.status==EXPR_OK);}
static unsigned pixels(void){unsigned hash=0;for(int i=0;i<DWIDTH*DHEIGHT;i++)hash=31*hash+gint_vram[i];return hash;}
int main(void)
{
    for(int method=ODE_RK4;method<=ODE_RK45;method++) {
        model_defaults(&d,EQ_GENERAL,1);strcpy(d.text[0],"y");d.ic[0].y[0]=1;d.solver.sf=0;
        d.adaptive.method=method;d.event.enabled=1;d.event.action=EVENT_STOP;strcpy(d.event.text,"y-10");
        d.view.ymin=-1;d.view.ymax=12;
        compile();GraphResult graph=graph_render(&d,&m,true);
        assert(graph.status==ODE_EVENT_STOP && solver_report()->status==ODE_EVENT_STOP && solver_report()->hits==1);
        assert(solver_report()->work.rhs>0 && solver_report()->work.min_h<=solver_report()->work.max_h);
        int marker_x,marker_y;const EventMarker *mark=&solver_report()->markers.point[0];
        assert(graph_point(&d.view,mark->x,mark->y[0],&marker_x,&marker_y));
        assert(gint_vram[(UI_Y+marker_y)*DWIDTH+UI_X+marker_x]==C_RGB(31,15,0));
        assert(trace_prepare(&d,&m,0,0));TracePoint point;assert(trace_point_near(0,&point));
        assert(trace_navigate(&d,&m,6,true,&point)==ODE_EVENT_STOP);
        assert(fabs(point.x-log(10))<3e-6 && fabs(point.y[0]-10)<1e-10);
        OdeSettings extent=*trace_extent();double terminal=point.x;unsigned rendered=pixels();
        unsigned before=m.work.rhs;
        assert(trace_navigate(&d,&m,100,false,&point)==ODE_EVENT_STOP && point.x==terminal && m.work.rhs==before);
        assert(!memcmp(trace_extent(),&extent,sizeof(extent)) && pixels()==rendered);
        TableIndex index;TablePage page;assert(table_index_build(&d,&m,&index,NULL,NULL)==ODE_OK);
        assert(index.high==ODE_EVENT_STOP && fabs(index.xmax-log(10))<3e-6);
        table_read_page(&d,&m,&index,table_bottom(&index),&page,NULL,NULL);
        assert(page.result.status==ODE_OK && page.row[page.count-1][0]==index.xmax);
        assert((page.valid[page.count-1]&2) && fabs(page.row[page.count-1][1]-10)<1e-10);
        GsolvePoint value;assert(gsolve_ycal(&d,&m,(GsolveCurve){0,0},4,&value,NULL,NULL)==ODE_EVENT_STOP);
        GsolveResults roots=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_XCAL,20,NULL,NULL);
        assert(roots.status==ODE_OK && roots.count==0);
        roots=gsolve_search(&d,&m,(GsolveCurve){0,0},GSOLVE_XCAL,5,NULL,NULL);
        assert(roots.status==ODE_OK && roots.count==1 && fabs(roots.point[0].x-log(5))<2e-6);
        assert(solver_report()->hits==1); /* Table/G-Solve do not overwrite graph diagnostics. */
        model_defaults(&d,EQ_SYSTEM,2);d.adaptive.method=method;d.event.enabled=1;
        strcpy(d.event.text,"y1");d.event.action=EVENT_STOP;compile();
        assert(graph_render(&d,&m,true).status==ODE_EVENT_STOP && solver_report()->hits==2);
        unsigned count=solver_report()->markers.count;double event_x=solver_report()->markers.point[0].x;
        d.view.phase=1;d.phase_field=0;
        assert(graph_render(&d,&m,false).status==ODE_EVENT_STOP && m.work.rhs==0);
        assert(solver_report()->markers.count==count && solver_report()->markers.point[0].x==event_x);
        assert(trace_prepare(&d,&m,0,1));assert(trace_point_near(0,&point));
        assert(trace_navigate(&d,&m,5,true,&point)==ODE_EVENT_STOP);
        assert(fabs(point.x-acos(-1)/2)<2e-6 && fabs(point.y[0])<1e-10);
        model_defaults(&d,EQ_GENERAL,1);d.adaptive.method=method;d.solver.sf=0;
        strcpy(d.text[0],"y^2");d.nic=2;d.ic[0].y[0]=-1;d.ic[1]=d.ic[0];d.ic[1].y[0]=1;
        d.event.enabled=1;d.event.action=EVENT_STOP;strcpy(d.event.text,"y-10");compile();
        graph=graph_render(&d,&m,true);
        assert(graph.status!=ODE_OK && graph.status!=ODE_EVENT_STOP && solver_report()->hits==1);
        assert(solver_report()->status!=ODE_EVENT_STOP); /* A later family's STOP cannot hide an earlier limit. */
        model_defaults(&d,EQ_GENERAL,1);d.adaptive.method=method;d.solver.sf=0;compile();
        unsigned long rhs_before=host_rhs_calls();host_cancel_after(30);
        assert(graph_render(&d,&m,true).status==ODE_CANCELLED);
        assert(solver_report()->status==ODE_CANCELLED && solver_report()->work.rhs==host_rhs_calls()-rhs_before);
        assert(solver_report()->work.rhs>0 && solver_report()->hits==0);
    }
    model_defaults(&d,EQ_SYSTEM,2);d.adaptive.method=ODE_RK45;d.event.enabled=1;strcpy(d.event.text,"y1");compile();
    assert(graph_render(&d,&m,true).status==ODE_OK && trace_prepare(&d,&m,0,0));
    unsigned old_marks=solver_report()->markers.count;double last_x=solver_report()->markers.point[old_marks-1].x;
    ui_trace_input(true);TracePoint point;assert(trace_point_near(0,&point));
    host_cancel_after(30);assert(trace_navigate(&d,&m,12,false,&point)==ODE_CANCELLED);
    assert(solver_report()->markers.count==old_marks && solver_report()->markers.point[old_marks-1].x==last_x);
    UiBlink blink={0};assert(ui_trace_key(&blink).key==KEY_EXIT);
    assert(trace_navigate(&d,&m,12,false,&point)==ODE_OK && solver_report()->markers.count>old_marks);
    ui_trace_input(false);
    puts("Event consumers: STOP Graph/Table/TRACE/G-Solve bounds, marker/diagnostic persistence, both methods and shared Phase cache passed.");
}
