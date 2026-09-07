#include "phase.h"
#include <float.h>
#include <math.h>
#include <string.h>

enum { ROOT_GRID=12, ROOT_ITERATIONS=24, EDGE_ITERATIONS=18 };
typedef struct {
    const CompiledModel *m;
    const ViewWindow *v;
    double x,span[2];
    OdeCancel cancel;
    void *cancel_ctx;
    unsigned evaluations;
    OdeStatus terminal;
    bool invalid;
} Analysis;

bool phase_supported(const CompiledModel *m)
{
    return m && m->kind==EQ_SYSTEM && m->dim==2
        && m->eq[0].length && m->eq[0].length<=EXPR_CODE
        && m->eq[1].length && m->eq[1].length<=EXPR_CODE;
}
bool phase_autonomous(const CompiledModel *m)
{return phase_supported(m) && !expr_uses_x(&m->eq[0]) && !expr_uses_x(&m->eq[1]);}

OdeStatus phase_vector(const CompiledModel *m,double x,const double y[2],double out[2])
{
    if(!phase_supported(m) || !y || !out || ode_values_status(&x,1)!=ODE_OK
        || ode_values_status(y,2)!=ODE_OK)return ODE_BAD_INPUT;
    double next[2];
    for(int k=0;k<2;k++) {
        ExprStatus s=expr_eval(&m->eq[k],x,y,2,&next[k]);
        if(s!=EXPR_OK)return s==EXPR_NONFINITE ? ODE_NONFINITE:ODE_DOMAIN;
    }
    OdeStatus s=ode_values_status(next,2);
    if(s==ODE_OK)memcpy(out,next,sizeof(next));
    return s;
}
static bool window_valid(const ViewWindow *v)
{
    if(!v)return false;
    double bounds[]={v->xmin,v->xmax,v->ymin,v->ymax};
    return ode_values_status(bounds,4)==ODE_OK && v->xmin<v->xmax
        && v->ymin<v->ymax && isfinite(v->xmax-v->xmin)
        && isfinite(v->ymax-v->ymin);
}
bool phase_direction(const ViewWindow *v,const double vector[2],double width,
    double height,double out[2])
{
    if(!window_valid(v) || !vector || !out || ode_values_status(vector,2)!=ODE_OK
        || !isfinite(width) || !isfinite(height) || width<=0 || height<=0
        || (!vector[0] && !vector[1]))return false;
    double span[]={v->xmax-v->xmin,v->ymax-v->ymin};
    double size[]={width,height},mantissa[2];int exponent[2];
    for(int k=0;k<2;k++) {
        int a,b,c;
        double ma=frexp(vector[k],&a),mb=frexp(span[k],&b),mc=frexp(size[k],&c);
        mantissa[k]=ma*mc/mb;exponent[k]=a+c-b;
    }
    int common=vector[0] && vector[1] ? (exponent[0]>exponent[1] ? exponent[0]:exponent[1])
        : exponent[vector[0] ? 0:1];
    double sx=ldexp(mantissa[0],exponent[0]-common);
    double sy=ldexp(mantissa[1],exponent[1]-common),norm=hypot(sx,sy);
    if(!isfinite(norm) || !norm)return false;
    out[0]=sx/norm;out[1]=-sy/norm;return true;
}
static Analysis analysis(const CompiledModel *m,double x,const ViewWindow *v,
    OdeCancel cancel,void *cancel_ctx)
{
    Analysis a={.m=m,.v=v,.x=x,.cancel=cancel,.cancel_ctx=cancel_ctx};
    if(!phase_supported(m) || !window_valid(v) || ode_values_status(&x,1)!=ODE_OK)
        a.terminal=ODE_BAD_INPUT;
    else {a.span[0]=v->xmax-v->xmin;a.span[1]=v->ymax-v->ymin;}
    return a;
}
static OdeStatus evaluate(Analysis *a,const double y[2],double f[2])
{
    if(a->terminal!=ODE_OK)return a->terminal;
    if(a->cancel && a->cancel(a->cancel_ctx))return a->terminal=ODE_CANCELLED;
    if(a->evaluations>=PHASE_EVAL_LIMIT)return a->terminal=ODE_WORK_LIMIT;
    a->evaluations++;
    OdeStatus s=phase_vector(a->m,a->x,y,f);
    if(s!=ODE_OK)a->invalid=true;
    return s;
}
static void grid_point(const Analysis *a,unsigned i,unsigned j,unsigned n,double p[2])
{
    p[0]=i==n ? a->v->xmax:a->v->xmin+a->span[0]*(double)i/n;
    p[1]=j==n ? a->v->ymax:a->v->ymin+a->span[1]*(double)j/n;
}
static bool same_sign(double a,double b)
{return (a>0 && b>0) || (a<0 && b<0);}
static void between(const double a[2],const double b[2],double t,double p[2])
{for(int k=0;k<2;k++)p[k]=(1-t)*a[k]+t*b[k];}

/* A sign change alone is not a root: insist on a large residual reduction.
   This rejects poles and finite jumps, including valid samples on both sides. */
static bool edge_root(Analysis *a,int component,const double p[2],const double q[2],
    double fp,double fq,double root[2])
{
    if(fp==0){memcpy(root,p,2*sizeof(double));return true;}
    if(fq==0){memcpy(root,q,2*sizeof(double));return true;}
    if(same_sign(fp,fq))return false;
    double lo[2]={p[0],p[1]},hi[2]={q[0],q[1]};
    double scale=fmax(fabs(fp),fabs(fq));
    for(int i=0;i<EDGE_ITERATIONS;i++) {
        double ratio=fabs(fp)/scale,other=fabs(fq)/scale;
        double t=ratio/(ratio+other);
        if(!isfinite(t) || t<1e-6 || t>1-1e-6 || i%4==3)t=.5;
        between(lo,hi,t,root);
        double f[2];if(evaluate(a,root,f)!=ODE_OK)return false;
        if(f[component]==0 || fabs(f[component])/scale<=1e-8)return true;
        if(same_sign(fp,f[component])) {memcpy(lo,root,sizeof(lo));fp=f[component];}
        else {memcpy(hi,root,sizeof(hi));fq=f[component];}
    }
    return false;
}
static bool segment_valid(Analysis *a,int k,const double p[2],const double q[2],double scale)
{
    for(int i=1;i<=3;i++) {
        double y[2],f[2];between(p,q,i*.25,y);
        if(evaluate(a,y,f)!=ODE_OK || (scale && fabs(f[k])/scale>.5))return false;
    }
    return true;
}
static OdeStatus emit_segment(Analysis *a,int k,const double p[2],const double q[2],
    double scale,PhaseSegment segment,void *ctx)
{
    if(segment_valid(a,k,p,q,scale) && segment && !segment(k,p,q,ctx))return ODE_SAMPLE_STOP;
    return a->terminal;
}
static OdeStatus contour_cell(Analysis *a,double p[4][2],double f[4][2],
    PhaseSegment segment,void *ctx)
{
    for(int k=0;k<2;k++) {
        double crossing[4][2],scale=0;int count=0;
        for(int i=0;i<4;i++)scale=fmax(scale,fabs(f[i][k]));
        if(!scale)continue; /* An identically zero cell is a region, not a line. */
        for(int i=0;i<4;i++) {
            int next=(i+1)%4;
            if(f[i][k]==0 && f[next][k]==0)continue;
            double point[2];
            if(!edge_root(a,k,p[i],p[next],f[i][k],f[next][k],point))continue;
            bool duplicate=false;
            for(int j=0;j<count;j++)if(fabs(point[0]-crossing[j][0])/a->span[0]<1e-12
                && fabs(point[1]-crossing[j][1])/a->span[1]<1e-12)duplicate=true;
            if(!duplicate){memcpy(crossing[count],point,sizeof(point));count++;}
        }
        if(a->terminal!=ODE_OK)return a->terminal;
        if(count==2) {
            OdeStatus s=emit_segment(a,k,crossing[0],crossing[1],scale,segment,ctx);
            if(s!=ODE_OK)return s;
        } else if(count==4) {
            double center[2],value[2];between(p[0],p[2],.5,center);
            if(evaluate(a,center,value)!=ODE_OK)continue;
            if(value[k]==0) {
                for(int i=0;i<4;i++) {
                    OdeStatus s=emit_segment(a,k,crossing[i],center,scale,segment,ctx);
                    if(s!=ODE_OK)return s;
                }
            } else {
                int other=same_sign(f[0][k],value[k]) ? 1:3;
                OdeStatus s=emit_segment(a,k,crossing[0],crossing[other],scale,segment,ctx);
                if(s!=ODE_OK)return s;
                s=emit_segment(a,k,crossing[2],crossing[4-other],scale,segment,ctx);
                if(s!=ODE_OK)return s;
            }
        }
    }
    return a->terminal;
}
OdeStatus phase_nullclines(const CompiledModel *m,double reference_x,
    const ViewWindow *v,unsigned grid,PhaseSegment segment,void *segment_ctx,
    OdeCancel cancel,void *cancel_ctx)
{
    Analysis a=analysis(m,reference_x,v,cancel,cancel_ctx);
    if(a.terminal!=ODE_OK)return a.terminal;
    if(grid<2 || grid>PHASE_GRID_MAX)return ODE_BAD_INPUT;
    double row[2][PHASE_GRID_MAX+1][2];bool valid[2][PHASE_GRID_MAX+1];
    for(unsigned j=0;j<=grid;j++) {
        unsigned current=j%2,previous=1-current;
        for(unsigned i=0;i<=grid;i++) {
            double p[2];grid_point(&a,i,j,grid,p);
            valid[current][i]=evaluate(&a,p,row[current][i])==ODE_OK;
            if(a.terminal!=ODE_OK)return a.terminal;
        }
        if(!j)continue;
        for(unsigned i=0;i<grid;i++) {
            if(!valid[previous][i] || !valid[previous][i+1]
                || !valid[current][i] || !valid[current][i+1])continue;
            double p[4][2],f[4][2];
            grid_point(&a,i,j-1,grid,p[0]);grid_point(&a,i+1,j-1,grid,p[1]);
            grid_point(&a,i+1,j,grid,p[2]);grid_point(&a,i,j,grid,p[3]);
            memcpy(f[0],row[previous][i],sizeof(f[0]));
            memcpy(f[1],row[previous][i+1],sizeof(f[1]));
            memcpy(f[2],row[current][i+1],sizeof(f[2]));
            memcpy(f[3],row[current][i],sizeof(f[3]));
            OdeStatus s=contour_cell(&a,p,f,segment,segment_ctx);
            if(s!=ODE_OK)return s;
        }
    }
    return a.terminal;
}

/* Two central differences detect unresolved derivatives and nondifferentiable
   corners. Their step scales with both coordinate magnitude and view span. */
static bool jacobian(Analysis *a,const double y[2],const double f[2],double j[4],double *error)
{
    *error=0;
    for(int col=0;col<2;col++) {
        double magnitude=fmax(fabs(y[col]),a->span[col]*.01);
        double h=fmin(cbrt(DBL_EPSILON)*magnitude,a->span[col]*.001);
        if(!h || !isfinite(h) || y[col]+h==y[col] || y[col]-h==y[col])return false;
        double p[2]={y[0],y[1]},q[2]={y[0],y[1]},fp[2],fm[2],fp2[2],fm2[2];
        p[col]=y[col]+h;q[col]=y[col]-h;
        double width=p[col]-q[col];
        if(evaluate(a,p,fp)!=ODE_OK || evaluate(a,q,fm)!=ODE_OK)return false;
        p[col]=y[col]+h*.5;q[col]=y[col]-h*.5;
        double width2=p[col]-q[col];
        if(p[col]==y[col] || q[col]==y[col]
            || evaluate(a,p,fp2)!=ODE_OK || evaluate(a,q,fm2)!=ODE_OK)return false;
        for(int row=0;row<2;row++) {
            double coarse=(fp[row]-fm[row])/width,fine=(fp2[row]-fm2[row])/width2;
            double bend=2*fabs((fp[row]-f[row])+(fm[row]-f[row]))/width;
            double bend2=2*fabs((fp2[row]-f[row])+(fm2[row]-f[row]))/width2;
            double noise=128*DBL_EPSILON*fmax(fabs(f[row]),fmax(fabs(fp[row]),fabs(fm[row])))/width;
            if(!isfinite(fine) || !isfinite(coarse) || fabs(fine)>1e100
                || (bend2>.8*bend && bend2>fmax(noise,1e-3*fabs(fine))))return false;
            double difference=fabs(fine-coarse);
            j[2*row+col]=fine;
            *error=fmax(*error,fmax(difference,noise));
        }
    }
    return true;
}
static void classify(PhaseRoot *root,double error)
{
    double scale=0;
    for(int i=0;i<4;i++)scale=fmax(scale,fabs(root->jacobian[i]));
    root->type=PHASE_INCONCLUSIVE;
    if(!scale)return;
    double a=root->jacobian[0]/scale,b=root->jacobian[1]/scale;
    double c=root->jacobian[2]/scale,d=root->jacobian[3]/scale;
    double tr=a+d,det=a*d-b*c,disc=(a-d)*(a-d)+4*b*c;
    double tolerance=fmax(256*DBL_EPSILON,16*error/scale);
    bool inconclusive=fabs(det)<=tolerance;
    if(disc<0) {
        root->eigen_real[0]=root->eigen_real[1]=tr*.5*scale;
        root->eigen_imag[0]=sqrt(-disc)*.5*scale;
        root->eigen_imag[1]=-root->eigen_imag[0];
        if(!inconclusive && disc< -tolerance)
            root->type=fabs(tr)<=tolerance ? PHASE_CENTER_NEUTRAL
                : tr<0 ? PHASE_STABLE_SPIRAL:PHASE_UNSTABLE_SPIRAL;
    } else {
        double delta=sqrt(fmax(0,disc));
        double q=.5*(tr+copysign(delta,tr)),r=q ? det/q:0;
        root->eigen_real[0]=q*scale;root->eigen_real[1]=r*scale;
        if(det< -tolerance)root->type=PHASE_SADDLE;
        else if(!inconclusive && fabs(q)>tolerance && fabs(r)>tolerance)
            root->type=tr<0 ? PHASE_STABLE_NODE:PHASE_UNSTABLE_NODE;
    }
    if(ode_values_status(root->eigen_real,2)!=ODE_OK
        || ode_values_status(root->eigen_imag,2)!=ODE_OK)root->type=PHASE_UNAVAILABLE;
}
static bool in_window(const Analysis *a,const double y[2])
{
    return y[0]>=a->v->xmin && y[0]<=a->v->xmax
        && y[1]>=a->v->ymin && y[1]<=a->v->ymax;
}
static double norm(const double f[2],const double scale[2])
{
    double n=0;
    for(int k=0;k<2;k++) {
        if(!scale[k]){if(f[k])return HUGE_VAL;}
        else n=fmax(n,fabs(f[k])/scale[k]);
    }
    return n;
}
static bool refine(Analysis *a,const double seed[2],PhaseRoot *root)
{
    double y[2]={seed[0],seed[1]},f[2];
    if(evaluate(a,y,f)!=ODE_OK)return false;
    for(int iteration=0;iteration<ROOT_ITERATIONS;iteration++) {
        double j[4],error;
        bool derivative=jacobian(a,y,f,j,&error);
        if(a->terminal!=ODE_OK)return false;
        if(f[0]==0 && f[1]==0) {
            memset(root,0,sizeof(*root));memcpy(root->y,y,sizeof(y));
            root->type=PHASE_UNAVAILABLE;
            if(derivative){memcpy(root->jacobian,j,sizeof(j));classify(root,error);}
            return true;
        }
        if(!derivative)return false;
        double scale[2]={fabs(j[0])*a->span[0]+fabs(j[1])*a->span[1],
            fabs(j[2])*a->span[0]+fabs(j[3])*a->span[1]};
        double residual=norm(f,scale);
        if(residual<1e-9) {
            memset(root,0,sizeof(*root));memcpy(root->y,y,sizeof(y));
            memcpy(root->jacobian,j,sizeof(j));root->residual=residual;classify(root,error);
            return true;
        }
        if(!isfinite(residual) || !scale[0] || !scale[1])return false;
        /* Row- and coordinate-scaled Newton solve avoids overflow and prevents
           an equation with small physical units being ignored. */
        double aa=j[0]*a->span[0]/scale[0],b=j[1]*a->span[1]/scale[0];
        double c=j[2]*a->span[0]/scale[1],d=j[3]*a->span[1]/scale[1];
        double det=aa*d-b*c;
        if(!isfinite(det) || fabs(det)<128*DBL_EPSILON)return false;
        double f0=f[0]/scale[0],f1=f[1]/scale[1];
        double delta[2]={(d*f0-b*f1)/det,(aa*f1-c*f0)/det};
        double largest=fmax(fabs(delta[0]),fabs(delta[1]));
        if(!isfinite(largest))return false;
        double damping=largest>.25 ? .25/largest:1;
        bool accepted=false;
        for(int trial=0;trial<10;trial++,damping*=.5) {
            double next[2]={y[0]-damping*delta[0]*a->span[0],
                y[1]-damping*delta[1]*a->span[1]},value[2];
            if(!in_window(a,next) || (next[0]==y[0] && next[1]==y[1]))continue;
            if(evaluate(a,next,value)!=ODE_OK) {if(a->terminal!=ODE_OK)return false;continue;}
            if(norm(value,scale)<residual) {
                memcpy(y,next,sizeof(y));memcpy(f,value,sizeof(f));accepted=true;break;
            }
        }
        if(!accepted)return false;
    }
    return false;
}
static void add_root(Analysis *a,PhaseResults *out,const double seed[2])
{
    PhaseRoot root;
    if(!refine(a,seed,&root))return;
    for(unsigned i=0;i<out->count;i++)if(fabs(root.y[0]-out->root[i].y[0])/a->span[0]<=1e-6
        && fabs(root.y[1]-out->root[i].y[1])/a->span[1]<=1e-6)return;
    if(out->count==PHASE_MAX_ROOTS){out->truncated=true;return;}
    out->root[out->count++]=root;
}
static OdeStatus discover(Analysis *a,PhaseResults *out)
{
    double row[2][ROOT_GRID+1][2];bool valid[2][ROOT_GRID+1];
    for(unsigned j=0;j<=ROOT_GRID;j++) {
        unsigned current=j%2,previous=1-current;
        for(unsigned i=0;i<=ROOT_GRID;i++) {
            double p[2];grid_point(a,i,j,ROOT_GRID,p);
            valid[current][i]=evaluate(a,p,row[current][i])==ODE_OK;
            if(a->terminal!=ODE_OK)return a->terminal;
            if(valid[current][i] && row[current][i][0]==0 && row[current][i][1]==0)
                add_root(a,out,p);
            if(a->terminal!=ODE_OK)return a->terminal;
            if(out->truncated)return ODE_OK;
        }
        if(!j)continue;
        for(unsigned i=0;i<ROOT_GRID;i++) {
            if(!valid[previous][i] || !valid[previous][i+1]
                || !valid[current][i] || !valid[current][i+1])continue;
            double center[2]={a->v->xmin+a->span[0]*(i+.5)/ROOT_GRID,
                a->v->ymin+a->span[1]*(j-.5)/ROOT_GRID},fc[2];
            if(evaluate(a,center,fc)!=ODE_OK)continue;
            double *corner[]={row[previous][i],row[previous][i+1],row[current][i+1],row[current][i]};
            bool candidate=true;
            for(int k=0;k<2;k++) {
                double lo=fc[k],hi=fc[k],small=fabs(fc[k]),large=small;
                for(int n=0;n<4;n++) {
                    lo=fmin(lo,corner[n][k]);hi=fmax(hi,corner[n][k]);
                    small=fmin(small,fabs(corner[n][k]));large=fmax(large,fabs(corner[n][k]));
                }
                if(!(lo<=0 && hi>=0) && !(small<=.15*large))candidate=false;
            }
            if(candidate)add_root(a,out,center);
            if(a->terminal!=ODE_OK)return a->terminal;
            if(out->truncated)return ODE_OK;
        }
    }
    return a->terminal;
}
OdeStatus phase_equilibria(const CompiledModel *m,double reference_x,
    const ViewWindow *v,PhaseResults *out,OdeCancel cancel,void *cancel_ctx)
{
    Analysis a=analysis(m,reference_x,v,cancel,cancel_ctx);
    if(a.terminal!=ODE_OK)return a.terminal;
    if(!out || !phase_autonomous(m))return ODE_BAD_INPUT;
    PhaseResults next={0};
    OdeStatus status=discover(&a,&next);
    if(status!=ODE_OK)return status;
    if(cancel && cancel(cancel_ctx))return ODE_CANCELLED;
    next.evaluations=a.evaluations;next.has_invalid=a.invalid;
    /* Stable order independent of which adjacent cell first converged. */
    for(unsigned i=1;i<next.count;i++) {
        PhaseRoot p=next.root[i];unsigned j=i;
        while(j && (next.root[j-1].y[0]>p.y[0]
            || (next.root[j-1].y[0]==p.y[0] && next.root[j-1].y[1]>p.y[1]))) {
            next.root[j]=next.root[j-1];j--;
        }
        next.root[j]=p;
    }
    *out=next;return ODE_OK;
}
const char *phase_type_name(PhaseType type)
{
    static const char *const names[]={"Unavailable","Saddle","Stable Node","Unstable Node",
        "Stable Spiral","Unstable Spiral","Center / Neutral candidate","Inconclusive"};
    return (unsigned)type<sizeof(names)/sizeof(*names) ? names[type]:names[0];
}
