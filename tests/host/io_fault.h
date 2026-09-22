#ifndef HOST_IO_FAULT_H
#define HOST_IO_FAULT_H
enum {IO_NORMAL,IO_READ,IO_WRITE,IO_OPEN,IO_CLOSE,IO_SHORT,IO_SEEK};
/* Fail after this many successful calls of the selected operation. */
void host_io_fault(int mode,long remaining);
unsigned host_io_failed(void);
unsigned long host_write_calls(void);
#endif
