#ifndef DIFFEQ_PHASE_GRAPH_H
#define DIFFEQ_PHASE_GRAPH_H
#include "graph.h"
#include "phase.h"
#define PHASE_FIELD_COLUMNS 20
#define PHASE_CONTOUR_GRID 20
OdeStatus graph_phase_preflight(const Document *d,const CompiledModel *m,OdeCancel cancel,void *ctx);
void graph_phase_layers(const Document *d,const CompiledModel *m);
void graph_phase_markers(const Document *d,int selected);
void graph_phase_reset(void);
OdeStatus graph_phase_search(const Document *d,const CompiledModel *m,OdeCancel cancel,void *ctx);
const PhaseResults *graph_phase_results(void);
#endif
