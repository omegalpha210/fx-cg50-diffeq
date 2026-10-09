#include "menu.h"
#include "ui.h"
#include "power.h"
#include "menu_icons.inc"

MenuTile ui_menu_tile(int i)
{
    return (MenuTile){4+(i%2)*192,27+(i/2)*62,MENU_TILE_W,
        i<4 ? MENU_TILE_H:MENU_TEXT_H};
}
bool ui_menu_move(int key,int *selected,int rows)
{
    if(rows<1 || *selected<0 || *selected>=rows*2)return false;
    int row=*selected/2,col=*selected%2;
    if(key==KEY_LEFT || key==KEY_RIGHT)col=1-col;
    else if(key==KEY_UP || key==KEY_DOWN)row=(row+rows+(key==KEY_UP ? -1:1))%rows;
    else return false;
    *selected=row*2+col;return true;
}
int ui_menu_background(int icon)
{
    static const unsigned short colors[]={C_RGB(24,29,31),C_RGB(31,27,21),
        C_RGB(25,24,31),C_RGB(24,31,25)};
    /* Match the paired menu's authored pastel colors. */
    if(icon==6)return colors[3];
    if(icon==7)return colors[2];
    return colors[icon%4];
}
void ui_menu_icon(int icon,int x,int y)
{
    if(icon<0 || icon>=8)return;
    int ax=icon==3 ? 54:20,ay=icon==1 || icon==2 || icon==3 || icon==7 ? 17:29;
    ui_line(x+4,y+ay,x+104,y+ay,C_BLACK);
    ui_line(x+104,y+ay,x+101,y+ay-2,C_BLACK);
    ui_line(x+104,y+ay,x+101,y+ay+2,C_BLACK);
    ui_line(x+ax,y+32,x+ax,y+1,C_BLACK);
    ui_line(x+ax,y+1,x+ax-2,y+4,C_BLACK);
    ui_line(x+ax,y+1,x+ax+2,y+4,C_BLACK);
    for(unsigned s=icon_ranges[icon][0];s<icon_ranges[icon][0]+icon_ranges[icon][1];s++) {
        unsigned start=icon_strokes[s].offset,count=icon_strokes[s].count;
        for(unsigned i=1;i<count;i++) {
            const unsigned char *p=icon_points[start+i-1],*q=icon_points[start+i];
            ui_line(x+p[0],y+p[1],x+q[0],y+q[1],icon_strokes[s].color);
            ui_line(x+p[0],y+p[1]+1,x+q[0],y+q[1]+1,icon_strokes[s].color);
        }
    }
}
static void outline(MenuTile t,int inset,int color)
{
    int x=t.x+inset,y=t.y+inset,r=t.x+t.w-1-inset,b=t.y+t.h-1-inset;
    ui_line(x,y,r,y,color);ui_line(x,b,r,b,color);
    ui_line(x,y,x,b,color);ui_line(r,y,r,b,color);
}
static int badge_color(int icon)
{
    /* Deep tone of each pastel family keeps the digit legible and related. */
    static const unsigned short colors[]={C_RGB(5,13,23),C_RGB(22,10,1),
        C_RGB(11,8,23),C_RGB(3,16,7)};
    if(icon==6)return colors[3];
    if(icon==7)return colors[2];
    return colors[icon%4];
}
static void text_tile_icon(int i,int x,int y)
{
    if(i==4) { /* folder */
        ui_rect(x,y+2,14,10,C_RGB(28,20,4));ui_rect(x,y,6,2,C_RGB(28,20,4));
        ui_rect(x+1,y+4,12,1,C_RGB(31,27,14));
    } else { /* diskette */
        ui_rect(x,y,13,12,C_RGB(5,11,21));ui_rect(x+3,y,7,4,C_RGB(26,28,31));
        ui_rect(x+2,y+7,9,5,C_WHITE);
    }
}
void ui_menu_draw_tile(bool subtype,int i,bool selected)
{
    static const char *const names[]={"1st","2nd","N-th","SYSTEM","RECALL","SAVE",
        "Separable","Linear","Bernoulli","Others"};
    MenuTile t=ui_menu_tile(i);int icon=subtype ? i+4:i;
    /* The focus ring sits 2px outside the tile; clear it before repainting. */
    ui_rect(t.x-2,t.y-2,t.w+4,t.h+4,C_WHITE);
    ui_rect(t.x,t.y,t.w,t.h,i<4 ? ui_menu_background(icon):UI_SURFACE);
    if(i>=4)outline(t,0,UI_LINE);
    if(i<4)ui_menu_icon(icon,t.x+(t.w-MENU_ICON_W)/2,t.y+4);
    const char *name=names[subtype ? i+6:i];int width;
    dsize(name,NULL,&width,NULL);
    if(i<4)ui_text(t.x+(t.w-width)/2,t.y+44,C_BLACK,"%s",name);
    else {
        int x=t.x+(t.w-width-20)/2;text_tile_icon(i,x,t.y+6);
        ui_text(x+20,t.y+8,UI_INK,"%s",name);
    }
    ui_rect(t.x+t.w-21,t.y+4,16,15,i<4 ? badge_color(icon):UI_MUTED);
    char digit[2]={(char)('1'+i),0};dsize(digit,NULL,&width,NULL);
    ui_text(t.x+t.w-13-width/2,t.y+7,C_WHITE,"%s",digit);
    if(selected) {
        MenuTile ring={t.x-2,t.y-2,t.w+4,t.h+4};
        outline(ring,0,UI_ACCENT);outline(ring,1,UI_ACCENT);
    }
}
int ui_menu_choose(bool subtype,int *selected)
{
    int count=subtype ? 4:6;
    if(*selected<0 || *selected>=count)*selected=0;
    ui_frame(subtype ? "First-order equation":"DIFF EQ",NULL);
    for(int i=0;i<count;i++)ui_menu_draw_tile(subtype,i,i==*selected);
    ui_help(8,184,subtype ? "MENU: return to MAIN MENU   1-4: open":"MENU: return to MAIN MENU   1-6: open",true);
    ui_softkeys("","","","","","OPEN");dupdate();
    unsigned epoch=power_input_epoch();
    for(;;) {
        int key=ui_getkey().key,previous=*selected;
        if(power_input_epoch()!=epoch) {
            epoch=power_input_epoch();
            ui_frame(subtype ? "First-order equation":"DIFF EQ",NULL);
            for(int i=0;i<count;i++)ui_menu_draw_tile(subtype,i,i==*selected);
            ui_help(8,184,subtype ? "MENU: return to MAIN MENU   1-4: open":"MENU: return to MAIN MENU   1-6: open",true);
            ui_softkeys("","","","","","OPEN");dupdate();
        }
        if(key==KEY_EXIT){if(subtype)return -1;continue;}
        int digit=ui_digit(key);
        if(digit>=1 && digit<=count) {
            /* Move focus in VRAM too: a following modal dims this exact screen. */
            if(digit-1!=previous) {
                ui_menu_draw_tile(subtype,previous,false);ui_menu_draw_tile(subtype,digit-1,true);
            }
            *selected=digit-1;return *selected;
        }
        if(key==KEY_EXE || key==KEY_F6)return *selected;
        if(ui_menu_move(key,selected,count/2)) {
            ui_menu_draw_tile(subtype,previous,false);
            ui_menu_draw_tile(subtype,*selected,true);dupdate();
        }
    }
}
