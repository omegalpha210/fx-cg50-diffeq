#include "expression.h"
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum {OP_NUM,OP_X,OP_Y,OP_ADD,OP_SUB,OP_MUL,OP_DIV,OP_POW,OP_NEG,
    OP_SIN,OP_COS,OP_TAN,OP_EXP,OP_LN,OP_LOG,OP_SQRT,OP_ABS,
    OP_ASIN,OP_ACOS,OP_ATAN,OP_SINH,OP_COSH,OP_TANH,OP_ASINH,OP_ACOSH,OP_ATANH};
typedef struct {
    const char *text,*cur;
    ExprProgram *p;
    ExprScope scope;
    ExprError error;
    int stack,depth;
} Parser;
static void fail(Parser *p,ExprStatus status)
{ if(p->error.status==EXPR_OK) p->error=(ExprError){status,(int)(p->cur-p->text)}; }
static void space(Parser *p) { while(isspace((unsigned char)*p->cur)) p->cur++; }
static bool accept(Parser *p,char c)
{ space(p); if(*p->cur!=c) return false; p->cur++; return true; }
static void emit(Parser *p,int op,int arg)
{
    if(p->error.status!=EXPR_OK) return;
    int change=op<=OP_Y ? 1 : (op<=OP_POW ? -1 : 0);
    p->stack+=change;
    if(p->p->length>=EXPR_CODE || p->stack<1 || p->stack>EXPR_STACK) {
        fail(p,EXPR_LIMIT); return;
    }
    if(p->stack>p->p->stack_size) p->p->stack_size=(uint8_t)p->stack;
    unsigned i=p->p->length++;
    p->p->op[i]=(uint8_t)op; p->p->arg[i]=(uint8_t)arg;
}
static void number(Parser *p,double value)
{
    if(!isfinite(value)) { fail(p,EXPR_NONFINITE); return; }
    if(p->p->literals>=EXPR_LITERALS) { fail(p,EXPR_LIMIT); return; }
    unsigned i=p->p->literals++;
    p->p->literal[i]=value; emit(p,OP_NUM,(int)i);
}
static void addition(Parser *p);
static void unary(Parser *p);
static void primary(Parser *p)
{
    if(p->error.status!=EXPR_OK) return;
    space(p);
    if(accept(p,'(')) {
        addition(p);
        if(!accept(p,')')) fail(p,EXPR_SYNTAX);
        return;
    }
    if(isdigit((unsigned char)*p->cur) || *p->cur=='.') {
        char *end;
        double value=strtod(p->cur,&end);
        if(end==p->cur) { fail(p,EXPR_SYNTAX); return; }
        p->cur=end; number(p,value); return;
    }
    if(!isalpha((unsigned char)*p->cur)) { fail(p,EXPR_SYNTAX); return; }
    const char *start=p->cur;
    while(isalnum((unsigned char)*p->cur)) p->cur++;
    unsigned len=(unsigned)(p->cur-start);
    char name[16];
    if(len>=sizeof(name)) { fail(p,EXPR_VARIABLE); return; }
    memcpy(name,start,len); name[len]=0;
    if(!strcmp(name,"x") || !strcmp(name,"X")) {
        if(!p->scope.allow_x) fail(p,EXPR_VARIABLE);
        else emit(p,OP_X,0);
        return;
    }
    if(name[0]=='y' || name[0]=='Y') {
        int index=0;
        if(len!=1) {
            if(len!=2 || name[1]<'1' || name[1]>'9') { fail(p,EXPR_VARIABLE); return; }
            index=name[1]-'1'+(p->scope.derivatives ? 1 : 0);
        }
        if(!p->scope.allow_y || index>=p->scope.dim) fail(p,EXPR_VARIABLE);
        else emit(p,OP_Y,index);
        return;
    }
    if(!strcmp(name,"pi")) { number(p,3.14159265358979323846); return; }
    if(!strcmp(name,"e")) { number(p,2.71828182845904523536); return; }
    int constant=-1;
    if(len==1 && name[0]>='A' && name[0]<='Z') constant=name[0]-'A';
    if(!strcmp(name,"r")) constant=26;
    if(!strcmp(name,"theta")) constant=27;
    if(constant>=0) {
        if(!p->scope.constants) fail(p,EXPR_VARIABLE);
        else number(p,p->scope.constants[constant]);
        return;
    }
    static const char *const names[]={"sin","cos","tan","exp","ln","log","sqrt","abs",
        "asin","acos","atan","sinh","cosh","tanh","asinh","acosh","atanh"};
    int op=-1;
    for(unsigned i=0;i<sizeof(names)/sizeof(names[0]);i++)
        if(!strcmp(name,names[i])) op=OP_SIN+(int)i;
    if(op<0) { fail(p,EXPR_VARIABLE); return; }
    if(!accept(p,'(')) { fail(p,EXPR_SYNTAX); return; }
    addition(p);
    if(!accept(p,')')) fail(p,EXPR_SYNTAX);
    emit(p,op,0);
}
static void power(Parser *p)
{
    primary(p);
    if(accept(p,'^')) { unary(p); emit(p,OP_POW,0); }
}
static void unary(Parser *p)
{
    if(p->error.status!=EXPR_OK) return;
    if(++p->depth>32) { fail(p,EXPR_LIMIT); p->depth--; return; }
    if(accept(p,'-')) { unary(p); emit(p,OP_NEG,0); }
    else if(accept(p,'+')) unary(p);
    else power(p);
    p->depth--;
}
static void product(Parser *p)
{
    unary(p);
    while(p->error.status==EXPR_OK) {
        if(accept(p,'*')) { unary(p); emit(p,OP_MUL,0); }
        else if(accept(p,'/')) { unary(p); emit(p,OP_DIV,0); }
        else break;
    }
}
static void addition(Parser *p)
{
    product(p);
    while(p->error.status==EXPR_OK) {
        if(accept(p,'+')) { product(p); emit(p,OP_ADD,0); }
        else if(accept(p,'-')) { product(p); emit(p,OP_SUB,0); }
        else break;
    }
}
ExprError expr_compile(const char *text,ExprScope scope,ExprProgram *out)
{
    if(!text || !out || scope.dim<0 || scope.dim>9) return (ExprError){EXPR_VARIABLE,0};
    memset(out,0,sizeof(*out));
    if(strlen(text)>=EXPR_TEXT) return (ExprError){EXPR_LIMIT,EXPR_TEXT-1};
    Parser p={.text=text,.cur=text,.p=out,.scope=scope};
    addition(&p); space(&p);
    if(*p.cur || p.stack!=1) fail(&p,EXPR_SYNTAX);
    if(p.error.status!=EXPR_OK) out->length=0;
    return p.error;
}
ExprStatus expr_eval(const ExprProgram *p,double x,const double *y,int n,double *out)
{
    if(!p || !out || !p->length || p->length>EXPR_CODE || p->literals>EXPR_LITERALS)
        return EXPR_SYNTAX;
    double stack[EXPR_STACK]; int top=0;
    for(unsigned i=0;i<p->length;i++) {
        int op=p->op[i],arg=p->arg[i]; double v=0;
        if(op<=OP_Y) {
            if(top>=EXPR_STACK) return EXPR_LIMIT;
            if(op==OP_NUM) { if(arg>=p->literals) return EXPR_SYNTAX; v=p->literal[arg]; }
            else if(op==OP_X) v=x;
            else { if(!y || arg>=n) return EXPR_VARIABLE; v=y[arg]; }
            stack[top++]=v;
        } else {
            if(top < (op<=OP_POW ? 2 : 1)) return EXPR_SYNTAX;
            double b=stack[top-1],a=op<=OP_POW ? stack[top-2] : 0;
            switch(op) {
                case OP_ADD:v=a+b;break; case OP_SUB:v=a-b;break;
                case OP_MUL:v=a*b;break;
                case OP_DIV:if(b==0) return EXPR_DOMAIN; v=a/b;break;
                case OP_POW:v=pow(a,b);break; case OP_NEG:v=-b;break;
                case OP_SIN:v=sin(b);break; case OP_COS:v=cos(b);break;
                case OP_TAN:v=tan(b);break; case OP_EXP:v=exp(b);break;
                case OP_LN:if(b<=0) return EXPR_DOMAIN;v=log(b);break;
                case OP_LOG:if(b<=0) return EXPR_DOMAIN;v=log10(b);break;
                case OP_SQRT:if(b<0) return EXPR_DOMAIN;v=sqrt(b);break;
                case OP_ABS:v=fabs(b);break;
                case OP_ASIN:if(b < -1 || b > 1)return EXPR_DOMAIN;v=asin(b);break;
                case OP_ACOS:if(b < -1 || b > 1)return EXPR_DOMAIN;v=acos(b);break;
                case OP_ATAN:v=atan(b);break;
                case OP_SINH:v=sinh(b);break;case OP_COSH:v=cosh(b);break;
                case OP_TANH:v=tanh(b);break;case OP_ASINH:v=asinh(b);break;
                case OP_ACOSH:if(b<1)return EXPR_DOMAIN;v=acosh(b);break;
                case OP_ATANH:if(b<=-1 || b>=1)return EXPR_DOMAIN;v=atanh(b);break;
                default:return EXPR_SYNTAX;
            }
            if(op<=OP_POW) top--;
            stack[top-1]=v;
        }
        if(isnan(v)) return EXPR_DOMAIN;
        if(!isfinite(v)) return EXPR_NONFINITE;
    }
    if(top!=1) return EXPR_SYNTAX;
    *out=stack[0]; return EXPR_OK;
}
const char *expr_status_text(ExprStatus s)
{
    static const char *const names[]={"OK","Syntax error","Expression too complex",
        "Variable not allowed","Math domain error","NaN or infinity"};
    return (unsigned)s<sizeof(names)/sizeof(names[0]) ? names[s] : "Expression error";
}
bool expr_to_system(const char *src,char *dst,unsigned capacity)
{
    unsigned pos=0;
    while(*src) {
        const char *start=src;
        char replacement[4]; unsigned len;
        if(isalpha((unsigned char)*src)) {
            while(isalnum((unsigned char)*src)) src++;
            len=(unsigned)(src-start);
            if((start[0]=='y' || start[0]=='Y') && (len==1
                || (len==2 && start[1]>='1' && start[1]<='8'))) {
                replacement[0]='y'; replacement[1]=len==1 ? '1' : start[1]+1;
                start=replacement; len=2;
            }
        } else { src++; len=1; }
        if(pos+len>=capacity) return false;
        memcpy(dst+pos,start,len); pos+=len;
    }
    if(!capacity) return false;
    dst[pos]=0; return true;
}
