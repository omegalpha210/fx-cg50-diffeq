#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static key_event_t key(int code){return (key_event_t){.key=(unsigned)code,.type=KEYEV_DOWN};}
int main(void)
{
    UiInlineEdit edit={0};int selected=0;
    assert(ui_field_complete(KEY_EXE,false,&selected,2)==0);
    assert(selected==1 && !edit.active);
    assert(ui_field_complete(KEY_EXE,true,&selected,2)==0);
    assert(selected==1 && !edit.active);
    assert(ui_field_complete(KEY_EXE,false,&selected,2)==KEY_F6);
    selected=0;assert(ui_field_complete(KEY_EXIT,true,&selected,2)==KEY_EXIT && selected==0);
    assert(ui_field_complete(KEY_EXE,true,&selected,2)==0 && selected==1);
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
    puts("Field state: edit EXE next/stay, selected last EXE primary, EXIT unchanged, LEFT/RIGHT entry and physical replace passed.");
}
