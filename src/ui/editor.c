#include "ui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
static const char *select_token(int kind)
{
    static const char *const variables[]={"y1","y2","y3","y4","y5","y6","y7","y8","y9"};
    static const char *const functions[]={"abs(","asin(","acos(","atan(","sinh(","cosh(",
        "tanh(","asinh(","acosh(","atanh("};
    const char *const *list=kind==0 ? variables:functions;
    int count=kind==0 ? 9:10;
    int choice=ui_choose(kind==0 ? "Insert variable":"Insert function",list,count,0);
    return choice<0 ? "":list[choice];
}
static const char *physical_token(key_event_t event)
{
    static char digit[2];int key=event.key,num=ui_digit(key);
    if(num>=0){digit[0]=(char)('0'+num);digit[1]=0;return digit;}
    if(key==KEY_XOT)return "x";
    if(key==KEY_ADD)return event.alpha ? "x":"+";
    if(key==KEY_SUB)return event.alpha ? "y":"-";
    if(key==KEY_NEG)return "-";
    if(key==KEY_MUL)return event.shift ? "{":"*";
    if(key==KEY_DIV)return event.shift ? "}":"/";
    if(key==KEY_FRAC)return "/";
    if(key==KEY_COMMA)return ",";
    if(key==KEY_POWER)return "^";
    if(key==KEY_SQUARE)return event.shift ? "sqrt(":"^2";
    if(key==KEY_LEFTP)return "(";
    if(key==KEY_RIGHTP)return ")";
    if(key==KEY_DOT)return ".";
    if(key==KEY_EXP)return event.shift ? "pi":"e";
    if(key==KEY_LN)return event.shift ? "exp(":"ln(";
    if(key==KEY_LOG)return event.shift ? "10^(":"log(";
    if(key==KEY_SIN)return event.shift ? "asin(":"sin(";
    if(key==KEY_COS)return event.shift ? "acos(":"cos(";
    if(key==KEY_TAN)return event.shift ? "atan(":"tan(";
    return "";
}
void ui_inline_begin(UiInlineEdit *edit,const char *text,bool replace)
{
    snprintf(edit->text,sizeof(edit->text),"%s",text);
    edit->cursor=(int)strlen(edit->text);edit->active=true;edit->replace=replace;edit->limited=false;
}
bool ui_inline_input(key_event_t event) {return physical_token(event)[0]!=0;}
int ui_list_complete(int key,bool committed,int *selected,int count)
{
    if(key!=KEY_EXE)return key;
    if(*selected+1<count){(*selected)++;return 0;}
    return committed ? 0:KEY_F6;
}
int ui_field_complete(int key,bool committed,int *selected,int count)
{
    if(key==KEY_EXE && !committed)return KEY_F6;
    return ui_list_complete(key,committed,selected,count);
}
int ui_equation_variables(const Document *d)
{
    if(d->kind==EQ_HIGHER)return d->dim-1;
    if(d->kind==EQ_SYSTEM)return d->dim;
    return 0;
}
bool ui_field_select(UiInlineEdit *edit,key_event_t event,const char *value,int *selected,int count)
{
    int key=event.key;
    if(ui_select_move(key,selected,count))return true;
    if(key==KEY_LEFT || key==KEY_RIGHT) {
        ui_inline_begin(edit,value,false);
        edit->cursor=key==KEY_LEFT ? 0:(int)strlen(value);return true;
    }
    if(ui_inline_input(event)) {
        ui_inline_begin(edit,value,true);ui_inline_key(edit,event);return true;
    }
    return false;
}
void ui_inline_insert(UiInlineEdit *edit,const char *token)
{
    unsigned n=(unsigned)strlen(token),len=(unsigned)strlen(edit->text);
    if(!n)return;
    if(edit->replace){edit->text[0]=0;edit->cursor=0;len=0;edit->replace=false;}
    if(len+n>=sizeof(edit->text)){edit->limited=true;return;}
    edit->limited=false;
    memmove(edit->text+edit->cursor+n,edit->text+edit->cursor,len-(unsigned)edit->cursor+1);
    memcpy(edit->text+edit->cursor,token,n);edit->cursor+=(int)n;
}
int ui_inline_key(UiInlineEdit *edit,key_event_t event)
{
    int key=event.key;
    if(key==KEY_EXIT)return -1;
    if(key==KEY_EXE || key==KEY_F6)return 1;
    if(key==KEY_LEFT && edit->cursor>0){edit->cursor--;edit->replace=false;}
    if(key==KEY_RIGHT && edit->cursor<(int)strlen(edit->text)){edit->cursor++;edit->replace=false;}
    if((key==KEY_DEL || key==KEY_F5) && edit->cursor>0) {
        edit->replace=false;
        memmove(edit->text+edit->cursor-1,edit->text+edit->cursor,strlen(edit->text+edit->cursor)+1);
        edit->cursor--;edit->limited=false;
    }
    if(key==KEY_F4 || key==KEY_ACON){edit->text[0]=0;edit->cursor=0;edit->replace=false;edit->limited=false;}
    const char *token=physical_token(event);
    if(key==KEY_F1)token=select_token(0);
    if(key==KEY_F2)token=select_token(1);
    if(key==KEY_OPTN)token=select_token(1);
    ui_inline_insert(edit,token);
    return 0;
}
static const char *const function_names[]={"ABS","SINH","COSH","TANH","ASINH","ACOSH","ATANH"};
static const char *const function_tokens[]={"abs(","sinh(","cosh(","tanh(","asinh(","acosh(","atanh("};
static const char *const variable_tokens[]={"y1","y2","y3","y4","y5","y6","y7","y8","y9"};
void ui_equation_menu(int kind,int page,int variables)
{
    int count=kind==1 ? variables:7;
    const char *keys[6]={"","","","","",count>5 ? (page ? "<":">"):"BACK"};
    for(int i=0;i<5 && page*5+i<count;i++)keys[i]=kind==1 ? variable_tokens[page*5+i]:function_names[page*5+i];
    ui_softkeys(keys[0],keys[1],keys[2],keys[3],keys[4],keys[5]);
}
const char *ui_equation_token(int kind,int page,int variables,int key)
{
    int index=page*5+key-KEY_F1,count=kind==1 ? variables:7;
    if(key<KEY_F1 || key>KEY_F5 || index<0 || index>=count)return "";
    return kind==1 ? variable_tokens[index]:function_tokens[index];
}
void ui_inline_draw(const UiInlineEdit *edit,int x,int y,int width,int foreground,int background)
{ui_inline_draw_cursor(edit,x,y,width,foreground,background,true);}
void ui_inline_draw_cursor(const UiInlineEdit *edit,int x,int y,int width,int foreground,int background,bool cursor)
{
    int start=edit->cursor>30 ? edit->cursor-30:0,w=0;
    while(start<edit->cursor) {
        dnsize(edit->text+start,edit->cursor-start,NULL,&w,NULL);
        if(w<=width-5)break;
        start++;
    }
    char visible[EXPR_TEXT];snprintf(visible,sizeof(visible),"%s",edit->text+start);
    while(visible[0]) {
        dsize(visible,NULL,&w,NULL);if(w<=width)break;
        visible[strlen(visible)-1]=0;
    }
    ui_rect(x-3,y-3,width+5,18,background);ui_text(x,y,foreground,"%s",visible);
    w=0;if(edit->cursor>start)dnsize(visible,edit->cursor-start,NULL,&w,NULL);
    if(cursor)ui_line(x+w,y-1,x+w,y+12,foreground);
}
bool ui_edit(const char *title,char *text,unsigned capacity,int position)
{
    unsigned limit=capacity<EXPR_TEXT ? capacity:EXPR_TEXT;
    UiInlineEdit edit;ui_inline_begin(&edit,text,false);
    edit.cursor=position<0 ? (int)strlen(edit.text):position;
    if(edit.cursor>(int)strlen(edit.text))edit.cursor=(int)strlen(edit.text);
    for(;;) {
        ui_frame(title,"Left/Right: cursor   DEL: backspace");
        ui_rect(6,46,372,96,UI_PALE);
        for(int row=0;row<5;row++) {
            int offset=row*40;
            if(offset>(int)strlen(edit.text)) break;
            char line[41];snprintf(line,sizeof(line),"%.40s",edit.text+offset);
            ui_text(12,54+row*16,UI_INK,"%s",line);
        }
        int row=edit.cursor/40,w=0;
        if(edit.cursor%40)dnsize(edit.text+row*40,edit.cursor%40,NULL,&w,NULL);
        ui_line(12+w,53+row*16,12+w,65+row*16,UI_TEAL);
        ui_text(8,151,UI_MUTED,"Use * for products; functions need ( ).");
        ui_text(8,169,UI_MUTED,"Radians.  %d / %u characters",(int)strlen(edit.text),limit-1);
        ui_softkeys("VAR","FUNC","","CLEAR","DEL","OK");dupdate();
        key_event_t event=ui_getkey();
        if(event.key==KEY_UP)edit.cursor=edit.cursor>=40 ? edit.cursor-40:0;
        else if(event.key==KEY_DOWN) {
            edit.cursor+=40;if(edit.cursor>(int)strlen(edit.text))edit.cursor=(int)strlen(edit.text);
        } else {
            int action=ui_inline_key(&edit,event);
            if(action<0)return false;
            if(action>0){snprintf(text,capacity,"%s",edit.text);return true;}
        }
        if(strlen(edit.text)>=limit)ui_message("Input limit","Expression is full (191 characters).");
    }
}
bool ui_number(const char *title,double *value)
{
    char text[EXPR_TEXT];snprintf(text,sizeof(text),"%.12g",*value);
    for(;;) {
        if(!ui_edit(title,text,sizeof(text),-1)) return false;
        ExprProgram p;ExprError e=expr_compile(text,(ExprScope){0,false,false,false},&p);
        double result=0;ExprStatus status=e.status;
        if(status==EXPR_OK) status=expr_eval(&p,0,NULL,0,&result);
        if(status==EXPR_OK && isfinite(result)) {*value=result;return true;}
        ui_message("Invalid number",expr_status_text(status));
    }
}
