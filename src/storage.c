#include "storage.h"
#include "table.h"
#include <errno.h>
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
#define RECORD_VERSION 4u

typedef struct {
    uint32_t magic,version,size,generation,has_recall;
} RecordHeader;

/* This type describes the version-4 byte layout. No object of this
   type is allocated; session data is streamed directly from App documents. */
typedef struct {
    uint32_t magic,version,size,generation,has_recall;
    Document current,recall;
    uint32_t checksum;
} RecordLayout;

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
    /* v4 appends colors; v3 ends after list_mask, including ABI tail padding. */
    return version==3 ? aligned_size(offsetof(Document,color),_Alignof(Document)):sizeof(Document);
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
    return header->magic==RECORD_MAGIC && (header->version==3 || header->version==RECORD_VERSION)
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
    memset(request->current,0,sizeof(Document));memset(request->recall,0,sizeof(Document));
    size_t size=document_size(request->expected.version);
    if(ok)ok=native_read_hashed(fd,request->current,size,&hash)
        && native_read_hashed(fd,request->recall,size,&hash)
        && native_read_all(fd,&checksum,sizeof(checksum))
        && hash==checksum;
    if(close(fd)!=0)ok=false;
    if(ok) {
        if(actual.version==3) {
            model_color_defaults(request->current);model_color_defaults(request->recall);
        } else {
            model_sanitize_colors(request->current);model_sanitize_colors(request->recall);
        }
    }
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
    RecordRead request={probe->path,probe->header,&app->load.current,&app->load.recall};
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

bool storage_csv(const Document *document,CompiledModel *model,const char *directory,
    char *path,unsigned capacity,OdeStatus *status,OdeCancel cancel,void *cancel_context)
{
    *status=ODE_BAD_INPUT;
    if(document->nic<1)return false;
    *status=ODE_IO_ERROR;
    if(unique_csv(directory,"DIFFEQ",path,capacity)<0)return false;
    bool removable=true;size_t used=0;
    bool ok=append_format(&used,"family,direction,step");
    for(int i=-1;i<document->dim && ok;i++) {
        char label[20];model_variable_label(document,i,label,sizeof(label));
        ok=append_format(&used,",%s",label);
    }
    ok=ok && append_format(&used,"\n") && append_file(path,used,&removable);
    unsigned rows=0;*status=ok ? ODE_OK:ODE_IO_ERROR;
    for(int family=0;family<document->nic && *status==ODE_OK;family++) {
        if(!document->list_mask[family])continue;
        for(int direction=-1;direction<=1;direction+=2) {
            unsigned start=0;
            for(;;) {
                table_page(document,model,family,direction,start,&csv_page,cancel,cancel_context);
                if(csv_page.result.status!=ODE_OK && csv_page.result.status!=ODE_SAMPLE_STOP) {
                    *status=csv_page.result.status;break;
                }
                used=0;
                for(unsigned row=0;row<csv_page.count;row++) {
                    if(rows++>=CSV_MAX_ROWS){*status=ODE_STEP_LIMIT;break;}
                    ok=append_format(&used,"%d,%d,%lu",family+1,direction,
                        (unsigned long)csv_page.step[row]);
                    for(int i=0;i<=document->dim && ok;i++) {
                        if(document->list_mask[family]&(1u<<i))
                            ok=append_format(&used,",%.17g",csv_page.row[row][i]);
                        else ok=append_format(&used,",");
                    }
                    if(ok)ok=append_format(&used,"\n");
                    if(!ok){*status=ODE_IO_ERROR;break;}
                }
                if(*status==ODE_OK && used && !append_file(path,used,&removable))
                    *status=ODE_IO_ERROR;
                if(*status!=ODE_OK || !csv_page.more)break;
                start+=csv_page.count;
            }
            if(*status!=ODE_OK)break;
        }
    }
    return finish_export(path,*status==ODE_OK,removable,status);
}

bool storage_stat_csv(const Document *document,CompiledModel *model,int family,int direction,
    const char *directory,char *path,unsigned capacity,OdeStatus *status,
    OdeCancel cancel,void *cancel_context)
{
    *status=ODE_BAD_INPUT;
    if(family<0 || family>=document->nic || (direction!=1 && direction!=-1)
        || !document->list_mask[family])return false;
    *status=ODE_IO_ERROR;
    if(unique_csv(directory,"DIFFSTAT",path,capacity)<0)return false;
    bool removable=true;size_t used=0;int columns=0;bool ok=true;
    for(int i=0;i<=document->dim && ok;i++)if(document->list_mask[family]&(1u<<i)) {
        char label[20];model_variable_label(document,i-1,label,sizeof(label));
        ok=append_format(&used,"%s'%s",columns++ ? ",":"",label);
    }
    ok=ok && columns<=26 && append_format(&used,"\n")
        && append_file(path,used,&removable);
    unsigned start=0,rows=0;*status=ok ? ODE_OK:ODE_IO_ERROR;
    while(*status==ODE_OK) {
        table_page(document,model,family,direction,start,&csv_page,cancel,cancel_context);
        if(csv_page.result.status!=ODE_OK && csv_page.result.status!=ODE_SAMPLE_STOP) {
            *status=csv_page.result.status;break;
        }
        used=0;
        for(unsigned row=0;row<csv_page.count;row++) {
            if(rows++>=CSV_MAX_ROWS){*status=ODE_STEP_LIMIT;break;}
            columns=0;ok=true;
            for(int i=0;i<=document->dim && ok;i++)if(document->list_mask[family]&(1u<<i))
                ok=append_format(&used,"%s%.17g",columns++ ? ",":"",csv_page.row[row][i]);
            if(ok)ok=append_format(&used,"\n");
            if(!ok){*status=ODE_IO_ERROR;break;}
        }
        if(*status==ODE_OK && used && !append_file(path,used,&removable))
            *status=ODE_IO_ERROR;
        if(*status!=ODE_OK || !csv_page.more)break;
        start+=csv_page.count;
    }
    return finish_export(path,*status==ODE_OK,removable,status);
}
