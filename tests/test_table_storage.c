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
static bool after_create(void *ctx)
{const char *path=ctx;return path[0] && access(path,F_OK)==0;}
int main(void)
{
    char directory[]="storage-test-XXXXXX";assert(mkdtemp(directory));
    memset(&a,0xa5,sizeof(a));app_initialize(&a,directory);
    assert(a.doc.kind==EQ_GENERAL && a.doc.nic==1 && !a.has_recall);
    assert(a.doc.field_style==FIELD_ARROW && a.doc.field_color==0);
    strcpy(a.doc.text[0],"y");a.doc.solver.h=.1;a.doc.solver.step=2;
    assert(model_compile(&a.doc,&a.model).expression.status==EXPR_OK);
    TableIndex index;TablePage page;
    assert(table_index_build(&a.doc,&a.model,&index,NULL,NULL)==ODE_OK && index.total==61);
    assert(sizeof(index)<512 && table_x_at(&index,index.mid+3)==0);
    table_read_page(&a.doc,&a.model,&index,index.mid,&page,NULL,NULL);
    assert(page.count==7 && page.more && page.result.status==ODE_OK);
    assert(fabs(page.row[0][0]+.6)<1e-12 && fabs(page.row[0][1]-exp(-.6))<2e-6);
    assert(page.row[3][0]==0 && page.row[3][1]==1);
    table_read_page(&a.doc,&a.model,&index,0,&page,NULL,NULL);assert(fabs(page.row[0][0]+6)<1e-12);
    table_read_page(&a.doc,&a.model,&index,table_bottom(&index),&page,NULL,NULL);
    assert(!page.more && page.row[6][0]==6);
    a.recall=a.doc;a.has_recall=true;assert(storage_save(&a,directory));
    strcpy(a.doc.text[0],"-y");a.doc.ic[0].y[0]=3;assert(storage_save(&a,directory));
    assert(storage_load(&b,directory) && !strcmp(b.doc.text[0],"-y") && b.doc.ic[0].y[0]==3);
    assert(!strcmp(b.recall.text[0],"y"));
    char path[256];snprintf(path,sizeof(path),"%s/DIFFEQ1.dat",directory);
    FILE *f=fopen(path,"wb");assert(f);fputs("interrupted write",f);fclose(f);
    assert(storage_load(&b,directory) && !strcmp(b.doc.text[0],"y"));
    assert(storage_save(&a,directory) && storage_load(&b,directory) && !strcmp(b.doc.text[0],"-y"));
    strcpy(b.doc.text[0],"current-must-survive");
    for(int slot=0;slot<2;slot++) {
        snprintf(path,sizeof(path),"%s/DIFFEQ%d.dat",directory,slot);
        f=fopen(path,"wb");assert(f);fputs("broken slot",f);fclose(f);
    }
    assert(!storage_load(&b,directory) && !strcmp(b.doc.text[0],"current-must-survive"));
    assert(model_compile(&a.doc,&a.model).expression.status==EXPR_OK);
    OdeStatus status;assert(storage_csv(&a.doc,&a.model,directory,path,sizeof(path),&status,NULL,NULL));
    f=fopen(path,"r");assert(f);char line[256];assert(fgets(line,sizeof(line),f) && !strcmp(line,"'x,'y\n"));
    double x,y;assert(fgets(line,sizeof(line),f) && sscanf(line,"%lf,%lf",&x,&y)==2);
    assert(x==-6 && fabs(y-3*exp(6))<.006);fclose(f);
    char stat[256];assert(storage_stat_csv(&a.doc,&a.model,directory,stat,sizeof(stat),&status,NULL,NULL));
    assert(strcmp(stat,path));
    a.doc.solver.h=.005;a.doc.solver.step=1;char overflow[256];
    assert(!storage_stat_csv(&a.doc,&a.model,directory,overflow,sizeof(overflow),&status,NULL,NULL));
    assert(status==ODE_STEP_LIMIT && access(overflow,F_OK)!=0);
    a.doc.solver.h=.1;a.doc.solver.step=2;
    char second[256];assert(storage_csv(&a.doc,&a.model,directory,second,sizeof(second),&status,NULL,NULL));
    assert(strcmp(path,second));
    char incomplete[256];assert(!storage_csv(&a.doc,&a.model,directory,incomplete,sizeof(incomplete),&status,cancelled,NULL));
    assert(status==ODE_CANCELLED && access(incomplete,F_OK)!=0);
    assert(!storage_csv(&a.doc,&a.model,directory,incomplete,sizeof(incomplete),&status,after_create,incomplete));
    assert(status==ODE_CANCELLED && incomplete[0] && access(incomplete,F_OK)!=0);
    remove(path);remove(second);remove(stat);
    for(int i=0;i<2;i++){snprintf(path,sizeof(path),"%s/DIFFEQ%d.dat",directory,i);remove(path);}
    assert(rmdir(directory)==0);
    printf("Ascending Table/MID/endpoints/Step and explicit session recovery/CSV bounds passed. Index=%zu Page=%zu bytes.\n",sizeof(index),sizeof(page));
}
