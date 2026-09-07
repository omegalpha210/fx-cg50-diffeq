#ifndef DIFFEQ_ODE_H
#define DIFFEQ_ODE_H
#include <stdbool.h>
#include <stdint.h>
#define ODE_MAX_DIM 9
#define ODE_MAX_IC 9

typedef enum {
    ODE_OK, ODE_CANCELLED, ODE_SAMPLE_STOP, ODE_BAD_INPUT,
    ODE_BAD_STEP, ODE_STEP_LIMIT, ODE_NONFINITE, ODE_DIVERGED, ODE_DOMAIN, ODE_IO_ERROR,
    ODE_HAS_INVALID, ODE_WORK_LIMIT
} OdeStatus;
typedef struct {
    double xmin, xmax, h;
    uint32_t max_steps;
    int step, sf;
} OdeSettings;
typedef OdeStatus (*OdeRhs)(double x, const double *y, double *dy, void *ctx);
typedef bool (*OdeSample)(double x, const double *y, uint32_t step, void *ctx);
typedef bool (*OdeCancel)(void *ctx);
typedef struct {
    OdeStatus status;
    uint32_t steps;
    double x, y[ODE_MAX_DIM];
} OdeResult;

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
