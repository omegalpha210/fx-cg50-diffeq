#ifndef DIFFEQ_EXPRESSION_H
#define DIFFEQ_EXPRESSION_H
#include <stdbool.h>
#include <stdint.h>
#define EXPR_TEXT 192
#define EXPR_CODE 192
#define EXPR_LITERALS 64
#define EXPR_STACK 64
#define EXPR_CONSTANTS 28

typedef enum {EXPR_OK,EXPR_SYNTAX,EXPR_LIMIT,EXPR_VARIABLE,
    EXPR_DOMAIN,EXPR_NONFINITE} ExprStatus;
typedef struct {ExprStatus status; int position;} ExprError;
typedef struct {
    uint8_t op[EXPR_CODE],arg[EXPR_CODE];
    double literal[EXPR_LITERALS];
    uint16_t length;
    uint8_t literals,stack_size;
} ExprProgram;
typedef struct {
    int dim;
    bool derivatives,allow_x,allow_y;
    const double *constants;
} ExprScope;
ExprError expr_compile(const char *text,ExprScope scope,ExprProgram *out);
ExprStatus expr_eval(const ExprProgram *p,double x,const double *y,int n,double *out);
const char *expr_status_text(ExprStatus status);
bool expr_to_system(const char *src,char *dst,unsigned capacity);
#endif
