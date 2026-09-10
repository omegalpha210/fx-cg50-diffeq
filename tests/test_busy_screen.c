#include "ui.h"
#include <gint/rtc.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Host-only snapshots assert the temporary LCD canvas never replaces the
   ongoing/stable source VRAM. These buffers are absent from firmware. */
static uint16_t original[DWIDTH*DHEIGHT],previous[DWIDTH*DHEIGHT];
typedef struct {bool cancel;unsigned calls;} Cancel;
static bool cancel_first(void *context)
{Cancel *c=context;c->calls++;return c->cancel;}
static void source_unchanged(void)
{assert(!memcmp(original,gint_vram,sizeof(original)));}
static void expected_canvas(UiBusyArea area,const char *label,char spinner)
{
    char title[40];snprintf(title,sizeof(title),"%s %c",label,spinner);
    if(area==UI_BUSY_DRAW) {
        int width;dsize("EXIT cancels",NULL,&width,NULL);
        ui_rect(0,198,384,18,UI_BLUE);
        ui_text(5,202,C_WHITE,"%s",title);
        ui_text(384-5-width,202,C_WHITE,"EXIT cancels");
    } else ui_frame(title,"EXIT cancels");
    assert(!memcmp(gint_vram,host_display_pixels(),sizeof(original)));
    memcpy(gint_vram,original,sizeof(original));
}
static void check(UiBusyArea area,const char *label)
{
    for(unsigned i=0;i<DWIDTH*DHEIGHT;i++)gint_vram[i]=(uint16_t)(i*31u+7u);
    memcpy(original,gint_vram,sizeof(original));dupdate();
    memcpy(previous,host_display_pixels(),sizeof(previous));
    Cancel cancel={0};UiBusy b;host_tick_step(8);
    ui_busy_begin(&b,label,area,cancel_first,&cancel);
    unsigned count=host_display_uploads();
    assert(!ui_busy_cancel(&b) && !b.visible);
    assert(!ui_busy_cancel(&b) && !b.visible);
    assert(host_display_uploads()==count && !memcmp(previous,host_display_pixels(),sizeof(previous)));
    assert(!ui_busy_cancel(&b) && b.visible && b.frame==1);
    assert(host_display_uploads()-count==(area==UI_BUSY_DRAW ? 6:(DHEIGHT+2)/3));source_unchanged();
    expected_canvas(area,label,'/');
    for(unsigned i=1;i<4;i++) {
        count=host_display_uploads();memcpy(previous,host_display_pixels(),sizeof(previous));
        assert(!ui_busy_cancel(&b) && b.frame==i);
        assert(host_display_uploads()==count);
        assert(!ui_busy_cancel(&b) && b.frame==i+1);source_unchanged();
        assert(host_display_uploads()-count==(area==UI_BUSY_DRAW ? 6:7));
        for(int y=0;y<DHEIGHT;y++)if(area==UI_BUSY_DRAW ? (y<UI_Y+198 || y>=UI_Y+216):(y<UI_Y || y>=UI_Y+21))
            assert(!memcmp(previous+y*DWIDTH,host_display_pixels()+y*DWIDTH,DWIDTH*2));
        expected_canvas(area,label,"/-\\|"[i]);
    }
    /* Cancellation wins even when the next refresh is due. */
    count=host_display_uploads();cancel.cancel=true;
    assert(ui_busy_cancel(&b) && b.frame==4 && host_display_uploads()==count);
    ui_busy_end(&b);source_unchanged();assert(!b.visible && host_display_uploads()==count);
    /* The owner replaces the whole preparation screen on rollback/success. */
    dupdate();assert(!memcmp(original,host_display_pixels(),sizeof(original)));
    ui_busy_begin(&b,label,area,cancel_first,&cancel);count=host_display_uploads();
    assert(ui_busy_cancel(&b) && !b.visible && host_display_uploads()==count);
    ui_busy_end(&b);source_unchanged();assert(cancel.calls==11);
    host_tick_step(0);
}
int main(void)
{
    check(UI_BUSY_TABLE,"Preparing Table...");check(UI_BUSY_DRAW,"Drawing...");
    puts("Busy canvas: shared frame geometry, hidden softkeys, four phases, delay, header-only updates, exact VRAM preservation and cancel priority PASS.");
    return 0;
}
