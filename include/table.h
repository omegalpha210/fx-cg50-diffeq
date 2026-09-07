#ifndef DIFFEQ_TABLE_H
#define DIFFEQ_TABLE_H
#include "model.h"
#define TABLE_ROWS 7
#define TABLE_COLUMNS ((ODE_MAX_IC>ODE_MAX_DIM) ? ODE_MAX_IC:ODE_MAX_DIM)
#define TABLE_EXTRA (2*ODE_MAX_IC+3) /* endpoints, window starts, optional zero */
typedef struct {
    double origin,spacing,xmin,xmax,extra[TABLE_EXTRA];
    int first,last,columns[TABLE_COLUMNS],count;
    unsigned extras,total,mid;
    OdeStatus low,high,status;
    bool solutions;
} TableIndex;
typedef struct {
    double row[TABLE_ROWS+1][TABLE_COLUMNS+1];
    uint16_t valid[TABLE_ROWS+1];
    unsigned count;
    bool more;
    OdeResult result;
} TablePage;
/* Small index of a regular output grid plus reachable terminal points. No rows
   or trajectories are retained here; both integration branches remain internal. */
OdeStatus table_index_build(const Document *d,CompiledModel *m,TableIndex *index,
    OdeCancel cancel,void *cancel_ctx);
double table_x_at(const TableIndex *index,unsigned row);
unsigned table_bottom(const TableIndex *index);
unsigned table_nearest(const TableIndex *index,double x);
void table_column_label(const Document *d,const TableIndex *index,int column,char *out,unsigned size);
void table_read_page(const Document *d,CompiledModel *m,const TableIndex *index,unsigned start,
    TablePage *page,OdeCancel cancel,void *cancel_ctx);
#endif
