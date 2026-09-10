/* Equivalence-class mathematical audit: independently known solutions and
   state/index checks, through the actual compiled model and selected solver. */
#include "initial.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static Document d,original;
static CompiledModel m;
static unsigned comparisons;
static double largest_error[2];
static void setup(EquationKind kind,int dimension,int method)
{
    model_defaults(&d,kind,dimension);d.adaptive.method=method;
    d.adaptive.reltol=1e-9;d.adaptive.abstol=1e-12;
    d.solver.h=method==ODE_RK4 ? .005:.4;
    d.solver.xmin=-2;d.solver.xmax=2;d.solver.sf=0;d.solver_custom=1;
}
static void compile(void)
{
    ModelError e=model_compile(&d,&m);
    assert(e.values==ODE_OK && e.expression.status==EXPR_OK);
}
static OdeResult solve(int family,double target)
{
    compile();OdeResult r=model_value_at(&d,&m,family,target,NULL,NULL);
    assert(r.status==ODE_OK && r.x==target && ode_values_status(r.y,d.dim)==ODE_OK);
    assert(m.work.accepted+m.work.rejected<=ODE_ATTEMPT_BUDGET);
    assert(m.work.rhs<=ODE_RHS_BUDGET && m.work.work<=ODE_RHS_WORK_BUDGET);
    if(d.adaptive.method==ODE_RK45) {
        assert(r.steps==m.work.accepted+m.work.rejected);
        assert(m.work.rhs==7*r.steps);
    } else assert(r.steps==m.work.accepted && m.work.rejected==0 && m.work.rhs==4*r.steps);
    return r;
}
static void close_to(double actual,double expected,double tolerance)
{
    double error=fabs(actual-expected);
    if(!(error<=tolerance))fprintf(stderr,"method%d actual%.17g expected%.17g error%.9g limit%.9g\n",
        d.adaptive.method,actual,expected,error,tolerance);
    assert(error<=tolerance);comparisons++;
    largest_error[d.adaptive.method]=fmax(largest_error[d.adaptive.method],error);
}
static void analytical_matrix(int method)
{
    /* A-C: increasing/decreasing/inhomogeneous scalar equations. */
    const char *scalar[]={"y","-2*y","x"};
    for(int equation=0;equation<3;equation++)for(int side=-1;side<=1;side+=2) {
        setup(EQ_GENERAL,1,method);strcpy(d.text[0],scalar[equation]);
        double x=side*.873;OdeResult r=solve(0,x);
        close_to(r.y[0],equation==2 ? 1+x*x/2:exp((equation==1 ? -2:1)*x),3e-8);
    }
    /* D-E: y''+y=0 and y''-y=0, both full state components. */
    for(int sign=-1;sign<=1;sign+=2)for(int side=-1;side<=1;side+=2) {
        setup(EQ_SECOND,2,method);snprintf(d.text[1],EXPR_TEXT,"%d",sign);
        double x=side*1.173;OdeResult r=solve(0,x);
        close_to(r.y[0],sign==1 ? cos(x):cosh(x),3e-8);
        close_to(r.y[1],sign==1 ? -sin(x):sinh(x),3e-8);
    }
    /* Nonhomogeneous Second input coefficients use y''+f y'+g y=h. */
    setup(EQ_SECOND,2,method);strcpy(d.text[0],"2");strcpy(d.text[1],"3");
    strcpy(d.text[2],"6*exp(x)");d.ic[0].y[1]=1;
    for(int side=-1;side<=1;side+=2) {
        double x=side*.737;OdeResult r=solve(0,x);
        for(int j=0;j<2;j++)close_to(r.y[j],exp(x),3e-8);
    }
    /* F: all n=1..9 constant-coefficient equations y^(n)=y. */
    for(int n=1;n<=9;n++)for(int side=-1;side<=1;side+=2) {
        setup(EQ_HIGHER,n,method);assert(d.dim==n && d.nic==1);
        strcpy(d.text[0],"y");for(int j=0;j<n;j++)d.ic[0].y[j]=1;
        double x=side*.937;OdeResult r=solve(0,x);
        for(int j=0;j<n;j++)close_to(r.y[j],exp(x),3e-8);
    }
    /* Highest initial derivative anchors a ninth-order polynomial. */
    setup(EQ_HIGHER,9,method);memset(d.ic[0].y,0,sizeof(d.ic[0].y));d.ic[0].y[8]=1;
    for(int side=-1;side<=1;side+=2) {
        double x=side*1.137;OdeResult r=solve(0,x);
        for(int j=0;j<9;j++) {
            double exact=1;for(int k=1;k<=8-j;k++)exact*=x/k;
            close_to(r.y[j],exact,3e-9);
        }
    }
    /* SYS1..9: unequal components/rates expose swapped/dropped state indices. */
    for(int n=1;n<=9;n++)for(int side=-1;side<=1;side+=2) {
        setup(EQ_SYSTEM,n,method);assert(d.dim==n && d.nic==1);
        for(int j=0;j<n;j++) {
            snprintf(d.text[j],EXPR_TEXT,"%d*y%d/10",j%2 ? -j-1:j+1,j+1);
            d.ic[0].y[j]=j+1;
        }
        double x=side*.937;OdeResult r=solve(0,x);
        for(int j=0;j<n;j++)close_to(r.y[j],(j+1)*exp((j%2 ? -j-1:j+1)*x/10),3e-8);
    }
    /* G: coupled harmonic system, independent of Second's coefficient path. */
    setup(EQ_SYSTEM,2,method);
    for(int side=-1;side<=1;side+=2) {
        double x=side*1.173;OdeResult r=solve(0,x);
        close_to(r.y[0],cos(x),3e-8);close_to(r.y[1],-sin(x),3e-8);
        close_to(hypot(r.y[0],r.y[1]),1,3e-8);
    }
}
static void subtype_mapping(int method)
{
    for(int kind=EQ_SEPARABLE;kind<=EQ_GENERAL;kind++) {
        setup((EquationKind)kind,1,method);d.ic[0].y[0]=1;
        if(kind==EQ_SEPARABLE){strcpy(d.text[0],"0.5");strcpy(d.text[1],"y");}
        if(kind==EQ_LINEAR){strcpy(d.text[0],"-0.5");strcpy(d.text[1],"0");}
        if(kind==EQ_BERNOULLI){strcpy(d.text[0],"0.25");strcpy(d.text[1],"0.75");d.power=1;}
        if(kind==EQ_GENERAL)strcpy(d.text[0],"0.5*y");
        for(int side=-1;side<=1;side+=2) {
            double x=side*.731;OdeResult r=solve(0,x);close_to(r.y[0],exp(.5*x),3e-8);
        }
        compile();double y=3,dy=0;assert(model_rhs(.37,&y,&dy,&m)==ODE_OK && dy==1.5);
    }
    setup(EQ_BERNOULLI,1,method);strcpy(d.text[0],"1");strcpy(d.text[1],"2");d.power=0;
    for(int side=-1;side<=1;side+=2) {
        double x=side*.731;OdeResult r=solve(0,x);close_to(r.y[0],2-exp(-x),3e-8);
    }
    /* n=2 nonlinear logistic path, including a noninteger-domain rejection. */
    setup(EQ_BERNOULLI,1,method);
    for(int side=-1;side<=1;side+=2) {
        double x=side*.731;OdeResult r=solve(0,x);close_to(r.y[0],2/(1+exp(-2*x)),3e-8);
    }
    d.power=.5;compile();double y=-1,dy=37;
    assert(model_rhs(0,&y,&dy,&m)==ODE_DOMAIN);
}
static void indexing_and_conversion(int method)
{
    for(int n=1;n<=9;n++) {
        setup(EQ_HIGHER,n,method);strcpy(d.text[0],"x+y");
        for(int j=1;j<n;j++) {
            char term[20];snprintf(term,sizeof(term),"+%d*Y%d",j+1,j);strcat(d.text[0],term);
        }
        double y[9],dy[9],expected=.25;
        for(int j=0;j<n;j++){y[j]=(j+1)*.1;d.ic[0].y[j]=y[j];expected+=(j+1)*y[j];}
        compile();assert(model_rhs(.25,y,dy,&m)==ODE_OK);
        for(int j=0;j<n-1;j++)assert(dy[j]==y[j+1]);
        close_to(dy[n-1],expected,2e-14);
        char outside[20];snprintf(outside,sizeof(outside),"Y%d",n);original=d;
        strcpy(d.text[0],outside);assert(model_compile(&d,&m).expression.status==EXPR_VARIABLE);d=original;
        OdeResult higher=solve(0,.137);original=d;assert(model_convert_system(&d));
        assert(d.dim==n && d.nic==1 && !memcmp(d.ic,original.ic,sizeof(d.ic)));
        OdeResult system=solve(0,.137);
        for(int j=0;j<n;j++)close_to(system.y[j],higher.y[j],2e-14);
        assert(model_rhs(.25,y,dy,&m)==ODE_OK);close_to(dy[n-1],expected,2e-14);
        for(int j=0;j<n-1;j++)assert(dy[j]==y[j+1]);
    }
}
static void multiple_ic(int method)
{
    const unsigned counts[]={1,2,5,10};
    for(unsigned k=0;k<sizeof(counts)/sizeof(counts[0]);k++) {
        setup(EQ_GENERAL,1,method);strcpy(d.text[0],"-2*y");d.ic[0].x=.25;
        InitialValues values={.count=counts[k]};
        for(unsigned f=0;f<values.count;f++)values.value[f]=(f+1)*.125;
        initial_values_apply(&d,&values);assert(d.nic==(int)values.count);
        for(int f=0;f<d.nic;f++) {
            assert(d.ic[f].x==.25 && d.ic[f].y[0]==values.value[f]);
            for(int side=-1;side<=1;side+=2) {
                double x=side*.713;OdeResult r=solve(f,x);
                close_to(r.y[0],values.value[f]*exp(-2*(x-.25)),3e-8);
            }
        }
    }
}
static void domain_checks(int method)
{
    setup(EQ_GENERAL,1,method);strcpy(d.text[0],"tan(x)");d.ic[0].y[0]=0;
    for(int side=-1;side<=1;side+=2) {
        double x=side*.75;OdeResult r=solve(0,x);close_to(r.y[0],-log(cos(x)),3e-8);
    }
    const char *rhs[]={"ln(x)","log(x)","sqrt(x)","1/(x-0.5)","exp(1000*x)"};
    for(unsigned i=0;i<sizeof(rhs)/sizeof(rhs[0]);i++) {
        setup(EQ_GENERAL,1,method);strcpy(d.text[0],rhs[i]);d.ic[0].x=1;d.ic[0].y[0]=0;
        if(i==4)d.ic[0].x=0;
        /* Exact binary stages hit this known pole. Arbitrary fixed-step grids
           cannot certify every missed pole (documented audit limitation). */
        if(i==3 && method==ODE_RK4)d.solver.h=.125;
        if(i<3)for(int side=-1;side<=1;side+=2) {
            double x=1+side*.25;
            double exact=i<2 ? x*log(x)-x+1:2*(pow(x,1.5)-1)/3;
            if(i==1)exact/=log(10);
            OdeResult r=solve(0,x);close_to(r.y[0],exact,3e-8);
        }
        compile();model_work_begin(&m);
        OdeResult r=model_value_at(&d,&m,0,i==4 ? 1:-1,NULL,NULL);
        assert(r.status!=ODE_OK && r.status!=ODE_BAD_INPUT);
        OdeStatus expected=i<3 ? ODE_DOMAIN:(i==4 ? ODE_DIVERGED:
            (method==ODE_RK4 ? ODE_DOMAIN:ODE_STEP_UNDERFLOW));
        assert(r.status==expected);
        assert(ode_values_status(r.y,d.dim)==ODE_OK);
        if(i<3)assert(r.x>=0 && r.x<=1);
        if(i==3)assert(r.x>.5 && r.x<=1);
        if(i==4)assert(r.x>=0 && r.x<1);
        if(method==ODE_RK45)assert(r.steps==m.work.accepted+m.work.rejected);
        printf("domain method%d %-12s x%.17g status%s A%u R%u RHS%u\n",method,rhs[i],
            r.x,ode_status_text(r.status),m.work.accepted,m.work.rejected,m.work.rhs);
    }
}
int main(void)
{
    for(int method=ODE_RK4;method<=ODE_RK45;method++) {
        analytical_matrix(method);subtype_mapping(method);indexing_and_conversion(method);
        multiple_ic(method);domain_checks(method);
    }
    printf("Full math matrix: %u analytic/mapping comparisons; maximum absolute error RK4 %.9g / RK45 %.9g.\n",
        comparisons,largest_error[0],largest_error[1]);
    puts("First4/Second/Higher1..9/SYS1..9, both solvers/directions, exact endpoints, full IC/derivative mapping, conversion, 1/2/5/10 IC and valid-domain terminals passed.");
}
