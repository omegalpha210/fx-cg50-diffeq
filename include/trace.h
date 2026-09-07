#ifndef DIFFEQ_TRACE_H
#define DIFFEQ_TRACE_H
#include "graph.h"
#define TRACE_BRANCH_POINTS 129
/* Fixed shared point budget across all IVPs. Segment links forbid gap interpolation. */
#define TRACE_POINTS (2*TRACE_BRANCH_POINTS)
typedef struct {double x,y[ODE_MAX_DIM];} TracePoint;
typedef struct {unsigned start,count;bool invalid;} TraceBranch;
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
bool trace_cache_matches(const Document *d);
void trace_cache_invalidate(void);
GraphResult trace_cache_result(void);
/* Draw cached solution segments only; the caller owns backdrop and overlays. */
void trace_cache_render(const Document *d);
bool trace_cache_phase_window(const Document *d,ViewWindow *window);
bool trace_cache_time_window(const Document *d,ViewWindow *window);
const OdeSettings *trace_extent(void);
bool trace_select(const Document *d,int family,int variable);
bool trace_step(double x,int direction,double dx,TracePoint *point);
/* Follow a known valid point without integrating or changing solver settings. */
void trace_follow(Document *d,CompiledModel *m,const TracePoint *point);
/* One transaction: extend only for an actual out-of-cache target, resolve, pan. */
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
