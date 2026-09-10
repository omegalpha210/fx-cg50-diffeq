#include "menu.h"
#include "ui.h"
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
void ui_menu_draw_tile(bool subtype,int i,bool selected)
{
    static const char *const names[]={"1st","2nd","N-th","SYSTEM","RECALL","SAVE",
        "Separable","Linear","Bernoulli","Others"};
    MenuTile t=ui_menu_tile(i);int icon=subtype ? i+4:i;
    ui_rect(t.x,t.y,t.w,t.h,i<4 ? ui_menu_background(icon):C_RGB(29,29,29));
    outline(t,0,UI_LINE);
    if(selected){outline(t,0,C_RGB(0,19,29));outline(t,1,C_RGB(0,19,29));}
    if(i<4)ui_menu_icon(icon,t.x+(t.w-MENU_ICON_W)/2,t.y+4);
    const char *name=names[subtype ? i+6:i];int width;
    dsize(name,NULL,&width,NULL);ui_text(t.x+(t.w-width)/2,t.y+(i<4 ? 44:8),C_BLACK,"%s",name);
    ui_rect(t.x+t.w-21,t.y+4,16,15,UI_MUTED);
    char digit[2]={(char)('1'+i),0};dsize(digit,NULL,&width,NULL);
    ui_text(t.x+t.w-13-width/2,t.y+7,C_WHITE,"%s",digit);
}
int ui_menu_choose(bool subtype,int *selected)
{
    int count=subtype ? 4:6;
    if(*selected<0 || *selected>=count)*selected=0;
    ui_frame(subtype ? "First-order equation":"DIFF EQ",NULL);
    for(int i=0;i<count;i++)ui_menu_draw_tile(subtype,i,i==*selected);
    ui_help(8,184,"MENU: return to MAIN MENU",true);
    ui_softkeys("","","","","","OPEN");dupdate();
    for(;;) {
        int key=ui_getkey().key,previous=*selected;
        if(key==KEY_EXIT){if(subtype)return -1;continue;}
        int digit=ui_digit(key);
        if(digit>=1 && digit<=count){*selected=digit-1;return *selected;}
        if(key==KEY_EXE || key==KEY_F6)return *selected;
        if(ui_menu_move(key,selected,count/2)) {
            ui_menu_draw_tile(subtype,previous,false);
            ui_menu_draw_tile(subtype,*selected,true);dupdate();
        }
    }
}
