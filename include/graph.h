#ifndef DIFFEQ_GRAPH_H
#define DIFFEQ_GRAPH_H
#include "app.h"
#define PLOT_LEFT 0
#define PLOT_RIGHT 383
#define PLOT_TOP 0
#define PLOT_BOTTOM 197
typedef struct {OdeStatus status;unsigned steps;int failed_family;OdeStatus invalid;} GraphResult;
typedef enum {
    UI_GRAPH_BACK,
    UI_GRAPH_TRACE,
    UI_GRAPH_VWINDOW,
    UI_GRAPH_TABLE,
    UI_GRAPH_SETTINGS
} UiGraphAction;
bool graph_clip(const ViewWindow *v,double *x0,double *y0,double *x1,double *y1);
bool graph_point(const ViewWindow *v,double x,double y,int *px,int *py);
bool graph_follow_window(ViewWindow *v,double x,double y);
void graph_backdrop(Document *d,CompiledModel *m);
void graph_event_markers(const Document *d);
void graph_solution_segment(const ViewWindow *v,double x0,double y0,double x1,double y1,int color);
bool graph_zoom(ViewWindow *v,double factor,double dx,double dy);
int graph_color(int family,int variable,int dimension);
int graph_palette_color(unsigned color);
int graph_field_color(unsigned color);
bool graph_field_direction(const ViewWindow *v,double slope,double *dx,double *dy);
int graph_highlight_color(int base,bool invert);
bool graph_family_enabled(const Document *d,int family);
void graph_segment(const ViewWindow *v,double x0,double y0,double x1,double y1,int color);
GraphResult graph_render(Document *d,CompiledModel *m,bool first);
OdeStatus graph_auto_window(Document *d,CompiledModel *m);
OdeStatus graph_highlight_curve(Document *d,CompiledModel *m,int family,int variable);
UiGraphAction ui_graph(App *a,bool first);
void ui_trace(App *a);
void ui_table(App *a);
#endif
