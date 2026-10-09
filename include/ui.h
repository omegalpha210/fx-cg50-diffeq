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
/* v0.13 chrome: crisp 1px geometry only (no rounded corners, no synthetic bold). */
#define UI_NAVY C_RGB(4,8,17)
#define UI_ACCENT C_RGB(6,16,31)
#define UI_SELECT C_RGB(23,27,31)
#define UI_SURFACE C_RGB(29,30,31)
#define UI_KEY_EDGE C_RGB(19,21,25)
#define UI_TAB C_RGB(5,7,11)
#define UI_HEADER_MUTED C_RGB(22,25,30)
#define UI_ERROR C_RGB(26,3,3)
#define UI_ERROR_BG C_RGB(31,28,28)
#define UI_LIMIT_HINT "Input too long; Max: 191 characters"
#define UI_EDIT_HINT "EXE: commit + next field   EXIT: commit"
#define UI_EDIT_LAST_HINT "EXE/EXIT: commit"
typedef struct {char text[EXPR_TEXT];int cursor;bool active,replace,limited;} UiInlineEdit;
typedef struct {int selected,top;UiInlineEdit edit;} UiStageState;
typedef enum {UI_STAGE_BACK,UI_STAGE_NEXT,UI_STAGE_VWINDOW,UI_STAGE_OUTPUT,UI_STAGE_SETTINGS,UI_STAGE_EVENT,UI_STAGE_INFO} UiStageAction;
typedef struct {int timer;volatile int timeout;bool highlighted;} UiBlink;
void ui_frame(const char *title,const char *subtitle);
void ui_progress(unsigned stage);
/* Long SELECT lists only; never replaces Equation/IC/Parameters stage labels. */
void ui_list_position(int selected,int count);
/* Key names before ':' (EXE:, LEFT/RIGHT:, F3: ...) are drawn as key caps. */
void ui_help(int x,int y,const char *text,bool main_menu);
int ui_help_width(const char *text);
/* Physical F-key slot i in logical coordinates (full 396px width). */
void ui_softkey_rect(int index,int *x,int *w);
/* As ui_softkeys, with one key filled as the active choice (TRACE speed). */
void ui_softkeys_active(const char *a,const char *b,const char *c,const char *d,const char *e,const char *f,int active);
/* F1-F5 as one read-only information panel (left text, optional curve swatch,
   right text from the panel centre); F6 stays a normal key. */
#define UI_INFO_HALF 158
void ui_softkeys_info(const char *left,const char *right,int swatch,const char *f6);
void ui_scrollbar(int top,int height,int first,int shown,int total);
/* Equation template card under the header. */
void ui_formula(const char *text);
bool ui_select_move(int key,int *selected,int count);
void ui_text(int x,int y,int color,const char *format,...);
void ui_rect(int x,int y,int w,int h,int color);
void ui_line(int x1,int y1,int x2,int y2,int color);
void ui_softkeys(const char *a,const char *b,const char *c,const char *d,const char *e,const char *f);
void ui_field(int row,const char *label,const char *value,bool selected);
void ui_field_at(int y,const char *label,const char *value,bool selected);
/* '=' rows define a mathematical value (equations, initial values, Event E);
   ':' rows are settings. Option rows show LEFT/RIGHT arrows while selected. */
void ui_field_eq(int row,const char *label,const char *value,bool selected);
void ui_field_option(int row,const char *label,const char *value,bool selected);
/* White edit box with accent border over a selected field's value. */
void ui_inline_field(const UiInlineEdit *edit,int y);
void ui_edit_frame(int y);
/* Caller-owned overlays take priority. Ordinary forms use logical EDIT state,
   then a useful SELECT context (NULL leaves the bottom help line blank). */
void ui_form_hint(const UiInlineEdit *edit,const char *context);
/* last: EXE on the final field only commits; the next EXE leaves the screen. */
void ui_form_hint_last(const UiInlineEdit *edit,const char *context,bool last);
void ui_color_swatch(int x,int y,int color);
void ui_short(char *out,unsigned capacity,const char *text,int width);
void ui_message(const char *title,const char *message);
/* Keep the current field/draft visible; acknowledgement returns to editing. */
void ui_form_error(const char *message);
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
typedef enum {UI_BUSY_RESULT,UI_BUSY_TRACE,UI_BUSY_TABLE,UI_BUSY_DRAW} UiBusyArea;
typedef struct {
    uint32_t start,last;unsigned frame;bool visible;
    const char *label;UiBusyArea area;OdeCancel cancel;void *context;
} UiBusy;
void ui_defer_input(void);
/* Full-width Drawing bar (logical rows 198..219): label, stepped block, EXIT cap. */
void ui_busy_bar(const char *label,unsigned frame);
void ui_busy_begin(UiBusy *busy,const char *label,UiBusyArea area,OdeCancel cancel,void *context);
void ui_busy_start(UiBusy *busy);
bool ui_busy_cancel(void *context);
void ui_busy_end(UiBusy *busy);
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
UiStageAction ui_parameters(Document *d,UiStageState *state);
void ui_event(Document *d);
void ui_solver_info(void);
void ui_vwindow_reset(Document *d);
void ui_vwindow(Document *d);
void ui_graph_settings(Document *d);
/* Only changed fields own text; at most10*(EXPR_TEXT) heap bytes, never saved.
   NEXT validates the complete set before touching numeric IC values. */
typedef struct {UiStageState stage;char *draft[ODE_MAX_DIM+1];unsigned limited;} UiInitialState;
void ui_initial_clear(UiInitialState *state);
UiStageAction ui_initial_conditions(Document *d,UiInitialState *state);
void ui_output(Document *d);
#endif
