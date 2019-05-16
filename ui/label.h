#ifndef _label_H_
#define _label_H_

#include "text.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct label_object_struct { 
    int             w, h, x, y;
    int             txt_w, txt_h;
    void            *rgb;
    int             clear;
} label_object, *label_handle;

extern label_handle label_create_ex(int w, int h, int x, int y);

extern void         label_show_text(label_handle h_label, int show, int color, const char *fmt, ...);
extern void         label_show_pic(label_handle h_label, int show, const char *pic);
extern void         label_clear(label_handle h_label);
extern void         label_reset(label_handle h_label);
extern void         label_free(label_handle h_label);
extern void         label_delete(label_handle h_label);

#if defined (__cplusplus)
}
#endif
 
#endif
