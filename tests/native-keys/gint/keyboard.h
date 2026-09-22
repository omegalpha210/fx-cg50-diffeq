#ifndef TEST_NATIVE_KEYBOARD_H
#define TEST_NATIVE_KEYBOARD_H
#include "../../host/gint/keyboard.h"
/* Values/signatures from the installed gint 2.11 keyboard.h. */
enum {GETKEY_MENU=0x0008,GETKEY_POWEROFF=0x0400,GETKEY_DEFAULT=0x05df};
key_event_t getkey_opt(int options,volatile int *timeout);
int keydown(int key);
#endif
