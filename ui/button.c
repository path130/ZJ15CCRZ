/*
 ============================================================================
 Name        : button.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : OSD button
  ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "osd.h"
#include "img.h"
#include "button.h"
#include "public.h"

button_handle button_create_ex(int x, int y, const char *f_img_up, const char *f_img_down)
{
    button_handle 	h_button;

	h_button = calloc(1, sizeof(button_object));
	if (h_button == NULL) {
		app_debug(DBG_ERROR, "Failed to allocate space for button object\n");
		return NULL;
	}

    h_button->h_img_up   = img_create(f_img_up, 1);
    h_button->h_img_down = img_create(f_img_down, 0);

    h_button->x = x;
    h_button->y = y;
    h_button->w = MAX(h_button->h_img_up->width,  h_button->h_img_down->width);
    h_button->h = MAX(h_button->h_img_up->height, h_button->h_img_down->height);

    return h_button;
}

button_handle button_create(button_attrs *attrs)
{
    if (attrs == NULL)  return NULL;
	
    return button_create_ex(attrs->x, attrs->y, attrs->f_img_up, attrs->f_img_down);
}


static void button_get_bg_rgba(button_handle h_button)
{
    if (h_button->h_img_up->type != IMG_PNG_32) return;

    if (h_button->bg_rgba == NULL) {
        h_button->bg_rgba = malloc(h_button->w * h_button->h * sizeof(pixel_u));
        app_debug(DBG_INFO, "button bg_rgba malloc\n");
        if (h_button->bg_rgba == NULL) {
		    app_debug(DBG_ERROR, "Failed to allocate space for button bg_rgba\n");
		    return;
	    }
    }
    //app_debug(DBG_INFO, "w:%d h:%d x:%d y:%d\n",h_button->w, h_button->h, h_button->x, h_button->y);

    osd_get_canvas(h_button->bg_rgba, h_button->w, h_button->h, h_button->x, h_button->y);
}

void button_fill_canvas(button_handle h_button, int status)
{
    if (h_button == NULL)  return;

    button_get_bg_rgba(h_button);
    if (status == BT_PRESS)
        img_fill_canvas(h_button->h_img_down, h_button->x, h_button->y);
    else
        img_fill_canvas(h_button->h_img_up, h_button->x, h_button->y);
}

void button_resume_canvas(button_handle h_button)
{
    if (h_button == NULL)  return;
    if (h_button->h_img_up->type != IMG_PNG_32) return;

    osd_put_canvas(h_button->bg_rgba, h_button->w, h_button->h, h_button->x, h_button->y);
}

void button_show(button_handle h_button)
{
    if (h_button == NULL)  return;
    //button_get_bg_rgba(h_button);
    img_show(h_button->h_img_up, h_button->x, h_button->y);
}

void button_release(button_handle h_button)
{
    if (h_button == NULL)  return;

    button_resume_canvas(h_button);
    button_show(h_button);
}

void button_press(button_handle h_button)
{
    if (h_button == NULL)  return;

    button_resume_canvas(h_button);
    img_show(h_button->h_img_down, h_button->x, h_button->y);
}

void button_free(button_handle h_button)
{
    if (h_button == NULL) return;
    if (h_button->bg_rgba) {
        free(h_button->bg_rgba);
        h_button->bg_rgba = NULL;
    }
    h_button->show = 0;
}

void button_delete(button_handle h_button)
{
    if (h_button == NULL)  return; 

    if (h_button->bg_rgba) {
        free(h_button->bg_rgba);
    }

    if (h_button->h_img_up) {
        img_delete(h_button->h_img_up);
    }

    if (h_button->h_img_down) {
        img_delete(h_button->h_img_down);
    }

    free(h_button);
}


