#ifndef DIFFEQ_GSOLVE_H
#define DIFFEQ_GSOLVE_H
#include "model.h"

#define GSOLVE_MAX_RESULTS 32

typedef enum {GSOLVE_ROOT,GSOLVE_MAXIMUM,GSOLVE_MINIMUM,GSOLVE_XCAL} GsolveMode;
typedef struct {int family,variable;} GsolveCurve;
typedef struct {double x,y;} GsolvePoint;
typedef struct {
    OdeStatus status;
    int count;
    uint32_t steps;
    bool has_invalid;
    GsolvePoint point[GSOLVE_MAX_RESULTS];
} GsolveResults;

int gsolve_curve_count(const Document *d);
bool gsolve_curve_at(const Document *d,int ordinal,GsolveCurve *curve);
OdeStatus gsolve_ycal(const Document *d,CompiledModel *m,GsolveCurve curve,double x,
    GsolvePoint *point,OdeCancel cancel,void *cancel_ctx);
GsolveResults gsolve_search(const Document *d,CompiledModel *m,GsolveCurve curve,
    GsolveMode mode,double target,OdeCancel cancel,void *cancel_ctx);
GsolveResults gsolve_intersections(const Document *d,CompiledModel *m,GsolveCurve first,
    GsolveCurve second,OdeCancel cancel,void *cancel_ctx);
#endif
