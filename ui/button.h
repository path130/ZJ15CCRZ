#ifndef _BUTTON_H_
#define _BUTTON_H_

#if defined (__cplusplus)
extern "C" {
#endif

#define CFG_GET_BT_BG_ALWAYS

#define BT_SHOW         0
#define BT_RELEASE      0
#define BT_PRESS        1

typedef struct button_attrs_struct {
    int         x, y;
    const char  *f_img_up;
    const char  *f_img_down;
} button_attrs;

typedef struct button_object_struct {
    int         x, y, w, h;
    img_handle  h_img_up;
    img_handle  h_img_down;
    void        *bg_rgba;
    int         show;
} button_object, *button_handle;

extern button_object button_dummy;
extern button_handle button_create(button_attrs *attrs);
extern button_handle button_create_ex(int x, int y, const char *f_img_up, const char *f_img_down);
extern void          button_fill_canvas(button_handle h_button, int status);
extern void          button_resume_canvas(button_handle h_button);
extern void          button_show(button_handle h_button);
extern void          button_release(button_handle h_button);
extern void          button_press(button_handle h_button);
extern void          button_delete(button_handle h_button);
extern void          button_free(button_handle h_button);

#if defined (__cplusplus)
}
#endif
 
#endif
