#include "file_close_guard.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
static bool allocated=true,native_open=true,fail=true;
static unsigned closes,frees;
static int native_close(void *data)
{
    assert(data==&native_open && native_open);closes++;
    if(fail)return -1;
    native_open=false;return 0;
}
static const fs_descriptor_type_t type={native_close};
static const fs_descriptor_t descriptor={&type,&native_open};
const fs_descriptor_t *fs_get_descriptor(int fd)
{return fd==3 && allocated ? &descriptor:NULL;}
void fs_free_descriptor(int fd)
{assert(fd==3 && allocated && !native_open);allocated=false;frees++;}
int main(void)
{
    FileCloseGuard guard={-1};assert(file_close_cleanup(&guard));
    assert(file_close_guard(&guard,3)<0 && guard.pending==3 && !frees);
    assert(!file_close_cleanup(&guard) && closes==2 && allocated && native_open);
    fail=false;assert(file_close_cleanup(&guard) && closes==3 && frees==1);
    assert(guard.pending==-1 && !allocated && !native_open);
    for(unsigned i=0;i<100;i++)assert(file_close_cleanup(&guard));
    assert(closes==3 && frees==1);
    assert(file_close_guard(&guard,3)<0 && errno==EBADF && frees==1);
    puts("Native close guard: retain failed descriptor, bounded retry, free once, no double-close PASS");
    return 0;
}
