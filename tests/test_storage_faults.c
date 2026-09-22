#include "storage.h"
#include "io_fault.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static App a,b;
static Document before;
int main(void)
{
    char directory[]="fault-test-XXXXXX",path[256];assert(mkdtemp(directory));
    app_initialize(&a,directory);strcpy(a.doc.text[0],"y");
    assert(storage_save(&a,directory));
    for(int n=0;n<3;n++) {
        strcpy(a.doc.text[0],"2*y");host_io_fault(IO_WRITE,n);
        assert(!storage_save(&a,directory));host_io_fault(IO_NORMAL,0);
        assert(storage_load(&b,directory) && !strcmp(b.doc.text[0],"y"));
    }
    before=b.doc;
    /* A transient read error during SAVE must not be mistaken for an empty
       store and truncate its only known-good slot. */
    for(int n=0;n<40;n++) {
        unsigned long writes=host_write_calls();host_io_fault(IO_READ,n);
        bool saved=storage_save(&a,directory);
        if(host_io_failed())assert(!saved);
        host_io_fault(IO_NORMAL,0);
        if(n==0)assert(host_write_calls()==writes);
        /* Post-write verification may itself fail. Either way slot0, the
           previous good generation, must still load byte-for-byte. */
        snprintf(path,sizeof(path),"%s/DIFFEQ1.dat",directory);remove(path);
        assert(storage_load(&b,directory) && !memcmp(&b.doc,&before,sizeof(before)));
    }
    const int failures[]={IO_READ,IO_OPEN,IO_CLOSE,IO_SEEK};
    for(unsigned i=0;i<4;i++) {
        host_io_fault(failures[i],0);assert(!storage_load(&b,directory));
        assert(!memcmp(&b.doc,&before,sizeof(before)));host_io_fault(IO_NORMAL,0);
        unsigned long writes=host_write_calls();host_io_fault(failures[i],0);
        assert(!storage_save(&a,directory) && host_write_calls()==writes);host_io_fault(IO_NORMAL,0);
    }
    host_io_fault(IO_SHORT,0);assert(storage_save(&a,directory) && storage_load(&b,directory));
    assert(!strcmp(b.doc.text[0],"2*y"));host_io_fault(IO_NORMAL,0);
    assert(model_compile(&a.doc,&a.model).expression.status==EXPR_OK);
    OdeStatus status;host_io_fault(IO_WRITE,0);
    assert(!storage_csv(&a.doc,&a.model,directory,path,sizeof(path),&status,NULL,NULL));
    assert(status==ODE_IO_ERROR && access(path,F_OK)!=0);host_io_fault(IO_NORMAL,0);
    host_io_fault(IO_CLOSE,0);assert(!storage_save(&a,directory));host_io_fault(IO_NORMAL,0);
    assert(storage_load(&b,directory)); /* Close error is reported even if bytes reached disk. */
    for(int i=0;i<2;i++){snprintf(path,sizeof(path),"%s/DIFFEQ%d.dat",directory,i);assert(!remove(path));}
    assert(!rmdir(directory));
    puts("Storage: interrupted writes/short reads+writes/open/read/close failures; old session/live document retained, incomplete CSV removed PASS.");
}
