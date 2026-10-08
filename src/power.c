#include "power.h"
#include "storage.h"
#include "usb_lifecycle.h"
#include "usb_native.h"
#include "menu_boundary.h"
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
extern int diffeq_os_enable_menu_return(void);
#ifndef DIFFEQ_TEST_POWER
#define OS_STUB(name,id) __asm__(".text\n.balign 4\n.global _" #name "\n_" #name ":\n" \
    "mov.l 1f,r0\nmov.l 2f,r1\njmp @r1\nnop\n.balign 4\n1: .long " #id "\n2: .long 0x80020070\n")
OS_STUB(diffeq_os_apo,0x1e91);
OS_STUB(diffeq_os_duration,0x12d9);
OS_STUB(diffeq_os_light,0x1e8f);
OS_STUB(diffeq_os_set_light,0x0199);
OS_STUB(diffeq_os_enable_menu_return,0x1ea6);
static int enable_menu_return(void *unused)
{ (void)unused; return diffeq_os_enable_menu_return(); }
#endif

#define DAY_TICKS (86400u*128u)
static struct {
    uint32_t last,apo,dim,seen;
    int light;
    bool active,dimmed,manual,pending,io_blocked;
} power;
static volatile uint32_t activity;
static UsbLifecycle usb;
static CgMenuBoundary menu;
static bool menu_closed;
static unsigned input_epoch;
static keydev_async_filter_t prior_filter;
static uint32_t since(uint32_t now,uint32_t before)
{return now>=before ? now-before:now+DAY_TICKS-before;}

static bool observe_key(key_event_t event)
{
    /* Interrupt context: no OS calls, drawing, allocation or RTC access. */
    if(event.type==KEYEV_DOWN || event.type==KEYEV_UP)activity++;
    return prior_filter ? prior_filter(event):true;
}
#ifndef DIFFEQ_TEST_POWER
static void show_poweroff_notice(void)
{
    int box_w = 340, box_h = 96;
    int box_x = (396 - box_w) / 2;
    int box_y = (224 - box_h) / 2;
    drect_border(box_x, box_y, box_x + box_w - 1, box_y + box_h - 1, C_WHITE, 2, C_RGB(0, 16, 31));
    dtext_opt(396 / 2, box_y + 16, C_BLACK, C_NONE, DTEXT_CENTER, DTEXT_TOP, "Back to Main Menu");
    dtext_opt(396 / 2, box_y + 46, C_RGB(0, 12, 28), C_NONE, DTEXT_CENTER, DTEXT_TOP, "To shutdown, press SHIFT AC/ON");
    dtext_opt(396 / 2, box_y + 66, C_DARK, C_NONE, DTEXT_CENTER, DTEXT_TOP, "again in Main Menu");
    dupdate();
}
#endif
static int read_settings(void)
{
    int minutes=diffeq_os_apo();unsigned half=(unsigned char)diffeq_os_duration();
#ifndef DIFFEQ_TEST_POWER
    /* KhiCAS Golden Rule: 5 minutes (300 seconds) auto-park on hardware */
    (void)minutes;
    power.apo = 5u * 60u * 128u;
#else
    power.apo=(minutes==10 || minutes==60) ? (uint32_t)minutes*60u*128u:0;
#endif
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
    if(menu_closed)usb_handoff_end(&usb,usb_native_sample());
    cg_menu_cancel(&menu);menu_closed=false;
    restore_light();keydev_set_async_filter(keydev_std(),prior_filter);
    power.active=false;
}
void power_osmenu(void)
{
    if(!power.active || menu.pending || usb.handling)return;
    /* Own the opening press. Its UP must be consumed by the normal foreground
       reader before either scanned or consumed state can form a quiet gate. */
    power.manual=power.pending=false; /* MENU wins an already pending OFF. */
    (void)usb_take_request(&usb); /* Own this edge; timeout cannot hot-rearm it. */
    cg_menu_request(&menu,rtc_ticks());
}
static bool menu_service(bool idle)
{
    if(!menu.pending)return false;
    if(!idle)return true; /* The numerical/UI owner rolls back first. */
    int result=cg_menu_step(&menu,keydev_std(),rtc_ticks(),DAY_TICKS);
    if(result==CG_MENU_INVALID || result==CG_MENU_TIMEOUT) {
        cg_menu_cancel(&menu);
        if(menu_closed)usb_handoff_end(&usb,usb_native_sample());
        menu_closed=false;
        return false; /* Cancel only: keep unsaved RAM, never force the helper. */
    }
    if(result!=CG_MENU_READY)return false;
    if(!menu_closed) {
        if(!usb_handoff_begin(&usb,usb_native_sample()))return false;
        restore_light();
        if(!storage_usb_ready()) {
            cg_menu_cancel(&menu);power.io_blocked=true;
            usb_handoff_end(&usb,usb_native_sample());return true;
        }
        menu_closed=true;
        /* Closing files/brightness can switch OS worlds. Require another
           consumed quiet boundary and fresh scan after those workers. */
        menu.quiet=false;
        return false;
    }
    cg_menu_cancel(&menu);menu_closed=false;
    bool fresh_manual=power.manual,fresh_power=power.pending;
#ifndef DIFFEQ_TEST_POWER
    while (keydown(KEY_MENU) || keydown(KEY_EXIT)) sleep();
    clearevents();
    (void)gint_world_switch(GINT_CALL(enable_menu_return,(void *)NULL));
#endif
    gint_osmenu();
    if(power.active)refresh();
    /* Requests accepted after this MENU was armed belong to the next
       foreground boundary, including after the settings refresh. */
    power.manual=fresh_manual;power.pending=fresh_power;
    dupdate();
    usb_handoff_end(&usb,usb_native_sample());
    return true;
}
unsigned power_input_epoch(void){return input_epoch;}
bool power_key(key_event_t event)
{
    if(!power.active)return false;
    if(event.type==KEYEV_DOWN || event.type==KEYEV_HOLD)activity++;
    if(event.type==KEYEV_DOWN && event.key==KEY_EXIT && menu.pending) {
        cg_menu_cancel(&menu);
        if(menu_closed)usb_handoff_end(&usb,usb_native_sample());
        menu_closed=false; /* EXIT remains a normal UI key, with RAM intact. */
    }
    if(event.type==KEYEV_DOWN && event.key==KEY_ACON && event.shift && !event.alpha) {
        power.manual=true;power.pending=true;return true;
    }
    return false;
}
bool power_poll(bool idle)
{
    if(!power.active)return false;
    usb_observe(&usb,usb_native_sample());
    if(usb.pending && !menu.pending && !usb.handling)power_osmenu();
    if(menu.pending)return menu_service(idle);
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
#ifndef DIFFEQ_TEST_POWER
        /* KhiCAS Rule: Display notice, wait 1 second (128 ticks), clear events,
           and safely park in Casio OS Main Menu via 0x1EA6 + gint_osmenu(). */
        show_poweroff_notice();
        uint32_t notice_t0 = rtc_ticks();
        while (since(rtc_ticks(), notice_t0) < 128) sleep();
        clearevents();
        (void)gint_world_switch(GINT_CALL(enable_menu_return,(void *)NULL));
        restore_light();
        gint_osmenu();
        clearevents();
        power.manual=power.pending=false;
        input_epoch++;
        refresh();
        usb_handoff_end(&usb,usb_native_sample());
        return true;
#else
        restore_light();gint_poweroff(true);
        /* Let the 128-Hz keyboard scanner observe ON, then require release.
           Otherwise automatic wake can look like a fresh AC editor clear. */
        uint32_t resumed=rtc_ticks();
        do {sleep();clearevents();}
        while(since(rtc_ticks(),resumed)<2 || !keydev_idle(keydev_std(),0));
        /* Drop queued pre-suspend/wake events before handing input to UI. */
        clearevents();input_epoch++;refresh();dupdate();usb_handoff_end(&usb,usb_native_sample());return true;
#endif
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
