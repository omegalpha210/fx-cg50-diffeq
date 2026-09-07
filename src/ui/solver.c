#include "ui.h"
#include <stdio.h>
#include <string.h>
static const char *const directions[]={"ANY","RISING","FALLING"};
static bool accept_event(Document *d,UiInlineEdit *edit)
{
    if(edit->text[0]) {
        ExprProgram program;ExprError error=expr_compile(edit->text,model_event_scope(d),&program);
        if(error.status!=EXPR_OK) {
            char message[96];snprintf(message,sizeof(message),"%s at character %d",expr_status_text(error.status),error.position+1);
            ui_message("Check Event E",message);edit->cursor=error.position;return false;
        }
    }
    memcpy(d->event.text,edit->text,sizeof(d->event.text));edit->active=false;return true;
}
void ui_event(Document *d)
{
    int selected=0,menu=0,page=0;UiInlineEdit edit={0};
    int variables=d->kind==EQ_SYSTEM ? d->dim:d->dim-1;
    for(;;) {
        if(!menu) {
            ui_frame("Event Settings",NULL);
            ui_field(0,"Enabled",d->event.enabled ? "ON":"OFF",selected==0);
            ui_field(1,"E",edit.active ? edit.text:d->event.text,selected==1);
            ui_field(2,"Direction",directions[d->event.direction],selected==2);
            ui_field(3,"Action",d->event.action==EVENT_STOP ? "STOP":"MARK",selected==3);
            if(edit.active)ui_inline_draw(&edit,138,53,226,C_WHITE,UI_BLUE);
            ui_form_hint(&edit,selected==1 ? (d->kind==EQ_SECOND ? "E=0; y1 is y'":"E(x, state)=0; EXE: done"):
                (selected==2 ? "Crossing direction for increasing x":"LEFT/RIGHT: toggle"));
        }
        if(menu)ui_equation_menu(menu,page,variables);
        else ui_softkeys(edit.active && variables ? "VAR":"",edit.active ? "FUNC":"","","","","DONE");
        dupdate();key_event_t event=ui_getkey();int key=event.key;
        if(menu) {
            if(key==KEY_EXIT){menu=0;continue;}
            if(key==KEY_F6){if(menu==2 || variables>5)page=1-page;else menu=0;continue;}
            const char *token=ui_equation_token(menu,page,variables,key);
            if(token[0]){ui_inline_insert(&edit,token);menu=0;continue;}
            if(key>=KEY_F1 && key<=KEY_F5)continue;
            menu=0;
        }
        if(edit.active) {
            if((key==KEY_F1 && variables) || key==KEY_F2){menu=key==KEY_F1 ? 1:2;page=0;continue;}
            if(key==KEY_EXE || key==KEY_EXIT || key==KEY_F6 || key==KEY_UP || key==KEY_DOWN) {
                if(!accept_event(d,&edit))continue;
                if(key==KEY_EXE || key==KEY_EXIT){ui_field_complete(key,true,&selected,4);continue;}
            } else {
                if(key!=KEY_OPTN && (key<KEY_F1 || key>KEY_F6))ui_inline_key(&edit,event);
                continue;
            }
        }
        if(key==KEY_EXIT || key==KEY_F6 || key==KEY_EXE)return;
        if(key==KEY_UP && selected>0){selected--;continue;}
        if(key==KEY_DOWN && selected<3){selected++;continue;}
        if(selected==1){ui_field_select(&edit,event,d->event.text,&selected,4);continue;}
        if(key!=KEY_LEFT && key!=KEY_RIGHT)continue;
        if(selected==0)d->event.enabled=(uint8_t)!d->event.enabled;
        if(selected==2)d->event.direction=(uint8_t)((d->event.direction+(key==KEY_RIGHT ? 1:2))%3);
        if(selected==3)d->event.action=(uint8_t)!d->event.action;
    }
}
static const char *short_status(OdeStatus status)
{
    switch(status) {
    case ODE_OK:return "COMPLETE";
    case ODE_EVENT_STOP:return "EVENT";
    case ODE_CANCELLED:return "CANCELLED";
    case ODE_STEP_LIMIT:return "MAX STEPS";
    case ODE_WORK_LIMIT:return "WORK LIMIT";
    case ODE_STEP_UNDERFLOW:return "STEP UNDERFLOW";
    case ODE_TOLERANCE:return "TOLERANCE TOO SMALL";
    case ODE_DIVERGED:return "MAGNITUDE LIMIT";
    case ODE_DOMAIN:return "DOMAIN ERROR";
    case ODE_NONFINITE:return "NaN / INF";
    default:return ode_status_text(status);
    }
}
static void info_row(const SolverReport *r,int index,const char **label,char *value,unsigned size)
{
    const char *const labels[]={"Method","Status","ICs","State dim","Xrange min","Xrange max",
        "h","RelTol","AbsTol","Accepted","Rejected","Attempts","RHS evals","h min","h max",
        "Event","Direction","Action","Hits","Stored","Unavailable","Stopped x"};
    *label=labels[index];
    switch(index) {
    case 0:snprintf(value,size,"%s",r->method==ODE_RK45 ? "RK45":"RK4");break;
    case 1:snprintf(value,size,"%s",short_status(r->status));break;
    case 2:snprintf(value,size,"%d",r->nic);break;
    case 3:snprintf(value,size,"%d",r->dim);break;
    case 4:snprintf(value,size,"%.9g",r->xmin);break;
    case 5:snprintf(value,size,"%.9g",r->xmax);break;
    case 6:*label=r->method==ODE_RK45 ? "Initial h":"h";snprintf(value,size,"%.9g",r->h);break;
    case 7:snprintf(value,size,"%.9g",r->reltol);break;
    case 8:snprintf(value,size,"%.9g",r->abstol);break;
    case 9:*label=r->method==ODE_RK45 ? "Accepted":"Steps";snprintf(value,size,"%lu",(unsigned long)r->work.accepted);break;
    case 10:snprintf(value,size,"%lu",(unsigned long)r->work.rejected);break;
    case 11:snprintf(value,size,"%lu",(unsigned long)(r->work.accepted+r->work.rejected));break;
    case 12:snprintf(value,size,"%lu",(unsigned long)r->work.rhs);break;
    case 13:snprintf(value,size,"%.9g",r->work.min_h);break;
    case 14:snprintf(value,size,"%.9g",r->work.max_h);break;
    case 15:snprintf(value,size,"%s",r->enabled ? "ON":"OFF");break;
    case 16:snprintf(value,size,"%s",directions[r->direction]);break;
    case 17:snprintf(value,size,"%s",r->action==EVENT_STOP ? "STOP":"MARK");break;
    case 18:snprintf(value,size,"%lu",(unsigned long)r->hits);break;
    case 19:snprintf(value,size,"%u / %d",r->markers.count,EVENT_CAPACITY);break;
    case 20:snprintf(value,size,"%lu",(unsigned long)r->unavailable);break;
    default:snprintf(value,size,"%.9g",r->markers.point[0].x);break;
    }
}
void ui_solver_info(void)
{
    const SolverReport *r=solver_report();int top=0,rows[22],count=0;
    for(int i=0;i<22;i++) {
        if(r->method==ODE_RK4 && (i==7 || i==8 || i==10 || i==11))continue;
        if(!r->enabled && i>=16)continue;
        if(i==21 && (r->action!=EVENT_STOP || r->hits!=1 || !r->markers.count))continue;
        rows[count++]=i;
    }
    for(;;) {
        ui_frame("Solver Info",NULL);
        if(!r->valid)ui_text(18,60,UI_INK,"No solver run yet");
        else for(int i=0;i<7 && i+top<count;i++) {
            const char *label;char value[48];info_row(r,rows[i+top],&label,value,sizeof(value));
            ui_field(i,label,value,false);
        }
        ui_form_hint(NULL,r->valid ? "Last trajectory run; UP/DOWN: scroll":NULL);
        ui_softkeys("","","","","","");dupdate();int key=ui_getkey().key;
        if(key==KEY_EXIT)return;
        if(key==KEY_UP && top>0)top--;
        if(key==KEY_DOWN && top+7<count)top++;
    }
}
