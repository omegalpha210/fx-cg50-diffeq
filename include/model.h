#ifndef DIFFEQ_MODEL_H
#define DIFFEQ_MODEL_H
#include "ode.h"
#include "expression.h"
typedef enum {EQ_SEPARABLE,EQ_LINEAR,EQ_BERNOULLI,EQ_GENERAL,
    EQ_SECOND,EQ_HIGHER,EQ_SYSTEM} EquationKind;
typedef struct {double x,y[ODE_MAX_DIM];} InitialCondition;
typedef struct {
    double xmin,xmax,ymin,ymax,xscale,yscale;
    int grid,labels,phase,phase_x,phase_y;
} ViewWindow;
typedef struct {
    int kind,dim,nic;
    char text[ODE_MAX_DIM][EXPR_TEXT];
    double power,constants[EXPR_CONSTANTS];
    InitialCondition ic[ODE_MAX_IC];
    OdeSettings solver;
    int solver_custom;
    ViewWindow view;
    uint16_t graph_mask[ODE_MAX_IC],list_mask[ODE_MAX_IC];
    uint8_t color[ODE_MAX_IC][ODE_MAX_DIM];
} Document;
typedef struct {
    int kind,dim;
    double power;
    ExprProgram eq[ODE_MAX_DIM];
} CompiledModel;
typedef struct {int equation; ExprError expression; OdeStatus values;} ModelError;
void model_defaults(Document *d,EquationKind kind,int dim);
unsigned model_default_color(int family,int variable,int dimension);
unsigned model_color(const Document *d,int family,int variable);
void model_color_defaults(Document *d);
void model_output_defaults(Document *d);
void model_sanitize_colors(Document *d);
void model_window_defaults(ViewWindow *v);
void model_sync_solver_window(Document *d);
double model_xdot(const ViewWindow *v);
bool model_set_xdot(ViewWindow *v,double xdot);
int model_equations(const Document *d);
const char *model_kind_name(int kind);
void model_equation_label(const Document *d,int eq,char *out,unsigned size);
void model_variable_label(const Document *d,int variable,char *out,unsigned size);
OdeStatus model_validate(const Document *d);
ModelError model_compile(const Document *d,CompiledModel *m);
OdeStatus model_rhs(double x,const double *y,double *dy,void *ctx);
bool model_convert_system(Document *d);
OdeResult model_trajectory(const Document *d,CompiledModel *m,int family,int direction,
    OdeSample sample,void *sample_ctx,OdeCancel cancel,void *cancel_ctx);
/* Segmented stream: y==NULL resets continuity. Only trusted finite samples
   follow a gap. The RK4 provider never guesses a restart after failed steps. */
typedef struct {OdeStatus status,invalid;uint32_t steps,points,segments;} ModelPathResult;
OdeResult model_trajectory_range(const Document *d,CompiledModel *m,int family,int direction,
    const OdeSettings *range,OdeSample sample,void *sample_ctx,OdeCancel cancel,void *cancel_ctx);
ModelPathResult model_path_branch(const Document *d,CompiledModel *m,int family,int direction,
    const OdeSettings *range,OdeSample sample,void *sample_ctx,OdeCancel cancel,void *cancel_ctx);
OdeResult model_value_at(const Document *d,CompiledModel *m,int family,double x,
    OdeCancel cancel,void *cancel_ctx);
#endif
