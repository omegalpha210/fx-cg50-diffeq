#include "expression.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static ExprProgram p;
static ExprScope scope={9,false,true,true};
static double value(const char *text)
{
    double y[]={2,3,4,5,6,7,8,9,10},v;
    ExprError e=expr_compile(text,scope,&p);
    if(e.status) fprintf(stderr,"%s at %d: %s\n",text,e.position,expr_status_text(e.status));
    assert(e.status==EXPR_OK);
    assert(expr_eval(&p,2,y,9,&v)==EXPR_OK); return v;
}
int main(void)
{
    assert(value("-2^2")==-4 && value("2^3^2")==512 && value("2^-3")==.125);
    assert(value("1e-3 + 2.5E+2")==250.001);
    assert(fabs(value("sin(pi/2)+cos(0)+tan(0)")-2)<1e-14);
    assert(fabs(value("ln(e)+log(100)+sqrt(9)+abs(-2)")-8)<1e-14);
    assert(fabs(value("asin(1)+acos(0)+atan(1)")-3.92699081698724154808)<1e-14);
    assert(fabs(value("sinh(0)+cosh(0)+tanh(0)+asinh(0)+acosh(1)+atanh(0)")-1)<1e-14);
    assert(value("y1+y9+x+y")==16);
    assert(expr_compile("A*y",scope,&p).status==EXPR_VARIABLE);
    assert(expr_compile("r+theta",scope,&p).status==EXPR_VARIABLE);
    scope.derivatives=true; assert(value("y1")==3);
    assert(expr_compile("y9",scope,&p).status==EXPR_VARIABLE);
    scope.derivatives=false; scope.allow_y=false;
    assert(expr_compile("y",scope,&p).status==EXPR_VARIABLE);
    scope.allow_y=true;
    const char *bad[]={"","2x","sin x","sin(","1+","xfoo","y0","y10","1..2","(1))","2**3"};
    for(unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);i++) assert(expr_compile(bad[i],scope,&p).status!=EXPR_OK);
    const char *domain[]={"1/0","sqrt(-1)","ln(0)","(-2)^0.5","0^-1","exp(10000)",
        "asin(1.01)","acos(-1.01)","acosh(0.99)","atanh(1)","atanh(-1)"};
    for(unsigned i=0;i<sizeof(domain)/sizeof(domain[0]);i++) {
        assert(expr_compile(domain[i],scope,&p).status==EXPR_OK);
        double v=123; assert(expr_eval(&p,0,NULL,0,&v)!=EXPR_OK && v==123);
    }
    char deep[120]; memset(deep,'(',50);deep[50]='1';memset(deep+51,')',50);deep[101]=0;
    assert(expr_compile(deep,scope,&p).status==EXPR_LIMIT);
    char converted[192];
    assert(expr_to_system("sin(x)-Y1-y2+y",converted,sizeof(converted)));
    assert(!strcmp(converted,"sin(x)-y2-y3+y1"));
    assert(!expr_to_system("y+y",converted,4));
    /* Deterministic malformed-input sweep exercises tokenizer/stack limits. */
    unsigned seed=123; const char alphabet[]="0129xy()+-*/^.sincos e";
    for(int i=0;i<10000;i++) {
        char text[80]; unsigned length=(unsigned)i%79;
        for(unsigned j=0;j<length;j++) { seed=1664525*seed+1013904223; text[j]=alphabet[seed%(sizeof(alphabet)-1)]; }
        text[length]=0;
        if(expr_compile(text,scope,&p).status==EXPR_OK) { double v,y[9]={0}; (void)expr_eval(&p,1,y,9,&v); }
    }
    printf("Expression checks and 10000 malformed-input cases passed; program=%zu bytes.\n",sizeof(p));
    return 0;
}
