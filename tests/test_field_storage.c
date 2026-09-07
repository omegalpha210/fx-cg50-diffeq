#include "storage.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* Frozen legacy layout, independent of the new Document's offsets/size. */
typedef struct {
    int kind,dim,nic;char text[9][192];double power,constants[28];
    InitialCondition ic[9];OdeSettings solver;int solver_custom;ViewWindow view;
    uint16_t graph_mask[9],list_mask[9];uint8_t color[9][9];uint8_t field_style,field_color;
} OldDocument;
typedef struct {uint32_t magic,version,size,generation,has_recall;OldDocument current,recall;uint32_t checksum;} OldRecord;
typedef struct {
    int kind,dim,nic;char text[9][192];double power;
    InitialCondition ic[9];OdeSettings solver;int solver_custom;ViewWindow view;
    uint16_t enabled;uint8_t color[9][9];uint8_t field_style,field_color;
} V6Document;
typedef struct {uint32_t magic,version,size,generation,has_recall;V6Document current,recall;uint32_t checksum;} V6Record;
typedef struct {
    int kind,dim,nic;char text[9][192];double power;
    InitialCondition ic[10];OdeSettings solver;int solver_custom;ViewWindow view;
    uint16_t enabled;uint8_t color[10][9];uint8_t field_style,field_color;
} V7Document;
typedef struct {uint32_t magic,version,size,generation,has_recall;V7Document current,recall;uint32_t checksum;} V7Record;
typedef struct {uint32_t magic,version,size,generation,has_recall;Document current,recall;uint32_t checksum;} CurrentRecord;
static V6Record six;
static V7Record seven;
static void from_six(V6Document *o,const Document *d)
{
    memset(o,0xa5,sizeof(*o));
#define COPY(member) memcpy(&o->member,&d->member,sizeof(o->member))
    COPY(kind);COPY(dim);COPY(nic);COPY(text);COPY(power);COPY(ic);COPY(solver);COPY(solver_custom);COPY(view);
    COPY(enabled);COPY(color);COPY(field_style);COPY(field_color);
#undef COPY
}
static void from_seven(V7Document *o,const Document *d)
{
    memset(o,0xa5,sizeof(*o));
#define COPY(member) memcpy(&o->member,&d->member,sizeof(o->member))
    COPY(kind);COPY(dim);COPY(nic);COPY(text);COPY(power);COPY(ic);COPY(solver);COPY(solver_custom);COPY(view);
    COPY(enabled);COPY(color);COPY(field_style);COPY(field_color);
#undef COPY
}

static App a,b;static OldDocument old[2];static unsigned char bytes[sizeof(OldRecord)];static CurrentRecord record;
static size_t aligned(size_t n,size_t a){return (n+a-1)/a*a;}
static bool same_window(const ViewWindow *a,const ViewWindow *b)
{
    return a->xmin==b->xmin && a->xmax==b->xmax && a->ymin==b->ymin
        && a->ymax==b->ymax && a->xscale==b->xscale && a->yscale==b->yscale
        && a->grid==b->grid && a->labels==b->labels && a->phase==b->phase
        && a->phase_x==b->phase_x && a->phase_y==b->phase_y;
}
static void assert_default_phase(const Document *d)
{
    ViewWindow expected;model_phase_window_defaults(&expected);
    assert(same_window(&d->phase_view,&expected));
    assert(d->phase_field==1 && d->phase_nullclines==0 && d->phase_ready==0);
}
static void write_bytes(const char *path,void *data,size_t size,size_t checksum)
{
    unsigned char *p=data;uint32_t hash=2166136261u;
    for(size_t i=0;i<checksum;i++){hash^=p[i];hash*=16777619u;}
    memcpy(p+checksum,&hash,4);FILE *f=fopen(path,"wb");assert(f);
    assert(fwrite(data,size,1,f)==1 && fclose(f)==0);
}
static void from_new(OldDocument *o,const Document *d)
{
    memset(o,0xa5,sizeof(*o));
#define COPY(member) memcpy(&o->member,&d->member,sizeof(o->member))
    COPY(kind);COPY(dim);COPY(nic);COPY(text);COPY(power);COPY(ic);COPY(solver);COPY(solver_custom);COPY(view);COPY(color);COPY(field_style);COPY(field_color);
#undef COPY
    for(int i=0;i<28;i++)o->constants[i]=0;
    for(int i=0;i<9;i++){o->graph_mask[i]=d->enabled;o->list_mask[i]=(uint16_t)((d->enabled<<1)|1);}
}
static unsigned old_has_recall=1;
static void write_old(const char *path,unsigned version)
{
    size_t doc_size=version==3 ? aligned(offsetof(OldDocument,color),_Alignof(OldDocument)):
        (version==4 ? aligned(offsetof(OldDocument,field_style),_Alignof(OldDocument)):sizeof(OldDocument));
    size_t prefix=offsetof(OldRecord,current),checksum=prefix+2*doc_size;
    size_t size=aligned(checksum+4,_Alignof(OldRecord));
    uint32_t header[]={0x44455131,version,(uint32_t)size,17,old_has_recall};
    memset(bytes,0xa5,sizeof(bytes));memcpy(bytes,header,sizeof(header));
    memcpy(bytes+prefix,&old[0],doc_size);memcpy(bytes+prefix+doc_size,&old[1],doc_size);
    write_bytes(path,bytes,size,checksum);
}
int main(void)
{
    char directory[]="migration-test-XXXXXX",paths[2][256];assert(mkdtemp(directory));
    for(int i=0;i<2;i++)snprintf(paths[i],sizeof(paths[i]),"%s/DIFFEQ%d.dat",directory,i);

    /* TIME and SYS2 phase windows are independent, validated model state. */
    model_defaults(&a.doc,EQ_SYSTEM,2);assert_default_phase(&a.doc);
    assert(model_phase_supported(&a.doc));
    assert(model_view(&a.doc)==&a.doc.view && model_view_const(&a.doc)==&a.doc.view);
    a.doc.view.xmin=-8.2;a.doc.view.xmax=9.8;
    a.doc.solver.xmin=-4;a.doc.solver.xmax=5;a.doc.solver_custom=0;a.doc.view.phase=1;
    assert(model_view(&a.doc)==&a.doc.phase_view);
    model_sync_solver_window(&a.doc);
    assert(a.doc.solver.xmin==-4 && a.doc.solver.xmax==5);
    a.doc.view.phase=0;model_sync_solver_window(&a.doc);
    assert(a.doc.solver.xmin==-8 && a.doc.solver.xmax==9);
    assert(model_validate(&a.doc)==ODE_OK);
    model_defaults(&b.doc,EQ_SYSTEM,3);b.doc.view.phase=1;
    assert(!model_phase_supported(&b.doc) && model_view(&b.doc)==&b.doc.view);
    b.doc=a.doc;b.doc.phase_view.xmin=NAN;assert(model_validate(&b.doc)==ODE_BAD_INPUT);
    b.doc=a.doc;b.doc.phase_view.xmin=-DBL_MAX;b.doc.phase_view.xmax=DBL_MAX;
    assert(model_validate(&b.doc)==ODE_BAD_INPUT);
    b.doc=a.doc;b.doc.phase_view.xmax=b.doc.phase_view.xmin;assert(model_validate(&b.doc)==ODE_BAD_INPUT);
    b.doc=a.doc;b.doc.phase_view.xscale=0;assert(model_validate(&b.doc)==ODE_BAD_INPUT);
    b.doc=a.doc;b.doc.phase_view.grid=2;assert(model_validate(&b.doc)==ODE_BAD_INPUT);
    b.doc=a.doc;b.doc.phase_view.labels=2;assert(model_validate(&b.doc)==ODE_BAD_INPUT);
    b.doc=a.doc;b.doc.phase_view.phase=0;assert(model_validate(&b.doc)==ODE_BAD_INPUT);
    b.doc=a.doc;b.doc.phase_view.phase_x=1;assert(model_validate(&b.doc)==ODE_BAD_INPUT);
    b.doc=a.doc;b.doc.phase_field=2;assert(model_validate(&b.doc)==ODE_BAD_INPUT);
    b.doc=a.doc;b.doc.phase_nullclines=2;assert(model_validate(&b.doc)==ODE_BAD_INPUT);
    b.doc=a.doc;b.doc.phase_ready=2;assert(model_validate(&b.doc)==ODE_BAD_INPUT);

    model_defaults(&a.doc,EQ_SYSTEM,9);a.doc.solver.sf=0;a.doc.field_style=FIELD_SEGMENT;a.doc.field_color=4;
    a.doc.color[0][8]=1;a.recall=a.doc;a.recall.solver.sf=100;a.has_recall=true;
    from_new(&old[0],&a.doc);from_new(&old[1],&a.recall);
    old[0].graph_mask[0]=1;old[0].list_mask[0]=1u<<9; /* old CSV X OFF + separate masks */
    old[0].constants[0]=3;old[0].constants[26]=2;old[0].constants[27]=1;
    old[0].constants[1]=NAN; /* unused retired constant cannot reject the session */
    strcpy(old[0].text[8],"A*y1+r+theta+1e-3");
    for(unsigned version=3;version<=5;version++) {
        write_old(paths[0],version);
        assert(storage_load(&b,directory) && b.has_recall && b.doc.enabled==257);
        assert(b.doc.solver.sf==0 && b.recall.solver.sf==100 && b.migration_warnings==STORAGE_LEGACY);
        assert(!strcmp(b.doc.text[8],"(3)*y1+(2)+(1)+1e-3"));
        assert(model_compile(&b.doc,&b.model).expression.status==EXPR_OK);
        assert(b.doc.field_style==(version==5 ? FIELD_SEGMENT:FIELD_ARROW));
        assert(b.doc.field_color==(version==5 ? 4:0));
        assert(model_color(&b.doc,0,8)==(version==3 ? model_default_color(0,8,9):1));
        assert_default_phase(&b.doc);assert_default_phase(&b.recall);
    }
    /* A saved session before the first successful graph has no recall document. */
    memset(&old[1],0,sizeof(old[1]));old_has_recall=0;
    for(unsigned version=3;version<=5;version++) {
        write_old(paths[0],version);
        assert(storage_load(&b,directory) && !b.has_recall && b.doc.dim==9);
        assert(b.migration_warnings==STORAGE_LEGACY);
    }
    from_new(&old[1],&a.recall);old_has_recall=1;
    /* Two old higher-order sets become one complete vector, with explicit notice. */
    old[0].nic=2;old[0].ic[1]=old[0].ic[0];old[0].ic[1].y[8]=99;
    write_old(paths[0],5);assert(storage_load(&b,directory) && b.doc.nic==1);
    assert(b.migration_warnings&STORAGE_IC_ADAPTED);
    /* Keep shared-x scalar families, never silently move a legacy IC's x. */
    model_defaults(&a.doc,EQ_GENERAL,1);from_new(&old[0],&a.doc);
    old[0].nic=3;old[0].ic[1].x=2;old[0].ic[2].y[0]=7;
    write_old(paths[0],5);assert(storage_load(&b,directory) && b.doc.nic==2 && b.doc.ic[1].y[0]==7);
    assert(b.migration_warnings&STORAGE_IC_ADAPTED);
    strcpy(old[0].text[0],"A");old[0].constants[0]=INFINITY;
    write_old(paths[0],4);assert(storage_load(&b,directory) && !strcmp(b.doc.text[0],"A"));
    assert(b.migration_warnings&STORAGE_EXPRESSION_REVIEW);
    /* Expansion overflow retains the source text and requires repair; never truncates it. */
    char long_text[192];memset(long_text,0,sizeof(long_text));
    for(int i=0;i<60;i++)strcat(long_text,i ? "+A":"A");
    strcpy(old[0].text[0],long_text);old[0].constants[0]=1.2345678901234567;
    write_old(paths[0],3);assert(storage_load(&b,directory) && !strcmp(b.doc.text[0],long_text));
    assert(b.migration_warnings&STORAGE_EXPRESSION_REVIEW);
    /* Frozen nine-IC v6 streams into expanded current/recall without offset drift. */
    model_defaults(&a.doc,EQ_GENERAL,1);a.doc.nic=9;
    for(int i=0;i<9;i++){a.doc.ic[i].y[0]=i;a.doc.color[i][0]=(uint8_t)(i%6);}
    a.doc.solver.xmin=-2;a.doc.solver.xmax=3;a.doc.solver_custom=1;
    a.doc.view.ymin=-7;a.doc.view.ymax=11;a.doc.field_style=FIELD_SEGMENT;a.doc.field_color=5;
    six.magic=0x44455131;six.version=6;six.size=sizeof(six);six.generation=20;six.has_recall=1;
    from_six(&six.current,&a.doc);a.doc.ic[8].y[0]=88;from_six(&six.recall,&a.doc);
    write_bytes(paths[0],&six,sizeof(six),offsetof(V6Record,checksum));
    assert(storage_load(&b,directory) && b.doc.nic==9 && b.recall.nic==9 && !b.migration_warnings);
    assert(b.doc.ic[8].y[0]==8 && b.recall.ic[8].y[0]==88 && b.doc.ic[9].y[0]==0);
    assert(b.doc.color[8][0]==2 && b.doc.solver_custom && b.doc.solver.xmin==-2 && b.doc.solver.xmax==3);
    assert(b.doc.view.ymin==-7 && b.doc.view.ymax==11 && b.doc.field_color==5 && b.doc.field_style==FIELD_SEGMENT);
    assert_default_phase(&b.doc);assert_default_phase(&b.recall);
    assert(model_compile(&b.doc,&b.model).expression.status==EXPR_OK);
    six.has_recall=0;memset(&six.recall,0,sizeof(six.recall));
    write_bytes(paths[0],&six,sizeof(six),offsetof(V6Record,checksum));
    assert(storage_load(&b,directory) && !b.has_recall && b.doc.nic==9);
    /* Frozen v7 has ten IC slots but only one shared TIME/PHASE window. */
    model_defaults(&a.doc,EQ_SYSTEM,2);a.doc.nic=10;a.doc.ic[9].y[1]=91;
    a.doc.view=(ViewWindow){-8,9,-4,5,2,3,0,1,1,1,0};
    a.doc.solver=(OdeSettings){-12,14,.125,3210,3,7};a.doc.solver_custom=0;
    model_defaults(&a.recall,EQ_SYSTEM,2);
    a.recall.view=(ViewWindow){-7,8,-9,10,1.5,2.5,1,0,0,0,1};
    a.recall.solver=(OdeSettings){-2,6,.25,4321,2,5};a.recall.solver_custom=1;
    from_seven(&seven.current,&a.doc);from_seven(&seven.recall,&a.recall);
    seven.current.field_style=255;seven.current.field_color=255;
    seven.magic=0x44455131;seven.version=7;seven.size=sizeof(seven);
    seven.generation=40;seven.has_recall=1;
    write_bytes(paths[0],&seven,sizeof(seven),offsetof(V7Record,checksum));
    assert(storage_load(&b,directory) && b.has_recall && !b.migration_warnings);
    assert(b.doc.nic==10 && b.doc.ic[9].y[1]==91);
    assert(b.doc.field_style==FIELD_ARROW && b.doc.field_color==0);
    assert(b.doc.view.xmin==-6.3 && b.doc.view.xmax==6.3 && b.doc.view.phase==1);
    assert(b.doc.phase_view.xmin==-8 && b.doc.phase_view.xmax==9);
    assert(b.doc.phase_view.ymin==-4 && b.doc.phase_view.ymax==5);
    assert(b.doc.phase_view.xscale==2 && b.doc.phase_view.yscale==3);
    assert(b.doc.phase_view.grid==0 && b.doc.phase_view.labels==1
        && b.doc.phase_view.phase==1 && b.doc.phase_view.phase_x==0 && b.doc.phase_view.phase_y==1);
    assert(!memcmp(&b.doc.solver,&a.doc.solver,sizeof(a.doc.solver)) && !b.doc.solver_custom);
    assert(model_view_const(&b.doc)==&b.doc.phase_view);
    assert(b.recall.view.xmin==-7 && b.recall.view.xmax==8 && b.recall.view.phase==0);
    assert_default_phase(&b.recall);
    assert(!memcmp(&b.recall.solver,&a.recall.solver,sizeof(a.recall.solver)) && b.recall.solver_custom);
    assert(b.doc.phase_field==1 && b.doc.phase_nullclines==0 && b.doc.phase_ready==0);

    /* Current v8 round-trips independent windows and phase preferences. */
    model_defaults(&a.doc,EQ_SYSTEM,2);a.doc.view.phase=1;
    a.doc.view.xmin=-20;a.doc.view.xmax=30;a.doc.view.ymin=-11;a.doc.view.ymax=12;
    a.doc.phase_view=(ViewWindow){-2.5,4.5,-6.5,7.5,.5,.75,0,1,1,0,1};
    a.doc.phase_field=0;a.doc.phase_nullclines=1;a.doc.phase_ready=1;
    model_defaults(&a.recall,EQ_SYSTEM,2);a.recall.view.phase=0;
    a.recall.phase_view=(ViewWindow){-9,10,-12,13,2,3,1,0,1,0,1};
    a.recall.phase_field=1;a.recall.phase_nullclines=1;a.recall.phase_ready=0;
    a.has_recall=true;
    for(unsigned c=0;c<FIELD_COLORS;c++) {
        a.doc.field_style=FIELD_SEGMENT;a.doc.field_color=(uint8_t)c;
        a.recall.field_style=FIELD_ARROW;a.recall.field_color=(uint8_t)(5-c);a.recall.enabled=0;
        assert(storage_save(&a,directory) && storage_load(&b,directory));
        assert(b.doc.field_style==FIELD_SEGMENT && b.doc.field_color==c && b.recall.field_color==5-c);
        assert(b.doc.enabled==3 && b.recall.enabled==0 && !b.migration_warnings);
        assert(same_window(&b.doc.view,&a.doc.view) && same_window(&b.doc.phase_view,&a.doc.phase_view));
        assert(same_window(&b.recall.view,&a.recall.view) && same_window(&b.recall.phase_view,&a.recall.phase_view));
        assert(b.doc.phase_field==0 && b.doc.phase_nullclines==1 && b.doc.phase_ready==1);
        assert(b.recall.phase_field==1 && b.recall.phase_nullclines==1 && b.recall.phase_ready==0);
    }
    b.doc=a.doc;b.doc.phase_field=2;assert(!storage_save(&b,directory));

    /* Current invalid phase bytes are rejected; current field appearance sanitizes. */
    seven.generation=100;write_bytes(paths[0],&seven,sizeof(seven),offsetof(V7Record,checksum));
    memset(&record,0,sizeof(record));record.magic=0x44455131;record.version=8;
    record.size=sizeof(record);record.generation=101;record.has_recall=1;
    record.current=a.doc;record.recall=a.recall;record.current.field_style=255;record.current.field_color=255;
    write_bytes(paths[1],&record,sizeof(record),offsetof(CurrentRecord,checksum));
    assert(storage_load(&b,directory) && b.doc.field_style==FIELD_ARROW && b.doc.field_color==0);
    record.generation=102;record.current.phase_nullclines=2;
    write_bytes(paths[1],&record,sizeof(record),offsetof(CurrentRecord,checksum));
    assert(storage_load(&b,directory) && b.doc.nic==10 && b.doc.phase_view.xmin==-8);
    record.version=9;record.current.phase_nullclines=1;
    write_bytes(paths[1],&record,sizeof(record),offsetof(CurrentRecord,checksum));
    assert(storage_load(&b,directory) && b.doc.nic==10 && b.doc.phase_view.xmin==-8);
    assert(truncate(paths[1],25)==0);assert(storage_load(&b,directory) && b.doc.field_style==FIELD_ARROW);
    FILE *f=fopen(paths[0],"rb");assert(f);unsigned char check[sizeof(seven)];size_t n=fread(check,1,sizeof(check),f);fclose(f);
    assert(n==sizeof(seven) && !memcmp(check,&seven,n));
    for(int i=0;i<2;i++)assert(remove(paths[i])==0);assert(rmdir(directory)==0);
    printf("v3-v7 migration, independent phase windows, v8 roundtrip/validation and two-slot recovery passed. Current record %zu, v7 %zu, oldest %zu bytes.\n",sizeof(record),sizeof(seven),sizeof(OldRecord));
}
