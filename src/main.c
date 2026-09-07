#include "app.h"
#include <gint/display.h>
#include <gint/keyboard.h>
#ifdef FXCG50
#include <fxlibc/printf.h>
#include <gint/drivers/keydev.h>
#endif
int main(void)
{
    /* fxlibc deliberately leaves %e/%f/%g disabled until requested. Every
       numeric field and CSV cell in DIFF EQ uses these conversions. */
    #ifdef FXCG50
    __printf_enable_fp();
    #endif

    /* The installed platform header documents single VRAM as the default.
       Keep this explicit; streaming rendering does not allocate frame copies. */
    dsetvram(gint_vram,0);
    /* Do not inherit a queued launcher/release event at application entry. */
    #ifdef FXCG50
    clearevents();
    keydev_transform_t transform=keydev_transform(keydev_std());
    transform.enabled|=KEYDEV_TR_DELAYED_MODS;
    keydev_set_transform(keydev_std(),transform);
    #endif
    return app_run();
}
