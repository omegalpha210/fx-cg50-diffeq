#ifndef DIFFEQ_TRACE_H
#define DIFFEQ_TRACE_H
#include "graph.h"
#define TRACE_BRANCH_POINTS 129
/* Fixed shared point budget across all IVPs. Segment links forbid gap interpolation. */
#define TRACE_POINTS (2*TRACE_BRANCH_POINTS)
typedef struct {double x,y[ODE_MAX_DIM];} TracePoint;
typedef struct {unsigned start,count;bool invalid,event;} TraceBranch;
typedef struct {
    TracePoint point[TRACE_POINTS];
    unsigned char link[TRACE_POINTS];
    TraceBranch branch[ODE_MAX_IC][2];
    OdeSettings extent;
    bool valid,has_invalid;int family,variable,dim;
    /* Bounds include every original valid SYS2 sample, before decimation. */
    double phase_min[2],phase_max[2];unsigned phase_points;
    GraphResult result;
} TraceSamples;
/* Capture the initial full-resolution graph stream without integrating twice.
   Begin only after preflight; side is 0 (backward) or 1 (forward). A failed or
   cancelled transaction leaves the committed cache untouched. */
bool trace_capture_begin(const Document *d);
bool trace_capture_branch_begin(const Document *d,int family,int side);
bool trace_capture_point(double x,const double *y,uint32_t step,void *unused);
void trace_capture_branch_end(ModelPathResult result);
void trace_capture_end(bool success);
/* Presentation-only reuse: identity and required original solver coverage. */
bool trace_plot_matches(const Document *d,double xmin,double xmax);
void trace_box_show(int x1,int y1,int x2,int y2,bool rectangle);
void trace_box_restore(void);
bool trace_cache_matches(const Document *d);
void trace_cache_invalidate(void);
GraphResult trace_cache_result(void);
/* Draw cached solution segments only; the caller owns backdrop and overlays. */
void trace_cache_render(const Document *d);
/* G-Solve borrows inactive overlay scratch; no TRACE cursor/session is created. */
void graph_overlay_begin(int top);
void graph_overlay_restore(void);
void graph_overlay_point(int x,int y);
void graph_overlay_curves(const Document *d,int family,int variable,int other_family,int other_variable,bool highlighted);
void graph_overlay_curve(const Document *d,int family,int variable,bool highlighted);
uint16_t *graph_busy_pixels(unsigned *capacity,bool staging);
OdeStatus graph_plot_prepare(Document *d,CompiledModel *m,OdeCancel cancel,void *context);
bool trace_cache_phase_window(const Document *d,ViewWindow *window);
bool trace_cache_time_window(const Document *d,ViewWindow *window);
const OdeSettings *trace_extent(void);
bool trace_select(const Document *d,int family,int variable);
bool trace_step(double x,int direction,double dx,TracePoint *point);
/* A TRACE session freezes its horizontal view and integration-time stride.
   samples.extent retains the entry trajectory range; navigation cannot extend it.
   In PHASE xmin/xmax refer to the selected state axis, never integration time. */
typedef struct {double xmin,xmax,xscale,xdot;bool phase;unsigned char axis;} TraceViewport;
const TraceViewport *trace_viewport(void);
/* Follow a valid point vertically only, preserving scales and solver settings. */
void trace_follow(Document *d,CompiledModel *m,const TracePoint *point);
/* Clamp to the visible connected valid trajectory, then perform Y follow only. */
OdeStatus trace_navigate(Document *d,CompiledModel *m,double target,bool jump,TracePoint *point);
bool trace_prepare(Document *d,CompiledModel *m,int family,int variable);
bool trace_point_near(double x,TracePoint *point);
bool trace_move(double x,int direction,TracePoint *point);
bool trace_has_invalid(void);
bool trace_direction_invalid(int direction);
void trace_overlay_begin(void);
void trace_overlay_restore(void);
void trace_overlay_show(const Document *d,const TracePoint *point,int variable,bool highlight);
#endif
