#ifndef DIFFEQ_STORAGE_H
#define DIFFEQ_STORAGE_H
#include "app.h"
/* Native path / maps to calculator storage memory through gint's Fugue FS. */
/* Session load/save reuse App's model/load union for bounded staging. Call
   them only at a safe UI boundary and recompile before using App.model. */
bool storage_load(App *app,const char *directory);
bool storage_save(App *app,const char *directory);
bool storage_csv(const Document *d,CompiledModel *m,const char *directory,
    char *path,unsigned capacity,OdeStatus *status,OdeCancel cancel,void *cancel_ctx);
bool storage_stat_csv(const Document *d,CompiledModel *m,int family,int direction,
    const char *directory,char *path,unsigned capacity,OdeStatus *status,
    OdeCancel cancel,void *cancel_ctx);
#endif
