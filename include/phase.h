#ifndef DIFFEQ_PHASE_H
#define DIFFEQ_PHASE_H
#include "model.h"

#define PHASE_MAX_ROOTS 16
#define PHASE_GRID_MAX 20
#define PHASE_EVAL_LIMIT 8192u /* model_rhs calls; two expressions per call */
typedef enum {
    PHASE_UNAVAILABLE, PHASE_SADDLE, PHASE_STABLE_NODE, PHASE_UNSTABLE_NODE,
    PHASE_STABLE_SPIRAL, PHASE_UNSTABLE_SPIRAL, PHASE_CENTER_NEUTRAL,
    PHASE_INCONCLUSIVE
} PhaseType;
typedef struct {
    double y[2],jacobian[4],eigen_real[2],eigen_imag[2],residual;
    PhaseType type;
} PhaseRoot;
typedef struct {
    PhaseRoot root[PHASE_MAX_ROOTS];
    unsigned count,evaluations;
    bool truncated,has_invalid;
} PhaseResults;
/* Coordinates are dependent values (y1,y2), not independent x. component is
   zero for f1=0, one for f2=0. Callback false terminates with SAMPLE_STOP.
   A NULL callback performs the same validation without drawing. Segments are
   streamed: the caller owns render cancellation/rollback. */
typedef bool (*PhaseSegment)(int component,const double a[2],const double b[2],void *ctx);
bool phase_supported(const CompiledModel *m);
bool phase_autonomous(const CompiledModel *m);
OdeStatus phase_vector(const CompiledModel *m,double x,const double y[2],double out[2]);
/* Unit screen vector (positive screen Y points down). False means zero/invalid.
   Pixel dimensions and window spans both contribute to the direction. */
bool phase_direction(const ViewWindow *v,const double vector[2],double width,
    double height,double out[2]);
OdeStatus phase_nullclines(const CompiledModel *m,double reference_x,
    const ViewWindow *v,unsigned grid,PhaseSegment segment,void *segment_ctx,
    OdeCancel cancel,void *cancel_ctx);
/* Searches the closed view rectangle. Results commit only on ODE_OK; failures
   and cancellation leave *out byte-for-byte unchanged. Autonomous systems only.
   The bounded grid is a candidate search, not a guarantee to find every root. */
OdeStatus phase_equilibria(const CompiledModel *m,double reference_x,
    const ViewWindow *v,PhaseResults *out,OdeCancel cancel,void *cancel_ctx);
const char *phase_type_name(PhaseType type);
#endif
