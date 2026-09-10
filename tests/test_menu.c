#include "menu.h"
#include "ui.h"
#include <assert.h>
#include <stdio.h>
unsigned host_clear_count(void);
unsigned long host_solves(void);
int main(void)
{
    const int main_up[]={4,5,0,1,2,3},main_down[]={2,3,4,5,0,1};
    for(int i=0;i<6;i++) {
        int p=i;assert(ui_menu_move(KEY_UP,&p,3) && p==main_up[i]);
        p=i;assert(ui_menu_move(KEY_DOWN,&p,3) && p==main_down[i]);
        p=i;assert(ui_menu_move(KEY_RIGHT,&p,3) && p==(i^1));
        assert(ui_menu_move(KEY_LEFT,&p,3) && p==i);
        MenuTile t=ui_menu_tile(i);assert(t.x==4+192*(i%2) && t.y==27+62*(i/2));
        assert(t.w==184 && t.h==(i<4 ? 58:25) && t.y+t.h<=176);
        if(i<4) {
            p=i;assert(ui_menu_move(KEY_UP,&p,2) && p==(i^2));
            assert(ui_menu_move(KEY_DOWN,&p,2) && p==i);
        }
    }
    const char *const labels[]={"1st","2nd","N-th","SYSTEM","RECALL","SAVE",
        "Separable","Linear","Bernoulli","Others"};
    for(int i=0;i<10;i++){int w,h;dsize(labels[i],NULL,&w,&h);assert(w<150 && h<=11);}
    dclear(C_WHITE);unsigned clears=host_clear_count();unsigned long solves=host_solves();
    for(int i=0;i<6;i++)ui_menu_draw_tile(false,i,i==0);
    for(int n=0;n<100;n++){ui_menu_draw_tile(false,n%6,false);ui_menu_draw_tile(false,(n+1)%6,true);}
    assert(host_clear_count()==clears && host_solves()==solves);
    for(int icon=0;icon<8;icon++) {
        dclear(C_WHITE);ui_rect(0,0,MENU_ICON_W,MENU_ICON_H,ui_menu_background(icon));
        ui_menu_icon(icon,0,0);dupdate(); /* Exact firmware geometry for PNG export. */
    }
    puts("Menu geometry, 2D wrap, text widths and bounded partial focus paint passed; 8 native icon frames rendered.");
}
