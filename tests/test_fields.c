#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static key_event_t key(int code){return (key_event_t){.key=(unsigned)code,.type=KEYEV_DOWN};}
int main(void)
{
    UiInlineEdit edit={0};int selected=0;
    assert(ui_field_complete(KEY_EXE,false,&selected,2)==KEY_F6);
    assert(selected==0 && !edit.active);
    assert(ui_field_complete(KEY_EXE,true,&selected,2)==0);
    assert(selected==1 && !edit.active);
    assert(ui_field_complete(KEY_EXE,false,&selected,2)==KEY_F6);
    selected=0;assert(ui_field_complete(KEY_EXIT,true,&selected,2)==KEY_EXIT && selected==0);
    assert(ui_field_complete(KEY_EXE,true,&selected,2)==0 && selected==1);
    selected=0;assert(ui_list_complete(KEY_EXE,false,&selected,2)==0 && selected==1);
    assert(ui_list_complete(KEY_EXE,true,&selected,2)==0 && selected==1);
    assert(ui_list_complete(KEY_EXE,false,&selected,2)==KEY_F6);
    Document d;
    for(int k=EQ_SEPARABLE;k<=EQ_SYSTEM;k++) {
        model_defaults(&d,(EquationKind)k,3);
        assert(ui_equation_variables(&d)==(k==EQ_SYSTEM ? 3:(k==EQ_HIGHER ? 2:0)));
    }
    ui_field_select(&edit,key(KEY_UP),"123.45",&selected,2);
    assert(selected==0 && !edit.active);
    ui_field_select(&edit,key(KEY_LEFT),"123.45",&selected,2);
    assert(edit.active && edit.cursor==0 && !strcmp(edit.text,"123.45"));
    ui_inline_key(&edit,key(KEY_2));assert(!strcmp(edit.text,"2123.45"));
    edit.active=false;ui_field_select(&edit,key(KEY_RIGHT),"123.45",&selected,2);
    assert(edit.active && edit.cursor==6);
    ui_inline_key(&edit,key(KEY_DEL));assert(!strcmp(edit.text,"123.4"));
    edit.active=false;ui_field_select(&edit,key(KEY_2),"123.45",&selected,2);
    assert(edit.active && !strcmp(edit.text,"2") && edit.cursor==1);
    ui_inline_begin(&edit,"",false);
    for(int i=0;i<191;i++)ui_inline_insert(&edit,"1");
    assert(strlen(edit.text)==191 && edit.cursor==191 && !edit.limited);
    ui_inline_insert(&edit,"22");assert(strlen(edit.text)==191 && edit.cursor==191 && edit.limited);
    ui_inline_key(&edit,key(KEY_DEL));assert(strlen(edit.text)==190 && !edit.limited);
    ui_inline_insert(&edit,"2");assert(strlen(edit.text)==191 && !edit.limited);
    ui_inline_key(&edit,key(KEY_ACON));assert(!edit.text[0] && !edit.limited);
    int width;dsize("Recall saved session",NULL,&width,NULL);assert(width<=226);
    printf("Recall saved session target-font width=%d / 226 pixels\n",width);
    const char *hints[]={UI_EDIT_HINT,UI_LIMIT_HINT,"LEFT/RIGHT: ON/OFF toggle","Comma: separator",
        "Integration start","Integration end","RK4 h > 0; smaller means more work",
        "Output spacing only; h is unchanged","Slope-field columns (0-100); 0 = Off",
        "Max RK4 steps per IC / direction","Xdot edits Xmax; Xmin/Xmax recalculate Xdot",
        "Enter an integer from 1 to 9","EXE: NEXT   LEFT/RIGHT: edit","EXE: open",
        "MENU: return to MAIN MENU",
        "Too many initial values","Max: 10","Input too long","Max: 191 characters",
        "One solution: x0 plus all state values","y0: scalar or {values}; at most 10"};
    for(unsigned i=0;i<sizeof(hints)/sizeof(*hints);i++) {
        int height;dsize(hints[i],NULL,&width,&height);
        assert(width<=UI_W-16 && height<=14 && !strchr(hints[i],'\n'));
        printf("Hint width=%d / %d pixels: %s\n",width,UI_W-16,hints[i]);
    }
    puts("Field state: edit EXE next/stay, selected EXE primary, EXIT unchanged, LEFT/RIGHT entry and physical replace passed.");
}
