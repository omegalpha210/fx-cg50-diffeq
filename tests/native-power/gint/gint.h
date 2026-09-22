#ifndef TEST_POWER_GINT_H
#define TEST_POWER_GINT_H
#include <stdbool.h>
typedef struct {int (*zero)(void);int (*one)(int);int arg;} gint_call_t;
#define POWER_PICK(_1,_2,NAME,...) NAME
#define POWER_ZERO(f) ((gint_call_t){.zero=(f)})
#define POWER_ONE(f,a) ((gint_call_t){.one=(f),.arg=(a)})
#define GINT_CALL(...) POWER_PICK(__VA_ARGS__,POWER_ONE,POWER_ZERO,unused)(__VA_ARGS__)
int gint_world_switch(gint_call_t call);
void gint_osmenu(void);
void gint_poweroff(bool logo);
#endif
