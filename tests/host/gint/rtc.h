#ifndef HOST_GINT_RTC_H
#define HOST_GINT_RTC_H
#include <stdint.h>
uint32_t rtc_ticks(void);
void host_tick_step(unsigned ticks);
#endif
