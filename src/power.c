#include "power.h"
#include "storage.h"
#include "usb_lifecycle.h"
#include "usb_native.h"
#include <gint/gint.h>
#include <gint/cpu.h>
#include <gint/rtc.h>
#include <gint/display.h>
#include <gint/drivers/keydev.h>
#include <gint/drivers/r61524.h>
#include <stdint.h>

/* Verified OS ABI: libfxcg system.h / syscall stubs and TeamFX's original
   backlight research. Independent dispatch stubs; no new library dependency.
   See docs/audits/POWER_SAFETY_AUDIT.md for sources and native limitations. */
extern int diffeq_os_apo(void);
extern char diffeq_os_duration(void);
extern char diffeq_os_light(void);
extern void diffeq_os_set_light(char level);
#ifndef DIFFEQ_TEST_POWER
#define OS_STUB(name,id) __asm__(".text\n.balign 4\n.global _" #name "\n_" #name ":\n" \
    "mov.l 1f,r0\nmov.l 2f,r1\njmp @r1\nnop\n.balign 4\n1: .long " #id "\n2: .long 0x80020070\n")
OS_STUB(diffeq_os_apo,0x1e91);
OS_STUB(diffeq_os_duration,0x12d9);
OS_STUB(diffeq_os_light,0x1e8f);
OS_STUB(diffeq_os_set_light,0x0199);
#endif

#define DAY_TICKS (86400u*128u)
static struct {
    uint32_t last,apo,dim,seen;
    int light;
    bool active,dimmed,manual,pending,io_blocked;
} power;
static volatile uint32_t activity;
static UsbLifecycle usb;
static keydev_async_filter_t prior_filter;
static uint32_t since(uint32_t now,uint32_t before)
{return now>=before ? now-before:now+DAY_TICKS-before;}

static bool observe_key(key_event_t event)
{
    /* Interrupt context: no OS calls, drawing, allocation or RTC access. */
    if(event.type==KEYEV_DOWN || event.type==KEYEV_UP)activity++;
    return prior_filter ? prior_filter(event):true;
}
static int read_settings(void)
{
    int minutes=diffeq_os_apo();unsigned half=(unsigned char)diffeq_os_duration();
    power.apo=(minutes==10 || minutes==60) ? (uint32_t)minutes*60u*128u:0;
    power.dim=(half==1 || half==2 || half==6) ? half*30u*128u:0;
    int light=(unsigned char)diffeq_os_light();
    power.light=light>=1 && light<=5 ? light:0;
    return 0;
}
static int set_light(int level)
{diffeq_os_set_light((char)level);return 0;}
static void restore_light(void)
{
    if(power.dimmed && power.light)gint_world_switch(GINT_CALL(set_light,power.light));
    power.dimmed=false;
}
static void refresh(void)
{
    gint_world_switch(GINT_CALL(read_settings));
    power.last=rtc_ticks();power.seen=activity;
    power.dimmed=false;power.manual=false;power.pending=false;power.io_blocked=false;
}
void power_init(void)
{
    if(power.active)return;
    prior_filter=keydev_async_filter(keydev_std());
    keydev_set_async_filter(keydev_std(),observe_key);
    power.active=true;refresh();usb_initialize(&usb,usb_native_sample());
}
void power_shutdown(void)
{
    if(!power.active)return;
    restore_light();keydev_set_async_filter(keydev_std(),prior_filter);
    power.active=false;
}
void power_osmenu(void)
{
    if(!usb_handoff_begin(&usb,usb_native_sample()))return;
    power.manual=power.pending=false; /* MENU wins an already pending OFF. */
    if(!storage_usb_ready()){
        power.io_blocked=true;usb_handoff_end(&usb,usb_native_sample());return;
    }
    restore_light();gint_osmenu();
    if(power.active)refresh();
    dupdate();
    usb_handoff_end(&usb,usb_native_sample());
}
bool power_key(key_event_t event)
{
    if(!power.active)return false;
    if(event.type==KEYEV_DOWN || event.type==KEYEV_HOLD)activity++;
    if(event.type==KEYEV_DOWN && event.key==KEY_ACON && event.shift && !event.alpha) {
        power.manual=true;power.pending=true;return true;
    }
    return false;
}
bool power_poll(bool idle)
{
    if(!power.active)return false;
    usb_observe(&usb,usb_native_sample());
    if(usb.pending){
        if(!idle)return true; /* Owner rolls back solver/drawing/table first. */
        power_osmenu();clearevents();return true; /* No autosave; consume raced keys. */
    }
    uint32_t now=rtc_ticks(),serial=activity;
    if(serial!=power.seen || !keydev_idle(keydev_std(),0)) {
        power.seen=serial;power.last=now;power.io_blocked=false;restore_light();
        if(!power.manual)power.pending=false;
    }
    uint32_t elapsed=since(now,power.last);
    if(power.apo && elapsed>=power.apo && !power.io_blocked)power.pending=true;
    if(power.pending) {
        if(!idle)return true;
        if(!usb_handoff_begin(&usb,usb_native_sample()))return true;
        if(!storage_usb_ready()){
            power.manual=power.pending=false;power.io_blocked=true;
            usb_handoff_end(&usb,usb_native_sample());return true;
        }
        restore_light();gint_poweroff(true);
        /* Let the 128-Hz keyboard scanner observe ON, then require release.
           Otherwise automatic wake can look like a fresh AC editor clear. */
        uint32_t resumed=rtc_ticks();
        do {sleep();clearevents();}
        while(since(rtc_ticks(),resumed)<2 || !keydev_idle(keydev_std(),0));
        /* Drop queued pre-suspend/wake events before handing input to UI. */
        clearevents();refresh();dupdate();usb_handoff_end(&usb,usb_native_sample());return true;
    }
    if(!power.dimmed && power.dim && elapsed>=power.dim && power.light) {
        gint_world_switch(GINT_CALL(set_light,1));
        /* CG50 low-power PWM value from the installed R61524 driver's level-0
           table (below user level 1). The OS call first sets the matching port
           range; gint's checked 16-bit register API supplies SYNCO. */
        r61524_set(0x5a1,0x14);
        power.dimmed=true;
    }
    return false;
}
key_event_t power_wait_key(volatile int *timeout)
{
    if(!power.active)return getkey_opt(GETKEY_DEFAULT,timeout);
    for(;;) {
        volatile int poll=1;
        key_event_t event=getkey_opt(GETKEY_DEFAULT & ~(GETKEY_MENU|GETKEY_POWEROFF),&poll);
        if(power_key(event)) {power_poll(true);continue;}
        if(event.type==KEYEV_DOWN && event.key==KEY_MENU && !event.shift && !event.alpha) {
            power_osmenu();continue;
        }
        if(power_poll(true))continue;
        if(event.type!=KEYEV_NONE)return event;
        if(timeout && *timeout)return event;
        /* Keyboard scans and the existing blink timer wake this loop. No new
           timer or busy-spin; a missed race waits at most one keyboard scan. */
        sleep();
    }
}
