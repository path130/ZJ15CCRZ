#ifndef _EDIT_H_
#define _EDIT_H_

#include "tim.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define ES_CUR          0x000100
#define ES_NUM          0x000200  
#define ES_PW           0x000400
#define ES_IP           0x000800
#define ES_DATE         0x001000
#define ES_TIME         0x002000
#define ES_BOOL         0x004000
#define ES_FT_N         0x008000
#define ES_FT_S         0x010000
#define ES_FT_L         0x020000
#define ES_FT_M         0x040000

#define ES_LM_1         0x000001
#define ES_LM_2         0x000002
#define ES_LM_4         0x000004
#define ES_LM_6         0x000006
#define ES_LM_8         0x000008
#define ES_LM_10        0x00000A
#define ES_LM_12        0x00000C
#define ES_LM_14        0x00000E

#define RGB_PROMPT      0xA8A8A8
#define TEXT_MAX_LEN    32

typedef struct edit_object_struct {
    int             x, y, w, h;
    int             chn_w, chn_h, num_w, num_h, txt_h;
    char            text[TEXT_MAX_LEN];
    void            *rgb;
    unsigned int    style;
    unsigned int    limit;
    unsigned int    index;
    unsigned int    empty;
    int             prompt;
    img_handle      h_img;
    font_handle     h_font;

} edit_object, *edit_handle;

extern edit_handle  edit_create_ex(font_handle h_font, const char *f_img, int x, int y, unsigned int style);
extern edit_handle  edit_get_focus(void);
extern unsigned int edit_get_style(edit_handle h_edit);
extern void         edit_set_style(edit_handle h_edit, unsigned int style);
extern void         edit_set_focus(edit_handle h_edit);
extern void         edit_cancel_focus(edit_handle h_edit);
extern void         edit_flash_init(tim_callback flash_fun, int arg1, int arg2);
extern void         edit_flash_suspend(int tim);
extern void         edit_flash_resume(void);
extern void         edit_flash_cursor(void);
extern int          edit_cursor_status(void);
extern void         edit_set_current(edit_handle h_edit);
extern void         edit_fill_canvas(edit_handle h_edit);
extern void         edit_fill_canvas_ex(edit_handle h_edit, unsigned int style);
extern void         edit_show(edit_handle h_edit);
extern void         edit_show_text(edit_handle h_edit, int show, const char *fmt, ...);
extern void         edit_text_fill(edit_handle h_edit, const char *text);
extern void         edit_text_show(edit_handle h_edit);
extern void         edit_text_clr(edit_handle h_edit);
extern void         edit_text_del(edit_handle h_edit);
extern void         edit_text_add(edit_handle h_edit, char ch);
extern void         edit_text_edit(int key);
extern int          edit_get_length(edit_handle h_edit);
extern int          edit_is_full(edit_handle h_edit);
extern int          edit_is_empty(edit_handle h_edit);
extern int          edit_want_return(edit_handle h_edit);
extern char*        edit_get_text(edit_handle h_edit);
extern void         edit_get_bcd(edit_handle h_edit, unsigned char bcd[]);
extern void         edit_show_prompt(edit_handle h_edit, int color, const char *string);
extern void         edit_free(edit_handle h_edit);
extern void         edit_delete(edit_handle h_edit);

#if defined (__cplusplus)
}
#endif
 
#endif
