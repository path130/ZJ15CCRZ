/*
 ============================================================================
 Name        : ui.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : UI slider
  ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "osd.h"
#include "img.h"
#include "text.h"
#include "slider.h"
#include "public.h"


slider_handle slider_create_ex(const char *f_img_bg, const char *f_img_fg, int x, int y, unsigned char range, unsigned char step)
{
    slider_handle 	h_slider;

	h_slider = calloc(1, sizeof(slider_object));
	if (h_slider == NULL) {
		app_debug(DBG_ERROR, "Alloc failed: slider object\n");
		return NULL;
	}

    h_slider->h_img_bg = img_create(f_img_bg, 0);
    h_slider->h_img_fg = img_create(f_img_fg, 0);

    h_slider->x     = x;
    h_slider->y     = y;
    h_slider->w     = h_slider->h_img_bg->width;
    h_slider->h     = h_slider->h_img_bg->height;
    h_slider->range = range;
    h_slider->step  = step;
    h_slider->tic   =  0;
    return h_slider;
}

static void slider_get_rgb(slider_handle h_slider)
{
    if (h_slider->rgb == NULL) {
        h_slider->rgb = malloc(h_slider->w * h_slider->h * sizeof(pixel_u));
        app_debug(DBG_INFO, "h_slider rgb malloc\n");
        if (h_slider->rgb == NULL) {
		    app_debug(DBG_ERROR, "Alloc failed: slider rgb\n");
		    return;
	    }
    }
    osd_get_canvas(h_slider->rgb, h_slider->w, h_slider->h, h_slider->x, h_slider->y);
}

static void slider_put_rgb(slider_handle h_slider)
{
    osd_put_canvas(h_slider->rgb, h_slider->w, h_slider->h, h_slider->x, h_slider->y);
}

void slider_fill_canvas(slider_handle h_slider, unsigned char tic)
{
    int i;    
    if (h_slider == NULL)  return;

    if (tic >= h_slider->range)
        h_slider->tic = h_slider->range;
    else
        h_slider->tic = tic;

    img_fill_canvas(h_slider->h_img_bg, h_slider->x, h_slider->y);
    slider_get_rgb(h_slider);
    for (i = 0; i < h_slider->tic; i++) {
        img_fill_canvas(h_slider->h_img_fg, h_slider->x+8+14*i, h_slider->y+8);
    }
}

void slider_show(slider_handle h_slider)
{
    int i;    
    if (h_slider == NULL)  return;

    slider_put_rgb(h_slider);
    for (i = 0; i < MIN(h_slider->tic, h_slider->range); i++) {
        img_fill_canvas(h_slider->h_img_fg, h_slider->x+8+14*i, h_slider->y+8);
    }
    osd_draw_canvas(h_slider->w, h_slider->h, h_slider->x, h_slider->y);
}

unsigned char slider_get_tic(slider_handle h_slider)
{
    if (h_slider == NULL)  return 0;

    return h_slider->tic;
}

void slider_set_tic(slider_handle h_slider, unsigned char tic)
{
    if (h_slider == NULL)  return;

    if (tic >= h_slider->range)
        h_slider->tic = h_slider->range;
    else
        h_slider->tic = tic;
}

void slider_dec(slider_handle h_slider)
{
    if (h_slider == NULL)  return;

    if(h_slider->tic > 0)
        h_slider->tic--;
    slider_show(h_slider);
}

void slider_inc(slider_handle h_slider)
{
    if (h_slider == NULL)  return;

    if(h_slider->tic < h_slider->range)
        h_slider->tic++;
    slider_show(h_slider);
}

void slider_free(slider_handle h_slider)
{
    if (h_slider == NULL) return;

    if (h_slider->rgb) {
        free(h_slider->rgb);
        h_slider->rgb = NULL;
    }
}

void slider_delete(slider_handle h_slider)
{
    if (h_slider == NULL)  return; 

    if (h_slider->rgb) {
        free(h_slider->rgb);
    }
    if (h_slider->h_img_bg) {
        img_delete(h_slider->h_img_bg);
    }
    if (h_slider->h_img_fg) {
        img_delete(h_slider->h_img_fg);
    }
    free(h_slider);
}

