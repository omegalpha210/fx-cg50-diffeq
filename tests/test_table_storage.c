#include "table.h"
#include "storage.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static App a,b;
static bool cancelled(void *ctx){(void)ctx;return true;}
int main(void)
{
    char empty_directory[]="lifecycle-test-XXXXXX";assert(mkdtemp(empty_directory));
    memset(&a,0xa5,sizeof(a));
    app_initialize(&a,empty_directory);
    assert(a.doc.kind==EQ_GENERAL && a.doc.dim==1 && !a.has_recall);
    assert(!strcmp(a.doc.text[0],"sin(x)-y"));
    memset(&a,0x5a,sizeof(a));
    app_initialize(&a,empty_directory);
    assert(a.doc.kind==EQ_GENERAL && a.doc.dim==1 && !a.has_recall);
    assert(rmdir(empty_directory)==0);

    model_defaults(&a.doc,EQ_GENERAL,1);strcpy(a.doc.text[0],"y");
    a.doc.solver.h=.1;a.doc.solver.step=2;
    ModelError e=model_compile(&a.doc,&a.model);assert(e.values==ODE_OK && e.expression.status==EXPR_OK);
    TablePage page;table_page(&a.doc,&a.model,0,1,0,&page,NULL,NULL);
    assert(page.count==7 && page.more && page.result.status==ODE_SAMPLE_STOP);
    assert(page.row[0][0]==0 && page.row[0][1]==1);
    assert(fabs(page.row[6][0]-1.2)<1e-12 && fabs(page.row[6][1]-exp(1.2))<4e-6);
    table_page(&a.doc,&a.model,0,1,7,&page,NULL,NULL);
    assert(fabs(page.row[0][0]-1.4)<1e-12);
    table_page(&a.doc,&a.model,0,-1,0,&page,NULL,NULL);
    assert(fabs(page.row[1][0]+.2)<1e-12 && fabs(page.row[1][1]-exp(-.2))<2e-6);
    OdeStatus bottom_status;unsigned bottom=table_last_page(&a.doc,&a.model,0,-1,
        &bottom_status,NULL,NULL);
    assert(bottom_status==ODE_OK && bottom==28);
    strcpy(a.doc.text[0],"-y");a.doc.ic[0].y[0]=1;
    e=model_compile(&a.doc,&a.model);assert(e.values==ODE_OK && e.expression.status==EXPR_OK);
    table_page(&a.doc,&a.model,0,1,0,&page,NULL,NULL);
    assert(page.count==7 && fabs(page.row[0][1]-1)<1e-15);
    assert(fabs(page.row[1][0]-.2)<1e-12 && fabs(page.row[1][1]-exp(-.2))<2e-6);
    strcpy(a.doc.text[0],"y");a.doc.ic[0].y[0]=1;
    e=model_compile(&a.doc,&a.model);assert(e.values==ODE_OK && e.expression.status==EXPR_OK);
    a.doc.solver.max_steps=3;table_page(&a.doc,&a.model,0,1,0,&page,NULL,NULL);
    assert(page.count==2 && page.result.status==ODE_STEP_LIMIT);
    a.doc.solver.max_steps=20000;
    char directory[]="storage-test-XXXXXX";assert(mkdtemp(directory));
    a.recall=a.doc;a.has_recall=true;assert(storage_save(&a,directory));
    strcpy(a.doc.text[0],"-y");a.doc.ic[0].y[0]=3;assert(storage_save(&a,directory));
    assert(storage_load(&b,directory));assert(!strcmp(b.doc.text[0],"-y") && b.doc.ic[0].y[0]==3);
    assert(!strcmp(b.recall.text[0],"y"));
    char path[256];snprintf(path,sizeof(path),"%s/DIFFEQ1.dat",directory);
    FILE *f=fopen(path,"wb");assert(f);fputs("interrupted write",f);fclose(f);
    assert(storage_load(&b,directory) && !strcmp(b.doc.text[0],"y"));
    assert(storage_save(&a,directory));assert(storage_load(&b,directory) && !strcmp(b.doc.text[0],"-y"));
    strcpy(b.doc.text[0],"current-must-survive");
    for(int slot=0;slot<2;slot++) {
        snprintf(path,sizeof(path),"%s/DIFFEQ%d.dat",directory,slot);
        f=fopen(path,"wb");assert(f);fputs("broken slot",f);fclose(f);
    }
    assert(!storage_load(&b,directory));
    assert(!strcmp(b.doc.text[0],"current-must-survive"));
    e=model_compile(&a.doc,&a.model);assert(e.values==ODE_OK && e.expression.status==EXPR_OK);
    OdeStatus status;assert(storage_csv(&a.doc,&a.model,directory,path,sizeof(path),&status,NULL,NULL));
    f=fopen(path,"r");assert(f);char line[256];assert(fgets(line,sizeof(line),f));
    assert(!strcmp(line,"family,direction,step,x,y\n"));
    assert(fgets(line,sizeof(line),f) && !strcmp(line,"1,-1,0,0,3\n"));fclose(f);
    char stat[256];assert(storage_stat_csv(&a.doc,&a.model,0,1,directory,stat,sizeof(stat),
        &status,NULL,NULL));
    f=fopen(stat,"r");assert(f);assert(fgets(line,sizeof(line),f));
    assert(!strcmp(line,"'x,'y\n"));assert(fgets(line,sizeof(line),f) && !strcmp(line,"0,3\n"));fclose(f);
    a.doc.solver.h=.005;a.doc.solver.step=1;char overflow[256];
    assert(!storage_stat_csv(&a.doc,&a.model,0,1,directory,overflow,sizeof(overflow),
        &status,NULL,NULL));
    assert(status==ODE_STEP_LIMIT && access(overflow,F_OK)!=0);
    a.doc.solver.h=.1;a.doc.solver.step=2;
    char second[256];assert(storage_csv(&a.doc,&a.model,directory,second,sizeof(second),&status,NULL,NULL));
    assert(strcmp(path,second));
    char incomplete[256];assert(!storage_csv(&a.doc,&a.model,directory,incomplete,sizeof(incomplete),&status,cancelled,NULL));
    assert(status==ODE_CANCELLED && access(incomplete,F_OK)!=0);
    remove(path);remove(second);remove(stat);
    for(int i=0;i<2;i++){snprintf(path,sizeof(path),"%s/DIFFEQ%d.dat",directory,i);remove(path);}
    rmdir(directory);
    printf("Table paging/Step/backward/partial checks and storage recovery/CSV checks passed. TablePage=%zu bytes.\n",sizeof(page));
    return 0;
}
