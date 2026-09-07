#include "ui.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#ifdef FXCG50
#include <gint/timer.h>
#include <gint/gint.h>
#include <gint/drivers/keydev.h>
#endif
/* Only a small queue of transformed input, never pixels or trajectories.
   Numerical cancellation polls must not discard keys during blink redraws. */
#ifdef FXCG50
static key_event_t pending[8];
static unsigned pending_count;
static keydev_transform_t trace_transform;
static int trace_repeater(int key,int duration,int count)
{(void)key;(void)duration;return count==0 ? 400000:125000;}
#endif
static key_event_t trace_pending;
static bool trace_control(int key) {return key==KEY_EXIT || key==KEY_MENU;}
static void trace_accept(key_event_t event)
{
    if(event.type==KEYEV_NONE)return;
    if(trace_control(trace_pending.key))return;
    if(trace_control(event.key) || event.type==KEYEV_DOWN || trace_pending.type==KEYEV_NONE
        || trace_pending.type==KEYEV_HOLD)trace_pending=event;
}
void ui_trace_input(bool active)
{
    trace_pending=(key_event_t){0};
#ifdef FXCG50
    if(active) {
        trace_transform=keydev_transform(keydev_std());
        keydev_transform_t transform=trace_transform;transform.repeater=trace_repeater;
        keydev_set_transform(keydev_std(),transform);
    } else keydev_set_transform(keydev_std(),trace_transform);
    pending_count=0;
#else
    (void)active;
#endif
}
bool ui_trace_cancel(void *unused)
{
    (void)unused;
#ifdef FXCG50
    for(;;) {
        volatile int timeout=1;
        key_event_t event=getkey_opt(GETKEY_DEFAULT & ~GETKEY_MENU,&timeout);
        if(event.type==KEYEV_NONE)break;
        trace_accept(event);
    }
    if(trace_pending.type==KEYEV_HOLD && !keydown(trace_pending.key))trace_pending=(key_event_t){0};
#else
    key_event_t event;
    while((event=pollevent()).type!=KEYEV_NONE)trace_accept(event);
#endif
    return trace_control(trace_pending.key);
}
key_event_t ui_trace_key(UiBlink *blink)
{
    ui_trace_cancel(NULL);
    if(trace_pending.type==KEYEV_NONE)trace_accept(ui_blink_key(blink));
    ui_trace_cancel(NULL);
    key_event_t event=trace_pending;trace_pending=(key_event_t){0};
#ifdef FXCG50
    if(event.key==KEY_MENU) {gint_osmenu();dupdate();}
#endif
    return event;
}
static key_event_t take_key(void)
{
#ifdef FXCG50
    while(pending_count) {
        key_event_t event=pending[0];
        memmove(pending,pending+1,(--pending_count)*sizeof(*pending));
        if(event.key==KEY_MENU && !event.shift && !event.alpha) {
            /* MENU observed by the compute poll was deliberately not handled
               by getkey_opt. All other MENU keys use getkey's normal path. */
            gint_osmenu();dupdate();continue;
        }
        return event;
    }
#endif
    return getkey();
}
key_event_t ui_getkey(void)
{
    for(;;) {
        key_event_t event=take_key();
        /* One physical EXIT closes one layer. Keep other queued input intact. */
        if(event.type==KEYEV_HOLD && event.key==KEY_EXIT)continue;
        return event;
    }
}
void ui_rect(int x,int y,int w,int h,int color)
{ drect(UI_X+x,UI_Y+y,UI_X+x+w-1,UI_Y+y+h-1,color); }
void ui_line(int x1,int y1,int x2,int y2,int color)
{ dline(UI_X+x1,UI_Y+y1,UI_X+x2,UI_Y+y2,color); }
void ui_text(int x,int y,int color,const char *format,...)
{
    char text[256]; va_list args;va_start(args,format);
    vsnprintf(text,sizeof(text),format,args);va_end(args);
    dtext(UI_X+x,UI_Y+y,color,text);
}
void ui_frame(const char *title,const char *subtitle)
{
    dclear(C_WHITE);
    ui_rect(0,0,UI_W,21,UI_BLUE);
    ui_text(8,5,C_WHITE,"%s",title);
    if(subtitle) ui_text(8,26,UI_MUTED,"%s",subtitle);
}
void ui_softkeys(const char *a,const char *b,const char *c,const char *d,const char *e,const char *f)
{
    const char *keys[]={a,b,c,d,e,f};
    for(int i=0;i<6;i++) {
        int background=UI_BLUE,foreground=C_WHITE;
        if(!strcmp(keys[i],"PREV")){background=0xf81f;foreground=C_BLACK;}
        if(!strcmp(keys[i],"NEXT")){background=UI_CYAN;foreground=C_BLACK;}
        if(!strcmp(keys[i],"V-WIN")){background=C_RGB(31,17,0);foreground=C_BLACK;}
        if(!strcmp(keys[i],"SET")){background=UI_BRIGHT_GREEN;foreground=C_BLACK;}
        if(!strcmp(keys[i],"NORMAL")){background=UI_YELLOW;foreground=C_BLACK;}
        if(!strcmp(keys[i],"FAST")){background=UI_BRIGHT_GREEN;foreground=C_BLACK;}
        if(!strcmp(keys[i],"FASTER")){background=UI_CYAN;foreground=C_BLACK;}
        if(i==5 && !strcmp(keys[i],"GRAPH"))background=C_RED;
        if(!strcmp(keys[i],"COLOR")) {
            static const int colors[]={0xf800,0xfc40,UI_BRIGHT_GREEN,UI_CYAN,0xf81f};
            ui_rect(i*64,198,63,18,C_WHITE);
            int width;dsize(keys[i],NULL,&width,NULL);int x=i*64+(63-width)/2;
            for(int j=0;j<5;j++) {
                char letter[2]={keys[i][j],0};ui_text(x,202,colors[j],"%s",letter);
                dsize(letter,NULL,&width,NULL);x+=width+1;
            }
            continue;
        }
        ui_rect(i*64,198,63,18,background);
        int w=0;dsize(keys[i],NULL,&w,NULL);
        ui_text(i*64+(63-w)/2,202,foreground,"%s",keys[i]);
    }
}
void ui_short(char *out,unsigned capacity,const char *text,int width)
{
    snprintf(out,capacity,"%s",text);
    int w=0;dsize(out,NULL,&w,NULL);
    if(w<=width) return;
    size_t n=strlen(out);
    while(n>3) {
        out[--n]=0;dsize(out,NULL,&w,NULL);
        if(w<=width-18) break;
    }
    if(n+3<capacity) strcat(out,"...");
}
void ui_row(int row,const char *label,const char *value,bool selected)
{
    int y=42+row*24;
    ui_rect(4,y-3,376,23,selected ? UI_PALE : C_WHITE);
    if(selected) ui_rect(4,y-3,3,23,UI_TEAL);
    ui_text(12,y+2,selected ? UI_BLUE:UI_INK,"%s",label);
    char short_value[192];ui_short(short_value,sizeof(short_value),value,242);
    ui_text(126,y+2,UI_INK,"%s",short_value);
    ui_line(8,y+20,376,y+20,UI_LINE);
}
void ui_field(int row,const char *label,const char *value,bool selected)
{
    int y=31+row*22;
    ui_rect(4,y-4,376,21,selected ? UI_BLUE:C_WHITE);
    ui_text(14,y,selected ? C_WHITE:UI_INK,"%s",label);
    ui_text(122,y,selected ? C_WHITE:UI_MUTED,":");
    char short_value[192];ui_short(short_value,sizeof(short_value),value,226);
    ui_text(138,y,selected ? C_WHITE:UI_INK,"%s",short_value);
    if(!selected)ui_line(10,y+17,374,y+17,UI_LINE);
}
void ui_form_hint(const UiInlineEdit *edit,const char *context)
{
    const char *text=edit && edit->active ? (edit->limited ? UI_LIMIT_HINT:UI_EDIT_HINT):context;
    if(text && text[0])ui_text(8,184,UI_MUTED,"%s",text);
}
void ui_color_swatch(int x,int y,int color)
{
    ui_rect(x,y,29,13,UI_INK);ui_rect(x+2,y+2,25,9,color);
}
void ui_message(const char *title,const char *message)
{
    ui_frame(title,NULL);
    size_t offset=0,length=strlen(message);
    for(int row=0;offset<length && row<8;row++) {
        size_t count=length-offset>42 ? 42:length-offset;
        const char *newline=memchr(message+offset,'\n',count);
        if(newline) count=(size_t)(newline-message-offset);
        else if(offset+count<length) {
            size_t word=count;
            while(word>0 && message[offset+word]!=' ') word--;
            if(word>0) count=word;
        }
        char line[43];memcpy(line,message+offset,count);line[count]=0;
        ui_text(10,43+row*17,UI_INK,"%s",line);
        offset+=count;
        while(message[offset]==' ' || message[offset]=='\n') offset++;
    }
    ui_softkeys("","","","","","OK");dupdate();
    for(;;) {int key=ui_getkey().key;if(key==KEY_EXIT || key==KEY_EXE || key==KEY_F6) return;}
}
bool ui_confirm(const char *title,const char *message)
{
    ui_frame(title,NULL);ui_text(10,62,UI_INK,"%s",message);
    ui_softkeys("YES","","","","","NO");dupdate();
    for(;;) {
        int key=ui_getkey().key;
        if(key==KEY_F1 || key==KEY_EXE) return true;
        if(key==KEY_EXIT || key==KEY_F6) return false;
    }
}
int ui_digit(int key)
{
    const int keys[]={KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9};
    for(int i=0;i<10;i++) if(key==keys[i]) return i;
    return -1;
}
int ui_choose(const char *title,const char *const *items,int count,int selected)
{
    if(count<1) return -1;
    if(selected<0 || selected>=count) selected=0;
    for(;;) {
        ui_frame(title,NULL);
        int page=selected/7;
        for(int row=0;row<7 && page*7+row<count;row++) {
            char number[16];snprintf(number,sizeof(number),"%d",page*7+row+1);
            ui_field(row,number,items[page*7+row],page*7+row==selected);
        }
        ui_form_hint(NULL,"EXE: open");
        ui_softkeys("",count>7 ? "PG-":"",count>7 ? "PG+":"","","","OPEN");dupdate();
        int key=ui_getkey().key;
        if(key==KEY_UP) selected=(selected+count-1)%count;
        if(key==KEY_DOWN) selected=(selected+1)%count;
        if(count>7 && key==KEY_F2) selected=selected>=7 ? selected-7:0;
        if(count>7 && key==KEY_F3) selected=selected+7<count ? selected+7:count-1;
        if(key==KEY_EXIT) return -1;
        if(key==KEY_EXE || key==KEY_F6) return selected;
        int digit=ui_digit(key);
        if(digit>=1 && digit<=9 && digit<=count) return digit-1;
    }
}
bool ui_cancel(void *unused)
{
    (void)unused;
#ifdef FXCG50
    while(pending_count<8) {
        volatile int timeout=1;
        key_event_t event=getkey_opt(GETKEY_DEFAULT & ~GETKEY_MENU,&timeout);
        if(event.type==KEYEV_NONE)break;
        if(event.type==KEYEV_DOWN && (event.key==KEY_EXIT || event.key==KEY_ACON))return true;
        pending[pending_count++]=event;
        if(event.key==KEY_MENU && !event.shift && !event.alpha)return true;
    }
    /* Yield a partial computation before input saturation can hide a later
       EXIT/MENU. The next UI boundary drains these retained keys. */
    if(pending_count==8)return true;
#else
    key_event_t event;
    while((event=pollevent()).type!=KEYEV_NONE)
        if(event.type==KEYEV_DOWN && (event.key==KEY_EXIT || event.key==KEY_ACON)) return true;
#endif
    return false;
}

void ui_blink_start(UiBlink *blink)
{
    blink->timer=-1;blink->timeout=0;blink->highlighted=true;
#ifdef FXCG50
    blink->timer=timer_configure(TIMER_ANY,250000,GINT_CALL_SET(&blink->timeout));
    if(blink->timer>=0)timer_start(blink->timer);
#endif
}

key_event_t ui_blink_key(UiBlink *blink)
{
    key_event_t event;
#ifdef FXCG50
    if(pending_count)return ui_getkey();
    if(blink->timer>=0) {
        blink->timeout=0;
        event=getkey_opt(GETKEY_DEFAULT,&blink->timeout);
    } else event=ui_getkey();
#else
    event=ui_getkey();
#endif
    if(event.type==KEYEV_NONE)blink->highlighted=!blink->highlighted;
    return event;
}

void ui_blink_stop(UiBlink *blink)
{
#ifdef FXCG50
    if(blink->timer>=0)timer_stop(blink->timer);
#endif
    blink->timer=-1;blink->timeout=0;
}
