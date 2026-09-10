#ifndef DIFFEQ_ODE_H
#define DIFFEQ_ODE_H
#include <stdbool.h>
#include <stdint.h>
#define ODE_MAX_DIM 9
#define ODE_MAX_IC 10
#define ODE_SF_MAX 50

typedef enum {
    ODE_OK, ODE_CANCELLED, ODE_SAMPLE_STOP, ODE_BAD_INPUT,
    ODE_BAD_STEP, ODE_STEP_LIMIT, ODE_NONFINITE, ODE_DIVERGED, ODE_DOMAIN, ODE_IO_ERROR,
    ODE_HAS_INVALID, ODE_WORK_LIMIT, ODE_STEP_UNDERFLOW, ODE_TOLERANCE, ODE_EVENT_STOP
} OdeStatus;
typedef struct {
    double xmin, xmax, h;
    uint32_t max_steps;
    int step, sf;
} OdeSettings;
typedef OdeStatus (*OdeRhs)(double x, const double *y, double *dy, void *ctx);
/* step=0 also forces a refined RK4 Event terminal through Step decimation;
   OdeResult.steps retains the actual canonical step count. */
typedef bool (*OdeSample)(double x, const double *y, uint32_t step, void *ctx);
typedef bool (*OdeCancel)(void *ctx);
typedef struct {
    OdeStatus status;
    uint32_t steps;
    double x, y[ODE_MAX_DIM];
} OdeResult;
#define ODE_ATTEMPT_BUDGET 200000u
#define ODE_RHS_BUDGET 800000u
#define ODE_RHS_WORK_BUDGET 2400000u
typedef struct {
    uint32_t accepted,rejected,rhs,work;
    double min_h,max_h;
} OdeWork;
/* Called only for an initial/accepted state, before canonical commit. A STOP
   may replace *at/next with a refined root. Scratch queries omit this hook. */
typedef OdeStatus (*OdeAccepted)(double from,const double *old,double *at,
    double *next,void *context);
OdeResult ode_integrate_control(OdeRhs rhs,void *ctx,int n,double x0,
    const double *y0,double target,const OdeSettings *settings,
    OdeSample sample,void *sample_ctx,OdeCancel cancel,void *cancel_ctx,
    OdeWork *work,OdeAccepted accepted,void *accepted_ctx);

OdeStatus ode_validate(const OdeSettings *s);
OdeStatus ode_rk4(OdeRhs rhs, void *ctx, int n, double x,
    const double *y, double h, double *next);
OdeResult ode_integrate(OdeRhs rhs, void *ctx, int n, double x0,
    const double *y0, double target, const OdeSettings *settings,
    OdeSample sample, void *sample_ctx, OdeCancel cancel, void *cancel_ctx);
const char *ode_status_text(OdeStatus status);
bool ode_invalid_region(OdeStatus status);
OdeStatus ode_values_status(const double *values,int count);
#endif
