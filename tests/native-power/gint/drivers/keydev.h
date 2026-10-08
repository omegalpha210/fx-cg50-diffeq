#ifndef DIFFEQ_NATIVE_POWER_KEYDEV_H
#define DIFFEQ_NATIVE_POWER_KEYDEV_H
#include <gint/keyboard.h>
#include <stdbool.h>
#include <stdint.h>
#define KEYBOARD_QUEUE_SIZE 32
typedef struct {
    unsigned time;
    int8_t queue_next,queue_end;
    uint8_t state_now[12],state_queue[12];
    key_event_t queue[KEYBOARD_QUEUE_SIZE];
} keydev_t;
typedef bool (*keydev_async_filter_t)(key_event_t event);
keydev_t *keydev_std(void);
keydev_async_filter_t keydev_async_filter(keydev_t *device);
void keydev_set_async_filter(keydev_t *device,keydev_async_filter_t filter);
bool keydev_idle(keydev_t *device,int key);
#endif
