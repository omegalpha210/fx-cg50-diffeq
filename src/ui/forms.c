#include "ui.h"
#include "graph.h"
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
static bool number_value(NumberEdit *e,const double *constants,double *out)
{
    ExprProgram p;ExprError error=expr_compile(e->text,
        (ExprScope){0,false,false,false,constants},&p);
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
UiStageAction ui_parameters(Document *d,UiStageState *state)
{
    static const char *const help[]={"Integration start","Integration end",
        "RK4 integration interval","Output every N RK4 steps",
        "Slope-field density","Iteration safety limit"};
    int selected=state->selected;NumberEdit edit=state->edit;
    for(;;) {
        const char *labels[]={"Xrange min","Xrange max","h","Step","SF","Max steps"};
        char values[6][48];
        snprintf(values[0],48,"%.9g",d->solver.xmin);snprintf(values[1],48,"%.9g",d->solver.xmax);
        snprintf(values[2],48,"%.9g",d->solver.h);snprintf(values[3],48,"%d",d->solver.step);
        snprintf(values[4],48,"%d",d->solver.sf);snprintf(values[5],48,"%lu",(unsigned long)d->solver.max_steps);
        ui_frame("Parameter",NULL);
        for(int i=0;i<6;i++)ui_field(i,labels[i],edit.active && i==selected ? edit.text:values[i],i==selected);
        number_cursor(&edit,selected);
        ui_text(10,174,UI_MUTED,"%s",help[selected]);
        ui_stage_softkeys(2);
        dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(edit.active) {
            int action=stage_leave(key) || key==KEY_UP || key==KEY_DOWN || key==KEY_F2 ? 1:number_key(&edit,event);
            if(action==0)continue;
            double value;OdeSettings before=d->solver;
            if(!number_value(&edit,d->constants,&value))continue;
            if(selected>=3 && (value<0 || value>100000 || value!=floor(value))) {
                ui_message("Invalid setting","Use an integer within the setting limit.");continue;
            }
            if(selected==0)d->solver.xmin=value;
            if(selected==1)d->solver.xmax=value;
            if(selected==2)d->solver.h=value;
            if(selected==3)d->solver.step=(int)value;
            if(selected==4)d->solver.sf=(int)value;
            if(selected==5)d->solver.max_steps=(uint32_t)value;
            OdeStatus status=ode_validate(&d->solver);
            if(status!=ODE_OK){d->solver=before;ui_message("Invalid parameter",ode_status_text(status));continue;}
            if(selected<=1)d->solver_custom=1;
            edit.active=false;
            if(key==KEY_EXE || key==KEY_EXIT){ui_field_complete(key,true,&selected,6);continue;}
        }
        key=ui_field_complete(key,false,&selected,6);event.key=(unsigned)key;
        if(stage_leave(key)) {
            state->selected=selected;state->edit=edit;return stage_action(key);
        }
        if(key==KEY_F2){
            d->solver=(OdeSettings){0,1,.1,20000,1,12};d->solver_custom=0;
            model_sync_solver_window(d);selected=0;continue;
        }
        double current=selected==0 ? d->solver.xmin:(selected==1 ? d->solver.xmax:(selected==2 ? d->solver.h:
            (selected==3 ? d->solver.step:(selected==4 ? d->solver.sf:(double)d->solver.max_steps))));
        number_select(&edit,event,current,&selected,6);
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
        ui_text(10,184,UI_MUTED,"Xdot edits Xmax; Xmin/Xmax recalculate Xdot");
        if(edit.active)ui_softkeys("","FUNC","CONST","CLEAR","DEL","OK");
        else ui_softkeys("INIT","","","","","DONE");
        dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(edit.active) {
            int action=key==KEY_EXIT || key==KEY_UP || key==KEY_DOWN ? 1:number_key(&edit,event);
            if(action==0)continue;
            double value;ViewWindow before=d->view;
            if(!number_value(&edit,d->constants,&value))continue;
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
void ui_graph_settings(Document *d)
{
    int selected=0;
    for(;;) {
        ui_frame("Graph settings",NULL);
        ui_field(0,"Grid",d->view.grid ? "On":"Off",selected==0);
        ui_field(1,"Axis Label",d->view.labels ? "On":"Off",selected==1);
        ui_text(10,184,UI_MUTED,"LEFT/RIGHT: option   EXE: next field");
        ui_softkeys("ON","OFF","","INIT","","DONE");dupdate();
        int key=ui_getkey().key;
        key=ui_field_complete(key,false,&selected,2);
        if(key==KEY_EXIT || key==KEY_F6)return;
        if(key==KEY_UP)selected=0;
        if(key==KEY_DOWN)selected=1;
        if(key==KEY_F4){ViewWindow defaults;model_window_defaults(&defaults);
            d->view.grid=defaults.grid;d->view.labels=defaults.labels;selected=0;continue;}
        int *value=selected ? &d->view.labels:&d->view.grid;
        if(key==KEY_F1)*value=1;
        if(key==KEY_F2)*value=0;
        if(key==KEY_LEFT || key==KEY_RIGHT)*value=!*value;
    }
}
void ui_constants(Document *d)
{
    int selected=0;NumberEdit edit={0};
    for(;;) {
        char labels[26][12],values[26][48];int indexes[26],count=0;
        for(int i=0;i<28;i++) {
            if(i==23 || i==24) continue;
            if(i<26) snprintf(labels[count],12,"%c",'A'+i);
            else snprintf(labels[count],12,"%s",i==26 ? "r":"theta");
            snprintf(values[count],48,"%.9g",d->constants[i]);
            indexes[count]=i;count++;
        }
        int page=selected/7;
        ui_frame("Private constants",NULL);
        for(int row=0;row<7 && page*7+row<count;row++) {
            int i=page*7+row;
            ui_field(row,labels[i],edit.active && i==selected ? edit.text:values[i],i==selected);
            if(edit.active && i==selected)number_cursor(&edit,row);
        }
        ui_text(10,184,UI_MUTED,"X and Y are reserved   UP/DOWN scroll");
        if(edit.active)ui_softkeys("","FUNC","CONST","CLEAR","DEL","OK");
        else ui_softkeys("ZERO","PG-","PG+","","ALL 0","DONE");
        dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(edit.active) {
            int action=key==KEY_EXIT || key==KEY_UP || key==KEY_DOWN ? 1:number_key(&edit,event);
            if(action==0)continue;
            double value;if(!number_value(&edit,d->constants,&value))continue;
            if(fabs(value)>1e100) {
                ui_message("Invalid value","Magnitude must be at most 1e100.");continue;
            }
            d->constants[indexes[selected]]=value;
            edit.active=false;ui_field_complete(key,true,&selected,count);
            if(key==KEY_UP && selected>0)selected--;
            if(key==KEY_DOWN && selected+1<count)selected++;
            continue;
        }
        key=ui_field_complete(key,false,&selected,count);event.key=(unsigned)key;
        if(key==KEY_EXIT || key==KEY_F6)return;
        if(key==KEY_F1)d->constants[indexes[selected]]=0;
        if(key==KEY_F2)selected=selected>=7 ? selected-7:0;
        if(key==KEY_F3)selected=selected+7<count ? selected+7:count-1;
        if(key==KEY_F5){memset(d->constants,0,sizeof(d->constants));selected=0;}
        number_select(&edit,event,d->constants[indexes[selected]],&selected,count);
    }
}
UiStageAction ui_initial_conditions(Document *d,UiStageState *state)
{
    int selected=state->selected,top=state->top;NumberEdit edit=state->edit;
    for(;;) {
        int fields=d->dim+1,total=d->nic*fields;
        if(selected>=total)selected=total ? total-1:0;
        int family=total ? selected/fields:0,field=total ? selected%fields:0;
        int block_lines=fields+1,line=family*block_lines+field+1;
        if(line<top)top=line;
        if(line>=top+7)top=line-6;
        int all_lines=d->nic*block_lines;if(top>all_lines-1)top=all_lines ? all_lines-1:0;
        ui_frame("Initial Conditions",NULL);
        if(d->nic==0) {
            ui_text(15,70,UI_INK,"No initial conditions.");
            ui_text(15,93,UI_MUTED,"First-order + SF > 0: slope field only.");
        } else for(int row=0;row<7 && top+row<all_lines;row++) {
            int display_line=top+row,display_family=display_line/block_lines;
            int position=display_line%block_lines;
            if(position==0) {
                ui_rect(8,27+row*22,372,19,UI_PALE);
                ui_text(14,31+row*22,UI_BLUE,"IC %d",display_family+1);
                continue;
            }
            int display_field=position-1;
            int ordinal=display_family*fields+display_field;
            char label[24],value[48];
            if(display_field==0)snprintf(label,sizeof(label),"x0");
            else {
                model_variable_label(d,display_field-1,label,sizeof(label));
                if(d->kind!=EQ_SYSTEM) {
                    size_t length=strlen(label);
                    if(length+1<sizeof(label)){label[length]='0';label[length+1]=0;}
                }
            }
            snprintf(value,sizeof(value),"%.12g",display_field ?
                d->ic[display_family].y[display_field-1]:d->ic[display_family].x);
            ui_field(row,label,edit.active && ordinal==selected ? edit.text:value,ordinal==selected);
            if(edit.active && ordinal==selected)number_cursor(&edit,row);
        }
        if(d->nic)ui_text(10,184,UI_MUTED,"IC %d   UP/DOWN scroll",family+1);
        ui_stage_softkeys(1);
        dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(key==KEY_F2)continue;
        if(edit.active) {
            int action=stage_leave(key) || key==KEY_UP || key==KEY_DOWN ? 1:number_key(&edit,event);
            if(action==0)continue;
            double value;if(!number_value(&edit,d->constants,&value))continue;
            if(fabs(value)>1e100){ui_message("Initial value","Magnitude must be at most 1e100.");continue;}
            if(field)d->ic[family].y[field-1]=value;else d->ic[family].x=value;
            edit.active=false;
            if(key==KEY_EXE || key==KEY_EXIT){ui_field_complete(key,true,&selected,total);continue;}
        }
        key=ui_field_complete(key,false,&selected,total);event.key=(unsigned)key;
        if(key==KEY_EXIT || key==KEY_F1 || key==KEY_F3 || key==KEY_F6) {
            if(key==KEY_F6 && d->nic==0) {
                if(d->kind>EQ_GENERAL || d->solver.sf==0) {
                    ui_message("Initial Conditions","Add an IC set. Slope-only needs first-order and SF > 0.");continue;
                }
            }
            state->selected=selected;state->top=top;state->edit=edit;return stage_action(key);
        }
        int management=key==KEY_F4 ? 0:(key==KEY_F5 ? 1:-1);
        if(management==0 && d->nic==9)ui_message("Initial Conditions","At most 9 sets.");
        if(management==0 && d->nic<9) {
            if(d->nic) d->ic[d->nic]=d->ic[family];
            for(int j=0;j<ODE_MAX_DIM;j++)d->color[d->nic][j]=(uint8_t)
                model_default_color(d->nic,j,d->dim);
            d->graph_mask[d->nic]=(uint16_t)((1u<<d->dim)-1);
            d->list_mask[d->nic]=(uint16_t)((1u<<(d->dim+1))-1);
            selected=d->nic*fields;d->nic++;
        }
        if(management==1 && d->nic==1 && (d->kind>EQ_GENERAL || d->solver.sf==0)) {
            ui_message("Initial Conditions","This mode needs at least one IC set.");continue;
        }
        if(management==1 && d->nic>0) {
            for(int i=family;i<d->nic-1;i++) {
                d->ic[i]=d->ic[i+1];d->graph_mask[i]=d->graph_mask[i+1];d->list_mask[i]=d->list_mask[i+1];
                memcpy(d->color[i],d->color[i+1],sizeof(d->color[i]));
            }
            d->nic--;if(family>=d->nic)family=d->nic ? d->nic-1:0;
            selected=family*fields;
        }
        if(management>=0)continue;
        if(d->nic>0)number_select(&edit,event,field ? d->ic[family].y[field-1]:d->ic[family].x,&selected,total);
    }
}
static int choose_color(unsigned initial)
{
    int selected=(int)initial;
    static const char *const names[]={"Blue","Red","Magenta","Black","Cyan","Bright Green"};
    for(;;) {
        ui_rect(71,42,242,139,UI_INK);ui_rect(74,45,236,133,C_WHITE);
        ui_text(84,53,UI_INK,"Curve color");
        for(int i=0;i<6;i++) {
            int x=94+(i%3)*68,y=78+(i/3)*34;
            ui_rect(x-4,y-4,56,30,i==selected ? UI_INK:UI_LINE);
            ui_rect(x-2,y-2,52,26,C_WHITE);
            ui_rect(x,y,48,22,graph_palette_color((unsigned)i));
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
    if(!d->nic){ui_message("Output","Add an initial condition before selecting output.");return;}
    int family=0,selected=0,property=0;bool modified=false;
    for(;;) {
        int family_page=family/5,row_page=selected/8;
        ui_frame("Output selection",NULL);
        ui_rect(4,26,54,20,UI_PALE);ui_text(9,31,UI_BLUE,"Var");
        for(int col=0;col<5 && family_page*5+col<d->nic;col++) {
            int set=family_page*5+col;char heading[12];snprintf(heading,sizeof(heading),"%d%s",set+1,
                set==0 ? "st":(set==1 ? "nd":(set==2 ? "rd":"th")));
            ui_rect(58+col*64,26,63,20,set==family ? UI_BLUE:UI_PALE);
            ui_text(76+col*64,31,set==family ? C_WHITE:UI_BLUE,"%s",heading);
        }
        for(int row=0;row<8 && row_page*8+row<=d->dim;row++) {
            int index=row_page*8+row,y=49+row*17;char label[16];
            model_variable_label(d,index-1,label,sizeof(label));
            ui_rect(4,y-2,54,17,index==selected ? UI_PALE:C_WHITE);ui_text(10,y,UI_INK,"%s",label);
            for(int col=0;col<5 && family_page*5+col<d->nic;col++) {
                int set=family_page*5+col;bool active=set==family && index==selected;
                char cell[16];snprintf(cell,sizeof(cell),"%c %c",index==0 ? 'n':
                    ((d->graph_mask[set]&(1u<<(index-1))) ? 'G':'_'),
                    (d->list_mask[set]&(1u<<index)) ? 'L':'_');
                ui_rect(58+col*64,y-2,63,17,active ? UI_BLUE:(row%2 ? UI_PALE:C_WHITE));
                ui_text(62+col*64,y,active ? C_WHITE:UI_INK,"%s",cell);
                if(index>0) {
                    int x=103+col*64;
                    ui_rect(x-2,y-1,17,14,active && property ? C_WHITE:UI_INK);
                    ui_rect(x,y+1,13,10,graph_palette_color(model_color(d,set,index-1)));
                }
            }
        }
        ui_text(8,184,UI_MUTED,"EXE: next/done   RIGHT/F3: color");
        ui_softkeys("GRAPH","LIST","COLOR","INIT","","DONE");dupdate();
        int key=ui_getkey().key;
        if(key==KEY_EXE) {
            key=ui_field_complete(key,modified,&selected,d->dim+1);
            modified=false;property=0;
        }
        if(key==KEY_EXIT || key==KEY_F6) return;
        if(key==KEY_UP || key==KEY_DOWN){
            if(key==KEY_UP && selected>0)selected--;
            if(key==KEY_DOWN && selected<d->dim)selected++;
            modified=false;property=0;
        }
        if(key==KEY_LEFT) {
            if(property)property=0;
            else if(family>0){family--;modified=false;}
        }
        bool color_action=key==KEY_F3 && selected;
        if(key==KEY_RIGHT) {
            if(selected && !property){property=1;color_action=true;}
            else if(family+1<d->nic){family++;property=0;modified=false;}
        }
        if(!selected)property=0;
        if(color_action) {
            unsigned old=model_color(d,family,selected-1);int color=choose_color(old);
            if(color>=0 && (unsigned)color!=old){d->color[family][selected-1]=(uint8_t)color;modified=true;}
        }
        if(key==KEY_F1 && selected){d->graph_mask[family]^=(uint16_t)(1u<<(selected-1));modified=true;}
        if(key==KEY_F2){d->list_mask[family]^=(uint16_t)(1u<<selected);modified=true;}
        if(key==KEY_F4) {
            model_output_defaults(d);family=selected=property=0;modified=false;
        }
    }
}
