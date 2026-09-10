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
bool graph_follow_y(ViewWindow *v,double y);
/* Local9px cross: black arms around a white3x3 center, clipped to plot. */
typedef struct {uint16_t pixels[81];int x,y;bool active;} GraphPointPatch;
void graph_point_cross(int x,int y,GraphPointPatch *patch);
void graph_point_restore(GraphPointPatch *patch);
void graph_backdrop(Document *d,CompiledModel *m);
void graph_event_markers(const Document *d);
/* Small, read-only view/event/Phase legends; never a reserved plot header. */
void graph_labels(const Document *d,const CompiledModel *m);
void graph_solution_segment(const ViewWindow *v,double x0,double y0,double x1,double y1,int color);
typedef struct {ViewWindow time,phase;bool phase_saved,valid;double xmin,xmax;} GraphEntryView;
void graph_entry_capture(GraphEntryView *entry,const Document *d);
void graph_entry_restore(const GraphEntryView *entry,Document *d);
bool graph_box_window(ViewWindow *view,int x1,int y1,int x2,int y2);
void graph_status(GraphResult result);
bool graph_redraw_cached(Document *d,CompiledModel *m,double xmin,double xmax);
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
void ui_trace(App *a,GraphResult result);
void ui_table(App *a);
#endif
