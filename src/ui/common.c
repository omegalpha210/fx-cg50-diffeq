#include "ui.h"
#include "trace.h"
#include <gint/rtc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <gint/drivers/r61524.h>
#ifdef FXCG50
#include <gint/timer.h>
#include <gint/gint.h>
#include <gint/drivers/keydev.h>
#endif
/* Only a small queue of transformed input, never pixels or trajectories.
   Numerical cancellation polls must not discard keys during blink redraws. */
#define UI_PENDING_CAPACITY 8
static key_event_t pending[UI_PENDING_CAPACITY];
static unsigned pending_count;
#ifdef FXCG50
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
    while(pending_count) {
        key_event_t event=pending[0];
        memmove(pending,pending+1,(--pending_count)*sizeof(*pending));
        if(event.key==KEY_MENU && !event.shift && !event.alpha) {
            /* MENU observed by the compute poll was deliberately not handled
               by getkey_opt. All other MENU keys use getkey's normal path. */
#ifdef FXCG50
            gint_osmenu();dupdate();continue;
#endif
        }
        return event;
    }
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
void ui_progress(unsigned stage)
{
    ui_rect(UI_W-40,0,40,21,UI_BLUE);
    if(stage<1 || stage>3)return;
    char text[4]={(char)('0'+stage),'/', '3',0};int width;
    dsize(text,NULL,&width,NULL);ui_text(UI_W-8-width,5,C_WHITE,"%s",text);
}
bool ui_select_move(int key,int *selected,int count)
{
    if(count<1 || (key!=KEY_UP && key!=KEY_DOWN))return false;
    *selected=(*selected+count+(key==KEY_UP ? -1:1))%count;return true;
}
/* dtext() and bounded dtext_opt() use the same normal-weight gint primitive.
   Draw disjoint segments once. dnsize(empty) is -char_spacing, NOT zero; a
   leading token must start at x, not x-1 (the beta.2 overdraw defect). */
static void help_segment(int x,int y,const char *text,int offset,int count,int color,int spacing)
{
    if(count<=0)return;
    int width=0;if(offset){dnsize(text,offset,NULL,&width,NULL);width+=spacing;}
    dtext_opt(UI_X+x+width,UI_Y+y,color,C_NONE,DTEXT_LEFT,DTEXT_TOP,text+offset,count);
}
void ui_help(int x,int y,const char *text,bool main_menu)
{
    int start=0,spacing;dnsize("",0,NULL,&spacing,NULL);spacing=-spacing;
    if(main_menu && !strncmp(text,"MENU:",5)) {
        help_segment(x,y,text,0,4,C_RED,spacing);start=4;
    }
    for(const char *p=text;(p=strstr(p,"EXE"));p+=3) {
        if((p!=text && (isalnum((unsigned char)p[-1]) || p[-1]=='_')) ||
            isalnum((unsigned char)p[3]) || p[3]=='_')continue;
        int offset=(int)(p-text);
        help_segment(x,y,text,start,offset-start,UI_MUTED,spacing);
        help_segment(x,y,text,offset,3,C_BLUE,spacing);start=offset+3;
    }
    help_segment(x,y,text,start,(int)strlen(text)-start,UI_MUTED,spacing);
}
typedef enum {SOFTKEY_NORMAL,SOFTKEY_INIT,SOFTKEY_ADV,SOFTKEY_VWIN,
    SOFTKEY_SET,SOFTKEY_NEXT,SOFTKEY_PREV,SOFTKEY_EXECUTE} SoftkeyStyle;
static SoftkeyStyle softkey_style(const char *label)
{
    if(!strcmp(label,"INIT"))return SOFTKEY_INIT;
    if(!strcmp(label,"ADV"))return SOFTKEY_ADV;
    if(!strcmp(label,"V-WIN") || !strcmp(label,"NORMAL"))return SOFTKEY_VWIN;
    if(!strcmp(label,"SET") || !strcmp(label,"FAST"))return SOFTKEY_SET;
    if(!strcmp(label,"NEXT") || !strcmp(label,"FASTER"))return SOFTKEY_NEXT;
    if(!strcmp(label,"PREV"))return SOFTKEY_PREV;
    if(!strcmp(label,"GRAPH") || !strcmp(label,"RUN"))return SOFTKEY_EXECUTE;
    return SOFTKEY_NORMAL;
}
void ui_softkeys(const char *a,const char *b,const char *c,const char *d,const char *e,const char *f)
{
    const char *keys[]={a,b,c,d,e,f};
    for(int i=0;i<6;i++) {
        static const int backgrounds[]={UI_BLUE,UI_YELLOW,C_BLACK,C_RGB(31,17,0),
            UI_BRIGHT_GREEN,UI_CYAN,0xf81f,C_RED};
        SoftkeyStyle style=softkey_style(keys[i]);int background=backgrounds[style];
        int foreground=style==SOFTKEY_NORMAL || style==SOFTKEY_ADV || style==SOFTKEY_EXECUTE ? C_WHITE:C_BLACK;
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
    if(out!=text)snprintf(out,capacity,"%s",text);
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
{ui_field_at(31+row*22,label,value,selected);}
void ui_field_at(int y,const char *label,const char *value,bool selected)
{
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
    if(text && text[0])ui_help(8,184,text,false);
}
void ui_color_swatch(int x,int y,int color)
{
    ui_rect(x,y,29,13,UI_INK);ui_rect(x+2,y+2,25,9,color);
}
void ui_form_error(const char *message)
{
    char text[192];snprintf(text,sizeof(text),"%s",message);
    for(char *p=text;*p;p++)if(*p=='\n')*p=' ';
    ui_short(text,sizeof(text),text,UI_W-16);
    ui_rect(0,179,UI_W,19,C_WHITE);ui_text(8,184,C_RED,"%s",text);
}
void ui_field_error(const char *message)
{
    ui_form_error(message);
    ui_softkeys("","","","","","EDIT");dupdate();
    for(;;){int key=ui_getkey().key;if(key==KEY_EXIT || key==KEY_EXE || key==KEY_F6)return;}
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
    ui_softkeys("","","","","NO","YES");dupdate();
    for(;;) {
        key_event_t event=ui_getkey();
        if(event.type==KEYEV_HOLD)continue; /* Opening key repeat is not new consent. */
        int key=event.key;
        if(key==KEY_F6 || key==KEY_EXE) return true;
        if(key==KEY_EXIT || key==KEY_F5) return false;
    }
}
bool ui_save_confirm(void)
{
    return ui_confirm("Save session","Save current session?");
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
        ui_softkeys("",count>7 ? "PG-":"",count>7 ? "PG+":"","","","OPEN");dupdate();
        int key=ui_getkey().key;
        ui_select_move(key,&selected,count);
        if(count>7 && key==KEY_F2) selected=selected>=7 ? selected-7:0;
        if(count>7 && key==KEY_F3) selected=selected+7<count ? selected+7:count-1;
        if(key==KEY_EXIT) return -1;
        if(key==KEY_EXE || key==KEY_F6) return selected;
        int digit=ui_digit(key);
        if(digit>=1 && digit<=9 && digit<=count) return digit-1;
    }
}
static bool poll_input(bool cancel)
{
    while(pending_count<UI_PENDING_CAPACITY) {
#ifdef FXCG50
        volatile int timeout=1;
        key_event_t event=getkey_opt(GETKEY_DEFAULT & ~GETKEY_MENU,&timeout);
#else
        key_event_t event=pollevent();
#endif
        if(event.type==KEYEV_NONE)break;
        if(event.type==KEYEV_HOLD && event.key==KEY_EXIT)continue;
        bool stop=event.type==KEYEV_DOWN && (event.key==KEY_EXIT || event.key==KEY_ACON);
        if(cancel && stop)return true;
        pending[pending_count++]=event;
        if(cancel && event.key==KEY_MENU && !event.shift && !event.alpha)return true;
    }
    return cancel && pending_count==UI_PENDING_CAPACITY;
}
bool ui_cancel(void *unused)
{(void)unused;return poll_input(true);}
void ui_defer_input(void){(void)poll_input(false);}

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
        do {event=getkey_opt(GETKEY_DEFAULT,&blink->timeout);}
        while(event.type==KEYEV_HOLD && event.key==KEY_EXIT);
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

static void busy_rect(const UiBusy *busy,int *top,int *width)
{
    char text[40];snprintf(text,sizeof(text),"%s -",busy->label);dsize(text,NULL,width,NULL);*width+=4;
    *top=busy->area==UI_BUSY_DRAW ? 202:(busy->area==UI_BUSY_TABLE ? 28:184);
}
static void busy_upload(int left,int top,int width,int height)
{
    /* Installed driver waits for prior DMA, then synchronously transfers this
       small rectangle by CPU. Safe to restore the source pixels on return. */
    r61524_display_rect(gint_vram,UI_X+left,UI_X+left+width-1,UI_Y+top,UI_Y+top+height-1);
}
static bool busy_screen(const UiBusy *busy)
{return busy->area==UI_BUSY_TABLE;}
static void busy_draw_paint(const char *text)
{
    unsigned capacity;uint16_t *saved=graph_busy_pixels(&capacity,true);
    int rows=(int)(capacity/UI_W);if(rows<1)return;
    int width;dsize("EXIT cancels",NULL,&width,NULL);
    struct dwindow old=dwindow;
    /* Exactly the six softkeys' bounding rectangle, including separators.
       The LCD keeps the prior graph while VRAM owns unfinished construction. */
    for(int y=198;y<216;y+=rows) {
        int height=216-y<rows ? 216-y:rows;
        for(int r=0;r<height;r++)memcpy(saved+r*UI_W,
            gint_vram+(UI_Y+y+r)*DWIDTH+UI_X,UI_W*2);
        dwindow_set((struct dwindow){UI_X,UI_Y+y,UI_X+UI_W,UI_Y+y+height});
        ui_rect(0,198,UI_W,18,UI_BLUE);
        if(y<202+dfont_default()->data_height && y+height>202) {
            ui_text(5,202,C_WHITE,"%s",text);
            ui_text(UI_W-5-width,202,C_WHITE,"EXIT cancels");
        }
        busy_upload(0,y,UI_W,height);
        for(int r=0;r<height;r++)memcpy(gint_vram+(UI_Y+y+r)*DWIDTH+UI_X,
            saved+r*UI_W,UI_W*2);
    }
    dwindow_set(old);
#ifndef FXCG50
    host_display_frame();
#endif
}
static void busy_screen_paint(UiBusy *busy,const char *text)
{
    unsigned capacity;uint16_t *saved=graph_busy_pixels(&capacity,true);
    int rows=(int)(capacity/DWIDTH);if(rows<1)return;
    /* The LCD owns this temporary screen; VRAM still owns the stable graph or
       in-progress construction. Transfer bounded strips synchronously and put
       every source pixel back. Only the first delayed frame clears the canvas. */
    int start=busy->visible ? UI_Y:0,end=busy->visible ? UI_Y+21:DHEIGHT;
    struct dwindow old=dwindow;
    for(int top=start;top<end;top+=rows) {
        int height=end-top<rows ? end-top:rows;
        memcpy(saved,gint_vram+top*DWIDTH,(unsigned)(DWIDTH*height)*2);
        dwindow_set((struct dwindow){0,top,DWIDTH,top+height});
        drect(0,top,DWIDTH-1,top+height-1,C_WHITE);
        ui_rect(0,0,UI_W,21,UI_BLUE);
        if(top<UI_Y+16 && top+height>UI_Y+5)ui_text(8,5,C_WHITE,"%s",text);
        if(!busy->visible && top<UI_Y+37 && top+height>UI_Y+26)
            ui_text(8,26,UI_MUTED,"EXIT cancels");
        r61524_display_rect(gint_vram,0,DWIDTH-1,top,top+height-1);
        memcpy(gint_vram+top*DWIDTH,saved,(unsigned)(DWIDTH*height)*2);
    }
    dwindow_set(old);
#ifndef FXCG50
    host_display_frame();
#endif
}
void ui_busy_begin(UiBusy *busy,const char *label,UiBusyArea area,OdeCancel cancel,void *context)
{
    *busy=(UiBusy){.start=rtc_ticks(),.label=label,.area=area,.cancel=cancel,.context=context};
    busy->last=busy->start;
}
void ui_busy_start(UiBusy *busy)
{ui_busy_begin(busy,"CALCULATING...",UI_BUSY_RESULT,ui_cancel,NULL);}
bool ui_busy_cancel(void *context)
{
    UiBusy *busy=context;
    /* Poll before reading the clock or touching pixels. */
    if(busy->cancel && busy->cancel(busy->context))return true;
    uint32_t now=rtc_ticks();
    uint32_t elapsed=now>=busy->start ? now-busy->start:now+86400u*128u-busy->start;
    uint32_t delta=now>=busy->last ? now-busy->last:now+86400u*128u-busy->last;
    if(elapsed>=20 && (!busy->visible || delta>=16)) {
        char text[40];snprintf(text,sizeof(text),"%s %c",busy->label,"/-\\|"[busy->frame++%4]);
        if(busy->area==UI_BUSY_DRAW) {
            busy_draw_paint(text);busy->visible=true;busy->last=now;return false;
        }
        if(busy_screen(busy)) {
            busy_screen_paint(busy,text);busy->visible=true;busy->last=now;return false;
        }
        int width,top;busy_rect(busy,&top,&width);
        int left=5,height=dfont_default()->data_height+1;
        unsigned capacity;uint16_t *saved=graph_busy_pixels(&capacity,busy->area!=UI_BUSY_TABLE);
        /* Borrow unused bytes after TRACE staging, never a new framebuffer.
           Restore VRAM immediately after upload, so ongoing plot drawing and
           scratch cancellation cannot leave a white patch in the source graph. */
        if((unsigned)(width*height)<=capacity) {
            for(int y=0;y<height;y++)memcpy(saved+y*width,
                gint_vram+(UI_Y+top+y)*DWIDTH+UI_X+left,(unsigned)width*2);
            ui_rect(left,top,width,height,C_WHITE);
            ui_text(left+2,top,UI_BLUE,"%s",text);busy_upload(left,top,width,height);
            for(int y=0;y<height;y++)memcpy(gint_vram+(UI_Y+top+y)*DWIDTH+UI_X+left,
                saved+y*width,(unsigned)width*2);
#ifndef FXCG50
            host_display_frame();
#endif
            busy->visible=true;busy->last=now;
        }
    }
    return false;
}
void ui_busy_end(UiBusy *busy)
{
    if(busy->visible && !busy_screen(busy) && busy->area!=UI_BUSY_DRAW) {int top,width;busy_rect(busy,&top,&width);
        busy_upload(5,top,width,dfont_default()->data_height+1);
#ifndef FXCG50
        host_display_frame();
#endif
    }
    /* Table's preparation screen and Drawing's bar are replaced after commit or
       rollback. Never upload a cancelled in-progress Graph framebuffer here. */
    busy->visible=false;
}
