#ifndef HOST_GINT_KEYBOARD_H
#define HOST_GINT_KEYBOARD_H
#include "keycodes.h"
typedef struct {unsigned time,mod,shift,alpha,type,key;} key_event_t;
enum {KEYEV_NONE,KEYEV_DOWN,KEYEV_UP,KEYEV_HOLD,KEYEV_OSMENU};
key_event_t getkey(void);
key_event_t pollevent(void);
#endif
