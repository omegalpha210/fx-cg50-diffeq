#include "app.h"
#include "graph.h"
#include "storage.h"
#include "ui.h"
#ifndef DIFFEQ_STORAGE_DIR
#define DIFFEQ_STORAGE_DIR "/"
#endif
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static App app;
static uint32_t pristine_input;
static int compile_error_equation=-1,compile_error_position=0;

typedef struct {
    AppNavigation navigation;
    int main_selected,first_selected,dimension_selected,equation_selected;
    EquationKind dimension_kind;
    UiInlineEdit equation_edit;
    UiStageState ic,parameters;
    bool graph_first,has_session;
} AppUi;

typedef enum {TRANSITION_STAY,TRANSITION_OPEN,TRANSITION_BACK,TRANSITION_REPLACE} TransitionKind;
typedef struct {TransitionKind kind;AppScreen screen;} ScreenTransition;

static ScreenTransition stay(void)
{
    return (ScreenTransition){TRANSITION_STAY,APP_SCREEN_MAIN};
}

static ScreenTransition open_screen(AppScreen screen)
{
    return (ScreenTransition){TRANSITION_OPEN,screen};
}

static ScreenTransition back_screen(void)
{
    return (ScreenTransition){TRANSITION_BACK,APP_SCREEN_MAIN};
}

static ScreenTransition replace_screen(AppScreen screen)
{
    return (ScreenTransition){TRANSITION_REPLACE,screen};
}

static uint32_t document_fingerprint(const Document *document)
{
    const unsigned char *bytes=(const unsigned char *)document;
    uint32_t hash=2166136261u;
    for(size_t i=0;i<sizeof(*document);i++){hash^=bytes[i];hash*=16777619u;}
    return hash;
}
static uint32_t input_fingerprint(const Document *document)
{
    const unsigned char *bytes=(const unsigned char *)document;
    uint32_t hash=2166136261u;
    for(size_t i=0;i<offsetof(Document,solver);i++){hash^=bytes[i];hash*=16777619u;}
    for(size_t i=offsetof(Document,enabled);i<offsetof(Document,field_style);i++){hash^=bytes[i];hash*=16777619u;}
    for(size_t i=offsetof(Document,event);i<offsetof(Document,event)+sizeof(document->event);i++){hash^=bytes[i];hash*=16777619u;}
    return hash;
}

bool app_compile(App *a)
{
    compile_error_equation=-1;
    ModelError error=model_compile(&a->doc,&a->model);
    if(error.values!=ODE_OK) {
        ui_field_error(ode_status_text(error.values));return false;
    }
    if(error.expression.status!=EXPR_OK) {
        char message[128],label[24];
        if(error.equation==ODE_MAX_DIM)snprintf(label,sizeof(label),"Event E (including IC)");
        else model_equation_label(&a->doc,error.equation,label,sizeof(label));
        snprintf(message,sizeof(message),"%s: %s @%d",label,
            expr_status_text(error.expression.status),error.expression.position+1);
        ui_field_error(message);
        compile_error_equation=error.equation;
        compile_error_position=error.expression.position;
        return false;
    }
    return true;
}

static void new_document(App *a,EquationKind kind,int dimension)
{
    solver_report_reset();
    OdeSettings solver=a->doc.solver;ViewWindow view=a->doc.view;OdeAdaptive adaptive=a->doc.adaptive;
    int solver_custom=a->doc.solver_custom;
    ViewWindow phase_view=a->doc.phase_view;
    uint8_t phase_field=a->doc.phase_field,phase_nullclines=a->doc.phase_nullclines,phase_ready=a->doc.phase_ready;
    uint8_t field_style=a->doc.field_style,field_color=a->doc.field_color;
    model_defaults(&a->doc,kind,dimension);
    a->doc.solver=solver;a->doc.solver_custom=solver_custom;a->doc.view=view;
    a->doc.field_style=field_style;a->doc.field_color=field_color;a->doc.adaptive=adaptive;
    a->doc.phase_view=phase_view;a->doc.phase_field=phase_field;
    a->doc.phase_nullclines=phase_nullclines;a->doc.phase_ready=phase_ready;
    a->doc.view.phase=0;
    model_sync_solver_window(&a->doc);a->dirty=true;
    pristine_input=input_fingerprint(&a->doc);
}

static void use_recall(App *a)
{
    solver_report_reset();
    bool was_phase_system=model_phase_supported(&a->doc);
    OdeSettings solver=a->doc.solver;ViewWindow view=a->doc.view;OdeAdaptive adaptive=a->doc.adaptive;
    int solver_custom=a->doc.solver_custom;
    ViewWindow phase_view=a->doc.phase_view;
    uint8_t phase_field=a->doc.phase_field,phase_nullclines=a->doc.phase_nullclines,phase_ready=a->doc.phase_ready;
    uint8_t field_style=a->doc.field_style,field_color=a->doc.field_color;
    a->doc=a->recall;a->doc.solver=solver;a->doc.solver_custom=solver_custom;a->doc.view=view;
    a->doc.field_style=field_style;a->doc.field_color=field_color;a->doc.adaptive=adaptive;
    a->doc.phase_view=phase_view;a->doc.phase_field=phase_field;
    a->doc.phase_nullclines=phase_nullclines;a->doc.phase_ready=phase_ready;
    if(a->doc.dim<2 || was_phase_system!=model_phase_supported(&a->doc))a->doc.view.phase=0;
    model_sync_solver_window(&a->doc);a->dirty=true;
    pristine_input=0;
}

static const char *formula(const Document *document,char *buffer,unsigned size)
{
    switch(document->kind) {
        case EQ_SEPARABLE:return "y' = f(x) * g(y)";
        case EQ_LINEAR:return "y' + f(x)*y = g(x)";
        case EQ_BERNOULLI:return "y' + f(x)*y = g(x)*y^n";
        case EQ_SECOND:return "y'' + f(x)*y' + g(x)*y = h(x)";
        case EQ_HIGHER:
            if(document->dim==1)return "y' = f(x,y)";
            if(document->dim==2)return "y'' = f(x,y,y1), y1=y'";
            snprintf(buffer,size,"y^(%d) = f(x,y,y1,...,y%d)",document->dim,document->dim-1);
            return buffer;
        case EQ_SYSTEM:
            if(document->dim==1)return "y1' = f1(x,y1)";
            snprintf(buffer,size,"yi' = fi(x,y1,...,y%d), i=1..%d",document->dim,document->dim);
            return buffer;
        default:return "y' = f(x,y)";
    }
}

static bool inline_value(const UiInlineEdit *edit,double *out)
{
    ExprProgram program;
    ExprError error=expr_compile(edit->text,(ExprScope){0,false,false,false},&program);
    double value=0;ExprStatus status=error.status;
    if(status==EXPR_OK)status=expr_eval(&program,0,NULL,0,&value);
    if(status!=EXPR_OK || !isfinite(value) || fabs(value)>1e100) {
        ui_field_error(status==EXPR_OK ?
            "Magnitude must be at most 1e100.":expr_status_text(status));
        return false;
    }
    *out=value;return true;
}

static int equation_count(const Document *document)
{
    return model_equations(document)+(document->kind==EQ_BERNOULLI);
}

static void equation_field(const Document *document,int index,char *label,
    unsigned label_size,char *value,unsigned value_size)
{
    int equations=model_equations(document);
    if(index<equations) {
        model_equation_label(document,index,label,label_size);
        snprintf(value,value_size,"%s",document->text[index]);return;
    }
    snprintf(label,label_size,"n");snprintf(value,value_size,"%.12g",document->power);
}

static bool accept_equation_edit(Document *document,int selected,const UiInlineEdit *edit)
{
    int equations=model_equations(document);
    if(selected>=equations) {
        double value;if(!inline_value(edit,&value))return false;
        document->power=value;return true;
    }
    ExprScope scope={document->dim,document->kind==EQ_HIGHER,true,true};
    if(document->kind==EQ_SEPARABLE){scope.allow_x=selected==0;scope.allow_y=selected==1;}
    if(document->kind==EQ_LINEAR || document->kind==EQ_BERNOULLI
        || document->kind==EQ_SECOND)scope.allow_y=false;
    ExprProgram program;ExprError error=expr_compile(edit->text,scope,&program);
    if(error.status!=EXPR_OK) {
        char message[128],label[24];
        model_equation_label(document,selected,label,sizeof(label));
        snprintf(message,sizeof(message),"%s: %s @%d",label,
            expr_status_text(error.status),error.position+1);
        ui_field_error(message);return false;
    }
    snprintf(document->text[selected],EXPR_TEXT,"%s",edit->text);return true;
}

static ScreenTransition screen_main(App *a,AppUi *ui)
{
    static const char *const labels[]={"1  1st","2  2nd","3  N-th","4  SYS","5  RCL","6  SAVE"};
    static const char *const descriptions[]={"First-order equation","Linear second-order equation",
        "Higher-order equation (1-9)","First-order system (1-9)",
        "Recall saved session","Save current session"};
    ui_frame("Differential Equation",NULL);
    for(int i=0;i<6;i++)ui_field_at(31+i*22+(i>=4 ? 4:0),labels[i],descriptions[i],i==ui->main_selected);
    /* This gap is not an item: SYS -> RCL keeps the same six indices. */
    ui_rect(10,114,365,5,C_WHITE);ui_line(10,116,374,116,UI_LINE);
    ui_help(8,184,"MENU: return to MAIN MENU",true);
    ui_softkeys("","","","","","OPEN");dupdate();
    int key=ui_getkey().key,choice=-1;
    if(key==KEY_EXIT)return stay();
    ui_select_move(key,&ui->main_selected,6);
    if(key==KEY_EXE || key==KEY_F6)choice=ui->main_selected;
    int digit=ui_digit(key);if(digit>=1 && digit<=6)choice=digit-1;
    if(choice<0)return stay();
    ui->main_selected=choice;
    if(choice==0)return open_screen(APP_SCREEN_FIRST_ORDER);
    if(choice==1) {
        if(a->doc.kind!=EQ_SECOND)new_document(a,EQ_SECOND,2);
        ui->has_session=true;ui->equation_selected=0;return open_screen(APP_SCREEN_EQUATION);
    }
    if(choice==2 || choice==3) {
        ui->dimension_kind=choice==2 ? EQ_HIGHER:EQ_SYSTEM;
        ui->dimension_selected=a->doc.kind==(int)ui->dimension_kind ? a->doc.dim:1;
        return open_screen(APP_SCREEN_DIMENSION);
    }
    if(choice==4) {
        const char *const items[]={"Last calculation (RAM)","Load saved session"};
        int recall=ui_choose("Recall",items,2,0);
        if(recall<0)return stay();
        if(recall==1)return open_screen(APP_SCREEN_LOAD);
        if(!a->has_recall) {
            ui_message("Recall","No previous calculation in this session.");return stay();
        }
        use_recall(a);ui->has_session=true;ui->equation_selected=0;
        return open_screen(APP_SCREEN_EQUATION);
    }
    return open_screen(APP_SCREEN_SAVE);
}

static ScreenTransition screen_first_order(App *a,AppUi *ui)
{
    static const char *const names[]={"Separable","Linear","Bernoulli",
        "Others: general first-order"};
    int selected=ui_choose("First-order equation",names,4,ui->first_selected);
    if(selected<0)return back_screen();
    ui->first_selected=selected;
    EquationKind kind=(EquationKind)selected;
    if(a->doc.kind!=(int)kind)new_document(a,kind,1);
    ui->has_session=true;ui->equation_selected=0;
    return open_screen(APP_SCREEN_EQUATION);
}

static ScreenTransition screen_dimension(App *a,AppUi *ui)
{
    char value[16];snprintf(value,sizeof(value),"%d",ui->dimension_selected);
    UiInlineEdit edit;ui_inline_begin(&edit,value,true);edit.active=false;
    for(;;) {
        ui_frame(ui->dimension_kind==EQ_HIGHER ? "Order (1-9)":"Variables (1-9)",NULL);
        ui_field(0,"Value",edit.text,true);
        if(edit.active)ui_inline_draw(&edit,138,31,226,C_WHITE,UI_BLUE);
        ui_form_hint(&edit,"Enter an integer from 1 to 9");
        ui_softkeys("","","","","","OPEN");dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(!edit.active){int field=0;key=ui_field_complete(key,false,&field,1);event.key=(unsigned)key;}
        if(key==KEY_EXIT && !edit.active)return back_screen();
        if((edit.active && (key==KEY_EXE || key==KEY_EXIT)) || key==KEY_F6) {
            if(strlen(edit.text)!=1 || edit.text[0]<'1' || edit.text[0]>'9') {
                ui_field_error("Enter one integer from 1 to 9.");continue;
            }
            int dimension=edit.text[0]-'0';
            edit.active=false;
            if(key!=KEY_F6)continue;
            if(a->doc.kind!=(int)ui->dimension_kind || a->doc.dim!=dimension) {
                if(ui->has_session && input_fingerprint(&a->doc)!=pristine_input) {
                    if(!ui_confirm("Change Equation Size","Replace equation and ICs with defaults?"))continue;
                }
                new_document(a,ui->dimension_kind,dimension);
                ui->ic=(UiStageState){0};ui->parameters=(UiStageState){0};
            }
            ui->dimension_selected=dimension;ui->has_session=true;ui->equation_selected=0;
            return open_screen(APP_SCREEN_EQUATION);
        }
        if(edit.active)ui_inline_key(&edit,event);
        else {int selected=0;char current[EXPR_TEXT];snprintf(current,sizeof(current),"%s",edit.text);
            ui_field_select(&edit,event,current,&selected,1);}
    }
}

static ScreenTransition screen_equation(App *a,AppUi *ui)
{
    int menu=0,menu_page=0;
    for(;;) {
        Document *document=&a->doc;int count=equation_count(document);
        int *selected=&ui->equation_selected;UiInlineEdit *edit=&ui->equation_edit;
        if(*selected>=count)*selected=count-1;
        char title[64];snprintf(title,sizeof(title),"DIFF EQ / %s",model_kind_name(document->kind));
        int variables=ui_equation_variables(document);
        char general[80];
        if(!menu) {
            ui_frame(title,formula(document,general,sizeof(general)));
            ui_progress(1);
            int page=*selected/6,row_offset=1;
            for(int row=0;row<6 && page*6+row<count;row++) {
                int index=page*6+row;char label[24],value[EXPR_TEXT];
                equation_field(document,index,label,sizeof(label),value,sizeof(value));
                ui_field(row+row_offset,label,edit->active && index==*selected ? edit->text:value,
                    index==*selected);
                if(edit->active && index==*selected)
                    ui_inline_draw(edit,138,31+(row+row_offset)*22,226,C_WHITE,UI_BLUE);
            }
            ui_form_hint(edit,"LEFT/RIGHT: edit");
        }
        if(menu){ui_progress(0);ui_equation_menu(menu,menu_page,variables);}
        else ui_softkeys(variables ? "VAR":"",edit->active ? "FUNC":"","","","","NEXT");
        dupdate();
        key_event_t event=ui_getkey();int key=event.key;
        if(menu) {
            if(key==KEY_EXIT){menu=0;continue;}
            if(key==KEY_F6){if(menu==2 || variables>5)menu_page=1-menu_page;else menu=0;continue;}
            const char *token=ui_equation_token(menu,menu_page,variables,key);
            if(token[0]) {
                if(!edit->active) {
                    char label[24],value[EXPR_TEXT];equation_field(document,*selected,label,sizeof(label),value,sizeof(value));
                    ui_inline_begin(edit,value,false);
                }
                ui_inline_insert(edit,token);menu=0;continue;
            }
            if(key>=KEY_F1 && key<=KEY_F5)continue;
            menu=0;ui_progress(1); /* Restore stage before an inline validation error. */
        }
        if((key==KEY_F1 && !variables) || (key==KEY_F2 && !edit->active))continue;
        if(key==KEY_F1 || key==KEY_F2){menu=key==KEY_F1 ? 1:2;menu_page=0;continue;}
        if(key==KEY_F3)continue;
        bool leave=key==KEY_EXIT || key==KEY_F6;
        if(edit->active) {
            if(leave || key==KEY_UP || key==KEY_DOWN || key==KEY_EXE) {
                if(!accept_equation_edit(document,*selected,edit))continue;
                a->dirty=true;edit->active=false;
                if(key==KEY_EXE || key==KEY_EXIT) {
                    ui_field_complete(key,true,selected,count);continue;
                }
                if(key==KEY_UP || key==KEY_DOWN) {
                    if(key==KEY_UP && *selected>0)(*selected)--;
                    if(key==KEY_DOWN && *selected+1<count)(*selected)++;
                    continue;
                }
            } else {
                if(key!=KEY_OPTN && key!=KEY_F4 && key!=KEY_F5)ui_inline_key(edit,event);
                continue;
            }
        }
        key=ui_field_complete(key,false,selected,count);event.key=(unsigned)key;
        if(key==KEY_EXIT)return back_screen();
        {
            char label[24],value[EXPR_TEXT];
            equation_field(document,*selected,label,sizeof(label),value,sizeof(value));
            if(ui_field_select(edit,event,value,selected,count))continue;
        }
        if(key==KEY_OPTN && document->kind==EQ_HIGHER) {
            if(app_compile(a) && ui_confirm("Convert to system","Replace with an editable first-order system?")) {
                model_convert_system(document);a->dirty=true;
            }
            continue;
        }
        if(key==KEY_F6) {
            bool valid=true;
            for(int i=0;i<count;i++) {
                char label[24],value[EXPR_TEXT];UiInlineEdit check;
                equation_field(document,i,label,sizeof(label),value,sizeof(value));
                ui_inline_begin(&check,value,false);
                if(!accept_equation_edit(document,i,&check)) {
                    *selected=i;*edit=check;valid=false;break;
                }
            }
            if(valid)return open_screen(APP_SCREEN_INITIAL_CONDITIONS);
        }
    }
}

static ScreenTransition screen_calculate(App *a,AppUi *ui)
{
    ModelWork plan=model_preflight(&a->doc,&a->doc.solver);
    if(plan.status!=ODE_OK) {
        solver_report_begin(&a->doc,&a->model);solver_report_end(&a->model,plan.status);
        ui->parameters.selected=3;ui->parameters.edit.active=false; /* h / h0 */
        ui_field_error(plan.status==ODE_STEP_LIMIT ? "Too many steps; increase h / Max Steps":
            (plan.status==ODE_WORK_LIMIT ? "Work limit; shorten range / fewer ICs":ode_status_text(plan.status)));
        return back_screen();
    }
    if(!app_compile(a)) {
        solver_report_reset();
        if(compile_error_equation>=0 && compile_error_equation<ODE_MAX_DIM) {
            ui->equation_selected=compile_error_equation;
            ui_inline_begin(&ui->equation_edit,a->doc.text[compile_error_equation],false);
            ui->equation_edit.cursor=compile_error_position;
            if(ui->equation_edit.cursor>(int)strlen(ui->equation_edit.text))
                ui->equation_edit.cursor=(int)strlen(ui->equation_edit.text);
        }
        return back_screen();
    }
    a->recall=a->doc;a->has_recall=true;a->dirty=true;ui->graph_first=true;
    return replace_screen(APP_SCREEN_GRAPH);
}

static ScreenTransition screen_graph(App *a,AppUi *ui)
{
    UiGraphAction action=ui_graph(a,ui->graph_first);ui->graph_first=false;
    if(action==UI_GRAPH_BACK)return back_screen();
    if(action==UI_GRAPH_TRACE)return open_screen(APP_SCREEN_TRACE);
    if(action==UI_GRAPH_VWINDOW)return open_screen(APP_SCREEN_VWINDOW);
    if(action==UI_GRAPH_TABLE)return open_screen(APP_SCREEN_TABLE);
    return open_screen(APP_SCREEN_GRAPH_SETTINGS);
}

static ScreenTransition stage_transition(UiStageAction action,AppScreen next)
{
    if(action==UI_STAGE_NEXT)return open_screen(next);
    if(action==UI_STAGE_VWINDOW)return open_screen(APP_SCREEN_VWINDOW);
    if(action==UI_STAGE_OUTPUT)return open_screen(APP_SCREEN_OUTPUT);
    if(action==UI_STAGE_SETTINGS)return open_screen(APP_SCREEN_GRAPH_SETTINGS);
    if(action==UI_STAGE_EVENT)return open_screen(APP_SCREEN_EVENT);
    if(action==UI_STAGE_INFO)return open_screen(APP_SCREEN_SOLVER_INFO);
    return back_screen();
}

static void apply_transition(AppNavigation *navigation,ScreenTransition transition)
{
    if(transition.kind==TRANSITION_OPEN)app_navigation_open(navigation,transition.screen);
    else if(transition.kind==TRANSITION_BACK)app_navigation_back(navigation);
    else if(transition.kind==TRANSITION_REPLACE) {
        app_navigation_back(navigation);
        app_navigation_open(navigation,transition.screen);
    }
}

int app_run(void)
{
    app_initialize(&app,DIFFEQ_STORAGE_DIR);
    pristine_input=input_fingerprint(&app.doc);
    AppUi ui={0};app_navigation_init(&ui.navigation);ui.dimension_selected=1;
    for(;;) {
        AppScreen from=ui.navigation.current;ScreenTransition transition=stay();
        uint32_t before;
        switch(from) {
            case APP_SCREEN_MAIN:transition=screen_main(&app,&ui);break;
            case APP_SCREEN_FIRST_ORDER:transition=screen_first_order(&app,&ui);break;
            case APP_SCREEN_DIMENSION:transition=screen_dimension(&app,&ui);break;
            case APP_SCREEN_EQUATION:transition=screen_equation(&app,&ui);break;
            case APP_SCREEN_PARAMETERS: {
                before=document_fingerprint(&app.doc);
                UiStageAction action=ui_parameters(&app.doc,&ui.parameters);
                if(document_fingerprint(&app.doc)!=before)app.dirty=true;
                transition=stage_transition(action,APP_SCREEN_CALCULATE);break;
            }
            case APP_SCREEN_OUTPUT: {
                before=document_fingerprint(&app.doc);ui_output(&app.doc);
                if(document_fingerprint(&app.doc)!=before)app.dirty=true;
                transition=back_screen();
                break;
            }
            case APP_SCREEN_VWINDOW:
                before=document_fingerprint(&app.doc);ui_vwindow(&app.doc);
                if(document_fingerprint(&app.doc)!=before)app.dirty=true;
                transition=back_screen();break;
            case APP_SCREEN_EVENT:
                before=document_fingerprint(&app.doc);ui_event(&app.doc);
                if(document_fingerprint(&app.doc)!=before)app.dirty=true;
                transition=back_screen();break;
            case APP_SCREEN_SOLVER_INFO:
                ui_solver_info();transition=back_screen();break;
            case APP_SCREEN_GRAPH_SETTINGS:
                before=document_fingerprint(&app.doc);ui_graph_settings(&app.doc);
                if(document_fingerprint(&app.doc)!=before)app.dirty=true;
                transition=back_screen();break;
            case APP_SCREEN_INITIAL_CONDITIONS: {
                before=document_fingerprint(&app.doc);
                UiStageAction action=ui_initial_conditions(&app.doc,&ui.ic);
                if(document_fingerprint(&app.doc)!=before)app.dirty=true;
                transition=stage_transition(action,APP_SCREEN_PARAMETERS);break;
            }
            case APP_SCREEN_CALCULATE:transition=screen_calculate(&app,&ui);break;
            case APP_SCREEN_GRAPH:transition=screen_graph(&app,&ui);break;
            case APP_SCREEN_TRACE:ui_trace(&app);transition=back_screen();break;
            case APP_SCREEN_TABLE:ui_table(&app);transition=back_screen();break;
            case APP_SCREEN_SAVE: {
                if(!ui_save_confirm()){transition=back_screen();break;}
                if(!ui.has_session){ui_message("Save session","No session.");transition=back_screen();break;}
                bool saved=storage_save(&app,DIFFEQ_STORAGE_DIR);if(saved)app.dirty=false;
                ui_message("Save session",saved ?
                    "Saved equation, settings, ICs and last calculation.":
                    "Save failed. Check settings and storage space.");
                transition=back_screen();break;
            }
            case APP_SCREEN_LOAD:
                if(ui_confirm("Load saved session","Replace current working equation?")) {
                    if(!storage_load(&app,DIFFEQ_STORAGE_DIR))
                        ui_message("Load failed","No valid saved session found.");
                    else {
                        if(app.migration_warnings)ui_message("Legacy session adapted",
                            app.migration_warnings&STORAGE_EXPRESSION_REVIEW ?
                            "Some old constant expressions need editing. Original files retained.":
                            (app.migration_warnings&STORAGE_IC_ADAPTED ?
                            "IC policy changed: kept first vector / common-x values. Original files retained.":
                            "Outputs combined; x always included. Private constants converted to numbers."));
                        solver_report_reset();app.dirty=false;ui.has_session=true;ui.equation_selected=0;
                        pristine_input=0;
                        ui.equation_edit=(UiInlineEdit){0};
                        ui.ic=(UiStageState){0};ui.parameters=(UiStageState){0};
                        transition=replace_screen(APP_SCREEN_EQUATION);break;
                    }
                }
                transition=back_screen();break;
            case APP_SCREEN_COUNT:app_navigation_init(&ui.navigation);break;
        }
        apply_transition(&ui.navigation,transition);
    }
}
