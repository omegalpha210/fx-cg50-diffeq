#ifndef DIFFEQ_TRACE_H
#define DIFFEQ_TRACE_H
#include "graph.h"
#define TRACE_BRANCH_POINTS 129
/* One selected IVP, bounded exact RK4 samples. No document or full VRAM copy. */
typedef struct {double x,y[ODE_MAX_DIM];} TracePoint;
typedef struct {TracePoint point[TRACE_BRANCH_POINTS];unsigned count;} TraceBranch;
typedef struct {TraceBranch branch[2];bool valid,has_invalid;int family,variable;} TraceSamples;
bool trace_prepare(Document *d,CompiledModel *m,int family,int variable);
bool trace_point_near(double x,TracePoint *point);
bool trace_move(double x,int direction,TracePoint *point);
bool trace_has_invalid(void);
void trace_overlay_begin(void);
void trace_overlay_restore(void);
void trace_overlay_show(const Document *d,const TracePoint *point,int variable,bool highlight);
#endif
