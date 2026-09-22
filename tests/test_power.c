#include "power.h"
#include <gint/gint.h>
#include <gint/drivers/keydev.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static uint32_t now;
static int apo=10,duration=1,light=4,hardware=4,world,interrupt,reads,sets;
static int offs,menus,updates,clears,sleeps,chains;
static bool held;
static keydev_async_filter_t filter;
static key_event_t queued;
static volatile int *wake_timeout;
static void (*on_sleep)(void),(*on_menu)(void),(*on_wake)(void);
static key_event_t key(int k) {return (key_event_t){.type=KEYEV_DOWN,.key=(unsigned)k};}
static bool prior(key_event_t e) {(void)e;chains++;return false;}
keydev_t *keydev_std(void) {return NULL;}
keydev_async_filter_t keydev_async_filter(keydev_t *d) {(void)d;return filter;}
void keydev_set_async_filter(keydev_t *d,keydev_async_filter_t f) {(void)d;filter=f;}
bool keydev_idle(keydev_t *d,int k) {(void)d;assert(!k);return !held;}
uint32_t rtc_ticks(void) {assert(!interrupt);return now;}
int gint_world_switch(gint_call_t call)
{
    assert(!world && !interrupt);world=1;
    int result=call.zero ? call.zero():call.one(call.arg);world=0;return result;
}
int diffeq_os_apo(void) {assert(world);reads++;return apo;}
char diffeq_os_duration(void) {assert(world);reads++;return (char)duration;}
char diffeq_os_light(void) {assert(world);reads++;return (char)light;}
void diffeq_os_set_light(char level) {assert(world && level>=1 && level<=5);hardware=level;sets++;}
void r61524_set(int id,uint16_t value)
{assert(!world && !interrupt && id==0x5a1 && value==0x14 && hardware==1);hardware=0;}
void gint_poweroff(bool logo)
{assert(!world && !interrupt && logo && hardware==light);offs++;if(on_wake)on_wake();}
void gint_osmenu(void)
{assert(!world && !interrupt && hardware==light);menus++;if(on_menu)on_menu();}
void dupdate(void) {assert(!world && !interrupt);updates++;}
void clearevents(void) {clears++;queued=(key_event_t){0};}
key_event_t getkey_opt(int options,volatile int *timeout)
{
    assert(timeout && *timeout);
    assert(!(options&(GETKEY_MENU|GETKEY_POWEROFF)));
    key_event_t e=queued;queued=(key_event_t){0};return e;
}
void sleep(void)
{
    assert(++sleeps<100 && !world && !interrupt);
    now=(now+1)%(86400u*128u);
    if(on_sleep)on_sleep();
    if(wake_timeout)*wake_timeout=1;
}
static void activity(void)
{
    int before=sets;interrupt=1;assert(!filter(key(KEY_SHIFT)));interrupt=0;
    assert(sets==before); /* No world/LCD/RTC work in asynchronous callback. */
}
static void begin(void)
{
    filter=prior;now=0;held=false;hardware=light;queued=(key_event_t){0};
    on_sleep=on_menu=on_wake=NULL;wake_timeout=NULL;sleeps=0;
    power_init();int before=reads;power_init();assert(reads==before);
}
static void end(void) {power_shutdown();assert(filter==prior);}
static void press(void) {queued=key(KEY_EXE);activity();}
static void change_settings(void) {apo=60;duration=6;light=2;hardware=light;now=77;}
static void stale_wake(void) {queued=key(KEY_ACON);now=123;}
static void held_wake(void) {held=true;queued=key(KEY_ACON);}
static void release_wake(void) {if(sleeps==4)held=false;}
int main(void)
{
    const int minutes[]={10,60},halves[]={1,2,6};
    for(unsigned a=0;a<2;a++)for(unsigned d=0;d<3;d++)for(light=1;light<=5;light++) {
        apo=minutes[a];duration=halves[d];begin();
        int before=offs;now=(uint32_t)duration*30*128-1;assert(!power_poll(true) && hardware==light);
        now++;assert(!power_poll(true) && hardware==0);
        now=(uint32_t)apo*60*128-1;assert(!power_poll(true) && offs==before);
        now++;assert(power_poll(false) && offs==before); /* Transaction still owns VRAM. */
        assert(power_poll(false) && offs==before);
        assert(power_poll(true) && offs==before+1 && hardware==light);
        assert(!power_poll(true) && offs==before+1);end();
    }
    apo=10;duration=1;light=4;begin();now=3840;power_poll(true);assert(hardware==0);
    activity();assert(hardware==0);power_poll(true);assert(hardware==4 && chains>0);
    now+=3839;power_poll(true);assert(hardware==4);now++;power_poll(true);assert(hardware==0);
    held=true;now+=600*128;int before=offs;power_poll(true);assert(hardware==4 && offs==before);
    held=false;now+=3840;power_poll(true);assert(hardware==0);end();assert(hardware==4);

    begin();now=86400u*128u-100;activity();power_poll(true);
    now=3740;power_poll(true);assert(hardware==0);end(); /* Midnight, exact30s. */
    begin();now=600u*128u;assert(power_poll(false));activity();
    assert(!power_poll(true) && offs==before);end(); /* New input wins automatic deadline. */

    begin();key_event_t off=key(KEY_ACON);off.shift=1;
    assert(power_key(off) && power_poll(false));activity();assert(power_poll(false));
    on_wake=stale_wake;assert(power_poll(true) && clears>0);
    assert(queued.type==KEYEV_NONE && !power_poll(false));end();
    begin();on_wake=held_wake;on_sleep=release_wake;
    assert(power_key(off));assert(power_poll(true) && sleeps==4 && !held);
    assert(queued.type==KEYEV_NONE);end();
    begin();off.type=KEYEV_HOLD;assert(!power_key(off));off.type=KEYEV_DOWN;off.alpha=1;
    assert(!power_key(off));off.alpha=0;off.shift=0;assert(!power_key(off));end();

    begin();now=3840;power_poll(true);on_menu=change_settings;
    power_osmenu();assert(menus && hardware==2 && reads>=3);
    now=77+3840;power_poll(true);assert(hardware==2);
    now=77+180*128;power_poll(true);assert(hardware==0);
    now=77+600*128;assert(!power_poll(false));end();assert(hardware==2);

    apo=0;duration=255;light=0;begin();before=sets;now=7200*128;
    assert(!power_poll(false) && sets==before);end();
    apo=10;duration=1;light=4;begin();on_sleep=press;
    assert(power_wait_key(NULL).key==KEY_EXE && sleeps==1);end();
    begin();volatile int timeout=0;wake_timeout=&timeout;
    assert(power_wait_key(&timeout).type==KEYEV_NONE && sleeps==1);end();
    begin();queued=key(KEY_MENU);on_sleep=press;before=menus;
    assert(power_wait_key(NULL).key==KEY_EXE && menus==before+1);end();
    begin();queued=key(KEY_ACON);queued.shift=1;on_sleep=press;before=offs;
    assert(power_wait_key(NULL).key==KEY_EXE && offs==before+1);end();
    printf("Power: 30 setting/brightness combinations; idle/deferred off, input priority, midnight, MENU refresh, wake queue, ISR safety and wait/blink PASS.\n");
}
