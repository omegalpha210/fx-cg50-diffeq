#include "ui.h"
#include "power.h"
#include <gint/timer.h>
#include <gint/rtc.h>
#include <gint/drivers/keydev.h>
#include <assert.h>
#include <stdio.h>
static key_event_t events[16];
static key_event_t direct[4];
static int direct_next,direct_length;
static int length,next,menu_count,timers,starts,stops,held=1;
static volatile int *timer_flag;
static keydev_transform_t transform;
static bool power_requested;
static bool idle_wait;
static int power_resumed;
keydev_t *keydev_std(void) {return NULL;}
keydev_transform_t keydev_transform(keydev_t *d) {(void)d;return transform;}
void keydev_set_transform(keydev_t *d,keydev_transform_t t) {(void)d;transform=t;}
int keydown(int key) {(void)key;return held;}
static key_event_t event(int key) {return (key_event_t){.type=KEYEV_DOWN,.key=(unsigned)key,.mod=1};}
key_event_t getkey(void) {return direct_next<direct_length ? direct[direct_next++]:event(KEY_5);}
key_event_t getkey_opt(int options,volatile int *timeout)
{
    assert(timeout);
    if(next<length) {
        /* Compute poll must NOT let gint consume MENU before safe return. */
        if(!idle_wait)assert(!(options&(GETKEY_MENU|GETKEY_POWEROFF)));
        if(events[next].key==KEY_MENU)assert(!(options&GETKEY_MENU));
        return events[next++];
    }
    *timeout=1;return (key_event_t){.type=KEYEV_NONE};
}
void gint_osmenu(void) {menu_count++;}
/* This original fixture isolates common.c; power's real native lifecycle is
   exercised separately with OS-world/RTC/keyboard adapters. */
bool power_poll(bool idle)
{if(!power_requested)return false;if(idle){power_requested=false;power_resumed++;}return true;}
bool power_key(key_event_t e)
{if(e.key!=KEY_ACON || !e.shift)return false;power_requested=true;return true;}
key_event_t power_wait_key(volatile int *timeout)
{idle_wait=true;key_event_t e=timeout ? getkey_opt(GETKEY_DEFAULT,timeout):getkey();idle_wait=false;return e;}
void power_osmenu(void) {gint_osmenu();dupdate();}
int timer_configure(int timer,uint64_t delay,gint_call_t callback)
{
    assert(timer==TIMER_ANY && delay==250000 && timers==0);
    timers++;timer_flag=callback.flag;return 3;
}
void timer_start(int timer) {assert(timer==3);starts++;}
void timer_stop(int timer) {assert(timer==3 && timers==1);timers--;stops++;}
static unsigned partial_uploads;
static int screen_upload;
void r61524_display_rect(uint16_t *vram,int xmin,int xmax,int ymin,int ymax)
{
    assert(vram==gint_vram);
    if(screen_upload==2)assert(xmin==UI_X && xmax==UI_X+383 && ymin>=UI_Y+198 && ymax<UI_Y+216 && ymax-ymin<3);
    else if(screen_upload==1)assert(xmin==0 && xmax==DWIDTH-1 && ymin>=0 && ymax<DHEIGHT && ymax-ymin<3);
    else assert(xmin==11 && xmax-xmin<120 && ymin==188 && ymax-ymin==11);
    partial_uploads++;
}
int main(void)
{
    events[0]=event(KEY_ADD);events[0].alpha=1;
    events[1]=event(KEY_ADD);events[2]=event(KEY_EXIT);length=3;
    assert(ui_cancel(NULL));
    key_event_t first=ui_getkey(),second=ui_getkey();
    assert(first.key==KEY_ADD && first.alpha && second.key==KEY_ADD && !second.alpha);
    length=1;next=0;events[0]=event(KEY_MENU);
    assert(ui_cancel(NULL) && menu_count==0);
    assert(ui_getkey().key==KEY_5 && menu_count==1);
    assert(ui_getkey().key==KEY_5 && menu_count==1);
    length=2;next=0;events[0]=event(KEY_RIGHT);events[1]=event(KEY_ACON);events[1].shift=1;
    assert(ui_cancel(NULL) && power_requested && power_resumed==0);
    assert(ui_getkey().key==KEY_5 && !power_requested && power_resumed==1);
    /* Resume removes our pre-off queued RIGHT only after calculation returned. */
    direct[0]=event(KEY_EXIT);direct[0].type=KEYEV_HOLD;direct[1]=direct[0];
    direct[2]=event(KEY_ADD);direct[2].alpha=1;direct[3]=event(KEY_EXE);direct_length=4;
    first=ui_getkey();assert(first.key==KEY_ADD && first.alpha && direct_next==3);
    assert(ui_getkey().key==KEY_EXE && direct_next==4); /* no broad flush */
    /* Saturation cancels, retains order, and leaves unread native events. */
    length=9;next=0;for(int i=0;i<9;i++)events[i]=event(KEY_RIGHT);
    assert(ui_cancel(NULL) && next==8);
    for(int i=0;i<8;i++)assert(ui_getkey().key==KEY_RIGHT);
    assert(!ui_cancel(NULL));assert(ui_getkey().key==KEY_RIGHT);
    for(int i=0;i<100;i++) {
        UiBlink blink;ui_blink_start(&blink);
        assert(timers==1 && blink.highlighted);
        *timer_flag=1;
        assert(ui_blink_key(&blink).type==KEYEV_NONE && !blink.highlighted);
        ui_blink_stop(&blink);assert(timers==0 && blink.timer==-1);
    }
    assert(starts==100 && stops==100);
    /* Deferred drawing does not consume a fresh EXIT as trajectory cancel. */
    length=3;next=0;events[0]=event(KEY_EXIT);events[0].type=KEYEV_HOLD;
    events[1]=event(KEY_EXIT);events[2]=event(KEY_RIGHT);ui_defer_input();
    assert(ui_getkey().key==KEY_EXIT && ui_getkey().key==KEY_RIGHT);
    UiBlink selection;ui_blink_start(&selection);length=2;next=0;
    events[0]=event(KEY_EXIT);events[0].type=KEYEV_HOLD;events[1]=event(KEY_EXIT);
    assert(ui_blink_key(&selection).type==KEYEV_DOWN && next==2);ui_blink_stop(&selection);
    ui_trace_input(true);assert(transform.repeater(0,0,0)==400000);
    assert(transform.repeater(0,2000000,50)==125000);
    length=16;next=0;
    for(int i=0;i<15;i++){events[i]=event(KEY_LEFT);events[i].type=KEYEV_HOLD;}
    events[15]=event(KEY_EXIT);
    assert(ui_trace_cancel(NULL));
    UiBlink trace_blink={0};assert(ui_trace_key(&trace_blink).key==KEY_EXIT && next==16);
    next=0;length=16;events[15]=events[14];
    assert(!ui_trace_cancel(NULL));assert(ui_trace_key(&trace_blink).key==KEY_LEFT);
    next=0;length=16;events[15]=event(KEY_MENU);
    assert(ui_trace_cancel(NULL));assert(menu_count==1);
    assert(ui_trace_key(&trace_blink).key==KEY_MENU && menu_count==2);
    next=0;length=15;held=0;
    assert(!ui_trace_cancel(NULL));
    assert(ui_trace_key(&trace_blink).type==KEYEV_NONE); /* released HOLD is stale */
    next=0;length=1;events[0]=event(KEY_F6);held=1;
    assert(!ui_trace_cancel(NULL) && ui_trace_key(&trace_blink).key==KEY_F6);
    next=0;length=2;events[0]=event(KEY_F6);events[1]=event(KEY_EXIT);
    assert(ui_trace_cancel(NULL) && ui_trace_key(&trace_blink).key==KEY_EXIT);
    ui_trace_input(false);assert(transform.repeater==NULL);
    ui_trace_input(true);length=2;next=0;events[0]=event(KEY_LEFT);
    events[1]=event(KEY_ACON);events[1].shift=1;
    assert(ui_trace_cancel(NULL) && power_requested && power_resumed==1);
    length=next=0;trace_blink.timer=-1;
    assert(ui_trace_key(&trace_blink).key==KEY_5 && power_resumed==2);
    ui_trace_input(false);
    length=next=0;host_tick_step(8);UiBusy busy;ui_busy_start(&busy);
    for(int i=0;i<3;i++)assert(!ui_busy_cancel(&busy));
    assert(busy.visible && partial_uploads==1);ui_busy_end(&busy);assert(partial_uploads==2);host_tick_step(0);
    for(int area=UI_BUSY_TABLE;area<=UI_BUSY_DRAW;area++) {
        screen_upload=area==UI_BUSY_DRAW ? 2:1;partial_uploads=0;host_tick_step(8);
        ui_busy_begin(&busy,area==UI_BUSY_TABLE ? "Preparing Table...":"Drawing...",area,ui_cancel,NULL);
        for(int i=0;i<3;i++)assert(!ui_busy_cancel(&busy));
        assert(busy.visible && partial_uploads==(area==UI_BUSY_DRAW ? 6:75)); /* Once-only 224-row canvas. */
        for(int i=0;i<2;i++)assert(!ui_busy_cancel(&busy));
        assert(partial_uploads==(area==UI_BUSY_DRAW ? 12:82)); /* Subsequent frame:21 header rows only. */
        ui_busy_end(&busy);assert(partial_uploads==(area==UI_BUSY_DRAW ? 12:82));host_tick_step(0);
    }
    puts("Target-branch key policy: retained events, one MENU, bounded saturation, 100 timer lifetimes passed.");
    return 0;
}
