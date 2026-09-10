#include "storage.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static App saved,cold;
static Document expected,recall;
static void preferences(Document *d,int kind,int dim,int method)
{
    model_defaults(d,kind,dim);
    if(kind<=EQ_GENERAL)d->nic=10;
    d->ic_enabled=method ? 0:0x155;d->power=3.25;d->solver=(OdeSettings){-2,3,.025,12345,7,50};
    d->solver_custom=1;d->adaptive=(OdeAdaptive){method,2e-7,3e-10};
    d->view.xmin=-8;d->view.xmax=11;d->view.ymin=-7;d->view.ymax=13;
    d->view.xscale=2;d->view.yscale=3;d->view.grid=0;d->view.labels=0;
    d->phase_view.xmin=-4;d->phase_view.xmax=9;d->phase_view.ymin=-5;d->phase_view.ymax=8;
    d->phase_view.xscale=.5;d->phase_view.yscale=2;d->phase_view.grid=0;
    d->phase_field=0;d->phase_nullclines=1;d->phase_ready=1;
    d->view.phase=model_phase_supported(d);
    d->enabled=(uint16_t)(0x155u&((1u<<d->dim)-1));
    d->field_style=FIELD_SEGMENT;d->field_color=5;
    d->event.enabled=1;d->event.direction=EVENT_FALLING;d->event.action=EVENT_STOP;
    strcpy(d->event.text,"x-0.75");
    for(int f=0;f<ODE_MAX_IC;f++) {
        d->ic[f].x=.125;
        for(int j=0;j<ODE_MAX_DIM;j++) {
            d->ic[f].y[j]=(f+1)*.25+j*.125;
            d->color[f][j]=(uint8_t)((f+2*j)%6);
        }
    }
    /* Inactive editable text is persistent too, without becoming active input. */
    for(int j=model_equations(d);j<ODE_MAX_DIM;j++)snprintf(d->text[j],EXPR_TEXT,"x+%d",j);
    assert(model_validate(d)==ODE_OK);
}
int main(void)
{
    char directory[]="preferences-test-XXXXXX";assert(mkdtemp(directory));
    unsigned cases=0;
    for(int kind=EQ_SEPARABLE;kind<=EQ_SYSTEM;kind++) {
        int limit=kind>=EQ_HIGHER ? 9:1;
        for(int dim=1;dim<=limit;dim++)for(int method=ODE_RK4;method<=ODE_RK45;method++) {
            preferences(&saved.doc,kind,dim,method);
            preferences(&saved.recall,EQ_GENERAL,1,1-method);
            saved.recall.ic[9].y[0]=19;saved.recall.color[9][0]=4;
            saved.has_recall=true;expected=saved.doc;recall=saved.recall;
            assert(storage_save(&saved,directory));
            memset(&cold,0xa5,sizeof(cold));app_initialize(&cold,directory);
            assert(!cold.has_recall && !solver_report()->valid); /* No automatic RCL. */
            assert(storage_load(&cold,directory) && cold.has_recall);
            assert(!memcmp(&cold.doc,&expected,sizeof(expected)));
            assert(!memcmp(&cold.recall,&recall,sizeof(recall)));
            assert(!solver_report()->valid); /* Runtime diagnostics are not a saved preference. */
            ModelError e=model_compile(&cold.doc,&cold.model);
            assert(e.values==ODE_OK && e.expression.status==EXPR_OK);
            cases++;
        }
    }
    /* Failure cannot replace either live document; the load union is scratch. */
    for(int i=0;i<2;i++) {
        char path[256];snprintf(path,sizeof(path),"%s/DIFFEQ%d.dat",directory,i);
        FILE *f=fopen(path,"wb");assert(f);fputs("truncated",f);assert(!fclose(f));
    }
    expected=cold.doc;recall=cold.recall;
    assert(!storage_load(&cold,directory));
    assert(!memcmp(&cold.doc,&expected,sizeof(expected)) && !memcmp(&cold.recall,&recall,sizeof(recall)));
    for(int i=0;i<2;i++){char path[256];snprintf(path,sizeof(path),"%s/DIFFEQ%d.dat",directory,i);assert(!remove(path));}
    assert(!rmdir(directory));
    printf("Full preference matrix: %u mode/dimension/method cold-RCL cases; all Document bytes, inactive slots and failure preservation PASS.\n",cases);
}
