#include "model.h"
#include <string.h>
#include <math.h>
void model_work_begin(CompiledModel *m)
{
    memset(&m->work,0,sizeof(m->work));m->event_hits=m->event_evals=m->event_unavailable=0;
    m->initial_events=0;
}
double model_output_spacing(const Document *d,const OdeSettings *range)
{
    return d->adaptive.method==ODE_RK45 ? fmax(model_xdot(&d->view),(range->xmax-range->xmin)/1024):range->h;
}
