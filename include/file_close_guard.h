#ifndef DIFFEQ_FILE_CLOSE_GUARD_H
#define DIFFEQ_FILE_CLOSE_GUARD_H
#include <gint/fs.h>
#include <stdbool.h>
#include <errno.h>
/* gint 2.11 close() frees its descriptor even when Fugue's close fails.
   Preserve the documented descriptor until its close callback succeeds so a
   safe boundary can make one bounded retry. All calls run in the OS world. */
typedef struct { int pending; } FileCloseGuard;
static inline int file_close_guard(FileCloseGuard *guard,int fd)
{
    fs_descriptor_t const *descriptor=fs_get_descriptor(fd);
    if(!descriptor){errno=EBADF;return -1;}
    if(descriptor->type->close && descriptor->type->close(descriptor->data)<0){
        guard->pending=fd;return -1;
    }
    fs_free_descriptor(fd);
    if(guard->pending==fd)guard->pending=-1;
    return 0;
}
static inline bool file_close_cleanup(FileCloseGuard *guard)
{return guard->pending<0 || file_close_guard(guard,guard->pending)==0;}
#endif
