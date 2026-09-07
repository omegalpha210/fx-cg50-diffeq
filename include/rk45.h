#ifndef DIFFEQ_RK45_H
#define DIFFEQ_RK45_H
#include "ode.h"
typedef enum {ODE_RK4,ODE_RK45} OdeMethod;
typedef struct {int method;double reltol,abstol;} OdeAdaptive;
#define ODE_ATTEMPT_BUDGET 200000u
#define ODE_RHS_BUDGET 800000u
#define ODE_RHS_WORK_BUDGET 2400000u
/* A caller-owned transaction budget/diagnostic record, shared across IVPs and
   refinements. Reset once per Graph/TRACE/Table/G-Solve action, not per step. */
typedef struct {
    uint32_t accepted,rejected,rhs,work;
    double min_h,max_h;
} OdeWork;
typedef struct {double k[7][ODE_MAX_DIM],next[ODE_MAX_DIM],error[ODE_MAX_DIM];int stage;} OdeDopri;
void ode_adaptive_defaults(OdeAdaptive *a);
OdeStatus ode_adaptive_validate(const OdeAdaptive *a);
/* Independent numerical stage calculation. Output workspace is only a trial;
   the adaptive driver alone decides whether to commit it. FSAL is not used. */
OdeStatus ode_dopri_step(OdeRhs rhs,void *ctx,int n,double x,const double *y,
    double h,OdeDopri *trial,OdeWork *work,OdeCancel cancel,void *cancel_ctx);
/* spacing > 0: land on an IC-anchored uniform output grid (plus final target).
   spacing == 0: endpoint only. Rejected/internal accepted steps are not emitted.
   A terminal failure may emit the last trusted accepted state before returning. */
OdeResult ode_rk45_integrate(OdeRhs rhs,void *ctx,int n,double x0,const double *y0,
    double target,const OdeSettings *s,const OdeAdaptive *a,double spacing,
    OdeSample sample,void *sample_ctx,OdeCancel cancel,void *cancel_ctx,OdeWork *work);
#endif
