#ifndef DIFFEQ_MENU_H
#define DIFFEQ_MENU_H
#include <stdbool.h>
#define MENU_TILE_W 184
#define MENU_TILE_H 58
#define MENU_TEXT_H 25
#define MENU_ICON_W 108
#define MENU_ICON_H 34
typedef struct {int x,y,w,h;} MenuTile;
MenuTile ui_menu_tile(int index);
bool ui_menu_move(int key,int *selected,int rows);
int ui_menu_background(int icon);
void ui_menu_icon(int icon,int x,int y);
void ui_menu_draw_tile(bool subtype,int index,bool selected);
/* Repaints only old/new focus tiles between full screen entries. */
int ui_menu_choose(bool subtype,int *selected);
#endif
