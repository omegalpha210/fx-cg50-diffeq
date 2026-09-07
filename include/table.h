#ifndef DIFFEQ_TABLE_H
#define DIFFEQ_TABLE_H
#include "model.h"
#define TABLE_ROWS 7
typedef struct {
    double row[TABLE_ROWS+1][ODE_MAX_DIM+1];
    uint32_t step[TABLE_ROWS+1];
    unsigned count;
    bool more;
    OdeResult result;
} TablePage;
void table_page(const Document *d,CompiledModel *m,int family,int direction,unsigned start,
    TablePage *page,OdeCancel cancel,void *cancel_ctx);
unsigned table_last_page(const Document *d,CompiledModel *m,int family,int direction,
    OdeStatus *status,OdeCancel cancel,void *cancel_ctx);
#endif
