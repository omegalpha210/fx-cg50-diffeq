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
/* UI indices refer only to visible rows. Solver fields keep their identity. */
static int parameter_field(int row,bool field_supported)
{return !field_supported && row>=4 ? row+1:row;}
UiStageAction ui_parameters(Document *d,UiStageState *state)
{
    static const char *const help[]={"Integration start","Integration end",
        "RK4 h > 0; smaller means more work","Output spacing only; h is unchanged",
        "Slope-field columns (0-100); 0 = Off",
        "Max RK4 steps per IC / direction"};
    bool field_supported=model_field_supported(d);int count=field_supported ? 6:5;
    int selected=state->selected;NumberEdit edit=state->edit;
    if(selected<0 || selected>=count){selected=count-1;edit.active=false;}
    for(;;) {
        const char *labels[]={"Xrange min","Xrange max","h","Step","SF","Max steps"};
        char values[6][48];
        snprintf(values[0],48,"%.9g",d->solver.xmin);snprintf(values[1],48,"%.9g",d->solver.xmax);
        snprintf(values[2],48,"%.9g",d->solver.h);snprintf(values[3],48,"%d",d->solver.step);
        snprintf(values[4],48,"%d",d->solver.sf);snprintf(values[5],48,"%lu",(unsigned long)d->solver.max_steps);
        ui_frame("Parameter",NULL);
        int field=parameter_field(selected,field_supported);
        for(int row=0;row<count;row++) {
            int item=parameter_field(row,field_supported);
            ui_field(row,labels[item],edit.active && row==selected ? edit.text:values[item],row==selected);
        }
        number_cursor(&edit,selected);
        ui_form_hint(&edit,help[field]);
        ui_softkeys("PREV","INIT","V-WIN","OUTPUT","SET","GRAPH");
        dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(edit.active) {
            int action=stage_leave(key) || key==KEY_UP || key==KEY_DOWN || key==KEY_F2 ? 1:number_key(&edit,event);
            if(action==0)continue;
            double value;OdeSettings before=d->solver;
            if(!number_value(&edit,&value))continue;
            if(field>=3 && (value<0 || value>100000 || value!=floor(value))) {
                ui_message("Invalid setting","Use an integer within the setting limit.");continue;
            }
            if(field==0)d->solver.xmin=value;
            if(field==1)d->solver.xmax=value;
            if(field==2)d->solver.h=value;
            if(field==3)d->solver.step=(int)value;
            if(field==4)d->solver.sf=(int)value;
            if(field==5)d->solver.max_steps=(uint32_t)value;
            OdeStatus status=ode_validate(&d->solver);
            if(status!=ODE_OK){d->solver=before;ui_message("Invalid parameter",status==ODE_BAD_STEP ? "h must be finite and > 0.":ode_status_text(status));continue;}
            if(field<=1)d->solver_custom=1;
            edit.active=false;
            if(key==KEY_EXE || key==KEY_EXIT){ui_field_complete(key,true,&selected,count);continue;}
        }
        key=ui_field_complete(key,false,&selected,count);event.key=(unsigned)key;
        if(stage_leave(key)) {
            state->selected=selected;state->edit=edit;return stage_action(key);
        }
        if(key==KEY_F2){
            int sf=field_supported ? 12:d->solver.sf;
            d->solver=(OdeSettings){0,1,.1,20000,1,sf};d->solver_custom=0;
            model_sync_solver_window(d);selected=0;continue;
        }
        double current=field==0 ? d->solver.xmin:(field==1 ? d->solver.xmax:(field==2 ? d->solver.h:
            (field==3 ? d->solver.step:(field==4 ? d->solver.sf:(double)d->solver.max_steps))));
        number_select(&edit,event,current,&selected,count);
    }
}
void ui_vwindow(Document *d)
{
    int selected=0;NumberEdit edit={0};
    for(;;) {
        const char *labels[]={"Xmin","max","scale","dot","Ymin","max","scale"};
        char values[7][48];double dot=model_xdot(&d->view);
        snprintf(values[0],48,"%.9g",d->view.xmin);snprintf(values[1],48,"%.9g",d->view.xmax);
        snprintf(values[2],48,"%.9g",d->view.xscale);snprintf(values[3],48,"%.9g",dot);
        snprintf(values[4],48,"%.9g",d->view.ymin);snprintf(values[5],48,"%.9g",d->view.ymax);
        snprintf(values[6],48,"%.9g",d->view.yscale);
        ui_frame("View Window",NULL);
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
            double value;ViewWindow before=d->view;
            if(!number_value(&edit,&value))continue;
            if(selected==0)d->view.xmin=value;
            if(selected==1)d->view.xmax=value;
            if(selected==2)d->view.xscale=value;
            if(selected==3 && !model_set_xdot(&d->view,value)) {
                d->view=before;ui_message("Invalid Xdot","Xdot must be positive and finite.");continue;
            }
            if(selected==4)d->view.ymin=value;
            if(selected==5)d->view.ymax=value;
            if(selected==6)d->view.yscale=value;
            ViewWindow *v=&d->view;
            if(v->xmin>=v->xmax || v->ymin>=v->ymax || v->xscale<=0 || v->yscale<=0
                || !isfinite(v->xmax-v->xmin) || !isfinite(v->ymax-v->ymin)) {
                d->view=before;ui_message("Invalid window","Require min < max and positive scales.");continue;
            }
            if(selected<=3)model_sync_solver_window(d);
            edit.active=false;
            ui_field_complete(key,true,&selected,7);
            if(key==KEY_UP && selected>0)selected--;
            if(key==KEY_DOWN && selected<6)selected++;
            continue;
        }
        key=ui_field_complete(key,false,&selected,7);event.key=(unsigned)key;
        if(key==KEY_EXIT || key==KEY_F6)return;
        if(key==KEY_F1){model_window_defaults(&d->view);model_sync_solver_window(d);selected=0;continue;}
        double current=selected==0 ? d->view.xmin:(selected==1 ? d->view.xmax:(selected==2 ? d->view.xscale:
            (selected==3 ? dot:(selected==4 ? d->view.ymin:(selected==5 ? d->view.ymax:d->view.yscale)))));
        number_select(&edit,event,current,&selected,7);
    }
}
static int choose_color(const char *title,const char *const names[6],int (*color)(unsigned),unsigned initial);
static const char *const field_names[]={"Pale Blue","Pale Red","Pale Cyan","Pale Magenta","Pale Gold","Gray"};
static const char *const curve_names[]={"Blue","Red","Magenta","Black","Cyan","Bright Green"};
void ui_graph_settings(Document *d)
{
    int selected=0;bool supported=model_field_supported(d);int count=supported ? 4:2;
    for(;;) {
        ui_frame("Graph settings",NULL);
        ui_field(0,"Grid",d->view.grid ? "On":"Off",selected==0);
        ui_field(1,"Axis Label",d->view.labels ? "On":"Off",selected==1);
        if(supported) {
            ui_text(14,94,UI_MUTED,"Slope Field");
            ui_field(4,"Style",d->field_style==FIELD_ARROW ? "Arrow":"Segment",selected==2);
            ui_field(5,"Color",field_names[d->field_color<FIELD_COLORS ? d->field_color:0],selected==3);
            ui_color_swatch(326,142,graph_field_color(d->field_color));
        }
        ui_form_hint(NULL,supported ? "Density SF: Parameters   EXE: DONE":
            "Field: first-order only; preference kept");
        if(selected<2)ui_softkeys("ON","OFF","","INIT","","DONE");
        else if(selected==2)ui_softkeys("SEG","ARROW","","INIT","","DONE");
        else ui_softkeys("","","COLOR","INIT","","DONE");
        dupdate();int key=ui_getkey().key;
        key=ui_field_complete(key,false,&selected,count);
        if(key==KEY_EXIT || key==KEY_F6)return;
        if(key==KEY_UP && selected>0)selected--;
        if(key==KEY_DOWN && selected+1<count)selected++;
        if(key==KEY_F4) {
            d->view.grid=d->view.labels=1;model_field_appearance_defaults(d);selected=0;continue;
        }
        if(selected<2) {
            int *value=selected ? &d->view.labels:&d->view.grid;
            if(key==KEY_F1)*value=1;
            if(key==KEY_F2)*value=0;
            if(key==KEY_LEFT || key==KEY_RIGHT)*value=!*value;
        } else if(selected==2) {
            if(key==KEY_F1)d->field_style=FIELD_SEGMENT;
            if(key==KEY_F2)d->field_style=FIELD_ARROW;
            if(key==KEY_LEFT || key==KEY_RIGHT)d->field_style=(uint8_t)!d->field_style;
        } else if(key==KEY_LEFT || key==KEY_RIGHT || key==KEY_F3) {
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
        if(scalar)ui_text(10,170,UI_MUTED,"y0: scalar or {values}; at most 9");
        ui_form_hint(&edit,scalar ? "Comma: separator":"One solution: x0 plus all state values");
        ui_softkeys("PREV","","V-WIN","","","NEXT");dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(key==KEY_F2 || key==KEY_F4 || key==KEY_F5)continue;
        if(edit.active) {
            int action=stage_leave(key) || key==KEY_UP || key==KEY_DOWN ? 1:number_key(&edit,event);
            if(!action)continue;
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
        }
        key=ui_field_complete(key,false,&selected,count);event.key=(unsigned)key;
        if(key==KEY_EXIT || key==KEY_F1 || key==KEY_F3 || key==KEY_F6) {
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
        ui_text(84,165,UI_MUTED,"EXE: select   EXIT: cancel");
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
        if(d->dim-page*7<7)ui_text(8,170,UI_MUTED,"ON/OFF: Graph, Table and CSV");
        ui_form_hint(NULL,"LEFT/RIGHT: ON/OFF toggle");
        ui_softkeys("","","COLOR","INIT","","DONE");dupdate();
        int key=ui_getkey().key;
        if(key==KEY_EXE){key=ui_list_complete(key,modified,&selected,d->dim);modified=false;}
        if(key==KEY_EXIT || key==KEY_F6)return;
        if(key==KEY_UP || key==KEY_DOWN) {
            if(key==KEY_UP && selected>0)selected--;
            if(key==KEY_DOWN && selected+1<d->dim)selected++;
            modified=false;
        }
        if(key==KEY_LEFT || key==KEY_RIGHT){d->enabled^=(uint16_t)(1u<<selected);modified=true;}
        if(key==KEY_F3) {
            unsigned old=model_color(d,0,selected);
            int color=choose_color("Curve color",curve_names,graph_palette_color,old);
            if(color>=0 && (unsigned)color!=old){model_output_color(d,selected,(unsigned)color);modified=true;}
        }
        if(key==KEY_F4){model_output_defaults(d);selected=0;modified=false;}
    }
}
