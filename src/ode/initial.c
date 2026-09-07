#include "initial.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

IcListStatus initial_values_parse(const char *text,InitialValues *out)
{
    if(!text || !out)return IC_LIST_SYNTAX;
    size_t length=0;while(length<EXPR_TEXT && text[length])length++;
    if(length==EXPR_TEXT)return IC_LIST_LENGTH;
    const char *begin=text,*end=text+length;
    while(begin<end && isspace((unsigned char)*begin))begin++;
    while(end>begin && isspace((unsigned char)end[-1]))end--;
    if(begin==end)return IC_LIST_SYNTAX;
    bool list=*begin=='{';
    if(list){if(end[-1]!='}')return IC_LIST_SYNTAX;begin++;end--;}
    InitialValues values={0};const char *item=begin;int depth=0;
    for(const char *p=begin;p<=end;p++) {
        char ch=p==end ? 0:*p;
        if(ch=='{' || ch=='}')return IC_LIST_SYNTAX;
        if(ch=='(')depth++;
        if(ch==')' && --depth<0)return IC_LIST_SYNTAX;
        if(ch && !(ch==',' && depth==0))continue;
        if(depth || (ch==',' && !list))return IC_LIST_SYNTAX;
        if(values.count==ODE_MAX_IC)return IC_LIST_LIMIT;
        size_t n=(size_t)(p-item);char expression[EXPR_TEXT];
        if(!n)return IC_LIST_SYNTAX;
        memcpy(expression,item,n);expression[n]=0;
        ExprProgram program;ExprError e=expr_compile(expression,(ExprScope){0,false,false,false},&program);
        if(e.status!=EXPR_OK)return e.status==EXPR_NONFINITE ? IC_LIST_VALUE:IC_LIST_EXPRESSION;
        double value;ExprStatus status=expr_eval(&program,0,NULL,0,&value);
        if(status!=EXPR_OK || !isfinite(value) || fabs(value)>1e100)return IC_LIST_VALUE;
        values.value[values.count++]=value;item=p+1;
    }
    *out=values;return IC_LIST_OK; /* Duplicate values intentionally remain independent ICs. */
}
bool initial_values_format(const Document *d,char *text,unsigned capacity)
{
    if(!capacity)return false;
    text[0]=0;unsigned used=0;
    for(int i=0;i<d->nic;i++) {
        int n=snprintf(text+used,capacity-used,"%s%.12g%s",i ? ",":(d->nic>1 ? "{":""),
            d->ic[i].y[0],d->nic>1 && i+1==d->nic ? "}":"");
        if(n<0 || (unsigned)n>=capacity-used)return false;
        used+=(unsigned)n;
    }
    return true;
}
void initial_values_apply(Document *d,const InitialValues *values)
{
    for(unsigned i=0;i<values->count;i++) {
        d->ic[i].x=d->ic[0].x;d->ic[i].y[0]=values->value[i];
    }
    d->nic=(int)values->count;
    model_output_color(d,0,model_color(d,0,0));
}
const char *initial_values_error(IcListStatus status)
{
    switch(status) {
        case IC_LIST_LIMIT:return "Too many initial values\nMax: 10";
        case IC_LIST_LENGTH:return "Input too long\nMax: 191 characters";
        case IC_LIST_EXPRESSION:return "Use numeric expressions separated by commas.";
        case IC_LIST_VALUE:return "Each value must be finite, defined and at most 1e100 in magnitude.";
        default:return "Enter one value or a nonempty list, such as {0,1}.";
    }
}
