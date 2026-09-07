#ifndef DIFFEQ_APP_H
#define DIFFEQ_APP_H
#include "model.h"
typedef struct {
    Document doc,recall;
    union {
        CompiledModel model;
        struct {Document current,recall;} load;
    };
    bool has_recall,dirty;
} App;

typedef enum {
    APP_SCREEN_MAIN,
    APP_SCREEN_FIRST_ORDER,
    APP_SCREEN_DIMENSION,
    APP_SCREEN_EQUATION,
    APP_SCREEN_SETTINGS,
    APP_SCREEN_PARAMETERS,
    APP_SCREEN_OUTPUT,
    APP_SCREEN_VWINDOW,
    APP_SCREEN_GRAPH_SETTINGS,
    APP_SCREEN_CONSTANTS,
    APP_SCREEN_INITIAL_CONDITIONS,
    APP_SCREEN_CALCULATE,
    APP_SCREEN_GRAPH,
    APP_SCREEN_TRACE,
    APP_SCREEN_TABLE,
    APP_SCREEN_SAVE,
    APP_SCREEN_LOAD,
    APP_SCREEN_COUNT
} AppScreen;

/* Navigation ancestry is data owned by the top-level dispatcher. Screen
   changes therefore never add C call frames, regardless of repetition. */
typedef struct {
    AppScreen current;
    AppScreen parent[APP_SCREEN_COUNT];
} AppNavigation;

int app_run(void);
void app_initialize(App *app,const char *storage_directory);
bool app_compile(App *app);
void app_navigation_init(AppNavigation *navigation);
void app_navigation_open(AppNavigation *navigation,AppScreen child);
void app_navigation_back(AppNavigation *navigation);
#endif
