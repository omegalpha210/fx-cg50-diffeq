#ifndef HOST_GINT_DISPLAY_H
#define HOST_GINT_DISPLAY_H
#include <stdint.h>
#include <stddef.h>
#define DWIDTH 396
#define DHEIGHT 224
#define C_WHITE 0xffff
#define C_BLACK 0
#define C_BLUE 0x001f
#define C_RED 0xf800
#define C_GREEN 0x07e0
#define C_NONE -1
#define C_RGB(r,g,b) (((r)<<11)|((g)<<6)|(b))
typedef uint16_t color_t;
typedef struct {int unused;} font_t;
extern uint16_t *gint_vram;
void dclear(color_t color);
void drect(int x1,int y1,int x2,int y2,int color);
void dline(int x1,int y1,int x2,int y2,int color);
void dpixel(int x,int y,int color);
void dtext(int x,int y,int color,const char *text);
void dsize(const char *text,const font_t *font,int *w,int *h);
void dnsize(const char *text,int size,const font_t *font,int *w,int *h);
void dsetvram(uint16_t *main,uint16_t *secondary);
void dupdate(void);
#endif
