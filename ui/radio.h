#ifndef _RADIO_H_
#define _RADIO_H_

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct radio_object_struct {
    int             x, y, w, h;
    void            *rgb;
    img_handle      h_img_t;
    img_handle      h_img_f;
    img_handle      h_img_f_t;
    img_handle      h_img_f_f;
} radio_object, *radio_handle;

extern radio_handle radio_create_ex(const char *f_img_t, const char *f_img_f, const char *f_img_f_t, const char *f_img_f_f, int x, int y);
extern radio_handle radio_get_select(void);
extern void         radio_set_select(radio_handle h_radio);
extern void         radio_set_focus(radio_handle h_radio);
extern void         radio_fill_canvas(radio_handle h_radio, int focus, int select);
extern void         radio_free(radio_handle h_radio);
extern void         radio_delete(radio_handle h_radio);

#if defined (__cplusplus)
}
#endif
 
#endif
