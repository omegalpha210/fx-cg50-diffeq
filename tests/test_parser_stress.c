#include "expression.h"
#include "initial.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static uint32_t seed=0x9cd72f31;
static uint32_t random_word(void) {seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
int main(void)
{
    const char *base[]={"sin(x)+y","y1*y2+exp(-x)","sqrt(x-1)","(x+1)^(-2)","1e-200","atanh(y)","{1,2,3}"};
    const char alphabet[]="xy1234567890.+-*/^(){} ,esinlog\xff\x80\t";
    double y[9]={1,2,3,4,5,6,7,8,9},value;
    unsigned accepted=0,rejected=0;
    for(unsigned i=0;i<40000;i++) {
        char text[EXPR_TEXT+8];unsigned length;
        if(i%2) {
            strcpy(text,base[random_word()%7]);length=(unsigned)strlen(text);
            for(unsigned j=0;j<1+i%7;j++)text[random_word()%length]=alphabet[random_word()%(sizeof(alphabet)-1)];
        } else {
            length=random_word()%(sizeof(text)-1);
            for(unsigned j=0;j<length;j++)text[j]=alphabet[random_word()%(sizeof(alphabet)-1)];
            text[length]=0;
        }
        ExprProgram p;ExprError e=expr_compile(text,(ExprScope){9,false,true,true},&p);
        assert(e.status>=EXPR_OK && e.status<=EXPR_NONFINITE);
        if(e.status==EXPR_OK) {
            accepted++;assert(p.length>0 && p.length<=EXPR_CODE && p.stack_size<=EXPR_STACK);
            ExprStatus s=expr_eval(&p,(int)(i%5)-2,y,9,&value);
            assert(s!=EXPR_SYNTAX && s!=EXPR_LIMIT && s!=EXPR_VARIABLE);
            if(s==EXPR_OK)assert(isfinite(value));
        } else {rejected++;assert(!p.length && e.position>=0 && e.position<=(int)length);}
        InitialValues values;IcListStatus s=initial_values_parse(text,&values);
        if(s==IC_LIST_OK)assert(values.count>0 && values.count<=10);
        /* Bounded conversion must not overwrite either adjacent canary. */
        unsigned char converted[EXPR_TEXT+2];memset(converted,0xa5,sizeof(converted));
        unsigned cap=1+i%EXPR_TEXT;expr_to_system(text,(char *)converted+1,cap);
        assert(converted[0]==0xa5 && converted[cap+1]==0xa5);
    }
    char deep[EXPR_TEXT];memset(deep,'(',80);deep[80]='1';memset(deep+81,')',80);deep[161]=0;
    ExprProgram p;assert(expr_compile(deep,(ExprScope){0,false,false,false},&p).status==EXPR_LIMIT);
    for(unsigned i=0;i<20000;i++) {
        unsigned char *bytes=(unsigned char *)&p;
        for(unsigned j=0;j<sizeof(p);j++)bytes[j]=(unsigned char)random_word();
        p.length=(uint16_t)(random_word()%(EXPR_CODE+3));p.literals=(uint8_t)(random_word()%(EXPR_LITERALS+3));
        ExprStatus s=expr_eval(&p,1,y,9,&value);assert(s>=EXPR_OK && s<=EXPR_NONFINITE);
        if(s==EXPR_OK)assert(isfinite(value));
    }
    printf("40,000 malformed/mutated texts (%u accepted/%u rejected), IC/conversion canaries, depth cap and20,000 corrupt programs PASS.\n",accepted,rejected);
}
