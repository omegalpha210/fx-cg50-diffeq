#include "app.h"
#include "storage.h"
#include <string.h>

void app_initialize(App *a,const char *storage_directory)
{
    /* gint clears BSS for a fresh add-in launch, but application startup must
       also be correct when called again in a retained process/test harness. */
    memset(a,0,sizeof(*a));
    solver_report_reset();
    model_defaults(&a->doc,EQ_GENERAL,1);
    /* Explicit RCL owns restoration. MENU resumes this execution without
       calling initialization; a true new execution always starts at defaults. */
    (void)storage_directory;
    a->dirty=false;
}

void app_navigation_init(AppNavigation *navigation)
{
    for(int i=0;i<APP_SCREEN_COUNT;i++)navigation->parent[i]=APP_SCREEN_MAIN;
    navigation->current=APP_SCREEN_MAIN;
}

void app_navigation_open(AppNavigation *navigation,AppScreen child)
{
    if(!navigation || child<0 || child>=APP_SCREEN_COUNT)return;
    navigation->parent[child]=navigation->current;
    navigation->current=child;
}

void app_navigation_back(AppNavigation *navigation)
{
    if(!navigation || navigation->current<0 || navigation->current>=APP_SCREEN_COUNT) return;
    navigation->current=navigation->parent[navigation->current];
}
