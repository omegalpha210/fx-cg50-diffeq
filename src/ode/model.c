#include "model.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

void model_window_defaults(ViewWindow *v)
{
    *v=(ViewWindow){.xmin=-6.3,.xmax=6.3,.ymin=-3.1,.ymax=3.1,
        .xscale=1,.yscale=1,.grid=1,.labels=1,.phase=0,.phase_x=0,.phase_y=1};
}
void model_sync_solver_window(Document *d)
{
    if(!d || d->solver_custom)return;
    double xmin=ceil(d->view.xmin),xmax=floor(d->view.xmax);
    if(xmin<xmax){d->solver.xmin=xmin;d->solver.xmax=xmax;}
}
double model_xdot(const ViewWindow *v)
{ return (v->xmax-v->xmin)/378.0; }
bool model_set_xdot(ViewWindow *v,double xdot)
{
    if(!v || !isfinite(xdot) || xdot<=0 || !isfinite(v->xmin+378.0*xdot))return false;
    v->xmax=v->xmin+378.0*xdot;return v->xmax>v->xmin;
}
void model_defaults(Document *d,EquationKind kind,int dim)
{
    memset(d,0,sizeof(*d));
    d->kind=kind;
    d->dim=kind<=EQ_GENERAL ? 1 : (kind==EQ_SECOND ? 2 : dim);
    if(d->dim<1 || d->dim>9) d->dim=2;
    d->nic=1; d->power=2;
    model_window_defaults(&d->view);
    d->solver=(OdeSettings){0,1,.1,20000,1,12};
    d->solver_custom=0;model_sync_solver_window(d);
    model_output_defaults(d);
    for(int i=0;i<9;i++) {
        d->ic[i].y[0]=1;
        strcpy(d->text[i],"0");
    }
    switch(kind) {
        case EQ_SEPARABLE:
            strcpy(d->text[0],"1");strcpy(d->text[1],"y^2-1");
            d->ic[0].y[0]=0;break;
        case EQ_LINEAR:strcpy(d->text[0],"x");strcpy(d->text[1],"x");d->ic[0].y[0]=-2;break;
        case EQ_BERNOULLI:strcpy(d->text[0],"-2");strcpy(d->text[1],"-1");break;
        case EQ_GENERAL:strcpy(d->text[0],"sin(x)-y");break;
        case EQ_SECOND:strcpy(d->text[1],"1");break;
        case EQ_HIGHER:break;
        case EQ_SYSTEM:
            for(int i=0;i<d->dim-1;i++) snprintf(d->text[i],EXPR_TEXT,"y%d",i+2);
            strcpy(d->text[d->dim-1],"-y1");break;
    }
}
unsigned model_default_color(int family,int variable,int dimension)
{
    static const uint8_t order[]={2,4,5,1,0,3};
    if(family<0 || family>=ODE_MAX_IC || variable<0 || variable>=ODE_MAX_DIM
        || dimension<1 || dimension>ODE_MAX_DIM)return 2;
    return order[(family*dimension+variable)%6];
}
unsigned model_color(const Document *d,int family,int variable)
{
    if(!d || family<0 || family>=ODE_MAX_IC || variable<0 || variable>=ODE_MAX_DIM)return 2;
    unsigned color=d->color[family][variable];
    return color<6 ? color:model_default_color(family,variable,d->dim);
}
void model_color_defaults(Document *d)
{
    for(int i=0;i<ODE_MAX_IC;i++)for(int j=0;j<ODE_MAX_DIM;j++)
        d->color[i][j]=(uint8_t)model_default_color(i,j,d->dim);
}
void model_output_defaults(Document *d)
{
    model_color_defaults(d);
    for(int i=0;i<ODE_MAX_IC;i++) {
        d->graph_mask[i]=(uint16_t)((1u<<d->dim)-1);
        d->list_mask[i]=(uint16_t)((1u<<(d->dim+1))-1);
    }
}
void model_sanitize_colors(Document *d)
{
    for(int i=0;i<ODE_MAX_IC;i++)for(int j=0;j<ODE_MAX_DIM;j++)
        d->color[i][j]=(uint8_t)model_color(d,i,j);
}
int model_equations(const Document *d)
{
    if(d->kind==EQ_SYSTEM) return d->dim;
    if(d->kind==EQ_SECOND) return 3;
    if(d->kind<=EQ_BERNOULLI) return 2;
    return 1;
}
const char *model_kind_name(int kind)
{
    static const char *const names[]={"Separable","Linear 1st","Bernoulli","General 1st",
        "Linear 2nd","N-th order","1st order system"};
    return (unsigned)kind<7 ? names[kind] : "Invalid type";
}
void model_variable_label(const Document *d,int variable,char *out,unsigned size)
{
    if(variable<0) snprintf(out,size,"x");
    else if(d->kind==EQ_SYSTEM) snprintf(out,size,"y%d",variable+1);
    else if(variable==0) snprintf(out,size,"y");
    else if(variable==1) snprintf(out,size,"y'");
    else if(variable==2) snprintf(out,size,"y''");
    else snprintf(out,size,"y(%d)",variable);
}
void model_equation_label(const Document *d,int eq,char *out,unsigned size)
{
    if(d->kind==EQ_SYSTEM) snprintf(out,size,"y%d'",eq+1);
    else if(d->kind==EQ_HIGHER) snprintf(out,size,"y(%d)",d->dim);
    else if(d->kind==EQ_GENERAL) snprintf(out,size,"y'");
    else snprintf(out,size,"%c(%c)",'f'+eq,d->kind==EQ_SEPARABLE && eq==1 ? 'y':'x');
}
OdeStatus model_validate(const Document *d)
{
    if(!d || d->kind<0 || d->kind>EQ_SYSTEM || d->dim<1 || d->dim>9
        || d->nic<0 || d->nic>9 || (d->kind<=EQ_GENERAL && d->dim!=1)
        || (d->kind==EQ_SECOND && d->dim!=2) || !isfinite(d->power)
        || (d->solver_custom!=0 && d->solver_custom!=1)) return ODE_BAD_INPUT;
    OdeStatus status=ode_validate(&d->solver);
    if(status!=ODE_OK) return status;
    const ViewWindow *v=&d->view;
    if(!isfinite(v->xmin) || !isfinite(v->xmax) || !isfinite(v->ymin)
        || !isfinite(v->ymax) || !isfinite(v->xmax-v->xmin) || !isfinite(v->ymax-v->ymin)
        || v->xmax<=v->xmin || v->ymax<=v->ymin || !isfinite(v->xscale)
        || !isfinite(v->yscale) || v->xscale<=0 || v->yscale<=0
        || (v->phase!=0 && v->phase!=1) || (v->grid!=0 && v->grid!=1)
        || (v->labels!=0 && v->labels!=1)
        || (v->phase && (d->dim<2 || v->phase_x<0 || v->phase_x>=d->dim
            || v->phase_y<0 || v->phase_y>=d->dim || v->phase_x==v->phase_y)))
        return ODE_BAD_INPUT;
    if(d->nic==0 && (d->kind>EQ_GENERAL || d->solver.sf==0)) return ODE_BAD_INPUT;
    for(int i=0;i<EXPR_CONSTANTS;i++) if(!isfinite(d->constants[i])) return ODE_BAD_INPUT;
    for(int i=0;i<9;i++) {
        if(!memchr(d->text[i],0,EXPR_TEXT)) return ODE_BAD_INPUT;
        if(d->graph_mask[i]>>d->dim || d->list_mask[i]>>(d->dim+1)) return ODE_BAD_INPUT;
        /* Validate inactive slots too: they can be re-enabled without parsing. */
        if(!isfinite(d->ic[i].x) || fabs(d->ic[i].x)>1e100) return ODE_BAD_INPUT;
        for(int j=0;j<9;j++) if(!isfinite(d->ic[i].y[j]) || fabs(d->ic[i].y[j])>1e100)
            return ODE_BAD_INPUT;
    }
    return ODE_OK;
}
ModelError model_compile(const Document *d,CompiledModel *m)
{
    ModelError error={.equation=-1,.values=model_validate(d)};
    if(error.values!=ODE_OK) return error;
    m->kind=d->kind;m->dim=d->dim;m->power=d->power;
    for(int i=0;i<model_equations(d);i++) {
        ExprScope scope={d->dim,d->kind==EQ_HIGHER,true,true,d->constants};
        if(d->kind==EQ_SEPARABLE) {scope.allow_x=i==0;scope.allow_y=i==1;}
        if(d->kind==EQ_LINEAR || d->kind==EQ_BERNOULLI || d->kind==EQ_SECOND)
            scope.allow_y=false;
        error.expression=expr_compile(d->text[i],scope,&m->eq[i]);
        if(error.expression.status!=EXPR_OK) {error.equation=i;return error;}
    }
    return error;
}
static OdeStatus evaluate(CompiledModel *m,int eq,double x,const double *y,double *out)
{
    ExprStatus s=expr_eval(&m->eq[eq],x,y,m->dim,out);
    if(s==EXPR_NONFINITE) return ODE_NONFINITE;
    return s==EXPR_OK ? ODE_OK : ODE_DOMAIN;
}
OdeStatus model_rhs(double x,const double *y,double *dy,void *ctx)
{
    CompiledModel *m=ctx;
    if(m->kind==EQ_SYSTEM) {
        for(int i=0;i<m->dim;i++) {
            OdeStatus s=evaluate(m,i,x,y,&dy[i]);if(s!=ODE_OK) return s;
        }
        return ODE_OK;
    }
    if(m->kind==EQ_HIGHER) {
        for(int i=0;i<m->dim-1;i++) dy[i]=y[i+1];
        return evaluate(m,0,x,y,&dy[m->dim-1]);
    }
    double a,b,c; OdeStatus s=evaluate(m,0,x,y,&a);
    if(s!=ODE_OK) return s;
    if(m->kind==EQ_GENERAL) {dy[0]=a;return ODE_OK;}
    s=evaluate(m,1,x,y,&b); if(s!=ODE_OK) return s;
    if(m->kind==EQ_SECOND) {
        s=evaluate(m,2,x,y,&c);if(s!=ODE_OK) return s;
        dy[0]=y[1];dy[1]=c-a*y[1]-b*y[0];
    } else if(m->kind==EQ_SEPARABLE) dy[0]=a*b;
    else if(m->kind==EQ_LINEAR) dy[0]=b-a*y[0];
    else dy[0]=b*pow(y[0],m->power)-a*y[0];
    for(int i=0;i<m->dim;i++) {
        if(isnan(dy[i])) return ODE_DOMAIN;
        if(!isfinite(dy[i])) return ODE_NONFINITE;
    }
    return ODE_OK;
}
bool model_convert_system(Document *d)
{
    if(d->kind!=EQ_HIGHER) return false;
    char text[ODE_MAX_DIM][EXPR_TEXT]={{0}};
    for(int i=0;i<d->dim-1;i++) snprintf(text[i],EXPR_TEXT,"y%d",i+2);
    if(!expr_to_system(d->text[0],text[d->dim-1],EXPR_TEXT)) return false;
    d->kind=EQ_SYSTEM;
    memcpy(d->text,text,sizeof(text));
    return true;
}
