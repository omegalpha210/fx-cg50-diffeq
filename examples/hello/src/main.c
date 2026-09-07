#include <gint/display.h>
#include <gint/keyboard.h>

int main(void)
{
    dclear(C_WHITE);
    drect(0, 0, 383, 24, C_BLUE);
    dtext(8, 6, C_WHITE, "DIFF EQ - native fx-CG50");
    dtext(10, 58, C_BLACK, "gint + SH GCC toolchain smoke test");
    dtext(10, 90, C_BLACK, "384 x 216 color display");
    dtext(10, 140, C_BLACK, "Press EXIT to return.");
    dupdate();
    while(getkey().key != KEY_EXIT) {}
    return 1;
}
