#include "initial.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Document d;static CompiledModel m;
int main(void)
{
    InitialValues v={.count=1,.value={77}};
    assert(initial_values_parse("0",&v)==IC_LIST_OK && v.count==1 && v.value[0]==0);
    assert(initial_values_parse("{0,1}",&v)==IC_LIST_OK && v.count==2 && v.value[1]==1);
    assert(initial_values_parse(" {0, 1, -1} ",&v)==IC_LIST_OK && v.count==3 && v.value[2]==-1);
    assert(initial_values_parse("{sin(pi/2),1/4,ln(e),1e-3,1}",&v)==IC_LIST_OK);
    assert(v.count==5 && v.value[0]==1 && v.value[1]==.25 && fabs(v.value[2]-1)<1e-15 && v.value[4]==1);
    const char *bad[]={"","{}","{ }","{1,}","{,1}","{1,,2}","{1,2","1,2}","{1}{2}",
        "{{1}}","{(1,2)}","{sin(1}","1,2","y","A","nan","inf","1e309","sqrt(-1)","{0,1/0}","{1e101}"};
    for(unsigned i=0;i<sizeof(bad)/sizeof(*bad);i++) {
        InitialValues before=v;assert(initial_values_parse(bad[i],&v)!=IC_LIST_OK);
        assert(!memcmp(&v,&before,sizeof(v)));
    }
    assert(initial_values_parse("{0,1,2,3,4,5,6,7,8,9,10}",&v)==IC_LIST_LIMIT);
    assert(initial_values_parse("{0,1,2,3,4,5,6,7,8}",&v)==IC_LIST_OK);
    assert(v.count==9);
    assert(initial_values_parse("{0,1,2,3,4,5,6,7,8,9}",&v)==IC_LIST_OK && v.count==10);
    char long_values[EXPR_TEXT]="{";
    for(int i=0;i<10;i++)strcat(long_values,i ? ",1.23456789012345":"1.23456789012345");
    strcat(long_values,"}");assert(strlen(long_values)<192);
    assert(initial_values_parse(long_values,&v)==IC_LIST_OK && v.count==10);
    char too_long[EXPR_TEXT+1];memset(too_long,'1',EXPR_TEXT);too_long[EXPR_TEXT]=0;
    InitialValues before=v;assert(initial_values_parse(too_long,&v)==IC_LIST_LENGTH);
    assert(!memcmp(&v,&before,sizeof(v)));
    assert(strcmp(initial_values_error(IC_LIST_LENGTH),initial_values_error(IC_LIST_LIMIT)));
    assert(initial_values_parse("{0,1,2,3,4,5,6,7,8,9}",&v)==IC_LIST_OK);
    model_defaults(&d,EQ_SEPARABLE,1);d.ic[0].x=0;initial_values_apply(&d,&v);
    assert(d.nic==10 && d.ic[9].x==0 && d.ic[9].y[0]==9);
    OdeSettings range=d.solver;range.h=.0005;
    assert(model_preflight(&d,&range).status==ODE_WORK_LIMIT); /* 10 * 2 * 12000 */
    assert(initial_values_parse("{0,1}",&v)==IC_LIST_OK);initial_values_apply(&d,&v);
    assert(model_compile(&d,&m).expression.status==EXPR_OK);
    assert(model_color(&d,0,0)!=model_color(&d,1,0));
    for(int f=0;f<2;f++) {
        OdeResult r=model_value_at(&d,&m,f,1,NULL,NULL);assert(r.status==ODE_OK);
        assert(fabs(r.y[0]-(f ? 1:-tanh(1)))<2e-6);
    }
    char text[EXPR_TEXT];assert(initial_values_format(&d,text,sizeof(text)) && !strcmp(text,"{0,1}"));
    for(int kind=EQ_SECOND;kind<=EQ_SYSTEM;kind++)for(int dim=1;dim<=9;dim++) {
        model_defaults(&d,(EquationKind)kind,dim);assert(d.nic==1);
        for(int j=0;j<d.dim;j++)d.ic[0].y[j]=j+1;
        assert(model_validate(&d)==ODE_OK && model_compile(&d,&m).expression.status==EXPR_OK);
    }
    puts("IC scalar/lists/numeric expressions/duplicates/transactional errors/10-item bound and separate 191-character limit/work budget/manual two solutions/full higher-state vectors passed.");
}
