
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "osd.h"
#include "img.h"
#include "label.h"
#include "public.h"

label_handle label_create_ex(int w, int h, int x, int y)
{
    label_handle h_label;

	h_label = calloc(1, sizeof(label_object));
	if (h_label == NULL) {
		app_debug(DBG_ERROR, "Alloc failed: label object\n");
		return NULL;
	}

    h_label->w = w;
    h_label->h = h;
    h_label->x = x; 
    h_label->y = y;
    h_label->txt_w  = 30;
    h_label->txt_h  = 30;
    h_label->clear  = 1;
    return h_label;
}

inline static void label_rgb_put(label_handle h_label)
{
    if (h_label->rgb == NULL) {
        h_label->rgb = malloc(h_label->w * h_label->h * sizeof(pixel_u));
        app_debug(DBG_INFO, "h_label bg_rgb malloc\n");
        h_label->clear = 1;
        if (h_label->rgb == NULL) {
            app_debug(DBG_ERROR, "Alloc failed: label rgb\n");
            return;
	    }
    }
    if (h_label->clear)
        osd_get_canvas(h_label->rgb, h_label->w, h_label->h, h_label->x, h_label->y);
    else
        osd_put_canvas(h_label->rgb, h_label->w, h_label->h, h_label->x, h_label->y);
}

void label_clear(label_handle h_label)
{
    if (h_label == NULL)      return;
    if (h_label->rgb == NULL) return;
    if (h_label->clear)       return;
    h_label->clear = 1;
    osd_put_canvas(h_label->rgb, h_label->w, h_label->h, h_label->x, h_label->y);
    osd_draw_canvas(h_label->w, h_label->h, h_label->x, h_label->y);
}

void label_reset(label_handle h_label)
{
    if (h_label == NULL)      return;
    h_label->clear = 1;
}

void label_show_text(label_handle h_label, int show, int color, const char *fmt, ...)
{
    char text[250];
    va_list args;
   // printf("label length : %d\n",strlen(text));
    if (h_label == NULL) return;

    va_start(args, fmt);
    vsprintf(text, fmt, args);
    va_end(args);
    label_rgb_put(h_label);//在贴图前做了当前区域的rgb保存操作，为清除做准备 add by wrm 20150119
    h_label->clear = 0;
    text_fill_canvas(text_font, ENCODING, color, h_label->txt_w, h_label->txt_h, h_label->w, h_label->h, h_label->x, h_label->y, text);
    if (show)
        osd_draw_canvas(h_label->w, h_label->h, h_label->x, h_label->y);
}

void label_show_pic(label_handle h_label, int show, const char *pic)
{
    if (h_label == NULL) return;
    label_rgb_put(h_label);
    h_label->clear = 0;
    img_fill_canvas_ex(pic, h_label->x, h_label->y);
    if (show)
        osd_draw_canvas(h_label->w, h_label->h, h_label->x, h_label->y);
}

void label_free(label_handle h_label)
{
    if (h_label == NULL) return;

    if (h_label->rgb) {
        free(h_label->rgb);
        h_label->rgb = NULL;
    }
}

void label_delete(label_handle h_label)
{
    if (h_label == NULL)  return; 

    label_free(h_label);

    free(h_label);
}

