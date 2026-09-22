#include "ode.h"
#include "model.h"
#include "gsolve.h"
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "io_fault.h"
static int fault_mode;
static long fault_remaining;
static unsigned faults;
void host_io_fault(int mode,long remaining) {fault_mode=mode;fault_remaining=remaining;faults=0;}
unsigned host_io_failed(void) {return faults;}
static bool fail(int mode)
{if(fault_mode!=mode)return false;if(fault_remaining>0){fault_remaining--;return false;}faults++;errno=EIO;return true;}
static unsigned long rhs_calls;
unsigned long host_rhs_calls(void){return rhs_calls;}
unsigned long host_solves(void);
static unsigned long solves,searches,reads,writes,opens,closes;
unsigned long host_write_calls(void) {return writes;}
off_t host_lseek(int fd,off_t offset,int whence)
{return fail(IO_SEEK) ? -1:lseek(fd,offset,whence);}
void __cyg_profile_func_enter(void *function,void *caller)
{
    (void)caller;
    /* Function-address comparison is the compiler instrumentation ABI. */
    if(function==(void *)(uintptr_t)ode_integrate_control || function==(void *)(uintptr_t)ode_rk45_control)solves++;
    if(function==(void *)(uintptr_t)model_rhs)rhs_calls++;
    if(function==(void *)(uintptr_t)gsolve_search ||
       function==(void *)(uintptr_t)gsolve_intersections)searches++;
}
void __cyg_profile_func_exit(void *function,void *caller) {(void)function;(void)caller;}
ssize_t host_write(int fd,const void *data,size_t size)
{writes++;if(fail(IO_WRITE))return -1;if(fault_mode==IO_SHORT && size>3)size=3;return write(fd,data,size);}
ssize_t host_read(int fd,void *data,size_t size)
{reads++;if(fail(IO_READ))return -1;if(fault_mode==IO_SHORT && size>3)size=3;return read(fd,data,size);}
int host_open(const char *path,int flags,...)
{
    va_list args;va_start(args,flags);int mode=va_arg(args,int);va_end(args);
    opens++;if(fail(IO_OPEN))return -1;return open(path,flags,mode);
}
int host_close(int fd) {closes++;int result=close(fd);return fail(IO_CLOSE) ? -1:result;}
void host_metrics_print(void)
{
    const unsigned char *r=(const unsigned char *)solver_report();uint32_t hash=2166136261u;
    for(unsigned i=0;i<sizeof(SolverReport);i++){hash^=r[i];hash*=16777619u;}
    printf("REPORT %08x\n",hash);
    printf("METRICS solves=%lu searches=%lu reads=%lu writes=%lu opens=%lu closes=%lu\n",
        solves,searches,reads,writes,opens,closes);
}

unsigned long host_solves(void){return solves;}
