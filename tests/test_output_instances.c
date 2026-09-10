#include "ui.h"
#include "initial.h"
#include "trace.h"
#include "gsolve.h"
#include "table.h"
#include "storage.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static App a,b;
static Document expected;
static const int counts[]={1,2,5,10};
static char script[32768];
static void keys(const char *text)
{assert(strlen(script)+strlen(text)<sizeof(script));strcat(script,text);}
static void select_row(int row)
{for(int i=0;i<row;i++)keys("DOWN ");}
static unsigned chosen(int family)
{return (model_default_color(family,0,1)+1)%6;}
static void make_script(void)
{
    for(unsigned c=0;c<sizeof(counts)/sizeof(*counts);c++) {
        int n=counts[c];
        for(int f=0;f<n;f++) {
            select_row(f);keys("F3 LEFT LEFT UP ");
            if(chosen(f)>=3)keys("DOWN ");
            for(unsigned j=0;j<chosen(f)%3;j++)keys("RIGHT ");
            keys("EXE F6 ");
        }
        for(int f=0;f<n;f++){select_row(f);keys("RIGHT F6 ");}
        for(int f=0;f<n;f+=2){select_row(f);keys("LEFT F6 ");}
        keys("UP F6 ");
        keys("F1 F6 ");
    }
    assert(!setenv("DIFFEQ_HOST_KEYS",script,1));
    assert(!setenv("DIFFEQ_HOST_MAX_FRAMES","5000",1));
}
static void preview(int selected)
{
    int count=a.doc.nic;
    int page=selected/7;
    for(int row=0;row<7 && page*7+row<count;row++) {
        int f=page*7+row;
        assert(gint_vram[(UI_Y+36+row*22)*DWIDTH+UI_X+340]
            ==graph_palette_color(model_color(&a.doc,f,0)));
    }
}
static void consumers(void)
{
    a.doc.solver.sf=0;a.doc.view.grid=0;a.doc.view.labels=0;
    a.doc.view.ymin=-1;a.doc.view.ymax=11;
    strcpy(a.doc.text[0],"0");
    assert(model_compile(&a.doc,&a.model).expression.status==EXPR_OK);
    assert(graph_render(&a.doc,&a.model,false).status==ODE_OK);
    int visible=0;for(int f=0;f<a.doc.nic;f++)if(model_curve_visible(&a.doc,f,0))visible++;
    assert(gsolve_curve_count(&a.doc)==visible);int ordinal=0;
    for(int f=0;f<a.doc.nic;f++) {
        GsolveCurve curve;
        if(!model_curve_visible(&a.doc,f,0)) {
            assert(!trace_prepare(&a.doc,&a.model,f,0));continue;
        }
        assert(gsolve_curve_at(&a.doc,ordinal++,&curve));
        assert(curve.family==f && curve.variable==0);
        int x,y;assert(graph_point(&a.doc.view,1,a.doc.ic[f].y[0],&x,&y));
        size_t pixel=(size_t)(UI_Y+y)*DWIDTH+(unsigned)(UI_X+x);
        int color=graph_palette_color(model_color(&a.doc,f,0));
        assert(gint_vram[pixel]==color);
        graph_overlay_begin(0);graph_overlay_curve(&a.doc,f,0,true);
        assert(gint_vram[pixel]==(color==C_BLUE ? C_BLACK:graph_highlight_color(color,true)));
        graph_overlay_restore();assert(gint_vram[pixel]==color);
        assert(trace_prepare(&a.doc,&a.model,f,0));TracePoint point;
        assert(trace_point_near(0,&point) && point.y[0]==a.doc.ic[f].y[0]);
        trace_overlay_begin();trace_overlay_show(&a.doc,&point,0,true);
        assert(gint_vram[pixel]==graph_highlight_color(color,true));
        trace_overlay_restore();assert(gint_vram[pixel]==color);
    }
    GsolveCurve beyond;assert(!gsolve_curve_at(&a.doc,visible,&beyond));
    TableIndex index;assert(table_index_build(&a.doc,&a.model,&index,NULL,NULL)==ODE_OK);
    assert(index.solutions && index.count==visible);
    ordinal=0;for(int f=0;f<a.doc.nic;f++)if(model_curve_visible(&a.doc,f,0))assert(index.columns[ordinal++]==f);
    uint16_t retained=a.doc.ic_enabled;a.doc.ic_enabled=0;
    assert(gsolve_curve_count(&a.doc)==0);
    assert(table_index_build(&a.doc,&a.model,&index,NULL,NULL)==ODE_OK && index.count==0);
    assert(graph_render(&a.doc,&a.model,false).status==ODE_OK);
    a.doc.ic_enabled=retained;
}
int main(void)
{
    make_script();
    char directory[]="instance-color-test-XXXXXX",path[256];assert(mkdtemp(directory));
    for(unsigned c=0;c<sizeof(counts)/sizeof(*counts);c++) {
        int n=counts[c];model_defaults(&a.doc,EQ_GENERAL,1);
        InitialValues values={.count=(unsigned)n};
        for(int f=0;f<n;f++)values.value[f]=f+1;
        initial_values_apply(&a.doc,&values);
        for(int f=0;f<n;f++) {
            expected=a.doc;expected.color[f][0]=(uint8_t)chosen(f);
            ui_output(&a.doc);assert(!memcmp(&a.doc,&expected,sizeof(expected)));
            preview(f);
        }
        consumers();
        for(int f=0;f<n;f++) {
            expected=a.doc;expected.ic_enabled^=(uint16_t)(1u<<f);
            ui_output(&a.doc);assert(!memcmp(&a.doc,&expected,sizeof(expected)));preview(f);
        }
        assert(gsolve_curve_count(&a.doc)==0);consumers();
        for(int f=0;f<n;f+=2) {
            expected=a.doc;expected.ic_enabled^=(uint16_t)(1u<<f);
            ui_output(&a.doc);assert(!memcmp(&a.doc,&expected,sizeof(expected)));preview(f);
        }
        ui_output(&a.doc);preview(n-1);
        consumers();a.recall=a.doc;a.has_recall=true;
        assert(storage_save(&a,directory) && storage_load(&b,directory));
        assert(b.doc.nic==n && b.doc.ic_enabled==a.doc.ic_enabled && !memcmp(b.doc.color,a.doc.color,sizeof(a.doc.color)));
        assert(!memcmp(b.recall.color,a.recall.color,sizeof(a.recall.color)));
        uint8_t colors[ODE_MAX_IC][ODE_MAX_DIM];memcpy(colors,a.doc.color,sizeof(colors));
        values.count=1;initial_values_apply(&a.doc,&values);
        assert(!memcmp(colors,a.doc.color,sizeof(colors)) && gsolve_curve_count(&a.doc)==1);
        values.count=ODE_MAX_IC;initial_values_apply(&a.doc,&values);
        assert(!memcmp(colors,a.doc.color,sizeof(colors)) && gsolve_curve_count(&a.doc)==ODE_MAX_IC-n+n/2+n%2);
        for(int f=n;f<ODE_MAX_IC;f++)assert(model_color(&a.doc,f,0)==model_default_color(f,0,1));
        values.count=(unsigned)n;initial_values_apply(&a.doc,&values);
        expected=a.doc;model_output_defaults(&expected);ui_output(&a.doc);
        assert(!memcmp(&a.doc,&expected,sizeof(expected)));preview(0);
    }
    for(int slot=0;slot<2;slot++){snprintf(path,sizeof(path),"%s/DIFFEQ%d.dat",directory,slot);assert(remove(path)==0);}
    assert(rmdir(directory)==0);
    puts("1/2/5/10 IC colors: actual F3/INIT/wrap/scroll, independent preferences, independent visibility, Graph/TRACE/G-Solve/Table, SAVE/RCL and shrink/grow passed.");
}
