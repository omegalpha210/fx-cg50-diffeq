#include "storage.h"
#include "table.h"
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#ifdef FXCG50
#include <gint/gint.h>
#endif

#define RECORD_MAGIC 0x44455131u
#define RECORD_VERSION 10u

typedef struct {
    uint32_t magic,version,size,generation,has_recall;
} RecordHeader;

/* This type describes the current version-10 byte layout. No object of this
   type is allocated; session data is streamed directly from App documents. */
typedef struct {
    uint32_t magic,version,size,generation,has_recall;
    Document current,recall;
    uint32_t checksum;
} RecordLayout;

/* Frozen v3/v4/v5 ABI. Used only for versioned streaming offsets, never as a
   live document or full-sized scratch allocation. Constants are migration-only. */
typedef struct {
    int kind,dim,nic;
    char text[ODE_MAX_DIM][EXPR_TEXT];
    double power,constants[28];
    InitialCondition ic[9];
    OdeSettings solver;
    int solver_custom;
    ViewWindow view;
    uint16_t graph_mask[9],list_mask[9];
    uint8_t color[9][9];
    uint8_t field_style,field_color;
} LegacyDocument;
/* v6 removed constants/masks but still had exactly nine initial conditions. */
typedef struct {
    int kind,dim,nic;
    char text[9][192];
    double power;
    InitialCondition ic[9];
    OdeSettings solver;
    int solver_custom;
    ViewWindow view;
    uint16_t enabled;
    uint8_t color[9][9];
    uint8_t field_style,field_color;
} Version6Document;
/* Frozen v7 ABI: unified output and ten IC slots, before phase-only state. */
typedef struct {
    int kind,dim,nic;
    char text[9][192];
    double power;
    InitialCondition ic[10];
    OdeSettings solver;
    int solver_custom;
    ViewWindow view;
    uint16_t enabled;
    uint8_t color[10][9];
    uint8_t field_style,field_color;
} Version7Document;
_Static_assert(offsetof(Document,phase_view)==sizeof(Version7Document),
    "v7 document prefix changed");
/* Frozen v8 retains the original OdeSettings ABI and appends Phase prefs. */
typedef struct {
    int kind,dim,nic;char text[9][192];double power;InitialCondition ic[10];
    OdeSettings solver;int solver_custom;ViewWindow view;uint16_t enabled;
    uint8_t color[10][9],field_style,field_color;ViewWindow phase_view;
    uint8_t phase_field,phase_nullclines,phase_ready;
} Version8Document;
_Static_assert(offsetof(Document,adaptive)==sizeof(Version8Document),"v8 prefix changed");
typedef struct {
    int kind,dim,nic;char text[9][192];double power;InitialCondition ic[10];
    OdeSettings solver;int solver_custom;ViewWindow view;uint16_t enabled;
    uint8_t color[10][9],field_style,field_color;ViewWindow phase_view;
    uint8_t phase_field,phase_nullclines,phase_ready;OdeAdaptive adaptive;
} Version9Document;
_Static_assert(offsetof(Document,event)==sizeof(Version9Document),"v9 prefix changed");
typedef struct {
    char path[256];
    RecordHeader header;
    uint32_t checksum;
    int slot;
} SlotProbe;

typedef struct {
    const char *path;
    RecordHeader expected;
    Document *current,*recall;
    unsigned *warnings;
} RecordRead;

typedef struct {
    const char *path;
    RecordHeader header;
    const Document *current,*recall;
} RecordWrite;

typedef struct {
    const char *path;
    const char *data;
    size_t size;
    int opened,closed;
} FileAppend;

typedef struct {
    const char *path;
    int exists,closed;
} FileCreate;

_Static_assert(sizeof(RecordHeader)<=64,"record header scratch is too small");
_Static_assert(offsetof(RecordLayout,current)<=64,"record prefix scratch is too small");
_Static_assert(offsetof(RecordLayout,current)>=sizeof(RecordHeader),"record layout changed");
_Static_assert(offsetof(RecordLayout,recall)==offsetof(RecordLayout,current)+sizeof(Document),
    "record layout changed");
_Static_assert(offsetof(RecordLayout,checksum)==offsetof(RecordLayout,recall)+sizeof(Document),
    "record layout changed");

static uint32_t hash_update(uint32_t hash,const void *data,size_t length)
{
    const unsigned char *bytes=data;
    for(size_t i=0;i<length;i++){hash^=bytes[i];hash*=16777619u;}
    return hash;
}

static bool filename(char *out,unsigned capacity,const char *directory,const char *name)
{
    size_t length=strlen(directory);
    int n=snprintf(out,capacity,"%s%s%s",directory,
        length && directory[length-1]=='/' ? "":"/",name);
    return n>=0 && (unsigned)n<capacity;
}

static bool native_read_all(int fd,void *data,size_t size)
{
    unsigned char *bytes=data;
    while(size) {
        ssize_t n=read(fd,bytes,size);
        if(n<=0)return false;
        bytes+=n;size-=(size_t)n;
    }
    return true;
}

static bool native_read_hashed(int fd,void *data,size_t size,uint32_t *hash)
{
    if(!native_read_all(fd,data,size))return false;
    *hash=hash_update(*hash,data,size);
    return true;
}

static size_t aligned_size(size_t size,size_t alignment)
{
    return (size+alignment-1)/alignment*alignment;
}
static size_t document_size(uint32_t version)
{
    /* Legacy sizes include their own ABI tail padding, not new field bytes. */
    if(version==3)return aligned_size(offsetof(LegacyDocument,color),_Alignof(LegacyDocument));
    if(version==4)return aligned_size(offsetof(LegacyDocument,field_style),_Alignof(LegacyDocument));
    if(version==5)return sizeof(LegacyDocument);
    if(version==6)return sizeof(Version6Document);
    if(version==7)return sizeof(Version7Document);
    if(version==8)return sizeof(Version8Document);
    if(version==9)return sizeof(Version9Document);
    return sizeof(Document);
}
static size_t checksum_offset(uint32_t version)
{
    return offsetof(RecordLayout,current)+2*document_size(version);
}
static size_t record_size(uint32_t version)
{
    return aligned_size(checksum_offset(version)+sizeof(uint32_t),_Alignof(RecordLayout));
}
static bool valid_header(const RecordHeader *header)
{
    return header->magic==RECORD_MAGIC && (header->version>=3 && header->version<=RECORD_VERSION)
        && header->size==record_size(header->version) && header->has_recall<=1;
}

static bool same_header(const RecordHeader *a,const RecordHeader *b)
{
    return a->magic==b->magic && a->version==b->version && a->size==b->size
        && a->generation==b->generation && a->has_recall==b->has_recall;
}

static bool native_write_all(int fd,const void *data,size_t size)
{
    const unsigned char *bytes=data;
    while(size) {
        ssize_t n=write(fd,bytes,size);
        if(n<=0)return false;
        bytes+=n;size-=(size_t)n;
    }
    return true;
}

static int slot_probe_native(void *opaque)
{
    SlotProbe *probe=opaque;
    int fd=open(probe->path,O_RDONLY,0);
    if(fd<0)return 0;
    off_t length=lseek(fd,0,SEEK_END);
    bool ok=lseek(fd,0,SEEK_SET)==0 && native_read_all(fd,&probe->header,sizeof(probe->header))
        && valid_header(&probe->header) && length==(off_t)probe->header.size
        && lseek(fd,0,SEEK_SET)==0;
    uint32_t hash=2166136261u;
    unsigned char chunk[256];
    size_t offset=0,end=ok ? checksum_offset(probe->header.version):0;
    while(ok && offset<end) {
        size_t size=end-offset;
        if(size>sizeof(chunk))size=sizeof(chunk);
        ok=native_read_all(fd,chunk,size);
        if(ok) {
            if(offset==0)memcpy(&probe->header,chunk,sizeof(probe->header));
            hash=hash_update(hash,chunk,size);offset+=size;
        }
    }
    if(ok)ok=native_read_all(fd,&probe->checksum,sizeof(probe->checksum));
    if(close(fd)!=0)ok=false;
    return ok && hash==probe->checksum;
}

static bool skip_hashed(int fd,size_t size,uint32_t *hash)
{
    unsigned char scratch[32];
    while(size) {
        size_t n=size<sizeof(scratch) ? size:sizeof(scratch);
        if(!native_read_hashed(fd,scratch,n,hash))return false;
        size-=n;
    }
    return true;
}
static bool legacy_field(int fd,void *out,size_t size,size_t offset,size_t *position,uint32_t *hash)
{
    if(offset<*position || !skip_hashed(fd,offset-*position,hash)
        || !native_read_hashed(fd,out,size,hash))return false;
    *position=offset+size;return true;
}
static bool migrate_expression(char text[EXPR_TEXT],const double constants[28])
{
    char result[EXPR_TEXT];size_t used=0;
    if(!memchr(text,0,EXPR_TEXT))return false;
    const char *p=text;
    while(*p) {
        const char *start=p,*token=start;char value[40];int index=-1;
        if(isdigit((unsigned char)*p) || *p=='.') {
            char *end;strtod(p,&end);p=end>p ? end:p+1;
        } else if(isalpha((unsigned char)*p)) {
            while(isalnum((unsigned char)*p))p++;
            if(p-start==1 && *start>='A' && *start<='Z' && *start!='X' && *start!='Y')index=*start-'A';
            if(p-start==1 && *start=='r')index=26;
            if(p-start==5 && !memcmp(start,"theta",5))index=27;
        } else p++;
        size_t length=(size_t)(p-start);
        if(index>=0) {
            if(!isfinite(constants[index]))return false;
            int n=snprintf(value,sizeof(value),"(%.17g)",constants[index]);
            if(n<0 || (unsigned)n>=sizeof(value))return false;
            token=value;length=(size_t)n;
        }
        if(length>=sizeof(result)-used)return false;
        memcpy(result+used,token,length);used+=length;
    }
    result[used]=0;memcpy(text,result,used+1);return true;
}

/* Old records had one window serving both time and phase plots. Preserve an
   active SYS2 phase window as phase geometry. Its time geometry cannot be
   recovered, so install safe defaults while retaining the selected phase mode
   and the stored solver (including an automatic solver) byte-for-byte. */
static void migrate_phase_state(Document *d,bool present)
{
    model_phase_window_defaults(&d->phase_view);
    d->phase_field=1;d->phase_nullclines=0;d->phase_ready=0;
    if(!present || !model_phase_supported(d) || !d->view.phase)return;
    d->phase_view=d->view;
    d->phase_view.phase=1;d->phase_view.phase_x=0;d->phase_view.phase_y=1;
    model_window_defaults(&d->view);
    d->view.phase=1;
}

static bool read_document(int fd,Document *d,uint32_t version,bool present,uint32_t *hash,unsigned *warnings)
{
    memset(d,0,sizeof(*d));
    ode_adaptive_defaults(&d->adaptive);
    if(version==9) {
        if(!native_read_hashed(fd,d,sizeof(Version9Document),hash))return false;
        model_sanitize_colors(d);model_sanitize_field(d);return true;
    }
    if(version==8) {
        if(!native_read_hashed(fd,d,sizeof(Version8Document),hash))return false;
        model_sanitize_colors(d);model_sanitize_field(d);return true;
    }
    if(version==RECORD_VERSION) {
        if(!native_read_hashed(fd,d,sizeof(*d),hash))return false;
        model_sanitize_colors(d);model_sanitize_field(d);return true;
    }
    if(version==7) {
        size_t position=0;
#define V7_FIELD(member) do {if(!legacy_field(fd,&d->member,sizeof(((Version7Document *)0)->member),offsetof(Version7Document,member),&position,hash))return false;} while(0)
        V7_FIELD(kind);V7_FIELD(dim);V7_FIELD(nic);V7_FIELD(text);V7_FIELD(power);
        V7_FIELD(ic);V7_FIELD(solver);V7_FIELD(solver_custom);V7_FIELD(view);
        V7_FIELD(enabled);V7_FIELD(color);V7_FIELD(field_style);V7_FIELD(field_color);
#undef V7_FIELD
        if(!skip_hashed(fd,document_size(version)-position,hash))return false;
        if(present && (d->nic<0 || d->nic>10))return false;
        migrate_phase_state(d,present);
        model_sanitize_colors(d);model_sanitize_field(d);return true;
    }
    if(version==6) {
        size_t position=0;
#define V6_FIELD(member) do {if(!legacy_field(fd,&d->member,sizeof(((Version6Document *)0)->member),offsetof(Version6Document,member),&position,hash))return false;} while(0)
        V6_FIELD(kind);V6_FIELD(dim);V6_FIELD(nic);V6_FIELD(text);V6_FIELD(power);
        V6_FIELD(ic);V6_FIELD(solver);V6_FIELD(solver_custom);V6_FIELD(view);
        V6_FIELD(enabled);V6_FIELD(color);V6_FIELD(field_style);V6_FIELD(field_color);
#undef V6_FIELD
        for(int j=0;j<ODE_MAX_DIM;j++)d->color[9][j]=(uint8_t)model_default_color(9,j,d->dim);
        if(!skip_hashed(fd,document_size(version)-position,hash))return false;
        if(present && (d->nic<0 || d->nic>9))return false;
        migrate_phase_state(d,present);
        model_sanitize_colors(d);model_sanitize_field(d);return true;
    }
    size_t position=0;double constants[28];uint16_t graph[9],list[9];
#define FIELD(member) do {if(!legacy_field(fd,&d->member,sizeof(((LegacyDocument *)0)->member),offsetof(LegacyDocument,member),&position,hash))return false;} while(0)
    FIELD(kind);FIELD(dim);FIELD(nic);FIELD(text);FIELD(power);
    if(!legacy_field(fd,constants,sizeof(constants),offsetof(LegacyDocument,constants),&position,hash))return false;
    FIELD(ic);FIELD(solver);FIELD(solver_custom);FIELD(view);
    if(!legacy_field(fd,graph,sizeof(graph),offsetof(LegacyDocument,graph_mask),&position,hash)
        || !legacy_field(fd,list,sizeof(list),offsetof(LegacyDocument,list_mask),&position,hash))return false;
    if(version>=4){FIELD(color);}else model_color_defaults(d);
    if(version>=4)for(int j=0;j<ODE_MAX_DIM;j++)d->color[9][j]=(uint8_t)model_default_color(9,j,d->dim);
    if(version>=5){FIELD(field_style);FIELD(field_color);}else model_field_appearance_defaults(d);
#undef FIELD
    if(!skip_hashed(fd,document_size(version)-position,hash))return false;
    migrate_phase_state(d,present);
    if(!present)return true; /* Unused recall bytes need only checksum validation. */
    if(d->dim<1 || d->dim>ODE_MAX_DIM || d->nic<0 || d->nic>9)return false;
    *warnings|=STORAGE_LEGACY;
    unsigned allowed=(1u<<d->dim)-1;
    for(int i=0;i<d->nic;i++)d->enabled|=(uint16_t)((graph[i]|(list[i]>>1))&allowed);
    for(int i=0;i<ODE_MAX_DIM;i++)if(!migrate_expression(d->text[i],constants))
        *warnings|=STORAGE_EXPRESSION_REVIEW; /* keep original text; parser requires explicit repair */
    if(d->kind>EQ_GENERAL && d->nic>1){d->nic=1;*warnings|=STORAGE_IC_ADAPTED;}
    if(d->kind<=EQ_GENERAL && d->nic>1) {
        int count=1;
        for(int i=1;i<d->nic;i++) {
            if(d->ic[i].x!=d->ic[0].x){*warnings|=STORAGE_IC_ADAPTED;continue;}
            d->ic[count]=d->ic[i];memcpy(d->color[count],d->color[i],sizeof(d->color[count]));count++;
        }
        d->nic=count;
    }
    model_sanitize_colors(d);model_sanitize_field(d);return true;
}

static int record_read_native(void *opaque)
{
    RecordRead *request=opaque;
    int fd=open(request->path,O_RDONLY,0);
    if(fd<0)return 0;
    bool ok=valid_header(&request->expected)
        && lseek(fd,0,SEEK_END)==(off_t)request->expected.size
        && lseek(fd,0,SEEK_SET)==0;
    uint32_t hash=2166136261u,checksum=0;
    unsigned char prefix[64];RecordHeader actual={0};
    size_t prefix_size=offsetof(RecordLayout,current);
    if(ok)ok=native_read_hashed(fd,prefix,prefix_size,&hash);
    if(ok) {
        memcpy(&actual,prefix,sizeof(actual));
        ok=same_header(&actual,&request->expected);
    }
    if(ok)ok=read_document(fd,request->current,actual.version,true,&hash,request->warnings)
        && read_document(fd,request->recall,actual.version,actual.has_recall!=0,&hash,request->warnings)
        && native_read_all(fd,&checksum,sizeof(checksum)) && hash==checksum;
    if(close(fd)!=0)ok=false;
    return ok;
}

static bool native_write_zeros(int fd,size_t size,uint32_t *hash)
{
    static const unsigned char zero[32]={0};
    while(size) {
        size_t part=size>sizeof(zero) ? sizeof(zero):size;
        if(!native_write_all(fd,zero,part))return false;
        if(hash)*hash=hash_update(*hash,zero,part);
        size-=part;
    }
    return true;
}

static bool native_write_hashed(int fd,const void *data,size_t size,uint32_t *hash)
{
    if(!native_write_all(fd,data,size))return false;
    *hash=hash_update(*hash,data,size);
    return true;
}

static int record_write_native(void *opaque)
{
    RecordWrite *request=opaque;
    int fd=open(request->path,O_WRONLY|O_CREAT|O_TRUNC,0644);
    if(fd<0)return 0;
    unsigned char prefix[64]={0};
    size_t prefix_size=offsetof(RecordLayout,current);
    memcpy(prefix,&request->header,sizeof(request->header));
    uint32_t hash=2166136261u;
    bool ok=native_write_hashed(fd,prefix,prefix_size,&hash)
        && native_write_hashed(fd,request->current,sizeof(Document),&hash)
        && native_write_zeros(fd,offsetof(RecordLayout,recall)
            -(offsetof(RecordLayout,current)+sizeof(Document)),&hash)
        && native_write_hashed(fd,request->recall,sizeof(Document),&hash)
        && native_write_zeros(fd,offsetof(RecordLayout,checksum)
            -(offsetof(RecordLayout,recall)+sizeof(Document)),&hash)
        && native_write_all(fd,&hash,sizeof(hash))
        && native_write_zeros(fd,sizeof(RecordLayout)
            -(offsetof(RecordLayout,checksum)+sizeof(hash)),NULL);
    int close_result=close(fd);
    if(!ok || close_result!=0) {
        /* A close failure can leave Fugue's native handle locked. Only remove
           an incomplete slot after close definitely succeeded. */
        if(close_result==0)remove(request->path);
        return 0;
    }
    return 1;
}

static int file_create_native(void *opaque)
{
    FileCreate *request=opaque;
    errno=0;
    int fd=open(request->path,O_WRONLY|O_CREAT|O_EXCL,0644);
    if(fd<0){request->exists=errno==EEXIST;return 0;}
    request->closed=close(fd)==0;
    return request->closed;
}

static int file_append_native(void *opaque)
{
    FileAppend *request=opaque;
    int fd=open(request->path,O_WRONLY|O_APPEND,0);
    if(fd<0)return 0;
    request->opened=1;
    bool ok=native_write_all(fd,request->data,request->size);
    request->closed=close(fd)==0;
    return ok && request->closed;
}

static int file_remove_native(void *opaque)
{
    return remove((const char *)opaque)==0;
}

static int run_slot_probe(SlotProbe *probe)
{
#ifdef FXCG50
    return gint_world_switch(GINT_CALL(slot_probe_native,(void *)probe));
#else
    return slot_probe_native(probe);
#endif
}

static int run_record_read(RecordRead *request)
{
#ifdef FXCG50
    return gint_world_switch(GINT_CALL(record_read_native,(void *)request));
#else
    return record_read_native(request);
#endif
}

static int run_record_write(RecordWrite *request)
{
#ifdef FXCG50
    return gint_world_switch(GINT_CALL(record_write_native,(void *)request));
#else
    return record_write_native(request);
#endif
}

static int run_file_create(FileCreate *request)
{
#ifdef FXCG50
    return gint_world_switch(GINT_CALL(file_create_native,(void *)request));
#else
    return file_create_native(request);
#endif
}

static int run_file_append(FileAppend *request)
{
#ifdef FXCG50
    return gint_world_switch(GINT_CALL(file_append_native,(void *)request));
#else
    return file_append_native(request);
#endif
}

static int run_file_remove(const char *path)
{
#ifdef FXCG50
    return gint_world_switch(GINT_CALL(file_remove_native,(void *)path));
#else
    return file_remove_native((void *)path);
#endif
}

static bool probe_slot(const char *directory,int slot,SlotProbe *probe)
{
    char name[24];snprintf(name,sizeof(name),"DIFFEQ%d.dat",slot);
    memset(probe,0,sizeof(*probe));
    probe->slot=slot;
    if(!filename(probe->path,sizeof(probe->path),directory,name)
        || !run_slot_probe(probe))return false;
    RecordHeader *header=&probe->header;
    return valid_header(header);
}

static bool load_slot_documents(App *app,const SlotProbe *probe)
{
    app->load.warnings=0;
    RecordRead request={probe->path,probe->header,&app->load.current,&app->load.recall,&app->load.warnings};
    if(!run_record_read(&request)
        || model_validate(&app->load.current)!=ODE_OK
        || (probe->header.has_recall && model_validate(&app->load.recall)!=ODE_OK))return false;
    return true;
}

static bool newest_valid_slot(App *app,const char *directory,SlotProbe *selected)
{
    SlotProbe slots[2];bool valid[2];
    for(int slot=0;slot<2;slot++)valid[slot]=probe_slot(directory,slot,&slots[slot]);
    for(int attempt=0;attempt<2;attempt++) {
        int candidate=-1;
        for(int slot=0;slot<2;slot++)if(valid[slot]
            && (candidate<0 || slots[slot].header.generation>=slots[candidate].header.generation))
            candidate=slot;
        if(candidate<0)return false;
        if(load_slot_documents(app,&slots[candidate])){*selected=slots[candidate];return true;}
        valid[candidate]=false;
    }
    return false;
}

bool storage_load(App *app,const char *directory)
{
    SlotProbe selected;
    if(!newest_valid_slot(app,directory,&selected))return false;
    app->migration_warnings=app->load.warnings;
    app->doc=app->load.current;
    app->recall=app->load.recall;
    app->has_recall=selected.header.has_recall!=0;
    return true;
}

bool storage_save(App *app,const char *directory)
{
    if(model_validate(&app->doc)!=ODE_OK
        || (app->has_recall && model_validate(&app->recall)!=ODE_OK))return false;
    SlotProbe latest;bool found=newest_valid_slot(app,directory,&latest);
    uint32_t generation=found ? latest.header.generation:0;
    if(generation==UINT32_MAX)return false;
    int slot=found && latest.slot==0 ? 1:0;
    char name[24],path[256];snprintf(name,sizeof(name),"DIFFEQ%d.dat",slot);
    if(!filename(path,sizeof(path),directory,name))return false;
    RecordHeader header={RECORD_MAGIC,RECORD_VERSION,sizeof(RecordLayout),
        generation+1,app->has_recall ? 1u:0u};
    RecordWrite request={path,header,&app->doc,&app->recall};
    if(!run_record_write(&request))return false;
    SlotProbe verified;
    return probe_slot(directory,slot,&verified)
        && verified.header.generation==header.generation
        && load_slot_documents(app,&verified);
}

#define CSV_MAX_ROWS 998u /* One of the 999 CSV lines is the ignored label row. */
#define CSV_BUFFER_SIZE 2048u
static TablePage csv_page;
static char csv_buffer[CSV_BUFFER_SIZE];

static bool append_format(size_t *used,const char *format,...)
{
    if(*used>=sizeof(csv_buffer))return false;
    va_list args;va_start(args,format);
    int n=vsnprintf(csv_buffer+*used,sizeof(csv_buffer)-*used,format,args);
    va_end(args);
    if(n<0 || (size_t)n>=sizeof(csv_buffer)-*used)return false;
    *used+=(size_t)n;return true;
}

static int unique_csv(const char *directory,const char *prefix,char *path,unsigned capacity)
{
    for(int i=0;i<100;i++) {
        char name[24];snprintf(name,sizeof(name),"%s%02d.csv",prefix,i);
        if(!filename(path,capacity,directory,name))return -1;
        FileCreate request={path,0,0};
        if(run_file_create(&request))return 0;
        if(!request.exists)return -1;
    }
    return -1;
}

static bool append_file(const char *path,size_t size,bool *removable)
{
    FileAppend request={path,csv_buffer,size,0,0};
    bool ok=run_file_append(&request)!=0;
    if(request.opened && !request.closed)*removable=false;
    return ok;
}

static bool finish_export(const char *path,bool ok,bool removable,OdeStatus *status)
{
    if(!ok && removable && !run_file_remove(path))*status=ODE_IO_ERROR;
    if(!ok && *status==ODE_OK)*status=ODE_IO_ERROR;
    return ok;
}

static bool export_table(const Document *document,CompiledModel *model,const char *directory,
    const char *prefix,char *path,unsigned capacity,OdeStatus *status,OdeCancel cancel,void *cancel_context)
{
    TableIndex index;if(capacity)path[0]=0;
    *status=table_index_build(document,model,&index,cancel,cancel_context);
    if(*status!=ODE_OK)return false;
    if(index.total>CSV_MAX_ROWS){*status=ODE_STEP_LIMIT;return false;}
    *status=ODE_IO_ERROR;
    if(unique_csv(directory,prefix,path,capacity)<0)return false;
    bool removable=true;size_t used=0;bool ok=append_format(&used,"'x");
    for(int column=0;column<index.count && ok;column++) {
        char label[20];table_column_label(document,&index,column,label,sizeof(label));
        ok=append_format(&used,",'%s",label);
    }
    ok=ok && append_format(&used,"\n") && append_file(path,used,&removable);
    *status=ok ? ODE_OK:ODE_IO_ERROR;
    for(unsigned start=0;start<index.total && *status==ODE_OK;start+=csv_page.count) {
        table_read_page_budgeted(document,model,&index,start,&csv_page,cancel,cancel_context);
        if(csv_page.result.status!=ODE_OK){*status=csv_page.result.status;break;}
        used=0;
        for(unsigned row=0;row<csv_page.count;row++) {
            for(int column=0;column<=index.count && ok;column++) {
                if(csv_page.valid[row]&(1u<<column))ok=append_format(&used,"%s%.17g",column ? ",":"",csv_page.row[row][column]);
                else ok=append_format(&used,",");
            }
            if(ok)ok=append_format(&used,"\n");
            if(!ok){*status=ODE_IO_ERROR;break;}
        }
        if(*status==ODE_OK && used && !append_file(path,used,&removable))*status=ODE_IO_ERROR;
    }
    return finish_export(path,*status==ODE_OK,removable,status);
}
bool storage_csv(const Document *document,CompiledModel *model,const char *directory,
    char *path,unsigned capacity,OdeStatus *status,OdeCancel cancel,void *cancel_context)
{return export_table(document,model,directory,"DIFFEQ",path,capacity,status,cancel,cancel_context);}
bool storage_stat_csv(const Document *document,CompiledModel *model,const char *directory,
    char *path,unsigned capacity,OdeStatus *status,OdeCancel cancel,void *cancel_context)
{return export_table(document,model,directory,"DIFFSTAT",path,capacity,status,cancel,cancel_context);}
