#include "initial.h"
#include "storage.h"
#include "table.h"
#include "gsolve.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static App a,b;
static void check_csv(const char *path,const char *header,const char *row)
{
    FILE *file=fopen(path,"r");assert(file);char line[256];
    assert(fgets(line,sizeof(line),file) && !strcmp(line,header));
    assert(fgets(line,sizeof(line),file) && !strcmp(line,row));
    assert(fclose(file)==0 && remove(path)==0);
}
int main(void)
{
    char directory[]="output-test-XXXXXX",path[256];assert(mkdtemp(directory));OdeStatus status;
    for(int kind=EQ_HIGHER;kind<=EQ_SYSTEM;kind++) {
        model_defaults(&a.doc,(EquationKind)kind,9);
        a.doc.solver.xmin=0;a.doc.solver.xmax=.2;
        for(int i=0;i<9;i++)a.doc.ic[0].y[i]=i+1;
        assert(model_compile(&a.doc,&a.model).expression.status==EXPR_OK);
        OdeResult before=model_value_at(&a.doc,&a.model,0,.2,NULL,NULL);
        a.doc.enabled=(1u<<0)|(1u<<8);
        OdeResult after=model_value_at(&a.doc,&a.model,0,.2,NULL,NULL);
        assert(before.status==ODE_OK && !memcmp(before.y,after.y,sizeof(before.y)) && a.model.dim==9);
        assert(gsolve_curve_count(&a.doc)==2);
        TableIndex index;TablePage page;assert(table_index_build(&a.doc,&a.model,&index,NULL,NULL)==ODE_OK);
        assert(index.count==2 && index.columns[0]==0 && index.columns[1]==8);
        table_read_page(&a.doc,&a.model,&index,0,&page,NULL,NULL);
        assert(page.row[0][0]==0 && page.row[0][1]==1 && page.row[0][2]==9);
        assert(storage_stat_csv(&a.doc,&a.model,directory,path,sizeof(path),&status,NULL,NULL));
        check_csv(path,kind==EQ_SYSTEM ? "'x,'y1,'y9\n":"'x,'y,'y(8)\n","0,1,9\n");
        a.recall=a.doc;a.has_recall=true;a.recall.enabled=2;
        assert(storage_save(&a,directory) && storage_load(&b,directory));
        assert(b.doc.enabled==257 && b.recall.enabled==2);
        assert(model_compile(&a.doc,&a.model).expression.status==EXPR_OK);
        a.doc.enabled=0;assert(gsolve_curve_count(&a.doc)==0);
        assert(storage_stat_csv(&a.doc,&a.model,directory,path,sizeof(path),&status,NULL,NULL));
        check_csv(path,"'x\n","0\n");
    }
    model_defaults(&a.doc,EQ_SEPARABLE,1);a.doc.solver.xmin=0;a.doc.solver.xmax=.2;
    InitialValues values;assert(initial_values_parse("{0,1,-1}",&values)==IC_LIST_OK);initial_values_apply(&a.doc,&values);
    assert(model_compile(&a.doc,&a.model).expression.status==EXPR_OK && gsolve_curve_count(&a.doc)==3);
    assert(storage_csv(&a.doc,&a.model,directory,path,sizeof(path),&status,NULL,NULL));
    check_csv(path,"'x,'y1,'y2,'y3\n","0,0,1,-1\n");
    assert(initial_values_parse("{2,1}",&values)==IC_LIST_OK);initial_values_apply(&a.doc,&values);
    a.doc.solver.xmin=-6;a.doc.solver.xmax=6;
    assert(storage_stat_csv(&a.doc,&a.model,directory,path,sizeof(path),&status,NULL,NULL));
    FILE *file=fopen(path,"r");assert(file);char line[256],last[256]={0};unsigned missing=0;
    assert(fgets(line,sizeof(line),file) && !strcmp(line,"'x,'y1,'y2\n"));
    double previous=-INFINITY;
    while(fgets(line,sizeof(line),file)) {
        double x=strtod(line,NULL);assert(x>previous);previous=x;
        if(strstr(line,",,1\n"))missing++;
        strcpy(last,line);
    }
    assert(missing && !strcmp(last,"6,,1\n"));assert(fclose(file)==0 && remove(path)==0);
    model_output_color(&a.doc,0,0);
    assert(model_color(&a.doc,0,0)==0 && model_color(&a.doc,1,0)!=0 && model_color(&a.doc,2,0)!=0);
    a.doc.field_color=4;a.doc.field_style=FIELD_SEGMENT;model_output_defaults(&a.doc);
    assert(a.doc.enabled==1 && a.doc.field_color==4 && a.doc.field_style==FIELD_SEGMENT);
    for(int slot=0;slot<2;slot++){snprintf(path,sizeof(path),"%s/DIFFEQ%d.dat",directory,slot);assert(remove(path)==0);}
    assert(rmdir(directory)==0);
    puts("Unified outputs: Graph/G-Solve/Table/CSV ON/OFF, x always first, ninth-state mapping, full-state equality, saved masks, multi-solution columns and seeded colors passed.");
}
