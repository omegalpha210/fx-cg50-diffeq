#ifndef DIFFEQ_UI_H
#define DIFFEQ_UI_H
#include "model.h"
#include <gint/display.h>
#include <gint/keyboard.h>
#define UI_X 6
#define UI_Y 4
#define UI_W 384
#define UI_H 216
#define UI_INK C_RGB(3,6,10)
#define UI_BLUE C_RGB(3,10,24)
#define UI_TEAL C_RGB(0,17,17)
#define UI_MUTED C_RGB(12,14,17)
#define UI_PALE C_RGB(28,29,31)
#define UI_LINE C_RGB(24,26,28)
#define UI_BRIGHT_GREEN 0x37e6
#define UI_YELLOW 0xffe0
#define UI_CYAN 0x07ff
#define UI_LIMIT_HINT "Input too long; Max: 191 characters"
#define UI_EDIT_HINT "EXE: commit / next   EXIT: commit"
typedef struct {char text[EXPR_TEXT];int cursor;bool active,replace,limited;} UiInlineEdit;
typedef struct {int selected,top;UiInlineEdit edit;} UiStageState;
typedef enum {UI_STAGE_BACK,UI_STAGE_NEXT,UI_STAGE_VWINDOW,UI_STAGE_OUTPUT,UI_STAGE_SETTINGS,UI_STAGE_EVENT,UI_STAGE_INFO} UiStageAction;
typedef struct {int timer;volatile int timeout;bool highlighted;} UiBlink;
void ui_frame(const char *title,const char *subtitle);
void ui_progress(unsigned stage);
void ui_help(int x,int y,const char *text,bool main_menu);
bool ui_select_move(int key,int *selected,int count);
void ui_text(int x,int y,int color,const char *format,...);
void ui_rect(int x,int y,int w,int h,int color);
void ui_line(int x1,int y1,int x2,int y2,int color);
void ui_softkeys(const char *a,const char *b,const char *c,const char *d,const char *e,const char *f);
void ui_row(int row,const char *label,const char *value,bool selected);
void ui_field(int row,const char *label,const char *value,bool selected);
void ui_field_at(int y,const char *label,const char *value,bool selected);
/* Caller-owned overlays take priority. Ordinary forms use logical EDIT state,
   then a useful SELECT context (NULL leaves the bottom help line blank). */
void ui_form_hint(const UiInlineEdit *edit,const char *context);
void ui_color_swatch(int x,int y,int color);
void ui_short(char *out,unsigned capacity,const char *text,int width);
void ui_message(const char *title,const char *message);
/* Keep the current field/draft visible; acknowledgement returns to editing. */
void ui_field_error(const char *message);
bool ui_confirm(const char *title,const char *message);
bool ui_save_confirm(void);
int ui_choose(const char *title,const char *const *items,int count,int selected);
int ui_digit(int key);
bool ui_cancel(void *unused);
bool ui_trace_cancel(void *unused);
void ui_trace_input(bool active);
key_event_t ui_trace_key(UiBlink *blink);
key_event_t ui_getkey(void);
void ui_blink_start(UiBlink *blink);
key_event_t ui_blink_key(UiBlink *blink);
void ui_blink_stop(UiBlink *blink);
void ui_inline_begin(UiInlineEdit *edit,const char *text,bool replace);
bool ui_field_select(UiInlineEdit *edit,key_event_t event,const char *value,int *selected,int count);
/* Translate EXE once: committed edit -> next/select; SELECT -> F6. */
int ui_equation_variables(const Document *d);
int ui_list_complete(int key,bool committed,int *selected,int count);
int ui_field_complete(int key,bool committed,int *selected,int count);
void ui_inline_insert(UiInlineEdit *edit,const char *token);
void ui_equation_menu(int kind,int page,int variables);
const char *ui_equation_token(int kind,int page,int variables,int key);
bool ui_inline_input(key_event_t event);
int ui_inline_key(UiInlineEdit *edit,key_event_t event);
void ui_inline_draw(const UiInlineEdit *edit,int x,int y,int width,int foreground,int background);
void ui_inline_draw_cursor(const UiInlineEdit *edit,int x,int y,int width,int foreground,int background,bool cursor);
bool ui_edit(const char *title,char *text,unsigned capacity,int position);
bool ui_number(const char *title,double *value);
UiStageAction ui_parameters(Document *d,UiStageState *state);
void ui_event(Document *d);
void ui_solver_info(void);
void ui_vwindow(Document *d);
void ui_graph_settings(Document *d);
UiStageAction ui_initial_conditions(Document *d,UiStageState *state);
void ui_output(Document *d);
#endif
