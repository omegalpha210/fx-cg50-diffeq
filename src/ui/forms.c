#include "ui.h"
#include "graph.h"
#include "initial.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef UiInlineEdit NumberEdit;
static bool number_select(NumberEdit *edit,key_event_t event,double value,int *selected,int count)
{
    char text[48];snprintf(text,sizeof(text),"%.12g",value);
    return ui_field_select(edit,event,text,selected,count);
}
/* 0: keep editing, 1: accept requested, -1: cancel edit. */
static int number_key(NumberEdit *e,key_event_t event) {return ui_inline_key(e,event);}
static bool number_value(NumberEdit *e,double *out)
{
    ExprProgram p;ExprError error=expr_compile(e->text,
        (ExprScope){0,false,false,false},&p);
    double value=0;ExprStatus status=error.status;
    if(status==EXPR_OK)status=expr_eval(&p,0,NULL,0,&value);
    if(status!=EXPR_OK || !isfinite(value)) {
        ui_message("Invalid value",expr_status_text(status));return false;
    }
    *out=value;return true;
}
static void number_cursor(const NumberEdit *e,int row)
{
    if(!e->active)return;
    ui_inline_draw(e,138,31+row*22,226,C_WHITE,UI_BLUE);
}
static bool stage_leave(int key)
{
    return key==KEY_EXIT || key==KEY_F1 || key==KEY_F3 || key==KEY_F4
        || key==KEY_F5 || key==KEY_F6;
}
static UiStageAction stage_action(int key)
{
    if(key==KEY_F3)return UI_STAGE_VWINDOW;
    if(key==KEY_F4)return UI_STAGE_OUTPUT;
    if(key==KEY_F5)return UI_STAGE_SETTINGS;
    return key==KEY_F6 ? UI_STAGE_NEXT:UI_STAGE_BACK;
}
/* Visible-row mapping keeps hidden Step/SF fields out of selection/editing. */
static int parameter_rows(const Document *d,int rows[8])
{
    int count=0;rows[count++]=0;rows[count++]=1;rows[count++]=6;rows[count++]=2;
    if(d->adaptive.method==ODE_RK45){rows[count++]=7;rows[count++]=8;}
    else rows[count++]=3;
    if(model_field_supported(d))rows[count++]=4;
    rows[count++]=5;return count;
}
UiStageAction ui_parameters(Document *d,UiStageState *state)
{
    static const char *const help[]={"Integration start","Integration end",
        "RK4 h > 0; smaller means more work","Output spacing only; h is unchanged",
        "Slope-field columns (0-100); 0 = Off","Max RK4 steps per IC / direction",
        "LEFT/RIGHT: RK4 / RK45","RelTol: relative error target","AbsTol: absolute error floor"};
    int selected=state->selected;NumberEdit edit=state->edit;
    bool advanced=state->top!=0;
    for(;;) {
        int rows[8],count=parameter_rows(d,rows);
        if(selected<0 || selected>=count){selected=count-1;edit.active=false;}
        int top=selected>6 ? selected-6:0,field=rows[selected];
        bool adaptive=d->adaptive.method==ODE_RK45;
        const char *labels[]={"Xrange min","Xrange max",adaptive ? "Initial h":"h","Step","SF",
            "Max steps","Method","RelTol","AbsTol"};
        char values[9][48];
        snprintf(values[0],48,"%.9g",d->solver.xmin);snprintf(values[1],48,"%.9g",d->solver.xmax);
        snprintf(values[2],48,"%.9g",d->solver.h);snprintf(values[3],48,"%d",d->solver.step);
        snprintf(values[4],48,"%d",d->solver.sf);snprintf(values[5],48,"%lu",(unsigned long)d->solver.max_steps);
        snprintf(values[6],48,"%s",adaptive ? "RK45":"RK4");
        snprintf(values[7],48,"%.9g",d->adaptive.reltol);snprintf(values[8],48,"%.9g",d->adaptive.abstol);
        ui_frame("Parameter",NULL);
        ui_progress(advanced ? 0:3);
        for(int row=top;row<count && row<top+7;row++) {
            int item=rows[row];
            ui_field(row-top,labels[item],edit.active && row==selected ? edit.text:values[item],row==selected);
        }
        number_cursor(&edit,selected-top);
        ui_form_hint(&edit,adaptive && field==2 ? "Initial step; RK45 adjusts internally":
            (adaptive && field==5 ? "Accepted + rejected attempts per path":help[field]));
        if(advanced)ui_softkeys("EVENT","INFO","","","","");
        else ui_softkeys("INIT","ADV","V-WIN","OUTPUT","SET","GRAPH");
        dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(advanced) {
            if(key==KEY_EXIT){advanced=false;continue;}
            if(key==KEY_F1 || key==KEY_F2) {
                state->selected=selected;state->edit=edit;state->top=1;
                return key==KEY_F1 ? UI_STAGE_EVENT:UI_STAGE_INFO;
            }
            continue;
        }
        if(edit.active) {
            int action=stage_leave(key) || key==KEY_UP || key==KEY_DOWN || key==KEY_F2 ? 1:number_key(&edit,event);
            if(action==0)continue;
            double value;OdeSettings before=d->solver;OdeAdaptive old=d->adaptive;
            if(!number_value(&edit,&value))continue;
            if(field>=3 && field<=5 && (value<0 || value>100000 || value!=floor(value))) {
                ui_message("Invalid setting","Use an integer within the setting limit.");continue;
            }
            if(field==0)d->solver.xmin=value;
            if(field==1)d->solver.xmax=value;
            if(field==2)d->solver.h=value;
            if(field==3)d->solver.step=(int)value;
            if(field==4)d->solver.sf=(int)value;
            if(field==5)d->solver.max_steps=(uint32_t)value;
            if(field==7)d->adaptive.reltol=value;
            if(field==8)d->adaptive.abstol=value;
            OdeStatus status=ode_validate(&d->solver);
            if(status==ODE_OK)status=ode_adaptive_validate(&d->adaptive);
            if(status!=ODE_OK) {
                d->solver=before;d->adaptive=old;
                ui_message("Invalid parameter",status==ODE_BAD_STEP ? "h must be finite and > 0.":ode_status_text(status));continue;
            }
            if(field<=1)d->solver_custom=1;
            edit.active=false;
            if(key==KEY_EXE || key==KEY_EXIT){ui_field_complete(key,true,&selected,count);continue;}
            if(key==KEY_UP || key==KEY_DOWN) {
                if(key==KEY_UP && selected>0)selected--;
                if(key==KEY_DOWN && selected+1<count)selected++;
                continue;
            }
        }
        key=ui_field_complete(key,false,&selected,count);event.key=(unsigned)key;
        if(stage_leave(key) && key!=KEY_F1) {
            state->selected=selected;state->edit=edit;state->top=0;return stage_action(key);
        }
        if(key==KEY_F2){advanced=true;continue;}
        if(key==KEY_F1) {
            int sf=model_field_supported(d) ? 12:d->solver.sf,step=adaptive ? d->solver.step:1;
            d->solver=(OdeSettings){0,1,.1,20000,step,sf};d->solver_custom=0;
            if(adaptive){ode_adaptive_defaults(&d->adaptive);d->adaptive.method=ODE_RK45;}
            int projection=d->view.phase;d->view.phase=0;
            model_sync_solver_window(d);d->view.phase=projection;selected=0;continue;
        }
        if(field==6) {
            if(key==KEY_LEFT || key==KEY_RIGHT)d->adaptive.method=adaptive ? ODE_RK4:ODE_RK45;
            ui_select_move(key,&selected,count);
            continue;
        }
        double current=field==0 ? d->solver.xmin:(field==1 ? d->solver.xmax:(field==2 ? d->solver.h:
            (field==3 ? d->solver.step:(field==4 ? d->solver.sf:(field==5 ? (double)d->solver.max_steps:
            (field==7 ? d->adaptive.reltol:d->adaptive.abstol))))));
        number_select(&edit,event,current,&selected,count);
    }
}
void ui_vwindow(Document *d)
{
    int selected=0;NumberEdit edit={0};ViewWindow *active=model_view(d);
    bool phase=model_phase_supported(d) && d->view.phase;
    for(;;) {
        const char *labels[]={"Xmin","max","scale","dot","Ymin","max","scale"};
        char values[7][48];double dot=model_xdot(active);
        snprintf(values[0],48,"%.9g",active->xmin);snprintf(values[1],48,"%.9g",active->xmax);
        snprintf(values[2],48,"%.9g",active->xscale);snprintf(values[3],48,"%.9g",dot);
        snprintf(values[4],48,"%.9g",active->ymin);snprintf(values[5],48,"%.9g",active->ymax);
        snprintf(values[6],48,"%.9g",active->yscale);
        ui_frame(phase ? "Phase View Window":"View Window",NULL);
        for(int i=0;i<7;i++)ui_field(i,labels[i],edit.active && i==selected ? edit.text:values[i],i==selected);
        number_cursor(&edit,selected);
        ui_form_hint(&edit,"Xdot edits Xmax; Xmin/Xmax recalculate Xdot");
        if(edit.active)ui_softkeys("","FUNC","","CLEAR","DEL","OK");
        else ui_softkeys("INIT","","","","","DONE");
        dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(edit.active) {
            int action=key==KEY_EXIT || key==KEY_UP || key==KEY_DOWN ? 1:number_key(&edit,event);
            if(action==0)continue;
            double value;ViewWindow before=*active;
            if(!number_value(&edit,&value))continue;
            if(selected==0)active->xmin=value;
            if(selected==1)active->xmax=value;
            if(selected==2)active->xscale=value;
            if(selected==3 && !model_set_xdot(active,value)) {
                *active=before;ui_message("Invalid Xdot","Xdot must be positive and finite.");continue;
            }
            if(selected==4)active->ymin=value;
            if(selected==5)active->ymax=value;
            if(selected==6)active->yscale=value;
            ViewWindow *v=active;
            if(v->xmin>=v->xmax || v->ymin>=v->ymax || v->xscale<=0 || v->yscale<=0
                || !isfinite(v->xmax-v->xmin) || !isfinite(v->ymax-v->ymin)) {
                *active=before;ui_message("Invalid window","Require min < max and positive scales.");continue;
            }
            if(selected<=3)model_sync_solver_window(d);
            if(phase)d->phase_ready=1;
            edit.active=false;
            ui_field_complete(key,true,&selected,7);
            if(key==KEY_UP && selected>0)selected--;
            if(key==KEY_DOWN && selected<6)selected++;
            continue;
        }
        key=ui_field_complete(key,false,&selected,7);event.key=(unsigned)key;
        if(key==KEY_EXIT || key==KEY_F6)return;
        if(key==KEY_F1){
            int grid=active->grid,labels=active->labels;
            int projection=active->phase,px=active->phase_x,py=active->phase_y;
            if(phase){model_phase_window_defaults(active);d->phase_ready=1;}
            else model_window_defaults(active);
            active->grid=grid;active->labels=labels;active->phase=projection;
            active->phase_x=px;active->phase_y=py;
            model_sync_solver_window(d);selected=0;continue;
        }
        double current=selected==0 ? active->xmin:(selected==1 ? active->xmax:(selected==2 ? active->xscale:
            (selected==3 ? dot:(selected==4 ? active->ymin:(selected==5 ? active->ymax:active->yscale)))));
        number_select(&edit,event,current,&selected,7);
    }
}
static int choose_color(const char *title,const char *const names[6],int (*color)(unsigned),unsigned initial);
static const char *const field_names[]={"Pale Blue","Pale Red","Pale Cyan","Pale Magenta","Pale Gold","Gray"};
static const char *const curve_names[]={"Blue","Red","Magenta","Black","Cyan","Bright Green"};
void ui_graph_settings(Document *d)
{
    ViewWindow *view=model_view(d);
    int selected=0;bool supported=model_field_supported(d);int count=supported ? 4:2;
    for(;;) {
        ui_frame("Graph settings",NULL);
        ui_field(0,"Grid",view->grid ? "On":"Off",selected==0);
        ui_field(1,"Axis Label",view->labels ? "On":"Off",selected==1);
        if(supported) {
            ui_text(14,94,UI_MUTED,"Slope Field");
            ui_field(4,"Style",d->field_style==FIELD_ARROW ? "Arrow":"Segment",selected==2);
            ui_field(5,"Color",field_names[d->field_color<FIELD_COLORS ? d->field_color:0],selected==3);
            ui_color_swatch(326,142,graph_field_color(d->field_color));
        }
        ui_form_hint(NULL,selected<2 ? "LEFT/RIGHT: ON/OFF toggle":
            (selected==2 ? "LEFT/RIGHT: SEGMENT/ARROW toggle":"RIGHT/F3: COLOR"));
        ui_softkeys(selected==2 ? "":"INIT","",selected==3 ? "COLOR":"","","","DONE");
        dupdate();int key=ui_getkey().key;
        key=ui_field_complete(key,false,&selected,count);
        if(key==KEY_EXIT || key==KEY_F6)return;
        if(ui_select_move(key,&selected,count))continue;
        if(key==KEY_F1 && selected!=2) {
            view->grid=view->labels=1;model_field_appearance_defaults(d);selected=0;continue;
        }
        if(selected<2) {
            int *value=selected ? &view->labels:&view->grid;
            if(key==KEY_LEFT || key==KEY_RIGHT)*value=!*value;
        } else if(selected==2) {
            if(key==KEY_LEFT || key==KEY_RIGHT)d->field_style=(uint8_t)!d->field_style;
        } else if(key==KEY_RIGHT || key==KEY_F3) {
            int color=choose_color("Field color",field_names,graph_field_color,d->field_color);
            if(color>=0)d->field_color=(uint8_t)color;
        }
    }
}
UiStageAction ui_initial_conditions(Document *d,UiStageState *state)
{
    int selected=state->selected;NumberEdit edit=state->edit;
    bool scalar=model_field_supported(d);int count=d->dim+1;
    if(selected>=count)selected=count-1;
    for(;;) {
        int page=selected/7;char current[EXPR_TEXT]={0};
        ui_frame("Initial Conditions",NULL);
        ui_progress(2);
        for(int row=0;row<7 && page*7+row<count;row++) {
            int field=page*7+row;char label[24],value[EXPR_TEXT];
            if(!field)snprintf(label,sizeof(label),"x0");
            else {model_variable_label(d,field-1,label,sizeof(label));
                size_t n=strlen(label);snprintf(label+n,sizeof(label)-n,d->kind==EQ_SYSTEM ? "_0":"0");}
            if(scalar && field)initial_values_format(d,value,sizeof(value));
            else snprintf(value,sizeof(value),"%.12g",field ? d->ic[0].y[field-1]:d->ic[0].x);
            ui_field(row,label,edit.active && field==selected ? edit.text:value,field==selected);
            if(field==selected){snprintf(current,sizeof(current),"%s",value);if(edit.active)number_cursor(&edit,row);}
        }
        if(scalar)ui_text(10,170,UI_MUTED,"y0: scalar or {values}; at most 10");
        ui_form_hint(&edit,scalar ? "Comma: separator":"One solution: x0 plus all state values");
        ui_softkeys("","","","","","NEXT");dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(key>=KEY_F1 && key<=KEY_F5)continue;
        if(edit.active) {
            int action=stage_leave(key) || key==KEY_UP || key==KEY_DOWN ? 1:number_key(&edit,event);
            if(!action)continue;
            if(edit.limited){ui_message("Initial values",initial_values_error(IC_LIST_LENGTH));continue;}
            if(scalar && selected==1) {
                InitialValues values;IcListStatus status=initial_values_parse(edit.text,&values);
                if(status!=IC_LIST_OK){ui_message("Initial values",initial_values_error(status));continue;}
                initial_values_apply(d,&values);
            } else {
                double value;if(!number_value(&edit,&value))continue;
                if(fabs(value)>1e100){ui_message("Initial value","Magnitude must be at most 1e100.");continue;}
                if(selected)d->ic[0].y[selected-1]=value;
                else {d->ic[0].x=value;if(scalar)for(int i=1;i<d->nic;i++)d->ic[i].x=value;}
                if(!scalar)d->nic=1;
            }
            edit.active=false;
            if(key==KEY_EXE || key==KEY_EXIT){ui_field_complete(key,true,&selected,count);continue;}
            if(key==KEY_UP || key==KEY_DOWN) {
                if(key==KEY_UP && selected>0)selected--;
                if(key==KEY_DOWN && selected+1<count)selected++;
                continue;
            }
        }
        key=ui_field_complete(key,false,&selected,count);event.key=(unsigned)key;
        if(key==KEY_EXIT || key==KEY_F6) {
            if(key==KEY_F6 && !d->nic){ui_message("Initial values","Enter y0 before continuing.");continue;}
            state->selected=selected;state->edit=edit;return stage_action(key);
        }
        ui_field_select(&edit,event,current,&selected,count);
    }
}
static int choose_color(const char *title,const char *const names[6],int (*color)(unsigned),unsigned initial)
{
    int selected=initial<6 ? (int)initial:0;
    for(;;) {
        ui_rect(71,42,242,139,UI_INK);ui_rect(74,45,236,133,C_WHITE);
        ui_text(84,53,UI_INK,"%s",title);
        for(int i=0;i<6;i++) {
            int x=94+(i%3)*68,y=78+(i/3)*34;
            ui_rect(x-4,y-4,56,30,i==selected ? UI_INK:UI_LINE);
            ui_rect(x-2,y-2,52,26,C_WHITE);
            ui_rect(x,y,48,22,color((unsigned)i));
        }
        ui_text(84,151,UI_INK,"%s",names[selected]);
        ui_help(84,165,"EXE: select   EXIT: cancel",false);
        ui_softkeys("","","","","","");dupdate();
        int key=ui_getkey().key;
        if(key==KEY_EXIT)return -1;
        if(key==KEY_EXE)return selected;
        if(key==KEY_LEFT && selected%3)selected--;
        if(key==KEY_RIGHT && selected%3<2)selected++;
        if(key==KEY_UP && selected>=3)selected-=3;
        if(key==KEY_DOWN && selected<3)selected+=3;
    }
}
void ui_output(Document *d)
{
    int selected=0;bool modified=false;
    for(;;) {
        ui_frame("Output selection",NULL);
        int page=selected/7;
        for(int row=0;row<7 && page*7+row<d->dim;row++) {
            int index=page*7+row;char label[20];model_variable_label(d,index,label,sizeof(label));
            ui_field(row,label,d->enabled&(1u<<index) ? "ON":"OFF",index==selected);
            ui_color_swatch(326,32+row*22,graph_palette_color(model_color(d,0,index)));
        }
        ui_form_hint(NULL,"LEFT/RIGHT: ON/OFF toggle, F3: COLOR");
        ui_softkeys("INIT","","COLOR","","","DONE");dupdate();
        int key=ui_getkey().key;
        if(key==KEY_EXE){key=ui_list_complete(key,modified,&selected,d->dim);modified=false;}
        if(key==KEY_EXIT || key==KEY_F6)return;
        if(key==KEY_UP || key==KEY_DOWN) {
            ui_select_move(key,&selected,d->dim);
            modified=false;
        }
        if(key==KEY_LEFT || key==KEY_RIGHT){d->enabled^=(uint16_t)(1u<<selected);modified=true;}
        if(key==KEY_F3) {
            unsigned old=model_color(d,0,selected);
            int color=choose_color("Curve color",curve_names,graph_palette_color,old);
            if(color>=0 && (unsigned)color!=old){model_output_color(d,selected,(unsigned)color);modified=true;}
        }
        if(key==KEY_F1){model_output_defaults(d);selected=0;modified=false;}
    }
}
