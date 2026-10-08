#include "power.h"
#include "usb_native.h"
MockUsbCpg mock_usb_cpg;
MockUsbPower mock_usb_power;
MockUsbRegisters mock_usb_registers={.SYSCFG={1}};
static bool usb_close_ok=true;
static unsigned usb_cleanups;
static void (*on_close)(void);
bool storage_usb_ready(void){usb_cleanups++;if(on_close)on_close();return usb_close_ok;}
#include <gint/gint.h>
#include <gint/drivers/keydev.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint32_t now;
static int apo=10,duration=1,light=4,hardware=4,world,interrupt,reads,sets;
static int offs,menus,updates,clears,sleeps,chains;
static bool held;
static keydev_t device;
static int auto_release=-1;
static keydev_async_filter_t filter;
static key_event_t queued;
static volatile int *wake_timeout;
static void (*on_sleep)(void),(*on_menu)(void),(*on_wake)(void);
static key_event_t key(int k) {return (key_event_t){.type=KEYEV_DOWN,.key=(unsigned)k};}
static bool prior(key_event_t e) {(void)e;chains++;return false;}
keydev_t *keydev_std(void) {return &device;}
keydev_async_filter_t keydev_async_filter(keydev_t *d) {(void)d;return filter;}
void keydev_set_async_filter(keydev_t *d,keydev_async_filter_t f) {(void)d;filter=f;}
bool keydev_idle(keydev_t *d,int k)
{assert(d==&device && !k);if(held)return false;for(unsigned r=0;r<12;r++)if(d->state_queue[r])return false;return true;}
int keydown(int k)
{return (device.state_queue[(unsigned)k>>4]&(0x80u>>((unsigned)k&7u)))!=0;}
static void scan_key(int k,bool down)
{
    unsigned row=(unsigned)k>>4,mask=0x80u>>((unsigned)k&7u);
    assert(row<12);
    bool was=(device.state_now[row]&mask)!=0;if(was==down)return;
    unsigned end=(unsigned)device.queue_end,next=(end+1u)%KEYBOARD_QUEUE_SIZE;
    assert(next!=(unsigned)device.queue_next);
    device.queue[end]=(key_event_t){.type=down?KEYEV_DOWN:KEYEV_UP,.key=(unsigned)k};
    device.queue_end=(int8_t)next;
    if(down)device.state_now[row]|=(uint8_t)mask;
    else device.state_now[row]&=(uint8_t)~mask;
}
static key_event_t consume(void)
{
    if(device.queue_next==device.queue_end)return (key_event_t){0};
    key_event_t event=device.queue[(unsigned)device.queue_next];
    device.queue_next=(int8_t)(((unsigned)device.queue_next+1u)%KEYBOARD_QUEUE_SIZE);
    unsigned row=event.key>>4,mask=0x80u>>(event.key&7u);
    if(event.type==KEYEV_DOWN)device.state_queue[row]|=(uint8_t)mask;
    if(event.type==KEYEV_UP)device.state_queue[row]&=(uint8_t)~mask;
    return event;
}
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
{
    assert(!world && !interrupt && hardware==light);
    assert(device.queue_next==device.queue_end);
    for(unsigned row=0;row<12;row++)assert(!device.state_now[row] && !device.state_queue[row]);
    menus++;if(on_menu)on_menu();
}
void dupdate(void) {assert(!world && !interrupt);updates++;}
void clearevents(void) {clears++;queued=(key_event_t){0};while(consume().type!=KEYEV_NONE);}
key_event_t getkey_opt(int options,volatile int *timeout)
{
    assert(timeout && *timeout);
    assert(!(options&(GETKEY_MENU|GETKEY_POWEROFF)));
    if(queued.type!=KEYEV_NONE) {
        key_event_t e=queued;queued=(key_event_t){0};
        scan_key((int)e.key,true);auto_release=(int)e.key;
        key_event_t native=consume();assert(native.type==KEYEV_DOWN);native.shift=e.shift;native.alpha=e.alpha;
        return native;
    }
    key_event_t event;
    do {event=consume();}
    while(event.type==KEYEV_UP || (event.type==KEYEV_HOLD && event.key!=KEY_LEFT &&
        event.key!=KEY_RIGHT && event.key!=KEY_UP && event.key!=KEY_DOWN));
    return event;
}
void sleep(void)
{
    assert(++sleeps<100 && !world && !interrupt);
    now=(now+1)%(86400u*128u);
    device.time++;
    if(auto_release>=0){scan_key(auto_release,false);auto_release=-1;}
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
    mock_usb_cpg.USBCLKCR.CLKSTP=0;mock_usb_power.MSTPCR2.USB0=0;
    mock_usb_registers.SYSCFG.SCKE=1;mock_usb_registers.INTSTS0.VBSTS=0;
    usb_close_ok=true;
    filter=prior;now=0;held=false;hardware=light;queued=(key_event_t){0};
    on_sleep=on_menu=on_wake=on_close=NULL;wake_timeout=NULL;sleeps=0;
    memset(&device,0,sizeof device);auto_release=-1;
    power_init();int before=reads;power_init();assert(reads==before);
}
static void end(void) {power_shutdown();assert(filter==prior);}
static void press(void) {queued=key(KEY_EXE);activity();}
static void change_settings(void) {apo=60;duration=6;light=2;hardware=light;now=77;}
static void stale_wake(void) {queued=key(KEY_ACON);now=123;}
static void held_wake(void) {held=true;queued=key(KEY_ACON);}
static void release_wake(void) {if(sleeps==4)held=false;}
static void insert_usb(void){mock_usb_registers.INTSTS0.VBSTS=1;}
static bool quiet_poll(void)
{
    /* The real foreground reader consumes UP, even when getkey filters it. */
    volatile int timeout=1;
    key_event_t event=getkey_opt(GETKEY_DEFAULT & ~(GETKEY_MENU|GETKEY_POWEROFF),&timeout);
    if(event.type==KEYEV_DOWN && event.key==KEY_MENU)power_osmenu();
    else (void)power_key(event);
    device.time++;
    if(auto_release>=0){scan_key(auto_release,false);auto_release=-1;}
    return power_poll(true);
}
static bool finish_menu(void)
{
    for(unsigned i=0;i<8;i++)if(quiet_poll())return true;
    return false;
}
static void consumed_menu_down(void)
{
    scan_key(KEY_MENU,true);volatile int timeout=1;
    assert(getkey_opt(GETKEY_DEFAULT & ~(GETKEY_MENU|GETKEY_POWEROFF),&timeout).key==KEY_MENU);
    assert(keydown(KEY_MENU));power_osmenu();
}
static void consumed_menu_up(void)
{
    scan_key(KEY_MENU,false);assert(keydown(KEY_MENU));
    volatile int timeout=1;
    assert(getkey_opt(GETKEY_DEFAULT & ~(GETKEY_MENU|GETKEY_POWEROFF),&timeout).type==KEYEV_NONE);
    assert(!keydown(KEY_MENU));
}
static void menu_during_close(void){scan_key(KEY_MENU,true);}
static void menu_boundary_cases(void)
{
    apo=10;duration=1;light=4;
    begin();int m=menus,o=offs;unsigned closes_before=usb_cleanups;
    consumed_menu_down(); /* Baseline wrongly calls the OS with this key held. */
    assert(menus==m && !power_poll(true) && power_poll(false));
    unsigned sleeps_before=(unsigned)sleeps;
    for(unsigned i=0;i<20;i++) {device.time++;assert(!power_poll(true) && menus==m);}
    assert((unsigned)sleeps==sleeps_before); /* Every gate check returns; no wait loop. */
    scan_key(KEY_MENU,false);
    assert(!device.state_now[KEY_MENU>>4] && keydown(KEY_MENU));
    assert(!power_poll(true) && menus==m); /* Physical release alone is insufficient. */
    volatile int timeout=1;
    assert(getkey_opt(GETKEY_DEFAULT & ~(GETKEY_MENU|GETKEY_POWEROFF),&timeout).type==KEYEV_NONE);
    assert(!keydown(KEY_MENU) && !power_poll(true)); /* First quiet snapshot. */
    device.time++;assert(!power_poll(true)); /* Close workers, then recheck. */
    assert(usb_cleanups==closes_before+1 && menus==m && !power_poll(true));
    device.time++;assert(power_poll(true) && menus==m+1 && offs==o);
    for(unsigned i=0;i<20;i++)assert(!power_poll(true) && menus==m+1);
    /* A fresh MENU after return owns a new press and produces one new call. */
    consumed_menu_down();assert(!power_poll(true) && menus==m+1);
    consumed_menu_up();assert(finish_menu() && menus==m+2);end();

    begin();m=menus;o=offs;consumed_menu_down();
    key_event_t off=key(KEY_ACON);off.shift=1;assert(power_key(off));
    consumed_menu_up();assert(finish_menu() && menus==m+1 && offs==o);
    assert(power_poll(true) && offs==o+1); /* Fresh OFF survives MENU refresh. */
    assert(!power_poll(true) && offs==o+1);end();

    begin();m=menus;power_osmenu();scan_key(KEY_EXE,true);
    for(unsigned i=0;i<20;i++){device.time++;assert(!power_poll(true) && menus==m);}
    /* Queue drain must not pretend EXE is physically released. */
    timeout=1;assert(getkey_opt(GETKEY_DEFAULT & ~(GETKEY_MENU|GETKEY_POWEROFF),&timeout).key==KEY_EXE);
    assert(!power_poll(true));scan_key(KEY_EXE,false);
    assert(finish_menu() && menus==m+1);end();

    begin();m=menus;power_osmenu();on_close=menu_during_close;
    assert(!quiet_poll() && !quiet_poll() && menus==m);
    assert(keydown(KEY_MENU)==0 && device.state_now[KEY_MENU>>4]);
    assert(!power_poll(true));timeout=1;
    assert(getkey_opt(GETKEY_DEFAULT & ~(GETKEY_MENU|GETKEY_POWEROFF),&timeout).key==KEY_MENU);
    consumed_menu_up();assert(finish_menu() && menus==m+1);end();

    begin();m=menus;consumed_menu_down();now=2u*128u;
    assert(!power_poll(true) && menus==m);consumed_menu_up();
    for(unsigned i=0;i<20;i++){device.time++;assert(!power_poll(true));}
    assert(menus==m); /* Timeout cancels; release later cannot silently force MENU. */
    consumed_menu_down();consumed_menu_up();assert(finish_menu() && menus==m+1);end();

    begin();m=menus;consumed_menu_down();
    for(unsigned i=0;i<600;i++)assert(!power_poll(true) && menus==m);
    /* Frozen RTC and scanner still hit the bounded cancellation watchdog. */
    consumed_menu_up();device.time++;assert(!power_poll(false) && !power_poll(true));end();
    begin();m=menus;power_osmenu();device.queue_end=KEYBOARD_QUEUE_SIZE;
    assert(!power_poll(true) && menus==m);device.queue_end=0;
    for(unsigned i=0;i<10;i++){device.time++;assert(!power_poll(true));}end();

    begin();m=menus;consumed_menu_down();
    assert(!power_key(key(KEY_EXIT)));consumed_menu_up();
    assert(!power_poll(false) && !power_poll(true) && menus==m);end();
    begin();m=menus;power_osmenu();assert(finish_menu() && menus==m+1);
    insert_usb();assert(!power_poll(true) && finish_menu() && menus==m+2);
    for(unsigned i=0;i<100;i++)assert(!power_poll(true) && menus==m+2);end();
    puts("MENU ownership: consumed UP, scanned/queue all-key release, fresh scan after close, held MENU/EXE, frozen/invalid cancellation, fresh MENU/OFF/USB; no busy wait or forced helper PASS. OS success UNVERIFIED.");
}
static void usb_cases(void)
{
    apo=10;duration=1;light=4;
    /* Busy owners defer MENU until their existing rollback has completed. */
    begin();int m=menus,o=offs;now=3840;assert(!power_poll(true));
    assert(hardware==0);insert_usb();queued=key(KEY_MENU);
    for(unsigned i=0;i<20;i++)assert(power_poll(false) && menus==m && hardware==0);
    assert(!power_poll(true) && finish_menu() && menus==m+1 && offs==o && hardware==light);
    assert(queued.type==KEYEV_NONE);
    for(unsigned i=0;i<100;i++)assert(!power_poll(true) && menus==m+1);
    mock_usb_cpg.USBCLKCR.CLKSTP=1;mock_usb_registers.INTSTS0.VBSTS=0;
    assert(!power_poll(true));mock_usb_cpg.USBCLKCR.CLKSTP=0;insert_usb();
    assert(!power_poll(true)); /* Unknown did not invent an unplug. */
    mock_usb_registers.INTSTS0.VBSTS=0;assert(!power_poll(true));insert_usb();
    assert(!power_poll(true) && finish_menu() && menus==m+2);end();
    begin();m=menus;o=offs;key_event_t off=key(KEY_ACON);off.shift=1;
    assert(power_key(off));insert_usb();assert(!power_poll(true) && finish_menu());
    assert(menus==m+1 && offs==o && !power_poll(true));end();
    /* Insert during a manual transition: absorb it, including reentry. */
    begin();m=menus;on_menu=insert_usb;power_osmenu();assert(finish_menu());
    assert(menus==m+1 && !power_poll(true));end();
    /* An unresolved descriptor blocks both transitions; no hot retry at APO. */
    begin();m=menus;o=offs;now=600u*128u;usb_close_ok=false;
    unsigned c=usb_cleanups;insert_usb();assert(!power_poll(true) && finish_menu());
    for(unsigned i=0;i<100;i++)assert(!power_poll(true));
    assert(menus==m && offs==o && usb_cleanups==c+1);
    usb_close_ok=true;power_osmenu();assert(finish_menu() && menus==m+1);end();
    /* Modal/idle wait uses the same real loop; no synthetic keyboard activity. */
    begin();m=menus;insert_usb();on_menu=press;
    assert(power_wait_key(NULL).key==KEY_EXE && menus==m+1);end();
    puts("USB native: idle/modal wait, deferred busy rollback, dim, MENU/OFF races, rearm, unknown and bounded close failure PASS; manual SAVE unchanged.");
}
int main(void)
{
    menu_boundary_cases();
    usb_cases();
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
    power_osmenu();assert(finish_menu() && menus && hardware==2 && reads>=3);
    now=77+3840;power_poll(true);assert(hardware==2);
    now=77+180*128;power_poll(true);assert(hardware==0);
    now=77+600*128;assert(!power_poll(false));end();assert(hardware==2);

    apo=0;duration=255;light=0;begin();before=sets;now=7200*128;
    assert(!power_poll(false) && sets==before);end();
    apo=10;duration=1;light=4;begin();on_sleep=press;
    assert(power_wait_key(NULL).key==KEY_EXE && sleeps==1);end();
    begin();volatile int timeout=0;wake_timeout=&timeout;
    assert(power_wait_key(&timeout).type==KEYEV_NONE && sleeps==1);end();
    begin();queued=key(KEY_MENU);on_menu=press;before=menus;
    assert(power_wait_key(NULL).key==KEY_EXE && menus==before+1);end();
    begin();queued=key(KEY_ACON);queued.shift=1;on_sleep=press;before=offs;
    assert(power_wait_key(NULL).key==KEY_EXE && offs==before+1);end();
    printf("Power: 30 setting/brightness combinations; idle/deferred off, input priority, midnight, MENU refresh, wake queue, ISR safety and wait/blink PASS.\n");
}
