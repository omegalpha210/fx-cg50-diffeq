#ifndef DIFFEQ_EVENTS_H
#define DIFFEQ_EVENTS_H
#include "expression.h"
#include "ode.h"
typedef enum {EVENT_ANY,EVENT_RISING,EVENT_FALLING} EventDirection;
typedef enum {EVENT_MARK,EVENT_STOP} EventAction;
typedef struct {char text[EXPR_TEXT];uint8_t enabled,direction,action;} EventConfig;
#define EVENT_CAPACITY 32
#define EVENT_ITERATIONS 48
#define EVENT_EVAL_BUDGET 200000u
typedef struct {double x,y[ODE_MAX_DIM];int family,direction;} EventMarker;
typedef struct {EventMarker point[EVENT_CAPACITY];unsigned count;} EventMarkers;
typedef struct {
    bool valid;int method,dim,nic;OdeStatus status;OdeWork work;
    double xmin,xmax,h,reltol,abstol;
    uint32_t hits,evaluations,unavailable;
    uint8_t enabled,direction,action;
    EventMarkers markers;
} SolverReport;
const SolverReport *solver_report(void);
void solver_report_reset(void);
#endif
