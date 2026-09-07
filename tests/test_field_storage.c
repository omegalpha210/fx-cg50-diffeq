#include "storage.h"
#include <assert.h>
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
typedef struct {uint32_t magic,version,size,generation,has_recall;Document current,recall;uint32_t checksum;} NewRecord;
static App a,b;static OldDocument old[2];static unsigned char bytes[sizeof(OldRecord)];static NewRecord record;
static size_t aligned(size_t n,size_t a){return (n+a-1)/a*a;}
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
    /* v6 contains no constant array or separate G/L/x flags. */
    model_defaults(&a.doc,EQ_GENERAL,1);a.recall=a.doc;a.has_recall=true;
    for(unsigned c=0;c<FIELD_COLORS;c++) {
        a.doc.field_style=FIELD_SEGMENT;a.doc.field_color=(uint8_t)c;
        a.recall.field_style=FIELD_ARROW;a.recall.field_color=(uint8_t)(5-c);a.recall.enabled=0;
        assert(storage_save(&a,directory) && storage_load(&b,directory));
        assert(b.doc.field_style==FIELD_SEGMENT && b.doc.field_color==c && b.recall.field_color==5-c);
        assert(b.doc.enabled==1 && b.recall.enabled==0 && !b.migration_warnings);
    }
    from_new(&old[0],&a.doc);from_new(&old[1],&a.recall);write_old(paths[0],5);
    memset(&record,0,sizeof(record));record.magic=0x44455131;record.version=6;record.size=sizeof(record);record.generation=100;record.has_recall=1;
    record.current=a.doc;record.recall=a.recall;record.current.field_style=255;record.current.field_color=255;
    write_bytes(paths[1],&record,sizeof(record),offsetof(NewRecord,checksum));
    assert(storage_load(&b,directory) && b.doc.field_style==FIELD_ARROW && b.doc.field_color==0);
    record.version=7;write_bytes(paths[1],&record,sizeof(record),offsetof(NewRecord,checksum));
    assert(storage_load(&b,directory) && b.doc.field_style==FIELD_SEGMENT);
    assert(truncate(paths[1],25)==0);assert(storage_load(&b,directory) && b.doc.field_style==FIELD_SEGMENT);
    FILE *f=fopen(paths[0],"rb");assert(f);unsigned char check[sizeof(bytes)];size_t n=fread(check,1,sizeof(check),f);fclose(f);
    assert(n>0 && !memcmp(check,bytes,n));
    for(int i=0;i<2;i++)assert(remove(paths[i])==0);assert(rmdir(directory)==0);
    printf("v3/v4/v5 streaming migration, constants/masks/IC policy, v6 roundtrip/defaults and two-slot recovery passed. New record %zu, old %zu bytes.\n",sizeof(record),sizeof(OldRecord));
}
