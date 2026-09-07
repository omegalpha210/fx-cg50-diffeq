#ifndef TEST_GINT_TIMER_H
#define TEST_GINT_TIMER_H
#include <stdint.h>
typedef struct {volatile int *flag;} gint_call_t;
#define TIMER_ANY -1
#define GINT_CALL_SET(flag) ((gint_call_t){flag})
int timer_configure(int timer,uint64_t delay,gint_call_t callback);
void timer_start(int timer);
void timer_stop(int timer);
#endif
