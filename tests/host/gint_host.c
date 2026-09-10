#include <stdbool.h>
#include <time.h>
#include <gint/rtc.h>
#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "font_data.h"
static uint16_t pixels[DWIDTH*DHEIGHT];
uint16_t *gint_vram=pixels;
static unsigned frame,clears;
static unsigned text_glyphs;
unsigned host_text_glyphs(void){return text_glyphs;}
unsigned host_clear_count(void){return clears;}
static char title[128];
const font_t *dfont_default(void)
{static const font_t font={9,11};return &font;}
void host_metrics_print(void);
void dsetvram(uint16_t *main,uint16_t *secondary) {(void)main;(void)secondary;}
void dpixel(int x,int y,int color)
{if(x>=0 && x<DWIDTH && y>=0 && y<DHEIGHT && color>=0) pixels[y*DWIDTH+x]=(uint16_t)color;}
void dclear(color_t color) {clears++;for(int i=0;i<DWIDTH*DHEIGHT;i++) pixels[i]=color;}
void drect(int x1,int y1,int x2,int y2,int color)
{
    if(x1<0)x1=0;if(y1<0)y1=0;if(x2>=DWIDTH)x2=DWIDTH-1;if(y2>=DHEIGHT)y2=DHEIGHT-1;
    for(int y=y1;y<=y2;y++)for(int x=x1;x<=x2;x++)dpixel(x,y,color);
}
void dline(int x1,int y1,int x2,int y2,int color)
{
    int dx=abs(x2-x1),sx=x1<x2?1:-1,dy=-abs(y2-y1),sy=y1<y2?1:-1,error=dx+dy;
    for(int guard=0;guard<10000;guard++) {
        dpixel(x1,y1,color);if(x1==x2 && y1==y2)break;
        int e=2*error;if(e>=dy){error+=dy;x1+=sx;}if(e<=dx){error+=dx;y1+=sy;}
    }
}
void dnsize(const char *text,int size,const font_t *font,int *w,int *h)
{
    (void)font;int width=0;
    for(int i=0;text[i] && (size<0 || i<size);i++) {
        unsigned ch=(unsigned char)text[i];if(ch<32 || ch>126)continue;
        width+=font_width[ch-32]+1;
    }
    /* gint subtracts trailing char_spacing even for an empty prefix. */
    if(w)*w=width-1;if(h)*h=9;
}
void dsize(const char *text,const font_t *font,int *w,int *h) {dnsize(text,-1,font,w,h);}
void dtext(int x,int y,int color,const char *text)
{dtext_opt(x,y,color,C_NONE,DTEXT_LEFT,DTEXT_TOP,text,-1);}
void dtext_opt(int x,int y,int color,int background,int halign,int valign,const char *text,int size)
{
    int width,height;dnsize(text,size,NULL,&width,&height);
    if(halign==DTEXT_RIGHT)x-=width-1;if(halign==DTEXT_CENTER)x-=width/2;
    if(valign==DTEXT_BOTTOM)y-=height-1;if(valign==DTEXT_MIDDLE)y-=height/2;
    printf("TEXT %d %d %.*s\n",x,y,size<0 ? (int)strlen(text):size,text);
    if(x==14 && y==9)snprintf(title,sizeof(title),"%.*s",size<0 ? (int)strlen(text):size,text);
    for(int i=0;text[i] && (size<0 || i<size);i++) {
        unsigned ch=(unsigned char)text[i];if(ch<32 || ch>126)continue;
        int index=(int)ch-32;text_glyphs++;
        for(int yy=0;yy<11;yy++)for(int xx=0;xx<font_width[index];xx++)
            if(font_rows[index][yy]&(1u<<xx))dpixel(x+xx,y+yy,color);
            else if(background!=C_NONE)dpixel(x+xx,y+yy,background);
        x+=font_width[index]+1;
    }
}
void dupdate(void)
{
    host_metrics_print();
    /* Hash only the plot, excluding the softkey strip. */
    uint32_t hash=2166136261u;
    for(int y=4;y<202;y++)for(int x=6;x<390;x++) {
        hash^=pixels[y*DWIDTH+x];hash*=16777619u;
    }
    printf("PLOT %08x\n",hash);
    const char *out=getenv("DIFFEQ_HOST_OUT");
    if(out) {
        char path[512];snprintf(path,sizeof(path),"%s/%03u.ppm",out,frame);
        FILE *f=fopen(path,"wb");if(!f){perror(path);exit(2);}
        fprintf(f,"P6\n%d %d\n255\n",DWIDTH,DHEIGHT);
        for(int i=0;i<DWIDTH*DHEIGHT;i++) {
            unsigned c=pixels[i];unsigned char rgb[]={ (unsigned char)(((c>>11)&31)*255/31),
                (unsigned char)(((c>>5)&63)*255/63),(unsigned char)((c&31)*255/31)};
            fwrite(rgb,1,3,f);
        }
        fclose(f);
    }
    printf("FRAME %u %s\n",frame++,title);
    const char *configured=getenv("DIFFEQ_HOST_MAX_FRAMES");
    unsigned limit=configured ? (unsigned)strtoul(configured,NULL,10):500;
    if(!limit)limit=500;
    if(frame>limit) {fputs("Excess UI frames\n",stderr);exit(2);}
}
#ifndef DIFFEQ_TEST_NATIVE_KEYS
static unsigned poll_remaining;
void host_cancel_after(unsigned polls){poll_remaining=polls;}
key_event_t pollevent(void)
{
    if(poll_remaining && --poll_remaining==0)return (key_event_t){.key=KEY_EXIT,.type=KEYEV_DOWN};
    return (key_event_t){.type=KEYEV_NONE};
}
key_event_t getkey(void)
{
    static const char *cursor;
    if(!cursor){cursor=getenv("DIFFEQ_HOST_KEYS");if(!cursor)cursor="";}
    while(*cursor==' ' || *cursor==',' || *cursor=='\n')cursor++;
    if(!*cursor){puts("SCRIPT COMPLETE");exit(0);}
    char token[32];int n=0;
    while(*cursor && *cursor!=' ' && *cursor!=',' && *cursor!='\n' && n<31)token[n++]=*cursor++;
    token[n]=0;char *name=token;unsigned shift=0,alpha=0,repeat=0;
    if(!strncmp(name,"TICKS:",6)) {host_tick_step((unsigned)strtoul(name+6,NULL,10));return getkey();}
    if(!strncmp(name,"CANCEL:",7)) {host_cancel_after((unsigned)strtoul(name+7,NULL,10));return getkey();}
    if(!strcmp(name,"BLINK")) {
        puts("KEY BLINK");
        return (key_event_t){.type=KEYEV_NONE};
    }
    if(!strncmp(name,"R:",2)){repeat=1;name+=2;}
    if(!strncmp(name,"S:",2)){shift=1;name+=2;}if(!strncmp(name,"A:",2)){alpha=1;name+=2;}
    struct Key {const char *name;int code;};
    static const struct Key keys[]={
#define K(name) {#name,KEY_##name}
        K(F1),K(F2),K(F3),K(F4),K(F5),K(F6),K(LEFT),K(RIGHT),K(UP),K(DOWN),K(EXIT),K(EXE),K(OPTN),
        K(XOT),K(LOG),K(LN),K(SIN),K(COS),K(TAN),K(POWER),K(SQUARE),K(DEL),K(ACON),K(COMMA),
        K(LEFTP),K(RIGHTP),K(NEG),K(SUB),K(ADD),K(MUL),K(DIV),K(EXP),K(DOT),
        K(0),K(1),K(2),K(3),K(4),K(5),K(6),K(7),K(8),K(9)
#undef K
    };
    for(unsigned i=0;i<sizeof(keys)/sizeof(keys[0]);i++)if(!strcmp(keys[i].name,name)) {
        printf("KEY %s\n",token);return (key_event_t){.key=(unsigned)keys[i].code,.type=repeat ? KEYEV_HOLD:KEYEV_DOWN,.mod=1,.shift=shift,.alpha=alpha};
    }
    fprintf(stderr,"Unknown scripted key: %s\n",token);exit(2);
}
#endif

static unsigned fake_ticks,tick_step;
void host_tick_step(unsigned ticks){fake_ticks=0;tick_step=ticks;}
uint32_t rtc_ticks(void)
{
    const char *step=getenv("DIFFEQ_HOST_TICK_STEP");
    if(step || tick_step){
        fake_ticks+=step ? (unsigned)strtoul(step,NULL,10):tick_step;
        const char *limit=getenv("DIFFEQ_HOST_TICK_LIMIT");
        if(limit && fake_ticks>(unsigned)strtoul(limit,NULL,10))fake_ticks=(unsigned)strtoul(limit,NULL,10);
        return fake_ticks;
    }
    struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);
    return (uint32_t)((t.tv_sec%86400)*128+(t.tv_nsec*128)/1000000000);
}

/* Key/editor-only targets do not link graph scratch; production UI targets use
   the strong implementation backed by their existing TRACE union. */
__attribute__((weak)) uint16_t *graph_busy_pixels(unsigned *capacity,bool staging)
{(void)staging;static uint16_t pixels[1420];*capacity=1420;return pixels;}
