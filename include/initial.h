#ifndef DIFFEQ_INITIAL_H
#define DIFFEQ_INITIAL_H
#include "model.h"
typedef enum {IC_LIST_OK,IC_LIST_SYNTAX,IC_LIST_EXPRESSION,IC_LIST_LIMIT,IC_LIST_VALUE} IcListStatus;
typedef struct {unsigned count;double value[ODE_MAX_IC];} InitialValues;
/* Bounded scalar/{expression,...} syntax. On failure, out is unchanged. */
IcListStatus initial_values_parse(const char *text,InitialValues *out);
bool initial_values_format(const Document *d,char *text,unsigned capacity);
void initial_values_apply(Document *d,const InitialValues *values);
const char *initial_values_error(IcListStatus status);
#endif
