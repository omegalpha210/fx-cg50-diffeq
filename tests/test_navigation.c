#include "app.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    AppNavigation navigation;
    app_navigation_init(&navigation);
    for(int iteration=0;iteration<1000;iteration++) {
        assert(navigation.current==APP_SCREEN_MAIN);

        app_navigation_open(&navigation,APP_SCREEN_SETTINGS);
        app_navigation_open(&navigation,APP_SCREEN_CONSTANTS);
        app_navigation_back(&navigation);
        assert(navigation.current==APP_SCREEN_SETTINGS);
        app_navigation_back(&navigation);

        app_navigation_open(&navigation,APP_SCREEN_FIRST_ORDER);
        app_navigation_open(&navigation,APP_SCREEN_EQUATION);
        app_navigation_open(&navigation,APP_SCREEN_INITIAL_CONDITIONS);
        app_navigation_open(&navigation,APP_SCREEN_PARAMETERS);
        app_navigation_open(&navigation,APP_SCREEN_CALCULATE);
        app_navigation_back(&navigation);
        app_navigation_open(&navigation,APP_SCREEN_GRAPH);
        app_navigation_open(&navigation,APP_SCREEN_TRACE);
        app_navigation_back(&navigation);
        assert(navigation.current==APP_SCREEN_GRAPH);
        app_navigation_back(&navigation);
        assert(navigation.current==APP_SCREEN_PARAMETERS);
        app_navigation_back(&navigation);
        assert(navigation.current==APP_SCREEN_INITIAL_CONDITIONS);
        app_navigation_back(&navigation);
        assert(navigation.current==APP_SCREEN_EQUATION);
        app_navigation_back(&navigation);
        assert(navigation.current==APP_SCREEN_FIRST_ORDER);
        app_navigation_back(&navigation);
    }
    assert(navigation.current==APP_SCREEN_MAIN);
    puts("1000 iterative navigation cycles passed without recursive screen calls.");
    return 0;
}
