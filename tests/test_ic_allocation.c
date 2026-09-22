#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned allocations;
void *host_ic_malloc(size_t n) {allocations++;return allocations==1 ? NULL:malloc(n);}
int main(void)
{
    Document d;model_defaults(&d,EQ_GENERAL,1);UiInitialState state={0};
    /* First EXE cannot store x0 draft. Second retries the retained EDIT text;
       EXIT keeps it, NEXT validates it on re-entry. */
    assert(!setenv("DIFFEQ_HOST_KEYS","2 EXE EXE EXIT F6",1));
    assert(ui_initial_conditions(&d,&state)==UI_STAGE_BACK);
    assert(allocations==2 && state.draft[0] && !strcmp(state.draft[0],"2") && d.ic[0].x==0);
    assert(ui_initial_conditions(&d,&state)==UI_STAGE_NEXT);
    assert(d.ic[0].x==2 && !state.draft[0]);ui_initial_clear(&state);
    puts("IC allocation failure: editing text retained, retry/EXIT/NEXT commit and free PASS.");
}
