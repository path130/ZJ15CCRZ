#ifndef _SLIDER_H_
#define _SLIDER_H_

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct slider_object_struct {
    int             x, y, w, h;
    unsigned char   range, step, tic;
    void            *rgb;
    img_handle      h_img_bg;
    img_handle      h_img_fg;
} slider_object, *slider_handle;

extern slider_handle  slider_create_ex(const char *f_img_bg, const char *f_img_fg, int x, int y, unsigned char level, unsigned char step);
extern unsigned char  slider_get_tic(slider_handle h_slider);
extern void           slider_set_tic(slider_handle h_slider, unsigned char tic);
extern void           slider_fill_canvas(slider_handle h_slider, unsigned char tic);
extern void           slider_show(slider_handle h_slider);
extern void           slider_dec(slider_handle h_slider);
extern void           slider_inc(slider_handle h_slider);
extern void           slider_free(slider_handle h_slider);
extern void           slider_delete(slider_handle h_slider);

#if defined (__cplusplus)
}
#endif
 
#endif
