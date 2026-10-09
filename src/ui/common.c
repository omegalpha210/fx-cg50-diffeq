#include "ui.h"
#include "trace.h"
#include "power.h"
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
static unsigned input_epoch;
static void sync_power_input(void)
{
    unsigned epoch=power_input_epoch();
    if(epoch!=input_epoch) {
        pending_count=0;trace_pending=(key_event_t){0};input_epoch=epoch;
    }
}
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
    if(power_poll(false))return true;
#ifdef FXCG50
    for(;;) {
        volatile int timeout=1;
        key_event_t event=getkey_opt(GETKEY_DEFAULT & ~(GETKEY_MENU|GETKEY_POWEROFF),&timeout);
        if(event.type==KEYEV_NONE)break;
        if(power_key(event))return true;
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
    (void)power_poll(true);sync_power_input();
    ui_trace_cancel(NULL);
    if(trace_pending.type==KEYEV_NONE) {
        key_event_t fresh=ui_blink_key(blink);sync_power_input();trace_accept(fresh);
    }
    ui_trace_cancel(NULL);
    key_event_t event=trace_pending;trace_pending=(key_event_t){0};
#ifdef FXCG50
    if(event.key==KEY_MENU)power_osmenu();
#endif
    return event;
}
static key_event_t take_key(void)
{
    (void)power_poll(true);sync_power_input();
    while(pending_count) {
        key_event_t event=pending[0];
        memmove(pending,pending+1,(--pending_count)*sizeof(*pending));
        if(event.key==KEY_MENU && !event.shift && !event.alpha) {
            /* MENU observed by the compute poll waits for stable UI state;
               ordinary idle MENU is handled by the shared power wait. */
#ifdef FXCG50
            power_osmenu();continue;
#endif
        }
        return event;
    }
    key_event_t event=power_wait_key(NULL);sync_power_input();return event;
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
/* Header and F-key chrome span the full 396x224 LCD; content and the plot keep
   the established logical 384x216 coordinates (TRACE/Xdot depend on them). */
static int selected_y=-1,edit_y=-1;
static void paint_header(void)
{
    drect(0,0,DWIDTH-1,UI_Y+20,UI_NAVY);
    drect(0,UI_Y+21,DWIDTH-1,UI_Y+22,UI_ACCENT);
}
void ui_frame(const char *title,const char *subtitle)
{
    dclear(C_WHITE);paint_header();selected_y=edit_y=-1;
    ui_text(8,5,C_WHITE,"%s",title);
    if(subtitle) ui_help(8,26,subtitle,false);
}
void ui_formula(const char *text)
{
    /* A neutral template card: no accent bar, so it never reads as a selection. */
    ui_rect(4,25,376,19,UI_LINE);ui_rect(5,26,374,17,UI_SURFACE);
    ui_text(12,29,UI_INK,"%s",text);
}
void ui_progress(unsigned stage)
{
    ui_rect(UI_W-92,0,92,21,UI_NAVY);
    if(stage<1 || stage>3)return;
    int left=UI_W-8-42;
    for(unsigned k=0;k<3;k++) {
        int x=left+(int)k*15;
        if(k+1<stage)ui_rect(x,8,12,4,C_RGB(14,19,27));
        else if(k+1==stage)ui_rect(x,7,12,6,C_WHITE);
        else {
            ui_rect(x,8,12,4,C_RGB(11,15,23));ui_rect(x+1,9,10,2,UI_NAVY);
        }
    }
    char text[4]={(char)('0'+stage),'/', '3',0};int width;
    dsize(text,NULL,&width,NULL);ui_text(left-6-width,5,C_WHITE,"%s",text);
}
void ui_list_position(int selected,int count)
{
    if(count<=7 || selected<0 || selected>=count)return;
    char text[24];snprintf(text,sizeof(text),"%d of %d",selected+1,count);
    int width;dsize(text,NULL,&width,NULL);
    ui_text(UI_W-8-width,5,C_WHITE,"%s",text);
}
bool ui_select_move(int key,int *selected,int count)
{
    if(count<1 || (key!=KEY_UP && key!=KEY_DOWN))return false;
    *selected=(*selected+count+(key==KEY_UP ? -1:1))%count;return true;
}
/* dtext() and bounded dtext_opt() use the same normal-weight gint primitive.
   Draw disjoint segments once. dnsize(empty) is -char_spacing, NOT zero; a
   leading token must start at x, not x-1 (the beta.2 overdraw defect). */
static int char_spacing(void)
{int spacing;dnsize("",0,NULL,&spacing,NULL);return -spacing;}
static int help_plain(int x,int y,const char *text,int count,int color,bool draw)
{
    if(count<=0)return x;
    int width;dnsize(text,count,NULL,&width,NULL);
    if(draw)dtext_opt(UI_X+x,UI_Y+y,color,C_NONE,DTEXT_LEFT,DTEXT_TOP,text,count);
    return x+width+char_spacing();
}
/* Plain words stay muted; a standalone EXE remains the blue confirmation key. */
static int help_words(int x,int y,const char *text,int count,bool draw)
{
    int start=0;
    for(int i=0;i+3<=count;i++) {
        const char *p=text+i;
        if(strncmp(p,"EXE",3))continue;
        if((i && (isalnum((unsigned char)p[-1]) || p[-1]=='_')) ||
            (i+3<count && (isalnum((unsigned char)p[3]) || p[3]=='_')))continue;
        x=help_plain(x,y,text+start,i-start,UI_MUTED,draw);
        x=help_plain(x,y,p,3,C_BLUE,draw);start=i+3;i+=2;
    }
    return help_plain(x,y,text+start,count-start,UI_MUTED,draw);
}
static bool key_word(const char *p,int n)
{
    static const char *const names[]={"LEFT","RIGHT","UP","DOWN","Left","Right","EXE","EXIT",
        "MENU","OPTN","AC","Comma","F1","F2","F3","F4","F5","F6"};
    for(unsigned i=0;i<sizeof(names)/sizeof(*names);i++)
        if((int)strlen(names[i])==n && !strncmp(p,names[i],(size_t)n))return true;
    return n==3 && isdigit((unsigned char)p[0]) && p[1]=='-' && isdigit((unsigned char)p[2]);
}
/* Length of "KEY[/KEY...]" directly followed by ':' at p, else 0. */
static int key_token(const char *p)
{
    int n=0,part=0;
    for(;;) {
        if(p[n]==':' || p[n]=='/') {
            if(!key_word(p+part,n-part))return 0;
            if(p[n]==':')return n;
            part=++n;continue;
        }
        if(!isalnum((unsigned char)p[n]) && p[n]!='-')return 0;
        n++;
    }
}
static void arrow(int x,int y,int direction)
{
    /* 3-column/3-row pixel arrow heads centred on text row y+4: crisp, no AA. */
    for(int k=0;k<3;k++) {
        if(direction==0)ui_rect(x+k,y+4-k,1,2*k+1,UI_INK);
        if(direction==1)ui_rect(x+2-k,y+4-k,1,2*k+1,UI_INK);
        if(direction==2)ui_rect(x+2-k,y+2+k,2*k+1,1,UI_INK);
        if(direction==3)ui_rect(x+2-k,y+6-k,2*k+1,1,UI_INK);
    }
}
static int key_cap(int x,int y,const char *name,int n,int color,bool draw)
{
    int kind=-1,w;
    if(n==4 && (!strncmp(name,"LEFT",4) || !strncmp(name,"Left",4)))kind=0;
    if(n==5 && (!strncmp(name,"RIGHT",5) || !strncmp(name,"Right",5)))kind=1;
    if(n==2 && !strncmp(name,"UP",2))kind=2;
    if(n==4 && !strncmp(name,"DOWN",4))kind=3;
    if(n==5 && !strncmp(name,"Comma",5)){name=",";n=1;}
    if(kind>=0)w=11;
    else {dnsize(name,n,NULL,&w,NULL);w+=8;}
    if(draw) {
        ui_rect(x,y-2,w,13,UI_KEY_EDGE);ui_rect(x+1,y-1,w-2,11,UI_SURFACE);
        if(kind>=0)arrow(x+4,y,kind);
        else dtext_opt(UI_X+x+4,UI_Y+y,color,C_NONE,DTEXT_LEFT,DTEXT_TOP,name,n);
    }
    return x+w;
}
static int key_caps(int x,int y,const char *p,int n,bool main_menu,bool draw)
{
    /* LEFT/RIGHT and UP/DOWN pairs share one cap with two arrow heads. */
    bool horizontal=(n==10 && (!strncmp(p,"LEFT/RIGHT",10) || !strncmp(p,"Left/Right",10)));
    if(horizontal || (n==7 && !strncmp(p,"UP/DOWN",7))) {
        if(draw) {
            ui_rect(x,y-2,19,13,UI_KEY_EDGE);ui_rect(x+1,y-1,17,11,UI_SURFACE);
            arrow(x+4,y,horizontal ? 0:2);arrow(x+12,y,horizontal ? 1:3);
        }
        return x+19;
    }
    for(int start=0,i=0;i<=n;i++) {
        if(i<n && p[i]!='/')continue;
        bool exe=i-start==3 && !strncmp(p+start,"EXE",3);
        bool menu=main_menu && i-start==4 && !strncmp(p+start,"MENU",4);
        if(start)x+=2;
        x=key_cap(x,y,p+start,i-start,exe ? C_BLUE:(menu ? C_RED:UI_INK),draw);start=i+1;
    }
    return x;
}
static int help_draw(int x,int y,const char *text,bool main_menu,bool draw)
{
    int start=x;const char *segment=text;
    for(const char *p=text;*p;) {
        bool boundary=p==text || p[-1]==' ' || p[-1]==',' || p[-1]==';' || p[-1]=='(';
        int n=boundary ? key_token(p):0;
        if(n<=0){p++;continue;}
        x=help_words(x,y,segment,(int)(p-segment),draw);
        x=key_caps(x,y,p,n,main_menu,draw)+4;
        p+=n+1;while(*p==' ')p++;
        segment=p;
    }
    x=help_words(x,y,segment,(int)strlen(segment),draw);
    return x>start ? x-start-char_spacing():0;
}
void ui_help(int x,int y,const char *text,bool main_menu)
{
#ifndef FXCG50
    /* Host tests assert which hint is shown; caps leave no literal TEXT line. */
    printf("HINT %d %d %s\n",UI_X+x,UI_Y+y,text);
#endif
    (void)help_draw(x,y,text,main_menu,true);
}
int ui_help_width(const char *text)
{return help_draw(0,0,text,false,false);}
static int softkey_accent(const char *label,bool *primary)
{
    *primary=!strcmp(label,"GRAPH") || !strcmp(label,"RUN");
    if(!strcmp(label,"INIT"))return UI_YELLOW;
    if(!strcmp(label,"ADV"))return C_RGB(20,22,26);
    if(!strcmp(label,"V-WIN") || !strcmp(label,"NORMAL"))return C_RGB(31,17,0);
    if(!strcmp(label,"SET") || !strcmp(label,"FAST"))return UI_BRIGHT_GREEN;
    if(!strcmp(label,"NEXT") || !strcmp(label,"FASTER"))return UI_CYAN;
    if(!strcmp(label,"PREV"))return 0xf81f;
    return -1;
}
void ui_softkey_rect(int index,int *x,int *w)
{*x=index*66+1-UI_X;*w=64;}
static void softkey(int index,const char *label,bool active)
{
    int x,w;ui_softkey_rect(index,&x,&w);
    /* Empty slot: no pixels (an empty dtext paints nothing; hosts still log it). */
    if(!label[0]){ui_text(x+w/2,206,C_WHITE,"%s","");return;}
    bool primary;int accent=softkey_accent(label,&primary);
    int background=primary ? C_RGB(26,4,4):(active && accent>=0 ? accent:UI_TAB);
    ui_rect(x,199,w,21,background);
    if(!strcmp(label,"COLOR")) {
        static const int colors[]={0xf800,0xfc40,UI_BRIGHT_GREEN,UI_CYAN,0xf81f};
        for(int j=0;j<5;j++)ui_rect(x+2+j*12,199,12,3,colors[j]);
    } else if(accent>=0 && !primary && !active)ui_rect(x+2,199,w-4,3,accent);
    int width;dsize(label,NULL,&width,NULL);
    ui_text(x+(w-width)/2,206,active && !primary ? C_BLACK:C_WHITE,"%s",label);
}
void ui_softkeys_active(const char *a,const char *b,const char *c,const char *d,const char *e,const char *f,int active)
{
    const char *keys[]={a,b,c,d,e,f};
    /* Empty slots stay blank: nothing that looks pressable. */
    drect(0,UI_Y+198,DWIDTH-1,DHEIGHT-1,C_WHITE);
    for(int i=0;i<6;i++)softkey(i,keys[i],i==active);
}
void ui_softkeys(const char *a,const char *b,const char *c,const char *d,const char *e,const char *f)
{ui_softkeys_active(a,b,c,d,e,f,-1);}
void ui_softkeys_info(const char *left,const char *right,int swatch,const char *f6)
{
    drect(0,UI_Y+198,DWIDTH-1,DHEIGHT-1,C_WHITE);
    int x,w,end;ui_softkey_rect(0,&x,&w);ui_softkey_rect(4,&end,&w);end+=w;
    /* F1-F5 carry no keys here: one read-only panel, visibly not a tab. */
    ui_rect(x,199,end-x,21,UI_KEY_EDGE);ui_rect(x+1,200,end-x-2,20,UI_SURFACE);
    int text=x+8;
    if(swatch>=0){ui_rect(x+8,209,10,3,swatch);text=x+22;}
    if(left && left[0])ui_text(text,206,UI_INK,"%s",left);
    if(right && right[0])ui_text(x+(end-x)/2+4,206,UI_INK,"%s",right);
    softkey(5,f6,false);
}
void ui_scrollbar(int top,int height,int first,int shown,int total)
{
    if(total<=shown || shown<1 || height<8)return;
    int thumb=height*shown/total;if(thumb<8)thumb=8;
    int range=total-shown;if(first>range)first=range;if(first<0)first=0;
    ui_rect(381,top,3,height,UI_LINE);
    ui_rect(381,top+(height-thumb)*first/range,3,thumb,UI_NAVY);
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
#define UI_LABEL C_RGB(8,10,14)
static void field_row(int y,const char *label,const char *separator,const char *value,bool selected,bool option)
{
    ui_rect(4,y-4,376,21,selected ? UI_SELECT:C_WHITE);
    if(selected){ui_rect(4,y-4,3,21,UI_ACCENT);selected_y=y;}
    ui_text(14,y,selected ? UI_INK:UI_LABEL,"%s",label);
    ui_text(122,y,UI_MUTED,"%s",separator);
    char short_value[192];ui_short(short_value,sizeof(short_value),value,226);
    ui_text(138,y,UI_INK,"%s",short_value);
    if(option && selected) {
        int width;dsize(short_value,NULL,&width,NULL);
        arrow(132,y,0);arrow(138+width+3,y,1);
    }
    if(!selected)ui_line(10,y+17,374,y+17,UI_LINE);
}
void ui_field(int row,const char *label,const char *value,bool selected)
{ui_field_at(31+row*22,label,value,selected);}
void ui_field_at(int y,const char *label,const char *value,bool selected)
{field_row(y,label,":",value,selected,false);}
void ui_field_eq(int row,const char *label,const char *value,bool selected)
{field_row(31+row*22,label,"=",value,selected,false);}
void ui_field_option(int row,const char *label,const char *value,bool selected)
{field_row(31+row*22,label,":",value,selected,true);}
static void edit_border(int y,int color)
{
    ui_rect(133,y-4,235,2,color);ui_rect(133,y+15,235,2,color);
    ui_rect(133,y-4,2,21,color);ui_rect(366,y-4,2,21,color);
}
void ui_edit_frame(int y)
{edit_border(y,UI_ACCENT);edit_y=y;}
void ui_form_hint_last(const UiInlineEdit *edit,const char *context,bool last)
{
    const char *text=edit && edit->active ? (edit->limited ? UI_LIMIT_HINT:
        (last ? UI_EDIT_LAST_HINT:UI_EDIT_HINT)):context;
    if(text && text[0])ui_help(8,184,text,false);
}
void ui_form_hint(const UiInlineEdit *edit,const char *context)
{ui_form_hint_last(edit,context,false);}
void ui_color_swatch(int x,int y,int color)
{
    ui_rect(x,y,29,13,UI_INK);ui_rect(x+2,y+2,25,9,color);
}
void ui_form_error(const char *message)
{
    char text[192];snprintf(text,sizeof(text),"%s",message);
    for(char *p=text;*p;p++)if(*p=='\n')*p=' ';
    ui_short(text,sizeof(text),text,UI_W-32);
    ui_rect(-UI_X,179,DWIDTH,19,UI_ERROR_BG);ui_rect(-UI_X,179,3,19,UI_ERROR);
    ui_rect(8,183,11,11,UI_ERROR);
    int width;dsize("!",NULL,&width,NULL);ui_text(8+(11-width)/2,184,C_WHITE,"!");
    ui_text(24,184,UI_ERROR,"%s",text);
    /* Only a field being edited is known to be the culprit: its marks turn red
       (text untouched). A merely selected row is left as it is. */
    if(selected_y>=0 && edit_y==selected_y) {
        ui_rect(4,selected_y-4,3,21,UI_ERROR);edit_border(edit_y,UI_ERROR);
    }
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
/* Single-buffered VRAM still holds the calling screen: darken it toward navy
   (exact RGB565 halving, no dithering) and draw a crisp card over it. */
static void dim_screen(void)
{
    const uint16_t tint=(uint16_t)((UI_NAVY>>1)&0x7bef);
    for(int i=0;i<DWIDTH*DHEIGHT;i++)gint_vram[i]=(uint16_t)(((gint_vram[i]>>1)&0x7bef)+tint);
}
static void confirm_card(const char *title,const char *message,const char *detail)
{
    dim_screen();
    char line[96],note[96];ui_short(line,sizeof(line),message,320);
    note[0]=0;if(detail)ui_short(note,sizeof(note),detail,320);
    int w1,w2=0;dsize(line,NULL,&w1,NULL);if(note[0])dsize(note,NULL,&w2,NULL);
    int width=(w1>w2 ? w1:w2)+56;if(width<240)width=240;if(width>368)width=368;
    int height=note[0] ? 100:84,x=(UI_W-width)/2,y=(198-height)/2;
    ui_rect(x+3,y+3,width,height,C_RGB(1,2,5));
    ui_rect(x-1,y-1,width+2,height+2,UI_NAVY);ui_rect(x,y,width,height,C_WHITE);
    ui_rect(x,y,width,20,UI_NAVY);ui_rect(x,y+20,width,2,UI_ACCENT);
    ui_text(x+10,y+5,C_WHITE,"%s",title);
    ui_rect(x+12,y+32,16,16,UI_ACCENT);
    int qw;dsize("?",NULL,&qw,NULL);ui_text(x+12+(16-qw)/2,y+35,C_WHITE,"?");
    ui_text(x+40,y+30,UI_INK,"%s",line);
    if(note[0])ui_text(x+40,y+47,UI_MUTED,"%s",note);
    ui_help(x+40,y+height-20,"EXE: Yes   EXIT: No",false);
}
static bool confirm(const char *title,const char *message,const char *detail)
{
    confirm_card(title,message,detail);
    ui_softkeys("","","","","NO","YES");dupdate();
    for(;;) {
        key_event_t event=ui_getkey();
        if(event.type==KEYEV_HOLD)continue; /* Opening key repeat is not new consent. */
        int key=event.key;
        if(key==KEY_F6 || key==KEY_EXE) return true;
        if(key==KEY_EXIT || key==KEY_F5) return false;
    }
}
bool ui_confirm(const char *title,const char *message)
{return confirm(title,message,NULL);}
bool ui_save_confirm(void)
{
    return confirm("Save session","Save current session?","Restore later with RECALL.");
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
        ui_list_position(selected,count);
        int page=selected/7;
        for(int row=0;row<7 && page*7+row<count;row++) {
            char number[16];snprintf(number,sizeof(number),"%d",page*7+row+1);
            ui_field(row,number,items[page*7+row],page*7+row==selected);
        }
        ui_scrollbar(27,153,page*7,7,count);
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
    if(power_poll(false) && cancel)return true;
    while(pending_count<UI_PENDING_CAPACITY) {
#ifdef FXCG50
        volatile int timeout=1;
        key_event_t event=getkey_opt(GETKEY_DEFAULT & ~(GETKEY_MENU|GETKEY_POWEROFF),&timeout);
#else
        key_event_t event=pollevent();
#endif
        if(event.type==KEYEV_NONE)break;
        if(power_key(event)) {if(cancel)return true;continue;}
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
        do {event=power_wait_key(&blink->timeout);}
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

static bool busy_screen(const UiBusy *busy)
{return busy->area==UI_BUSY_TABLE;}
void ui_busy_bar(const char *label,unsigned frame)
{
    drect(0,UI_Y+198,DWIDTH-1,DHEIGHT-1,C_WHITE);
    drect(1,UI_Y+199,DWIDTH-2,DHEIGHT-1,UI_TAB);
    ui_text(4,206,C_WHITE,"%s",label);
    int width;dsize(label,NULL,&width,NULL);
    /* Indeterminate progress: one block steps across a fixed track at <=8Hz. */
    int track=4+width+10;
    ui_rect(track,209,96,4,C_RGB(9,12,18));ui_rect(track+(int)(frame%4)*24,209,24,4,UI_ACCENT);
    int text,cap;dsize("cancels",NULL,&text,NULL);dsize("EXIT",NULL,&cap,NULL);cap+=8;
    int x=UI_W-2-text-4-cap;
    ui_rect(x,204,cap,13,C_RGB(14,17,23));ui_rect(x+1,205,cap-2,11,C_RGB(8,11,17));
    ui_text(x+4,206,C_WHITE,"EXIT");ui_text(x+cap+4,206,UI_HEADER_MUTED,"cancels");
}
static void busy_draw_paint(const char *label,unsigned frame)
{
    unsigned capacity;uint16_t *saved=graph_busy_pixels(&capacity,true);
    int rows=(int)(capacity/DWIDTH);if(rows<1)return;
    struct dwindow old=dwindow;
    /* Exactly the full-width F-key bar. The LCD keeps the prior graph while
       VRAM owns unfinished construction; every source pixel is put back. */
    for(int y=UI_Y+198;y<DHEIGHT;y+=rows) {
        int height=DHEIGHT-y<rows ? DHEIGHT-y:rows;
        memcpy(saved,gint_vram+y*DWIDTH,(unsigned)(DWIDTH*height)*2);
        dwindow_set((struct dwindow){0,y,DWIDTH,y+height});
        ui_busy_bar(label,frame);
        r61524_display_rect(gint_vram,0,DWIDTH-1,y,y+height-1);
        memcpy(gint_vram+y*DWIDTH,saved,(unsigned)(DWIDTH*height)*2);
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
        paint_header();
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
        if(busy_screen(busy))busy_screen_paint(busy,text);
        /* Drawing, G-Solve and TRACE all show one full-width bottom bar. The
           borrowed tail after TRACE staging is never written by staging. */
        else busy_draw_paint(busy->label,busy->frame-1);
        busy->visible=true;busy->last=now;
    }
    return false;
}
void ui_busy_end(UiBusy *busy)
{
    /* VRAM was restored strip by strip. Table's preparation screen and the
       bottom bar are replaced by the owner's next full update (commit,
       rollback or cancel); never upload an in-progress Graph framebuffer here. */
    busy->visible=false;
}
