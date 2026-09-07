#include "graph.h"
#include "gsolve.h"
#include "ui.h"
#include "trace.h"
#include <math.h>
#include <stdio.h>
static bool graph_more(App *a,GraphResult last)
{
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
        char text[256];snprintf(text,sizeof(text),"%s\nSteps: %lu  IC: %d\nX: %.7g to %.7g\nY: %.7g to %.7g\nIntegration h: %.7g\nRedraw Step: %d",
            ode_status_text(last.status),(unsigned long)last.steps,last.failed_family+1,d->view.xmin,d->view.xmax,
            d->view.ymin,d->view.ymax,d->solver.h,d->solver.step);
        ui_message("Graph details",text);
    }
    return false;
}
static bool trace_value(const UiInlineEdit *edit,const double *constants,double *value)
{
    ExprProgram program;
    ExprError error=expr_compile(edit->text,(ExprScope){0,false,false,false,constants},&program);
    return error.status==EXPR_OK && expr_eval(&program,0,NULL,0,value)==EXPR_OK && isfinite(*value);
}
void ui_trace(App *a)
{
    Document *d=&a->doc;int count=0,selected=0,prepared=-1;
    if(d->view.phase) {for(int i=0;i<d->nic;i++)if(graph_family_enabled(d,i))count++;}
    else count=gsolve_curve_count(d);
    if(count<1)return;
    TracePoint point={0};point.x=d->ic[0].x;
    UiInlineEdit edit={0};UiBlink blink;ui_blink_start(&blink);
    ui_trace_input(true);trace_overlay_begin();
    bool valid=false,input_error=false,boundary=false;
    for(;;) {
        trace_overlay_restore();
        GsolveCurve curve={0,0};
        if(d->view.phase) {
            int visible=0;
            for(int i=0;i<d->nic;i++)if(graph_family_enabled(d,i) && visible++==selected)curve.family=i;
            curve.variable=d->view.phase_y;
        } else gsolve_curve_at(d,selected,&curve);
        if(prepared!=selected) {
            /* Capture into bounded scratch without modifying any graph pixel.
               A cancelled preparation never replaces the complete plot. */
            valid=trace_prepare(d,&a->model,curve.family,curve.variable);
            if(valid)trace_point_near(point.x,&point);
            prepared=selected;
        }
        if(valid)trace_overlay_show(d,&point,curve.variable,blink.highlighted);
        ui_rect(0,179,384,19,C_WHITE);
        if(edit.active) {
            ui_text(8,184,UI_BLUE,"x=");ui_inline_draw(&edit,25,184,210,UI_BLUE,C_WHITE);
        } else if(valid) {
            char label[16];model_variable_label(d,curve.variable,label,sizeof(label));
            ui_text(8,184,UI_BLUE,"IC%d x=%.7g %s=%.7g",curve.family+1,point.x,label,point.y[curve.variable]);
        } else ui_text(8,184,UI_BLUE,"Trace unavailable; graph retained");
        if(boundary) {
            ui_rect(0,179,384,19,C_WHITE);ui_text(8,184,UI_BLUE,"TRACE: invalid region");
        }
        if(input_error)ui_softkeys("Invalid x","","","","","OK");
        else ui_softkeys("x=","","","","","BACK");
        dupdate();
        key_event_t event=edit.active ? ui_getkey():ui_trace_key(&blink);int key=event.key;
        if(edit.active) {
            if(key==KEY_EXE || key==KEY_EXIT || key==KEY_F6) {
                double value;
                if(!trace_value(&edit,d->constants,&value)){input_error=true;continue;}
                input_error=false;
                value=fmax(fmax(d->solver.xmin,d->view.xmin),fmin(fmin(d->solver.xmax,d->view.xmax),value));
                OdeResult r=ode_integrate(model_rhs,&a->model,d->dim,d->ic[curve.family].x,
                    d->ic[curve.family].y,value,&d->solver,NULL,NULL,ui_trace_cancel,NULL);
                if(r.status==ODE_CANCELLED) {
                    int control=ui_trace_key(&blink).key;
                    if(control==KEY_EXIT || control==KEY_F6)break;
                    continue;
                }
                if(r.status!=ODE_OK){input_error=true;continue;}
                if(r.status==ODE_OK){point.x=r.x;for(int j=0;j<d->dim;j++)point.y[j]=r.y[j];}
                edit.active=false;boundary=false;continue;
            }
            if((key<KEY_F1 || key>KEY_F6) && key!=KEY_OPTN)ui_inline_key(&edit,event);
            continue;
        }
        if(key==KEY_EXIT || key==KEY_F6)break;
        if(key==KEY_MENU && !valid)prepared=-1;
        if(key==KEY_F1 || ui_inline_input(event)) {
            boundary=false;
            char text[EXPR_TEXT];snprintf(text,sizeof(text),"%.12g",point.x);
            ui_inline_begin(&edit,text,true);
            if(key!=KEY_F1)ui_inline_key(&edit,event);
        }
        if(valid && (key==KEY_LEFT || key==KEY_RIGHT))
            boundary=!trace_move(point.x,key==KEY_LEFT ? -1:1,&point) && trace_has_invalid();
        if(key==KEY_UP){selected=(selected+count-1)%count;boundary=false;}
        if(key==KEY_DOWN){selected=(selected+1)%count;boundary=false;}
    }
    trace_overlay_restore();ui_blink_stop(&blink);ui_trace_input(false);
}
static void curve_name(const Document *d,GsolveCurve curve,char *out,unsigned size)
{
    char variable[16];model_variable_label(d,curve.variable,variable,sizeof(variable));
    snprintf(out,size,"IC%d %s",curve.family+1,variable);
}
static bool choose_curve(App *a,GsolveCurve *curve,const GsolveCurve *excluded,const char *prompt)
{
    int count=gsolve_curve_count(&a->doc),selected=0;
    if(count<1)return false;
    if(count==1) {
        gsolve_curve_at(&a->doc,0,curve);
        return !excluded || curve->family!=excluded->family || curve->variable!=excluded->variable;
    }
    UiBlink blink;ui_blink_start(&blink);
    for(;;) {
        gsolve_curve_at(&a->doc,selected,curve);
        if(excluded && curve->family==excluded->family && curve->variable==excluded->variable) {
            selected=(selected+1)%count;continue;
        }
        graph_render(&a->doc,&a->model,false);
        if(blink.highlighted)graph_highlight_curve(&a->doc,&a->model,curve->family,curve->variable);
        ui_rect(0,0,384,19,C_WHITE);char name[32];curve_name(&a->doc,*curve,name,sizeof(name));
        ui_text(7,4,UI_BLUE,"%s %s   UP/DOWN  EXE",prompt,name);
        ui_softkeys("","","","","","CANCEL");dupdate();
        int key=ui_blink_key(&blink).key;
        if(key==KEY_EXIT || key==KEY_F6){ui_blink_stop(&blink);return false;}
        if(key==KEY_UP)selected=(selected+count-1)%count;
        if(key==KEY_DOWN)selected=(selected+1)%count;
        if(key==KEY_EXE){ui_blink_stop(&blink);return true;}
    }
}
static bool gsolve_input(App *a,GsolveCurve curve,const char *label,double *value)
{
    UiInlineEdit edit;ui_inline_begin(&edit,"",false);int selected=0;
    UiBlink blink;ui_blink_start(&blink);bool error=false;
    graph_render(&a->doc,&a->model,false);
    graph_highlight_curve(&a->doc,&a->model,curve.family,curve.variable);
    for(;;) {
        ui_rect(0,179,384,19,C_WHITE);ui_text(8,184,UI_BLUE,"%s=",label);
        ui_inline_draw_cursor(&edit,31,184,200,UI_BLUE,C_WHITE,edit.active && blink.highlighted);
        if(error)ui_text(242,184,C_RED,"Invalid number");
        ui_softkeys("","","","","","RUN");dupdate();
        key_event_t event=ui_blink_key(&blink);int key=event.key;
        if(key==KEY_EXE || key==KEY_F6 || (key==KEY_EXIT && edit.active)) {
            double number;
            if(!trace_value(&edit,a->doc.constants,&number)){error=true;continue;}
            *value=number;error=false;edit.active=false;
            if(key==KEY_EXIT)continue;
            ui_blink_stop(&blink);return true;
        }
        if(key==KEY_EXIT){ui_blink_stop(&blink);return false;}
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
static void result_pointer(const Document *d,GsolvePoint point)
{
    int px,py;if(!graph_point(&d->view,point.x,point.y,&px,&py))return;
    ui_line(px-5,py,px+5,py,UI_BLUE);ui_line(px,py-5,px,py+5,UI_BLUE);
    ui_rect(px-1,py-1,3,3,C_WHITE);
}
static void show_results(App *a,GsolveCurve curve,const GsolveCurve *other,
    const char *mode,GsolveResults result)
{
    if(result.status==ODE_CANCELLED)return;
    int selected=0;
    for(;;) {
        graph_render(&a->doc,&a->model,false);
        graph_highlight_curve(&a->doc,&a->model,curve.family,curve.variable);
        if(other)graph_highlight_curve(&a->doc,&a->model,other->family,other->variable);
        ui_rect(0,179,384,19,C_WHITE);
        if(result.status!=ODE_OK)ui_text(7,184,C_RED,"%s: %s",mode,ode_status_text(result.status));
        else if(!result.count)ui_text(7,184,UI_BLUE,"%s: Not found%s",mode,
            result.has_invalid ? " (valid regions)":"");
        else {
            GsolvePoint point=result.point[selected];result_pointer(&a->doc,point);
            ui_text(7,184,UI_BLUE,"X=%.8g  Y=%.8g  %s %d/%d",point.x,point.y,mode,
                selected+1,result.count);
        }
        ui_softkeys("","","","","","BACK");dupdate();
        int key=ui_getkey().key;
        if(key==KEY_EXIT || key==KEY_F6 || key==KEY_EXE)return;
        if(result.count && key==KEY_LEFT)selected=(selected+result.count-1)%result.count;
        if(result.count && key==KEY_RIGHT)selected=(selected+1)%result.count;
    }
}
static void show_gsolve_notice(App *a,const char *mode,const char *message)
{
    for(;;) {
        graph_render(&a->doc,&a->model,false);
        ui_rect(0,179,384,19,C_WHITE);ui_text(7,184,UI_BLUE,"%s: %s",mode,message);
        ui_softkeys("","","","","","BACK");dupdate();
        int key=ui_getkey().key;if(key==KEY_EXIT || key==KEY_F6 || key==KEY_EXE)return;
    }
}
static void gsolve_run(App *a,int operation)
{
    GsolveCurve curve={0,0},other={0,0};int count=gsolve_curve_count(&a->doc);
    double target=0;const char *name="ROOT";
    if(operation==3)name="Y-ICPT";
    if(operation==4)name="ICPT";
    if(operation==5)name="Y-CAL";
    if(operation==6)name="X-CAL";
    if(operation==1)name="MAX";
    if(operation==2)name="MIN";
    if(operation==4) {
        if(count<2) {
            show_gsolve_notice(a,name,"Not available");return;
        }
        if(count==2){gsolve_curve_at(&a->doc,0,&curve);gsolve_curve_at(&a->doc,1,&other);}
        else if(!choose_curve(a,&curve,NULL,"Curve A")
            || !choose_curve(a,&other,&curve,"Curve B"))return;
        show_results(a,curve,&other,name,gsolve_intersections(&a->doc,&a->model,
            curve,other,ui_cancel,NULL));return;
    }
    if(!choose_curve(a,&curve,NULL,"Select")) {
        if(count<1)show_gsolve_notice(a,name,"Not available");
        return;
    }
    if(operation==5 && !gsolve_input(a,curve,"X",&target))return;
    if(operation==6 && !gsolve_input(a,curve,"Y",&target))return;
    if(operation==3 || operation==5) {
        double x=operation==3 ? 0:target;GsolvePoint point;
        double xmin=fmax(a->doc.solver.xmin,a->doc.view.xmin);
        double xmax=fmin(a->doc.solver.xmax,a->doc.view.xmax);
        OdeStatus status=x<xmin || x>xmax ? ODE_OK:
            gsolve_ycal(&a->doc,&a->model,curve,x,&point,ui_cancel,NULL);
        GsolveResults result={.status=status};
        if(x>=xmin && x<=xmax && status==ODE_OK) {
            result.count=1;result.point[0]=point;
        }
        show_results(a,curve,NULL,name,result);return;
    }
    GsolveMode mode=operation==1 ? GSOLVE_MAXIMUM:(operation==2 ? GSOLVE_MINIMUM:
        (operation==6 ? GSOLVE_XCAL:GSOLVE_ROOT));
    show_results(a,curve,NULL,name,gsolve_search(&a->doc,&a->model,curve,mode,target,ui_cancel,NULL));
}
static void gsolve_menu(App *a,GraphResult *last)
{
    int page=0;
    for(;;) {
        if(page==0)ui_softkeys("ROOT","MAX","MIN","Y-ICPT","ICPT",">");
        else ui_softkeys("Y-CAL","X-CAL","","","","<");
        dupdate();int key=ui_getkey().key,operation=-1;
        if(key==KEY_EXIT)return;
        if(key==KEY_F6){page=1-page;continue;}
        if(key==KEY_LEFT)graph_zoom(&a->doc.view,1,-.2,0);
        if(key==KEY_RIGHT)graph_zoom(&a->doc.view,1,.2,0);
        if(key==KEY_UP)graph_zoom(&a->doc.view,1,0,.2);
        if(key==KEY_DOWN)graph_zoom(&a->doc.view,1,0,-.2);
        if(key==KEY_LEFT || key==KEY_RIGHT || key==KEY_UP || key==KEY_DOWN)
            {model_sync_solver_window(&a->doc);a->dirty=true;*last=graph_render(&a->doc,&a->model,false);}
        if(page==0 && key>=KEY_F1 && key<=KEY_F5)operation=key-KEY_F1;
        if(page==1 && (key==KEY_F1 || key==KEY_F2))operation=key==KEY_F1 ? 5:6;
        if(operation>=0) {
            gsolve_run(a,operation);
            *last=graph_render(&a->doc,&a->model,false);
        }
    }
}
UiGraphAction ui_graph(App *a,bool first)
{
    bool redraw=true,zoom_menu=false;
    GraphResult result={.status=ODE_OK,.failed_family=-1};
    for(;;) {
        if(redraw) {
            /* Keep the displayed plot while streaming the next render to the
               same VRAM. Menu-only changes never enter this branch. */
            ui_rect(0,198,384,18,UI_BLUE);
            ui_text(7,202,C_WHITE,"Drawing... EXIT cancels");dupdate();
            result=graph_render(&a->doc,&a->model,first);first=false;redraw=false;
        }
        if(zoom_menu)ui_softkeys("IN","OUT","AUTO","ORIG","","");
        else ui_softkeys("TRACE","ZOOM","V-WIN","TABLE","G-SLV","BACK");
        dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(zoom_menu) {
            if(key==KEY_EXIT){zoom_menu=false;continue;}
            if(key==KEY_F4) {model_window_defaults(&a->doc.view);redraw=true;}
            if(key==KEY_F1 || key==KEY_F2) {
                redraw=graph_zoom(&a->doc.view,key==KEY_F1 ? .67:1.5,0,0);
            }
            if(key==KEY_F3) {
                ui_rect(0,198,384,18,UI_BLUE);ui_text(7,202,C_WHITE,"Fitting Y... EXIT cancels");dupdate();
                OdeStatus status=graph_auto_window(&a->doc,&a->model);
                redraw=status==ODE_OK;
                if(!redraw) {
                    ui_rect(0,198,384,18,UI_BLUE);
                    ui_text(7,202,C_WHITE,"AUTO: %s - EXE",status==ODE_BAD_INPUT ?
                        "No samples in X range":ode_status_text(status));dupdate();
                    while((key=ui_getkey().key)!=KEY_EXE && key!=KEY_EXIT) {}
                }
            }
            if(redraw){model_sync_solver_window(&a->doc);a->dirty=true;}
            continue;
        }
        if(key==KEY_EXIT || key==KEY_F6)return UI_GRAPH_BACK;
        if(key==KEY_F1){ui_trace(a);continue;}
        if(key==KEY_F2) {
            zoom_menu=true;continue;
        }
        if(key==KEY_ADD)graph_zoom(&a->doc.view,.67,0,0);
        if(key==KEY_SUB)graph_zoom(&a->doc.view,1.5,0,0);
        if(key==KEY_LEFT)graph_zoom(&a->doc.view,1,-.2,0);
        if(key==KEY_RIGHT)graph_zoom(&a->doc.view,1,.2,0);
        if(key==KEY_UP)graph_zoom(&a->doc.view,1,0,.2);
        if(key==KEY_DOWN)graph_zoom(&a->doc.view,1,0,-.2);
        if(key==KEY_ADD || key==KEY_SUB || key==KEY_LEFT || key==KEY_RIGHT
            || key==KEY_UP || key==KEY_DOWN){model_sync_solver_window(&a->doc);a->dirty=true;redraw=true;}
        if(key==KEY_F3)return UI_GRAPH_VWINDOW;
        if(key==KEY_F4)return UI_GRAPH_TABLE;
        /* A bounded softkey submenu, not a dispatcher screen re-entry. */
        if(key==KEY_F5) {
            if(a->doc.view.phase) {
                ui_rect(0,198,384,18,UI_BLUE);ui_text(7,202,C_WHITE,"G-Solve: turn Phase off (EXE)");dupdate();
                while((key=ui_getkey().key)!=KEY_EXE && key!=KEY_EXIT) {}
            } else gsolve_menu(a,&result);
        }
        if(key==KEY_OPTN) {
            if(graph_more(a,result))return UI_GRAPH_SETTINGS;
            redraw=true;
        }
    }
}
