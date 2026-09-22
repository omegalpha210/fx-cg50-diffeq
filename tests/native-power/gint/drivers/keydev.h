#include <gint/keyboard.h>
#include <stdbool.h>
typedef struct {int unused;} keydev_t;
typedef bool (*keydev_async_filter_t)(key_event_t event);
keydev_t *keydev_std(void);
keydev_async_filter_t keydev_async_filter(keydev_t *device);
void keydev_set_async_filter(keydev_t *device,keydev_async_filter_t filter);
bool keydev_idle(keydev_t *device,int key);
