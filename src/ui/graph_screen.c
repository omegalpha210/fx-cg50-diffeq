#include "graph.h"
#include "gsolve.h"
#include "ui.h"
#include "trace.h"
#include "phase_graph.h"
#include <math.h>
#include <stdio.h>
static bool pan_key(ViewWindow *view,int key)
{
    if(key==KEY_LEFT)return graph_zoom(view,1,-.2,0);
    if(key==KEY_RIGHT)return graph_zoom(view,1,.2,0);
    if(key==KEY_UP)return graph_zoom(view,1,0,.2);
    if(key==KEY_DOWN)return graph_zoom(view,1,0,-.2);
    return false;
}
static bool accept_window(Document *d,ViewWindow before,OdeSettings solver)
{
    model_sync_solver_window(d);
    ModelWork plan=model_preflight(d,&d->solver);
    if(plan.status==ODE_OK)return true;
    *model_view(d)=before;d->solver=solver;
    graph_overlay_begin(0);graph_message("Too many steps: increase h / Max Steps",true);dupdate();
    int key;do {key=ui_getkey().key;}while(key!=KEY_EXE && key!=KEY_EXIT);
    graph_overlay_restore();return false;
}
static bool graph_more(App *a,GraphResult last)
{
    if(model_phase_supported(&a->doc)) {
        const char *const items[]={"Graph settings","Calculation / window details"};
        int n=ui_choose("Graph options",items,2,0);
        if(n==0)return true;
        if(n==1) {
            const ViewWindow *v=model_view_const(&a->doc);char text[220];
            snprintf(text,sizeof(text),"%s view\n%s\nSteps: %u\nX: %.7g to %.7g\nY: %.7g to %.7g\n%s: %.7g",
                a->doc.view.phase ? "PHASE":"TIME",ode_status_text(last.status),last.steps,
                v->xmin,v->xmax,v->ymin,v->ymax,
                a->doc.adaptive.method==ODE_RK45 ? "RK45 h0":"Integration h",a->doc.solver.h);
            ui_message("Graph details",text);
        }
        return false;
    }
    const char *const choices[]={"Phase plot: toggle","Phase horizontal state","Phase vertical state",
        "Auto V-Window","Graph settings","Calculation / window details"};
    int choice=ui_choose("Graph options",choices,6,0);Document *d=&a->doc;
    if(choice>=0 && choice<=2 && d->dim<2) {ui_message("Phase plot","Use at least two states for phase plots.");return false;}
    if(choice==0) {d->view.phase=!d->view.phase;a->dirty=true;}
    if(choice==1 || choice==2) {
        char labels[9][16];const char *items[9];
        for(int i=0;i<d->dim;i++){model_variable_label(d,i,labels[i],16);items[i]=labels[i];}
        int selected=ui_choose("Select phase state",items,d->dim,choice==1 ? d->view.phase_x:d->view.phase_y);
        if(selected>=0) {
            int other=choice==1 ? d->view.phase_y:d->view.phase_x;
            if(selected==other){ui_message("Phase axes","Choose two different state variables.");return false;}
            if(choice==1)d->view.phase_x=selected;else d->view.phase_y=selected;
            a->dirty=true;
        }
    }
    if(choice==3) {
        ui_frame("Auto V-Window","Calculating bounds... EXIT cancels");dupdate();
        OdeStatus s=graph_auto_window(d,&a->model);if(s!=ODE_OK)ui_message("Auto window",ode_status_text(s));
        else {model_sync_solver_window(d);a->dirty=true;}
    }
    if(choice==4)return true;
    if(choice==5) {
        char text[256];
        if(d->adaptive.method==ODE_RK45)snprintf(text,sizeof(text),"%s\nAttempts: %lu  IC: %d\nX: %.7g to %.7g\nY: %.7g to %.7g\nRK45 h0: %.7g\nRelTol %.3g  AbsTol %.3g",
            ode_status_text(last.status),(unsigned long)last.steps,last.failed_family+1,d->view.xmin,d->view.xmax,
            d->view.ymin,d->view.ymax,d->solver.h,d->adaptive.reltol,d->adaptive.abstol);
        else snprintf(text,sizeof(text),"%s\nSteps: %lu  IC: %d\nX: %.7g to %.7g\nY: %.7g to %.7g\nIntegration h: %.7g\nRedraw Step: %d",
            ode_status_text(last.status),(unsigned long)last.steps,last.failed_family+1,d->view.xmin,d->view.xmax,
            d->view.ymin,d->view.ymax,d->solver.h,d->solver.step);
        ui_message("Graph details",text);
    }
    return false;
}
static bool trace_value(const UiInlineEdit *edit,double *value)
{
    ExprProgram program;
    ExprError error=expr_compile(edit->text,(ExprScope){0,false,false,false},&program);
    return error.status==EXPR_OK && expr_eval(&program,0,NULL,0,value)==EXPR_OK && isfinite(*value);
}
static void show_gsolve_notice(App *a,const char *mode,const char *message);
void ui_trace(App *a,GraphResult result)
{
    Document *d=&a->doc;int count=0,selected=0,prepared=-1,stride=1,origin_selected=0;
    if(d->view.phase){for(int i=0;i<d->nic;i++)if(graph_family_enabled(d,i))count++;}
    else count=gsolve_curve_count(d);
    if(count<1){show_gsolve_notice(a,"TRACE","No visible graph");return;}
    TracePoint point={.x=d->ic[0].x},origin={0};
    UiBlink blink;ui_blink_start(&blink);ui_trace_input(true);trace_overlay_begin();
    bool valid=false,initialized=false,anchored=false;OdeStatus notice=ODE_OK;
    for(;;) {
        trace_overlay_restore();
        GsolveCurve curve={0,0};
        if(d->view.phase) {
            int visible=0;
            for(int i=0;i<d->nic;i++)if(graph_family_enabled(d,i) && visible++==selected)curve.family=i;
            curve.variable=model_view_const(d)->phase_y;
        } else gsolve_curve_at(d,selected,&curve);
        if(prepared!=selected) {
            valid=initialized ? trace_select(d,curve.family,curve.variable):
                trace_prepare(d,&a->model,curve.family,curve.variable);
            if(valid) {
                initialized=true;result=trace_cache_result();
                valid=trace_point_near(point.x,&point);
                if(valid) {
                    notice=trace_navigate(d,&a->model,point.x,false,&point);
                    if(notice==ODE_OK && !anchored){origin=point;origin_selected=selected;anchored=true;}
                }
            }
            prepared=selected;
        }
        if(valid)trace_overlay_show(d,&point,curve.variable,blink.highlighted);
        graph_labels(d,&a->model);graph_status(result);
        ui_rect(0,179,384,19,C_WHITE);
        if(valid) {
            char label[16];model_variable_label(d,curve.variable,label,sizeof(label));
            if(model_phase_supported(d) && d->view.phase)
                ui_text(8,184,UI_BLUE,"x=%.5g y1=%.5g y2=%.5g",point.x,point.y[0],point.y[1]);
            else ui_text(8,184,UI_BLUE,"IC%d x=%.7g %s=%.7g",curve.family+1,point.x,label,point.y[curve.variable]);
        } else ui_text(8,184,UI_BLUE,"Trace unavailable in this view");
        if(notice!=ODE_OK && notice!=ODE_HAS_INVALID && notice!=ODE_EVENT_STOP && notice!=ODE_CANCELLED) {
            ui_rect(0,179,384,19,C_WHITE);ui_text(8,184,UI_BLUE,"TRACE: %s",ode_status_text(notice));
        }
        ui_softkeys("INIT","NORMAL","FAST","FASTER","LEFT","RIGHT");
        int left=64*stride+1;
        ui_line(left,199,left+61,199,C_BLACK);ui_line(left,215,left+61,215,C_BLACK);
        ui_line(left,199,left,215,C_BLACK);ui_line(left+61,199,left+61,215,C_BLACK);
        dupdate();key_event_t event=ui_trace_key(&blink);int key=event.key;
        if(key==KEY_EXIT)break;
        if(key>=KEY_F2 && key<=KEY_F4){stride=key-KEY_F2+1;continue;}
        if(key==KEY_MENU && !valid){prepared=-1;continue;}
        if(key==KEY_F1 && anchored) {
            selected=origin_selected;point=origin;prepared=-1;notice=ODE_OK;continue;
        }
        if(valid && (key==KEY_LEFT || key==KEY_RIGHT || key==KEY_F5 || key==KEY_F6)) {
            trace_overlay_restore();
            bool jump=key==KEY_F5 || key==KEY_F6;
            int direction=key==KEY_LEFT || key==KEY_F5 ? -1:1;
            double target=jump ? (direction<0 ? d->solver.xmin:d->solver.xmax):
                point.x+direction*stride*trace_viewport()->xdot;
            if(!isfinite(target) || (!jump && target==point.x)){notice=ODE_BAD_STEP;continue;}
            notice=trace_navigate(d,&a->model,target,jump,&point);
            if(notice==ODE_OK || notice==ODE_HAS_INVALID || notice==ODE_EVENT_STOP)a->dirty=true;
        }
        if(key==KEY_UP){selected=(selected+count-1)%count;notice=ODE_OK;}
        if(key==KEY_DOWN){selected=(selected+1)%count;notice=ODE_OK;}
    }
    trace_overlay_restore();graph_labels(d,&a->model);graph_status(result);
    ui_blink_stop(&blink);ui_trace_input(false);
}
static bool choose_curve(App *a,GsolveCurve *curve,const GsolveCurve *excluded)
{
    int count=gsolve_curve_count(&a->doc),selected=0;
    if(count<1)return false;
    if(count==1) {
        gsolve_curve_at(&a->doc,0,curve);
        return !excluded || curve->family!=excluded->family || curve->variable!=excluded->variable;
    }
    UiBlink blink;ui_blink_start(&blink);graph_overlay_begin(0);
    bool accepted=false;
    for(;;) {
        graph_overlay_restore();gsolve_curve_at(&a->doc,selected,curve);
        if(excluded && curve->family==excluded->family && curve->variable==excluded->variable) {
            selected=(selected+1)%count;continue;
        }
        graph_overlay_curve(&a->doc,curve->family,curve->variable,blink.highlighted);
        /* Active instruction temporarily owns the channel; the exact saved
           warning and graph pixels return when this operation layer closes. */
        graph_message("UP/DOWN: SELECT GRAPH, EXE: SELECT",false);
        ui_softkeys("","","","","","CANCEL");dupdate();
        key_event_t event=ui_blink_key(&blink);int key=event.key;
        if(event.type==KEYEV_HOLD && (key==KEY_F6 || key==KEY_EXE))continue;
        if(key==KEY_EXIT || key==KEY_F6)break;
        if(key==KEY_UP)selected=(selected+count-1)%count;
        if(key==KEY_DOWN)selected=(selected+1)%count;
        if(key==KEY_EXE){accepted=true;break;}
    }
    graph_overlay_restore();ui_blink_stop(&blink);return accepted;
}
static bool gsolve_input(App *a,GsolveCurve curve,const char *label,double *value)
{
    UiInlineEdit edit;ui_inline_begin(&edit,"",false);int selected=0;
    UiBlink blink;ui_blink_start(&blink);bool error=false;
    graph_overlay_begin(GRAPH_RESULT_TOP);
    graph_overlay_curve(&a->doc,curve.family,curve.variable,true);
    for(;;) {
        ui_rect(0,179,384,19,C_WHITE);ui_text(8,184,UI_BLUE,"%s=",label);
        ui_inline_draw_cursor(&edit,31,184,200,UI_BLUE,C_WHITE,edit.active && blink.highlighted);
        if(error)ui_text(242,184,C_RED,"Invalid number");
        ui_softkeys("","","","","","");dupdate();
        key_event_t event=ui_blink_key(&blink);int key=event.key;
        if(key==KEY_EXIT){graph_overlay_restore();ui_blink_stop(&blink);return false;}
        if(key==KEY_EXE && event.type!=KEYEV_HOLD) {
            double number;
            if(!trace_value(&edit,&number)){error=true;continue;}
            *value=number;error=false;edit.active=false;
            graph_overlay_restore();ui_blink_stop(&blink);return true;
        }
        if((key>=KEY_F1 && key<=KEY_F5) || key==KEY_OPTN)continue;
        bool numeric=ui_digit(key)>=0 || key==KEY_DOT || key==KEY_NEG || key==KEY_SUB
            || key==KEY_ADD || key==KEY_EXP || key==KEY_DEL || key==KEY_ACON
            || key==KEY_LEFT || key==KEY_RIGHT;
        if(!numeric)continue;
        if(edit.active)ui_inline_key(&edit,event);
        else {char text[EXPR_TEXT];snprintf(text,sizeof(text),"%s",edit.text);
            ui_field_select(&edit,event,text,&selected,1);}
    }
}
static void show_results(App *a,GsolveCurve curve,const GsolveCurve *other,
    const char *mode,GsolveResults result,GraphResult canonical)
{
    if(result.status==ODE_CANCELLED)return;
    int selected=0;graph_overlay_begin(GRAPH_RESULT_TOP);
    for(;;) {
        graph_overlay_restore();
        if(result.status==ODE_OK && result.count) {
            GsolvePoint point=result.point[selected];
            if(graph_result_visible_y(&a->doc.view,point.x,point.y)) {
                /* Existing prepared display samples only. Never rerun G-Solve
                   or redefine canonical trajectory/Event diagnostics for a pan. */
                graph_backdrop(&a->doc,&a->model);trace_cache_render(&a->doc);
                graph_event_markers(&a->doc);graph_labels(&a->doc,&a->model);graph_status(canonical);
                a->dirty=true;graph_overlay_begin(GRAPH_RESULT_TOP);
            }
            graph_overlay_curves(&a->doc,curve.family,curve.variable,other ? other->family:-1,other ? other->variable:0,true);
            int px,py;
            if(graph_point(&a->doc.view,point.x,point.y,&px,&py))graph_overlay_point(px,py);
        }
        graph_labels(&a->doc,&a->model);graph_status(canonical);
        ui_rect(0,GRAPH_RESULT_TOP,384,19,C_WHITE);
        if(result.status!=ODE_OK)ui_text(7,184,C_RED,"%s: %s",mode,ode_status_text(result.status));
        else if(!result.count)ui_text(7,184,UI_BLUE,"%s: Not found%s",mode,
            result.has_invalid ? " (valid regions)":"");
        else {
            GsolvePoint point=result.point[selected];
            ui_text(7,184,UI_BLUE,"X=%.8g  Y=%.8g  %s %d/%d",point.x,point.y,mode,selected+1,result.count);
        }
        ui_softkeys("","","","","","BACK");dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(event.type==KEYEV_HOLD && (key==KEY_F6 || key==KEY_EXE))continue;
        if(key==KEY_EXIT || key==KEY_F6 || key==KEY_EXE)break;
        if(result.count && key==KEY_LEFT)selected=(selected+result.count-1)%result.count;
        if(result.count && key==KEY_RIGHT)selected=(selected+1)%result.count;
    }
    graph_overlay_restore();graph_labels(&a->doc,&a->model);graph_status(canonical);
}
static void show_gsolve_notice(App *a,const char *mode,const char *message)
{
    (void)a;graph_overlay_begin(GRAPH_RESULT_TOP);
    for(;;) {
        ui_rect(0,GRAPH_RESULT_TOP,384,19,C_WHITE);ui_text(7,184,UI_BLUE,"%s: %s",mode,message);
        ui_softkeys("","","","","","BACK");dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(event.type!=KEYEV_HOLD && (key==KEY_EXIT || key==KEY_F6 || key==KEY_EXE))break;
    }
    graph_overlay_restore();
}
static void gsolve_run(App *a,int operation,GraphResult canonical)
{
    GsolveCurve curve={0,0},other={0,0};int count=gsolve_curve_count(&a->doc);
    double target=0;const char *name="ROOT";
    if(operation==3)name="Y-ICPT";
    if(operation==4)name="ICPT";
    if(operation==5)name="Y-CAL";
    if(operation==6)name="X-CAL";
    if(operation==1)name="MAX";
    if(operation==2)name="MIN";
    if(count>0) {
        /* Normally the existing graph stream already owns this cache. If not,
           prepare bounded display scratch while the original plot is untouched. */
        UiBusy busy;ui_busy_start(&busy);
        OdeStatus prepared=graph_plot_prepare(&a->doc,&a->model,ui_busy_cancel,&busy);
        ui_busy_end(&busy);
        if(prepared!=ODE_OK) {
            if(prepared!=ODE_CANCELLED)show_gsolve_notice(a,name,ode_status_text(prepared));
            return;
        }
    }
    if(operation==4) {
        if(count<2) {
            show_gsolve_notice(a,name,"Not available");return;
        }
        if(count==2){gsolve_curve_at(&a->doc,0,&curve);gsolve_curve_at(&a->doc,1,&other);}
        else if(!choose_curve(a,&curve,NULL)
            || !choose_curve(a,&other,&curve))return;
        UiBusy busy;ui_busy_start(&busy);
        GsolveResults result=gsolve_intersections(&a->doc,&a->model,curve,other,ui_busy_cancel,&busy);
        ui_busy_end(&busy);show_results(a,curve,&other,name,result,canonical);return;
    }
    if(!choose_curve(a,&curve,NULL)) {
        if(count<1)show_gsolve_notice(a,name,"Not available");
        return;
    }
    if(operation==5 && !gsolve_input(a,curve,"X",&target))return;
    if(operation==6 && !gsolve_input(a,curve,"Y",&target))return;
    if(operation==3 || operation==5) {
        double x=operation==3 ? 0:target;GsolvePoint point;
        double xmin=fmax(a->doc.solver.xmin,a->doc.view.xmin);
        double xmax=fmin(a->doc.solver.xmax,a->doc.view.xmax);
        UiBusy busy;ui_busy_start(&busy);
        OdeStatus status=x<xmin || x>xmax ? ODE_OK:
            gsolve_ycal(&a->doc,&a->model,curve,x,&point,ui_busy_cancel,&busy);
        ui_busy_end(&busy);
        GsolveResults result={.status=status};
        if(x>=xmin && x<=xmax && status==ODE_OK) {
            result.count=1;result.point[0]=point;
        }
        show_results(a,curve,NULL,name,result,canonical);return;
    }
    GsolveMode mode=operation==1 ? GSOLVE_MAXIMUM:(operation==2 ? GSOLVE_MINIMUM:
        (operation==6 ? GSOLVE_XCAL:GSOLVE_ROOT));
    UiBusy busy;ui_busy_start(&busy);
    GsolveResults result=gsolve_search(&a->doc,&a->model,curve,mode,target,ui_busy_cancel,&busy);
    ui_busy_end(&busy);show_results(a,curve,NULL,name,result,canonical);
}
static bool restore_plot(App *a,GraphResult last)
{
    Document *d=&a->doc;
    if(!graph_redraw_cached(d,&a->model,d->solver.xmin,d->solver.xmax)) {
        if(gsolve_curve_count(d)>0 || d->view.phase)return false;
        graph_backdrop(d,&a->model);graph_labels(d,&a->model);
    }
    graph_status(last);return true;
}
static bool gsolve_menu(App *a,GraphResult *last)
{
    int page=0;
    for(;;) {
        if(page==0)ui_softkeys("ROOT","MAX","MIN","Y-ICPT","ICPT",">");
        else ui_softkeys("Y-CAL","X-CAL","","","","<");
        dupdate();int key=ui_getkey().key,operation=-1;
        if(key==KEY_EXIT)return true;
        if(key==KEY_F6){page=1-page;continue;}
        ViewWindow before=a->doc.view;OdeSettings solver=a->doc.solver;
        bool panned=pan_key(&a->doc.view,key);
        if(panned && accept_window(&a->doc,before,solver)) {
            GraphResult next=graph_render(&a->doc,&a->model,false);
            if(next.status==ODE_CANCELLED) {
                a->doc.view=before;a->doc.solver=solver;
                if(!restore_plot(a,*last))return false;
            } else {a->dirty=true;*last=next;}
        }
        if(page==0 && key>=KEY_F1 && key<=KEY_F5)operation=key-KEY_F1;
        if(page==1 && (key==KEY_F1 || key==KEY_F2))operation=key==KEY_F1 ? 5:6;
        if(operation>=0) {
            gsolve_run(a,operation,*last);
        }
    }
}
static void equilibrium_info(int selected)
{
    const PhaseResults *r=graph_phase_results();
    if(selected<0 || (unsigned)selected>=r->count) {
        ui_message("Phase analysis","EQPT searches this Phase window.\nAutonomous 2D systems only.\nClassification is local linear.\nN1 (red): f1=0\nN2 (blue): f2=0");return;
    }
    const PhaseRoot *p=&r->root[selected];char text[360];
    snprintf(text,sizeof(text),"y1=%.9g  y2=%.9g\nLinearized: %s\nJ: %.6g  %.6g\n   %.6g  %.6g\nL1: %.6g %+.6gi\nL2: %.6g %+.6gi\nLocal linear; neutral is inconclusive.",
        p->y[0],p->y[1],phase_type_name(p->type),p->jacobian[0],p->jacobian[1],
        p->jacobian[2],p->jacobian[3],p->eigen_real[0],p->eigen_imag[0],p->eigen_real[1],p->eigen_imag[1]);
    ui_message("Equilibrium / INFO",text);
}
/* Small settings transaction: phase preflight can abort before a graph pixel
   changes. Never copy the Document or keep a second framebuffer on the stack. */
typedef struct {
    ViewWindow window,phase_window;OdeSettings solver;
    uint8_t projection,field,nullclines,ready;
} GraphChange;
static GraphChange change_begin(const Document *d)
{
    return (GraphChange){*model_view_const(d),d->phase_view,d->solver,d->view.phase,
        d->phase_field,d->phase_nullclines,d->phase_ready};
}
static void change_restore(Document *d,const GraphChange *before)
{
    d->view.phase=before->projection;d->phase_view=before->phase_window;*model_view(d)=before->window;d->solver=before->solver;
    d->phase_field=before->field;d->phase_nullclines=before->nullclines;d->phase_ready=before->ready;
}

static bool zoom_box(App *a,GraphResult result)
{
    int x=192,y=99,first_x=x,first_y=y;bool second=false,small=false;
    graph_overlay_begin(0);
    for(;;) {
        trace_box_show(first_x,first_y,x,y,second);
        graph_labels(&a->doc,&a->model);graph_status(result);
        graph_message(small ? "BOX TOO SMALL; move point 2":
            (second ? "Point 2  EXE: ZOOM   EXIT: cancel":"Point 1  EXE: SET   EXIT: cancel"),false);
        ui_softkeys("","","","","","");dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        trace_box_restore();
        if(key==KEY_EXIT)return false;
        if(key==KEY_EXE && event.type!=KEYEV_HOLD) {
            if(!second){first_x=x;first_y=y;second=true;}
            else {
                ViewWindow next=*model_view(&a->doc);
                if(graph_box_window(&next,first_x,first_y,x,y)) {*model_view(&a->doc)=next;return true;}
                small=true;
            }
        }
        if(key==KEY_LEFT || key==KEY_RIGHT || key==KEY_UP || key==KEY_DOWN)small=false;
        if(key==KEY_LEFT)x=x>=4 ? x-4:0;
        if(key==KEY_RIGHT)x=x<=379 ? x+4:383;
        if(key==KEY_UP)y=y>=4 ? y-4:0;
        if(key==KEY_DOWN)y=y<=193 ? y+4:197;
    }
}
UiGraphAction ui_graph(App *a,bool first)
{
    enum {BASE,ZOOM,VIEW,ANALYSIS} menu=BASE;
    bool redraw=true,pending=false,eq_shown=false,stable=false,recall_pending=first,notice_phase=false;
    int selected=-1;OdeStatus notice=ODE_OK;
    Document *d=&a->doc;GraphChange before=change_begin(d);
    GraphResult result={.status=ODE_OK,.failed_family=-1};
    for(;;) {
        bool system=model_phase_supported(d),phase=system && d->view.phase;
        if(redraw) {
            GraphResult next;
            if(!stable && !first && graph_redraw_cached(d,&a->model,d->solver.xmin,d->solver.xmax))
                next=trace_cache_result();
            else next=graph_render(d,&a->model,first);
            first=false;redraw=false;
            bool completed=next.status==ODE_OK || next.status==ODE_HAS_INVALID
                || next.status==ODE_EVENT_STOP || next.steps;
            if(next.status==ODE_CANCELLED) {
                if(pending)change_restore(d,&before);
                if(!stable || !restore_plot(a,result))return UI_GRAPH_BACK;
                pending=false;phase=system && d->view.phase;
            }
            else if(pending && !completed) {
                notice_phase=phase;change_restore(d,&before);notice=next.status;
                if(system && !phase && trace_cache_matches(d)) {
                    graph_backdrop(d,&a->model);trace_cache_render(d);graph_event_markers(d);graph_phase_markers(d,-1);
                    graph_labels(d,&a->model);
                }
            } else {
                /* A completed numerical-limit prefix already committed its
                   raster, display cache and report. Keep its geometry too. */
                result=next;stable=true;if(pending)a->dirty=true;
                if(recall_pending && completed) {
                    a->recall=*d;a->has_recall=true;a->dirty=true;recall_pending=false;
                }
            }
            pending=false;phase=system && d->view.phase;

        }
        if(menu==ZOOM)ui_softkeys("IN","OUT","AUTO","ORIG","BOX","");
        else if(menu==VIEW)ui_softkeys("TIME","PHASE","TABLE","","","");
        else if(menu==ANALYSIS)ui_softkeys("FIELD","NULL","EQPT","INFO","","");
        else ui_softkeys("TRACE","ZOOM","V-WIN",system ? "VIEW":"TABLE",phase ? "ANLYS":"G-SLV","INIT");
        if(menu==ANALYSIS && eq_shown) {
            const PhaseResults *r=graph_phase_results();
            if(selected>=0 && (unsigned)selected<r->count)graph_phase_markers(d,selected);
            graph_labels(d,&a->model);
            ui_rect(0,163,384,35,C_WHITE);
            if(selected>=0 && (unsigned)selected<r->count) {
                const PhaseRoot *p=&r->root[selected];
                ui_text(6,165,UI_BLUE,"EQPT %d/%u%s y1=%.6g y2=%.6g",selected+1,r->count,
                    r->truncated ? "+":"",p->y[0],p->y[1]);
                ui_text(6,182,UI_BLUE,"Linearized: %s",phase_type_name(p->type));
            } else ui_text(6,181,UI_BLUE,"EQPT: Not found%s",r->has_invalid ? " (valid regions)":"");
        }
        /* Also cover preflight returns and repaint after active Phase markers. */
        graph_status(result);
        if(notice!=ODE_OK) {
            graph_overlay_begin(0);
            char text[80];snprintf(text,sizeof(text),"%s: %s",notice_phase ? "Phase":"Graph",ode_status_text(notice));graph_message(text,false);
        }
        dupdate();int key=ui_getkey().key;
        if(notice!=ODE_OK)graph_overlay_restore();
        notice=ODE_OK;
        if(menu==VIEW) {
            if(key==KEY_EXIT){menu=BASE;continue;}
            if(key==KEY_F3)return UI_GRAPH_TABLE;
            if(key==KEY_F1 || key==KEY_F2) {
                before=change_begin(d);
                if(key==KEY_F2 && !d->phase_ready) {
                    if(!trace_cache_phase_window(d,&d->phase_view))model_phase_window_defaults(&d->phase_view);
                    d->phase_ready=1;
                }
                d->view.phase=key==KEY_F2;pending=true;redraw=true;menu=BASE;
            }
            continue;
        }
        if(menu==ANALYSIS) {
            if(key==KEY_EXIT){menu=BASE;redraw=true;eq_shown=false;continue;}
            if(key==KEY_F1 || key==KEY_F2) {
                before=change_begin(d);
                if(key==KEY_F1)d->phase_field=!d->phase_field;
                else d->phase_nullclines=!d->phase_nullclines;
                pending=true;redraw=true;
            }
            if(key==KEY_F3) {
                if(!phase_autonomous(&a->model)) {
                    ui_message("EQPT","Autonomous systems only.");redraw=true;continue;
                }
                OdeStatus s=graph_phase_search(d,&a->model,ui_cancel,NULL);
                if(s==ODE_OK){eq_shown=true;selected=graph_phase_results()->count ? 0:-1;redraw=true;}
                else {notice=s;notice_phase=true;}
            }
            if(key==KEY_F4){equilibrium_info(selected);redraw=true;}
            const PhaseResults *r=graph_phase_results();
            if(eq_shown && r->count && (key==KEY_LEFT || key==KEY_RIGHT)) {
                selected=(selected+(key==KEY_LEFT ? (int)r->count-1:1))%(int)r->count;redraw=true;
            }
            continue;
        }
        if(menu==ZOOM && key==KEY_EXIT){menu=BASE;continue;}
        if(menu==BASE && key==KEY_EXIT)return UI_GRAPH_BACK;
        if(menu==BASE && key==KEY_F6) {
            before=change_begin(d);
            ui_vwindow_reset(d);eq_shown=false;selected=-1;
            if(graph_redraw_cached(d,&a->model,d->solver.xmin,d->solver.xmax))
                {result=trace_cache_result();a->dirty=true;}
            else {pending=true;redraw=true;}
            continue;
        }
        if(menu==BASE && key==KEY_F1){ui_trace(a,result);continue;}
        if(menu==BASE && key==KEY_F2){menu=ZOOM;continue;}
        before=change_begin(d);ViewWindow *v=model_view(d);
        bool changed=pan_key(v,key);
        if(menu==ZOOM) {
            if(key==KEY_F5) {
                changed=zoom_box(a,result);
                if(changed)menu=BASE;
            }
            if(key==KEY_F4) {
                ui_vwindow_reset(d);
                changed=true;
            }
            if(key==KEY_F1 || key==KEY_F2)changed=graph_zoom(v,key==KEY_F1 ? .67:1.5,0,0);
            if(key==KEY_F3) {
                OdeStatus s=graph_auto_window(d,&a->model);changed=s==ODE_OK;
                if(!changed) {
                    graph_overlay_begin(0);
                    char hint[96];snprintf(hint,sizeof(hint),"AUTO: %s - EXE",s==ODE_BAD_INPUT ? "No samples in X range":ode_status_text(s));
                    graph_message(hint,false);dupdate();
                    while((key=ui_getkey().key)!=KEY_EXE && key!=KEY_EXIT) {}
                    graph_overlay_restore();
                }
            }
        } else {
            if(key==KEY_ADD || key==KEY_SUB)changed=graph_zoom(v,key==KEY_ADD ? .67:1.5,0,0);
            if(key==KEY_F3)return UI_GRAPH_VWINDOW;
            if(key==KEY_F4){if(system){menu=VIEW;continue;}return UI_GRAPH_TABLE;}
            if(key==KEY_F5) {
                if(phase){menu=ANALYSIS;continue;}
                if(d->view.phase) {
                    graph_overlay_begin(0);graph_message("G-Solve: turn Phase off (EXE)",false);dupdate();
                    while((key=ui_getkey().key)!=KEY_EXE && key!=KEY_EXIT) {}
                    graph_overlay_restore();
                } else if(!gsolve_menu(a,&result))return UI_GRAPH_BACK;
            }
            if(key==KEY_OPTN) {
                if(graph_more(a,result))return UI_GRAPH_SETTINGS;
                redraw=true;
            }
        }
        if(changed && accept_window(d,before.window,before.solver)) {
            if(phase)d->phase_ready=1;
            pending=true;redraw=true;
        }
    }
}
