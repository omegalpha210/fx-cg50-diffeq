/* Host CPU measurements of the actual bounded algorithms, not LCD timing. */
#include "phase.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

enum { FIELD_REPETITIONS=100, ANALYSIS_REPETITIONS=20 };
static Document document;
static CompiledModel model;
static double direction_checksum;

static bool count_poll(void *context)
{
    unsigned *polls=context;(*polls)++;return false;
}
static double milliseconds(clock_t start,unsigned repetitions)
{
    clock_t finish=clock();assert(start!=(clock_t)-1 && finish!=(clock_t)-1);
    return 1000.0*(double)(finish-start)/CLOCKS_PER_SEC/repetitions;
}
static void prepare(const char *first,const char *second,double minimum,double maximum)
{
    model_defaults(&document,EQ_SYSTEM,2);
    strcpy(document.text[0],first);strcpy(document.text[1],second);
    document.phase_view.xmin=document.phase_view.ymin=minimum;
    document.phase_view.xmax=document.phase_view.ymax=maximum;
    ModelError error=model_compile(&document,&model);
    assert(error.values==ODE_OK && error.expression.status==EXPR_OK);
}
static void measure_field(unsigned columns)
{
    const ViewWindow *view=&document.phase_view;
    const double width=383,height=197;
    unsigned rows=(unsigned)ceil(columns*height/width);
    unsigned evaluations=0,directions=0,zeros=0,invalid=0,polls=0;
    clock_t start=clock();
    for(unsigned repetition=0;repetition<FIELD_REPETITIONS;repetition++) {
        for(unsigned column=0;column<columns;column++) {
            count_poll(&polls);
            for(unsigned row=0;row<rows;row++) {
                double y[2]={view->xmin+(column+.5)/columns*(view->xmax-view->xmin),
                    view->ymin+(row+.5)/rows*(view->ymax-view->ymin)};
                double vector[2],unit[2];evaluations++;
                if(phase_vector(&model,0,y,vector)!=ODE_OK){invalid++;continue;}
                if(phase_direction(view,vector,width,height,unit)) {
                    directions++;direction_checksum+=unit[0]+unit[1];
                } else zeros++;
            }
        }
    }
    double elapsed=milliseconds(start,FIELD_REPETITIONS);
    printf("  FIELD %ux%u: %.6f ms/pass; vector evaluations=%u (expressions=%u), "
        "directions=%u zero/invalid=%u/%u cancel polls=%u (%u runs)\n",
        columns,rows,elapsed,evaluations/FIELD_REPETITIONS,
        2*evaluations/FIELD_REPETITIONS,directions/FIELD_REPETITIONS,
        zeros/FIELD_REPETITIONS,invalid/FIELD_REPETITIONS,
        polls/FIELD_REPETITIONS,FIELD_REPETITIONS);
}
static bool count_segment(int component,const double first[2],const double second[2],void *context)
{
    (void)component;(void)first;(void)second;
    unsigned *segments=context;(*segments)++;return true;
}
static void measure_nullclines(unsigned grid,bool validation)
{
    unsigned polls=0,segments=0,completed=0,evaluations=0;
    OdeStatus status=ODE_OK;clock_t start=clock();
    for(unsigned repetition=0;repetition<ANALYSIS_REPETITIONS;repetition++) {
        unsigned pass_polls=0;
        status=phase_nullclines(&model,0,&document.phase_view,grid,
            validation ? NULL:count_segment,&segments,count_poll,&pass_polls);
        assert(status==ODE_OK || status==ODE_WORK_LIMIT);
        polls+=pass_polls;
        /* Each evaluation polls first; a budget rejection has one extra poll. */
        evaluations+=pass_polls-(status==ODE_WORK_LIMIT);
        if(status==ODE_OK)completed++;
    }
    double elapsed=milliseconds(start,ANALYSIS_REPETITIONS);
    printf("  NULL %ux%u %s: %.6f ms/pass; vector evaluations=%u "
        "cancel polls=%u segments=%u status=%s completed=%u/%u\n",
        grid,grid,validation ? "preflight":"segments",elapsed,
        evaluations/ANALYSIS_REPETITIONS,polls/ANALYSIS_REPETITIONS,
        segments/ANALYSIS_REPETITIONS,ode_status_text(status),completed,ANALYSIS_REPETITIONS);
}
static void measure_equilibria(void)
{
    unsigned polls=0,evaluations=0,completed=0;
    PhaseResults results={0};OdeStatus status=ODE_OK;clock_t start=clock();
    for(unsigned repetition=0;repetition<ANALYSIS_REPETITIONS;repetition++) {
        status=phase_equilibria(&model,0,&document.phase_view,&results,count_poll,&polls);
        assert(status==ODE_OK);
        assert(results.evaluations<=PHASE_EVAL_LIMIT);
        evaluations+=results.evaluations;completed++;
    }
    double elapsed=milliseconds(start,ANALYSIS_REPETITIONS);
    printf("  EQPT: %.6f ms/pass; vector evaluations=%u cancel polls=%u "
        "roots=%u truncated=%d invalid=%d status=%s completed=%u/%u\n",
        elapsed,evaluations/ANALYSIS_REPETITIONS,polls/ANALYSIS_REPETITIONS,
        results.count,(int)results.truncated,(int)results.has_invalid,
        ode_status_text(status),completed,ANALYSIS_REPETITIONS);
}
static void measure_system(const char *name)
{
    printf("%s: y1'=%s; y2'=%s; view=[%g,%g]^2\n",name,
        document.text[0],document.text[1],document.phase_view.xmin,document.phase_view.xmax);
    const unsigned field_columns[]={15,20,30},contour_grids[]={10,15,20};
    for(unsigned i=0;i<3;i++)measure_field(field_columns[i]);
    for(unsigned i=0;i<3;i++) {
        measure_nullclines(contour_grids[i],true);
        measure_nullclines(contour_grids[i],false);
    }
    measure_equilibria();
}
int main(void)
{
    puts("Host CPU with actual Phase evaluator/direction/contour/root algorithms; no raster/LCD or RK4 work.");
    printf("clock resolution=%g us; evaluation cap=%u; max roots=%u; PhaseResults=%zu bytes.\n",
        1e6/CLOCKS_PER_SEC,PHASE_EVAL_LIMIT,PHASE_MAX_ROOTS,sizeof(PhaseResults));
    prepare("y2","-y1",-2,2);measure_system("Oscillator");
    prepare("y1*(1-y1-y2)","y2*(0.5-y1)",-.25,1.25);measure_system("Nonlinear example");
    printf("Direction checksum=%.12g. HARDWARE TEST REQUIRED: SH/LCD latency, key response and colors.\n",
        direction_checksum);
    return 0;
}
