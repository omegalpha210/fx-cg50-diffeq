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
} TraceSamples;
const OdeSettings *trace_extent(void);
bool trace_select(const Document *d,int family,int variable);
bool trace_step(double x,int direction,double dx,TracePoint *point);
OdeStatus trace_follow(Document *d,CompiledModel *m,double x);
bool trace_prepare(Document *d,CompiledModel *m,int family,int variable);
bool trace_point_near(double x,TracePoint *point);
bool trace_move(double x,int direction,TracePoint *point);
bool trace_has_invalid(void);
bool trace_direction_invalid(int direction);
void trace_overlay_begin(void);
void trace_overlay_restore(void);
void trace_overlay_show(const Document *d,const TracePoint *point,int variable,bool highlight);
#endif
