#include "graph.h"
#include "gsolve.h"
#include "storage.h"
#include "ui.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Frozen 06a491e layout; deliberately independent of the new color offsets. */
typedef struct {
    int kind,dim,nic;
    char text[ODE_MAX_DIM][EXPR_TEXT];
    double power,constants[EXPR_CONSTANTS];
    InitialCondition ic[ODE_MAX_IC];
    OdeSettings solver;
    int solver_custom;
    ViewWindow view;
    uint16_t graph_mask[ODE_MAX_IC],list_mask[ODE_MAX_IC];
} DocumentV3;
typedef struct {
    uint32_t magic,version,size,generation,has_recall;
    DocumentV3 current,recall;
    uint32_t checksum;
} RecordV3;
static App a,b;
static RecordV3 legacy;
static void legacy_write(const char *path)
{
    memset(&legacy,0,sizeof(legacy));
    legacy.magic=0x44455131;legacy.version=3;legacy.size=sizeof(legacy);
    legacy.generation=7;legacy.has_recall=1;
    memcpy(&legacy.current,&a.doc,sizeof(DocumentV3));
    memcpy(&legacy.recall,&a.recall,sizeof(DocumentV3));
    uint32_t hash=2166136261u;
    const unsigned char *bytes=(const unsigned char *)&legacy;
    for(size_t i=0;i<offsetof(RecordV3,checksum);i++){hash^=bytes[i];hash*=16777619u;}
    legacy.checksum=hash;
    FILE *file=fopen(path,"wb");assert(file);
    assert(fwrite(&legacy,sizeof(legacy),1,file)==1 && fclose(file)==0);
}
int main(void)
{
    model_defaults(&a.doc,EQ_SYSTEM,9);
    const unsigned defaults[]={2,4,5,1,0,3};
    for(int i=0;i<9;i++)for(int j=0;j<9;j++)
        assert(model_color(&a.doc,i,j)==defaults[(i*9+j)%6]);
    model_defaults(&a.doc,EQ_GENERAL,1);strcpy(a.doc.text[0],"0");
    a.doc.solver.sf=0;a.doc.view.grid=0;a.doc.ic[0].y[0]=1;
    assert(model_compile(&a.doc,&a.model).expression.status==EXPR_OK);
    int x,y;assert(graph_point(&a.doc.view,1,1,&x,&y));
    size_t pixel=(size_t)(y+UI_Y)*DWIDTH+(unsigned)(x+UI_X);
    for(unsigned color=0;color<6;color++) {
        a.doc.color[0][0]=(uint8_t)color;
        assert(graph_render(&a.doc,&a.model,false).status==ODE_OK);
        assert(gint_vram[pixel]==graph_palette_color(color));
        assert(graph_highlight_curve(&a.doc,&a.model,0,0)==ODE_OK);
        assert(gint_vram[pixel]!=graph_palette_color(color));
        a.doc.graph_mask[0]=0;a.doc.list_mask[0]=0;
        assert(gsolve_curve_count(&a.doc)==0);
        assert(graph_render(&a.doc,&a.model,false).status==ODE_OK);
        assert(gint_vram[pixel]==C_WHITE && model_color(&a.doc,0,0)==color);
        a.doc.graph_mask[0]=1;a.doc.list_mask[0]=3;
    }
    a.doc.color[0][0]=255;assert(model_color(&a.doc,0,0)==2);
    assert(graph_render(&a.doc,&a.model,false).status==ODE_OK);
    assert(gint_vram[pixel]==graph_palette_color(2));
    a.doc.color[0][0]=3;a.recall=a.doc;a.recall.color[0][0]=0;a.has_recall=true;
    char directory[]="color-test-XXXXXX",path[256];assert(mkdtemp(directory));
    snprintf(path,sizeof(path),"%s/DIFFEQ0.dat",directory);legacy_write(path);
    assert(storage_load(&b,directory));
    assert(b.has_recall && b.doc.ic[0].y[0]==1 && !strcmp(b.doc.text[0],"0"));
    assert(model_color(&b.doc,0,0)==2 && model_color(&b.recall,0,0)==2);
    assert(storage_save(&a,directory)); /* v4 generation 8 wins over intact v3. */
    assert(storage_load(&b,directory));
    assert(model_color(&b.doc,0,0)==3 && model_color(&b.recall,0,0)==0);
    FILE *file=fopen(path,"rb");assert(file);uint32_t header[5];
    assert(fread(header,sizeof(header),1,file)==1 && fclose(file)==0);
    assert(header[1]==3 && header[3]==7); /* old slot was not rewritten */
    a.doc.color[0][0]=255;a.recall.color[0][0]=255;
    assert(storage_save(&a,directory) && storage_load(&b,directory));
    assert(b.doc.color[0][0]==2 && b.recall.color[0][0]==2);
    for(int i=0;i<2;i++){snprintf(path,sizeof(path),"%s/DIFFEQ%d.dat",directory,i);assert(remove(path)==0);}
    assert(rmdir(directory)==0);
    puts("Colors: six exact RGB565 curves, distinct highlight, 81 defaults, G/L independence, v3 migration, v4 current/recall and invalid fallback passed.");
}
