#include "storage.h"
#include "trace.h"
#include "gsolve.h"
#include "table.h"
#include "initial.h"
#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static App a,b;
static void export_check(const char *directory,bool stat,const char *header)
{
    char path[256],line[256];OdeStatus status;
    bool ok=stat ? storage_stat_csv(&a.doc,&a.model,directory,path,sizeof(path),&status,NULL,NULL):
        storage_csv(&a.doc,&a.model,directory,path,sizeof(path),&status,NULL,NULL);
    assert(ok && status==ODE_OK);FILE *f=fopen(path,"rb");assert(f && fgets(line,sizeof(line),f));
    assert(!strcmp(line,header));
    unsigned columns=1;for(const char *p=header;*p;p++)columns+=*p==',';
    while(fgets(line,sizeof(line),f)) {unsigned cells=1;for(char *p=line;*p;p++)cells+=*p==',';assert(cells==columns);}
    assert(!fclose(f) && !remove(path));
}
int main(void)
{
    char dir[]="visibility-test-XXXXXX";assert(mkdtemp(dir));
    const int counts[]={1,2,5,10};
    for(int method=ODE_RK4;method<=ODE_RK45;method++)for(unsigned n=0;n<4;n++) {
        model_defaults(&a.doc,EQ_GENERAL,1);a.doc.nic=counts[n];a.doc.adaptive.method=method;
        strcpy(a.doc.text[0],"0");a.doc.solver.sf=0;a.doc.solver.xmin=-1;a.doc.solver.xmax=1;
        a.doc.view.xmin=-1;a.doc.view.xmax=1;a.doc.view.ymin=-1;a.doc.view.ymax=11;
        a.doc.view.grid=a.doc.view.labels=0;a.doc.event.enabled=1;strcpy(a.doc.event.text,"x-0.5");
        for(int f=0;f<a.doc.nic;f++){a.doc.ic[f].y[0]=f+1;a.doc.color[f][0]=(uint8_t)(f%6);}
        assert(model_compile(&a.doc,&a.model).values==ODE_OK);
        assert(graph_render(&a.doc,&a.model,false).status==ODE_OK);
        SolverReport numerical=*solver_report();assert(numerical.markers.count==(unsigned)a.doc.nic);
        uint16_t masks[]={0x155,0x12,0,MODEL_IC_MASK};
        for(unsigned k=0;k<4;k++) {
            a.doc.ic_enabled=masks[k];assert(model_compile(&a.doc,&a.model).values==ODE_OK);assert(graph_render(&a.doc,&a.model,false).status==ODE_OK);
            assert(!memcmp(&numerical,solver_report(),sizeof(numerical))); /* OFF never erases work/hits. */
            int visible=0;char header[128]="'x";
            for(int f=0;f<a.doc.nic;f++) {
                bool on=(masks[k]&(1u<<f))!=0;int x,y;
                assert(graph_point(&a.doc.view,.25,a.doc.ic[f].y[0],&x,&y));
                assert(gint_vram[(UI_Y+y)*DWIDTH+UI_X+x]==(on ? graph_palette_color((unsigned)f%6):C_WHITE));
                assert(graph_point(&a.doc.view,.5,a.doc.ic[f].y[0],&x,&y));
                if(!on)assert(gint_vram[(UI_Y+y)*DWIDTH+UI_X+x]==C_WHITE); /* Hidden Event marker. */
                GsolveCurve curve;GsolvePoint point={123,456};
                if(on) {
                    assert(gsolve_curve_at(&a.doc,visible++,&curve) && curve.family==f);
                    assert(gsolve_ycal(&a.doc,&a.model,curve,0,&point,NULL,NULL)==ODE_OK && point.y==f+1);
                    char label[20];snprintf(label,sizeof(label),a.doc.nic>1 ? ",'y%d":",'y",f+1);strcat(header,label);
                } else {
                    assert(!trace_prepare(&a.doc,&a.model,f,0) && !trace_select(&a.doc,f,0));
                    assert(gsolve_ycal(&a.doc,&a.model,(GsolveCurve){f,0},0,&point,NULL,NULL)==ODE_BAD_INPUT);
                    assert(point.x==123 && point.y==456);
                }
                OdeResult internal=model_value_at(&a.doc,&a.model,f,.25,NULL,NULL);
                assert(internal.status==ODE_OK && internal.y[0]==f+1); /* Numerical existence independent. */
            }
            strcat(header,"\n");assert(gsolve_curve_count(&a.doc)==visible);
            GsolveCurve invalid;assert(!gsolve_curve_at(&a.doc,visible,&invalid));
            TableIndex index;TablePage page;
            assert(table_index_build(&a.doc,&a.model,&index,NULL,NULL)==ODE_OK && index.count==visible);
            table_read_page(&a.doc,&a.model,&index,index.mid,&page,NULL,NULL);
            for(unsigned r=0;r<page.count;r++)for(int c=0;c<visible;c++)assert(page.row[r][c+1]==index.columns[c]+1);
            export_check(dir,false,header);export_check(dir,true,header);
            a.recall=a.doc;a.has_recall=true;
            assert(storage_save(&a,dir));app_initialize(&b,dir);assert(!b.has_recall);
            assert(storage_load(&b,dir) && b.doc.ic_enabled==masks[k] && b.recall.ic_enabled==masks[k]);
            assert(!memcmp(b.doc.ic,a.doc.ic,sizeof(a.doc.ic)) && !memcmp(b.doc.color,a.doc.color,sizeof(a.doc.color)));
        }
        a.doc.ic_enabled=0x12;uint8_t colors[10][9];memcpy(colors,a.doc.color,sizeof(colors));
        InitialValues values={.count=2,.value={1,2}};initial_values_apply(&a.doc,&values);
        values.count=10;for(int f=0;f<10;f++)values.value[f]=f+1;initial_values_apply(&a.doc,&values);
        assert(a.doc.ic_enabled==0x12 && !memcmp(colors,a.doc.color,sizeof(colors)));
        model_output_defaults(&a.doc);assert(a.doc.ic_enabled==MODEL_IC_MASK);
        a.doc.ic_enabled=0x8000;assert(model_validate(&a.doc)==ODE_BAD_INPUT && !storage_save(&a,dir));
    }
    for(int i=0;i<2;i++){char p[256];snprintf(p,sizeof(p),"%s/DIFFEQ%d.dat",dir,i);assert(!remove(p));}assert(!rmdir(dir));
    puts("Visibility: RK4/RK45 x 1/2/5/10 IC; sparse/all-OFF pixels, markers, numerical reports, selection, Table/CSV/STAT, cold v11 RCL and inactive preferences PASS.");
}
