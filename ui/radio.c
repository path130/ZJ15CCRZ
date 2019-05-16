/*
 ============================================================================
 Name        : ui.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : UI radio
  ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "osd.h"
#include "img.h"
#include "text.h"
#include "radio.h"
#include "public.h"

static radio_handle     h_radio_focus  = NULL;
static radio_handle     h_radio_select = NULL;

radio_handle radio_create_ex(const char *f_img_t, const char *f_img_f, const char *f_img_f_t, const char *f_img_f_f, int x, int y)
{
    radio_handle 	h_radio;

	h_radio = calloc(1, sizeof(radio_object));
	if (h_radio == NULL) {
		app_debug(DBG_ERROR, "Alloc failed: radio object\n");
		return NULL;
	}

    h_radio->h_img_t   = img_create(f_img_t, 0);
    h_radio->h_img_f   = img_create(f_img_f, 0);
    h_radio->h_img_f_t = img_create(f_img_f_t, 0);
    h_radio->h_img_f_f = img_create(f_img_f_f, 0);

    h_radio->x     = x;
    h_radio->y     = y;
    h_radio->w     = MAX(MAX(h_radio->h_img_t->width, h_radio->h_img_f->width), MAX(h_radio->h_img_f_t->width, h_radio->h_img_f_f->width));
    h_radio->h     = MAX(MAX(h_radio->h_img_t->height, h_radio->h_img_f->height), MAX(h_radio->h_img_f_t->height, h_radio->h_img_f_f->height));

    return h_radio;
}

static void radio_get_rgb(radio_handle h_radio)
{
    if (h_radio->rgb == NULL) {
        h_radio->rgb = malloc(h_radio->w * h_radio->h * sizeof(pixel_u));
        app_debug(DBG_INFO, "h_radio rgb malloc\n");
        if (h_radio->rgb == NULL) {
		    app_debug(DBG_ERROR, "Alloc failed: radio rgb\n");
		    return;
	    }
    }
    osd_get_canvas(h_radio->rgb, h_radio->w, h_radio->h, h_radio->x, h_radio->y);
}

static void radio_put_rgb(radio_handle h_radio)
{
    osd_put_canvas(h_radio->rgb, h_radio->w, h_radio->h, h_radio->x, h_radio->y);
}

void radio_fill_canvas(radio_handle h_radio, int focus, int select)
{   
    if (h_radio == NULL)  return;

    radio_get_rgb(h_radio);
    if (select) {
        h_radio_select = h_radio;
        if (focus) {
            h_radio_focus = h_radio;
            img_fill_canvas(h_radio->h_img_f_t, h_radio->x, h_radio->y);
        }
        else
            img_fill_canvas(h_radio->h_img_t, h_radio->x, h_radio->y);
    }
    else {
        if (focus) {
            h_radio_focus = h_radio;
            img_fill_canvas(h_radio->h_img_f_f, h_radio->x, h_radio->y);
        }
        else
            img_fill_canvas(h_radio->h_img_f, h_radio->x, h_radio->y);
    }
}

void radio_set_focus(radio_handle h_radio)
{
    if (h_radio_focus != NULL) {
        radio_put_rgb(h_radio_focus);
        radio_fill_canvas(h_radio_focus, 0, ((h_radio_focus == h_radio_select) ? 1 : 0));
        osd_draw_canvas(h_radio_focus->w, h_radio_focus->h, h_radio_focus->x, h_radio_focus->y);
    }
    h_radio_focus = h_radio;
    if (h_radio != NULL) {
        radio_put_rgb(h_radio);
        radio_fill_canvas(h_radio, 1, ((h_radio == h_radio_select) ? 1 : 0));
        osd_draw_canvas(h_radio->w, h_radio->h, h_radio->x, h_radio->y);
    }
}

void radio_set_select(radio_handle h_radio)
{
    if (h_radio_select != NULL) {
        radio_put_rgb(h_radio_select);
        radio_fill_canvas(h_radio_select, 0, 0);
        osd_draw_canvas(h_radio_select->w, h_radio_select->h, h_radio_select->x, h_radio_select->y);
    }
    h_radio_select = h_radio;
    if (h_radio != NULL) {
        radio_put_rgb(h_radio);
        radio_fill_canvas(h_radio, 1, 1);
        osd_draw_canvas(h_radio->w, h_radio->h, h_radio->x, h_radio->y);
    }
}

radio_handle radio_get_select(void)
{
    return h_radio_select;
}

void radio_free(radio_handle h_radio)
{
    if (h_radio == NULL) return;

    if (h_radio->rgb) {
        free(h_radio->rgb);
        h_radio->rgb = NULL;
    }
}

void radio_delete(radio_handle h_radio)
{
    if (h_radio == NULL)  return; 

    if (h_radio->rgb) {
        free(h_radio->rgb);
    }
    if (h_radio->h_img_t) {
        img_delete(h_radio->h_img_t);
    }
    if (h_radio->h_img_f) {
        img_delete(h_radio->h_img_f);
    }
    if (h_radio->h_img_f_t) {
        img_delete(h_radio->h_img_f_t);
    }
    if (h_radio->h_img_f_f) {
        img_delete(h_radio->h_img_f_f);
    }
    free(h_radio);
}

