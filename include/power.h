#ifndef DIFFEQ_POWER_H
#define DIFFEQ_POWER_H
#include <stdbool.h>
#include <gint/keyboard.h>
#ifdef FXCG50
/* OS work runs only in the main thread. Compute polls request cancellation;
   power-off waits until the owner has restored a stable screen. */
void power_init(void);
void power_shutdown(void);
/* True requests transaction cancellation (idle=false), or reports a completed
   handoff/close failure (idle=true). A waiting MENU returns to the caller. */
bool power_poll(bool idle);
bool power_key(key_event_t event);
key_event_t power_wait_key(volatile int *timeout);
void power_osmenu(void);
/* POWER wake alone invalidates old UI deferred input. MENU retains fresh keys. */
unsigned power_input_epoch(void);
#else
static inline void power_init(void) {}
static inline void power_shutdown(void) {}
static inline bool power_poll(bool idle) {(void)idle;return false;}
static inline bool power_key(key_event_t event) {(void)event;return false;}
static inline key_event_t power_wait_key(volatile int *timeout) {(void)timeout;return getkey();}
static inline void power_osmenu(void) {}
static inline unsigned power_input_epoch(void) {return 0;}
#endif
#endif
