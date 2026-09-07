#include "ode.h"
#include "model.h"
#include "gsolve.h"
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
static unsigned long rhs_calls;
unsigned long host_rhs_calls(void){return rhs_calls;}
unsigned long host_solves(void);
static unsigned long solves,searches,reads,writes,opens,closes;
void __cyg_profile_func_enter(void *function,void *caller)
{
    (void)caller;
    /* Function-address comparison is the compiler instrumentation ABI. */
    if(function==(void *)(uintptr_t)ode_integrate || function==(void *)(uintptr_t)ode_rk45_integrate)solves++;
    if(function==(void *)(uintptr_t)model_rhs)rhs_calls++;
    if(function==(void *)(uintptr_t)gsolve_search ||
       function==(void *)(uintptr_t)gsolve_intersections)searches++;
}
void __cyg_profile_func_exit(void *function,void *caller) {(void)function;(void)caller;}
ssize_t host_write(int fd,const void *data,size_t size) {writes++;return write(fd,data,size);}
ssize_t host_read(int fd,void *data,size_t size) {reads++;return read(fd,data,size);}
int host_open(const char *path,int flags,...)
{
    va_list args;va_start(args,flags);int mode=va_arg(args,int);va_end(args);
    opens++;return open(path,flags,mode);
}
int host_close(int fd) {closes++;return close(fd);}
void host_metrics_print(void)
{
    printf("METRICS solves=%lu searches=%lu reads=%lu writes=%lu opens=%lu closes=%lu\n",
        solves,searches,reads,writes,opens,closes);
}

unsigned long host_solves(void){return solves;}
