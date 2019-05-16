/*
 ============================================================================
 Name        : ui.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : UI edit (use one timer)
  ============================================================================
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "osd.h"
#include "img.h"
#include "text.h"
#include "edit.h"
#include "public.h"
#include "tim.h"
//#include "spi.h" //del by wrm 20141121
#include "key.h"

static edit_handle  h_edit_focus = NULL;

static int tim_edit_flash;
static volatile int edit_cur_flg = 1;
global_data  edit_cur_en = GBL_DATA_INIT;

edit_handle edit_create_ex(font_handle h_font, const char *f_img, int x, int y, unsigned int style)
{
    edit_handle 	h_edit;

	h_edit = calloc(1, sizeof(edit_object));
	if (h_edit == NULL) {
		app_debug(DBG_ERROR, "Alloc failed: edit object\n");
		return NULL;
	}
    if (f_img != NULL) {
        h_edit->h_img   = img_create(f_img, 0);
    }
    if (h_edit->h_img == NULL) {
        return NULL;
    }
    h_edit->h_font = h_font;
    h_edit->x      = x;
    h_edit->y      = y;
    h_edit->w      = h_edit->h_img->width;
    h_edit->h      = h_edit->h_img->height;
    h_edit->empty  = 1;
    edit_set_style(h_edit, style);

    return h_edit;
}

void edit_set_style(edit_handle h_edit, unsigned int style)
{
    if (h_edit == NULL) return;

    h_edit->style = style;
    if (style & ES_DATE) {
        h_edit->style |= ES_CUR;
        h_edit->limit = 10;
    }
    else
    if (style & ES_TIME) {
        h_edit->style |= ES_CUR;
        h_edit->limit = 8;
    }
    else
    if (style & ES_IP) {
        h_edit->style |= ES_CUR;
        h_edit->limit = 15;
    }
    else
    if (style & 0xff) 
        h_edit->limit = (style & 0xff);
    else
        h_edit->limit = 20;

    if (style & ES_FT_S) {
        h_edit->txt_h = 30;
        h_edit->num_w = 22;
        h_edit->num_h = 22;
        h_edit->chn_w = 13;
        h_edit->chn_h = 13;
    }
    else
    if (style & ES_FT_L) {
        h_edit->txt_h = 30;
        h_edit->num_w = 30-2;
        h_edit->num_h = 27-2;
        h_edit->chn_w = 17;
        h_edit->chn_h = 17; 
        if ((h_edit->limit > 10) && (h_edit->limit < 15)) {
            h_edit->num_w = 22;
            h_edit->num_h = 22;
        } else if (h_edit->limit > 15) {
            h_edit->num_w = 18;
            h_edit->num_h = 18;
        }
    }
    else {
        h_edit->txt_h = 20;
        h_edit->num_w = 18;
        h_edit->num_h = 18;
        h_edit->chn_w = 15;
        h_edit->chn_h = 15;
    }
}

unsigned int edit_get_style(edit_handle h_edit)
{
    return h_edit->style;
}

void edit_set_current(edit_handle h_edit)
{
    unsigned int cur_pos;
    //int blank = 0;   //add by wrm 20150122 for text in the middle of ip/time text label
    h_edit_focus = h_edit;
    if (h_edit == NULL) return;
    //if(h_edit->style & ES_IP)
		//blank = 3;
 /*   if (h_edit->style & ES_CUR) { //del by wrm 20150122 this codes i think it`s not needed
        edit_cur_flg = 1;
        cur_pos = text_fill_canvas(h_edit->h_font, ENCODING, RGB_WHITE, h_edit->num_w, h_edit->num_h, \
                                    0, 0, h_edit->x+1, h_edit->y+blank+(h_edit->txt_h-h_edit->num_h)/2, h_edit->text);
        osd_fill_canvas(RGB_WHITE, 1, h_edit->h-8, (cur_pos&0xffff)+1, h_edit->y+4);
    }*/
}

void edit_set_focus(edit_handle h_edit)
{
    edit_cur_flg = 0;
    edit_text_show(h_edit_focus);
    h_edit_focus = h_edit;
    edit_cur_flg = 1;
    edit_text_show(h_edit);
    edit_flash_suspend(TIME_1S(1));
}

inline edit_handle edit_get_focus(void)
{
    return h_edit_focus;
}

inline void edit_cancel_focus(edit_handle h_edit)
{
    h_edit_focus = NULL;
    edit_text_show(h_edit);
}

static inline void edit_cursor_flash(void)
{
    if(edit_cur_flg)   edit_cur_flg = 0;
    else               edit_cur_flg = 1;
}

int edit_cursor_status(void)
{
    return gbl_data_get(&edit_cur_en);
}

void edit_flash_init(tim_callback flash_fun, int arg1, int arg2)
{
    tim_edit_flash = tim_set_event(TIME_500MS(0), flash_fun, arg1, arg2, TIME_PERIODIC);
}

// tim == 0:直到调用edit_flash_resume才继续闪
// else tim 后闪
void edit_flash_suspend(int tim)
{
    if (h_edit_focus == NULL) return;

    if (h_edit_focus->style & ES_CUR) {
        tim_reset_time(tim_edit_flash, tim);
    }
    gbl_data_set(&edit_cur_en, 0);
    if (tim == 0)  edit_set_current(NULL);
}

void edit_flash_resume(void)
{
    if (h_edit_focus == NULL) return;

    gbl_data_set(&edit_cur_en, 1);
    if (h_edit_focus->style & ES_CUR) {
        tim_reset_time(tim_edit_flash, TIME_500MS(1));
    }
}

void edit_flash_cursor(void)
{
    app_debug(DBG_INFO,"%s running,edit_cur_en = %d\n", __func__,gbl_data_get(&edit_cur_en));
    if (gbl_data_get(&edit_cur_en)) {
        //edit_flash_resume();
        edit_text_show(h_edit_focus);
    }
}

static void edit_get_rgb(edit_handle h_edit)
{
    if (h_edit->rgb == NULL) {
        h_edit->rgb = malloc(h_edit->w * h_edit->h * sizeof(pixel_u));
        app_debug(DBG_INFO, "h_edit rgb malloc\n");
        if (h_edit->rgb == NULL) {
		    app_debug(DBG_ERROR, "Alloc failed: edit rgb\n");
		    return;
	    }
    }
    osd_get_canvas(h_edit->rgb, h_edit->w, h_edit->h, h_edit->x, h_edit->y);
}

static void edit_put_rgb(edit_handle h_edit)
{
    osd_put_canvas(h_edit->rgb, h_edit->w, h_edit->h, h_edit->x, h_edit->y);
}

static void edit_fill_bg_text(edit_handle h_edit)
{
    char bg_text[20];
    if (h_edit->style & ES_DATE) {
        strcpy(bg_text,"YYYY-MM-DD");
    }
    else
    if (h_edit->style & ES_TIME) {
        strcpy(bg_text,"HH:MM:SS");
    }
    else
    if (h_edit->style & ES_IP) {
        strcpy(bg_text, "XXX.XXX.XXX.XXX");
    }
    else
        return;
    text_fill_canvas(h_edit->h_font, ENCODING, 0x58505B, h_edit->chn_w, h_edit->chn_h , \
                     0, 0, h_edit->x+5, h_edit->y+5+(h_edit->txt_h-h_edit->chn_h )/2, bg_text);
}

void edit_fill_canvas(edit_handle h_edit)
{
    if (h_edit == NULL)  return;
    
    img_fill_canvas(h_edit->h_img, h_edit->x, h_edit->y);
    edit_get_rgb(h_edit);
    edit_fill_bg_text(h_edit);
    memset(h_edit->text, '\0', sizeof(h_edit->text));
    h_edit->index = 0;
}

void edit_fill_canvas_ex(edit_handle h_edit, unsigned int style)
{
    edit_set_style(h_edit, style);
    edit_fill_canvas(h_edit);
}

void edit_show(edit_handle h_edit)
{
    if (h_edit == NULL)  return;

    edit_put_rgb(h_edit);
    osd_draw_canvas(h_edit->w, h_edit->h, h_edit->x, h_edit->y);
}

void edit_text_fill(edit_handle h_edit, const char *text)
{
    if (h_edit == NULL)  return;

    h_edit->index = MIN(strlen(text), h_edit->limit);
    memset(h_edit->text, '\0', TEXT_MAX_LEN);
    strncpy(h_edit->text, text, h_edit->index);
    edit_put_rgb(h_edit);
    text_fill_canvas(h_edit->h_font, ENCODING, RGB_WHITE, h_edit->num_w, h_edit->num_h, \
                                0, 0, h_edit->x+1, h_edit->y+(h_edit->txt_h-h_edit->num_h)/2, h_edit->text); 
}

void edit_show_text(edit_handle h_edit, int show, const char *fmt, ...)
{
    va_list      args;
    char         str_show[TEXT_MAX_LEN];
    unsigned int cur_pos;
    int blank = 3; //add by wrm 20150116 in the middle of text label
    
    if (h_edit == NULL)  return;
    memset(str_show,     '\0', TEXT_MAX_LEN);
    memset(h_edit->text, '\0', TEXT_MAX_LEN);
    va_start(args, fmt);
    vsnprintf(h_edit->text, h_edit->limit+1, fmt, args); //+1 for '\0'
    va_end(args);
    if (show) {
        if (h_edit->style & ES_PW) {
            memset(str_show, '*', strlen(h_edit->text));
            blank = 3;
        }
	 else if (h_edit->style & ES_NUM) {
       		 memcpy(str_show, h_edit->text, strlen(h_edit->text));
        	blank = -3;
    }	
        else
            memcpy(str_show, h_edit->text, strlen(h_edit->text));
    }
	
    else{
		 if (h_edit->style & ES_NUM) {
        		memcpy(str_show, h_edit->text, strlen(h_edit->text));
        		blank = -3;
    		}
		 else
        		memcpy(str_show, h_edit->text, strlen(h_edit->text));
    	}
    h_edit->index = strlen(h_edit->text);
    edit_put_rgb(h_edit);
    cur_pos = text_fill_canvas(h_edit->h_font, ENCODING, RGB_WHITE, h_edit->num_w, h_edit->num_h, \
                                0, 0, h_edit->x+1, h_edit->y+blank+(h_edit->txt_h-h_edit->num_h)/2, str_show);
    if (show) {
        if (h_edit->style & ES_CUR) {
            if ((h_edit == h_edit_focus) && edit_cur_flg)
               osd_fill_canvas(RGB_WHITE, 1, h_edit->h-8, (cur_pos&0xffff)+1, h_edit->y+4);
        }
        if (h_edit->index== 0) edit_fill_bg_text(h_edit);
        osd_draw_canvas(h_edit->w, h_edit->h, h_edit->x, h_edit->y+3);
        edit_cursor_flash();
    }
    if (h_edit->index)
        h_edit->empty = 0;
}

void edit_text_show(edit_handle h_edit)
{
    if (h_edit == NULL)  return;

    char str_show[TEXT_MAX_LEN] = {'\0'};
    unsigned int cur_pos;//, blank = 0;
    int blank = 3; //add by wrm 20150122 for text in the middle of ip/time text label

    if (h_edit->style & ES_PW) {
        memset(str_show, '*', strlen(h_edit->text));
        blank = 3;
    }
    else if (h_edit->style & ES_NUM) {
        memcpy(str_show, h_edit->text, strlen(h_edit->text));
        blank = -3;    //add by wrm 20150122 for text in the middle of call text label
    }	
    else
        memcpy(str_show, h_edit->text, strlen(h_edit->text));

    edit_put_rgb(h_edit);
    cur_pos = text_fill_canvas(h_edit->h_font, ENCODING, RGB_WHITE, h_edit->num_w, h_edit->num_h, \
                                0, 0, h_edit->x+1, h_edit->y+blank+(h_edit->txt_h-h_edit->num_h)/2, str_show);

    if (h_edit->style & ES_CUR) {
        if ((h_edit == h_edit_focus) && edit_cur_flg)
            osd_fill_canvas(RGB_WHITE, 1, h_edit->h-8, (cur_pos&0xffff)+1, h_edit->y+4);
    }
    if (h_edit->index== 0) edit_fill_bg_text(h_edit);
    osd_draw_canvas(h_edit->w, h_edit->h, h_edit->x, h_edit->y+3);
    edit_cursor_flash();
    h_edit->prompt = 0;
}

void edit_show_prompt(edit_handle h_edit, int color, const char *string)
{
    edit_flash_suspend(TIME_1S(2));
    if ((h_edit_focus != NULL) &&  (h_edit_focus != h_edit)){
        edit_cur_flg = 0;
        edit_text_show(h_edit_focus);
        h_edit_focus = h_edit;
    }
    edit_put_rgb(h_edit);
    app_debug(DBG_INFO,"[%s]string:%s %d \n", __func__,string, strlen(string));
    int blank = 3; //add by wrm 20150302 for text in the middle of ip/time text label
    if (h_edit->style & ES_NUM || h_edit->style & ES_PW) {
        blank = -3;    //add by wrm 20150302 for text in the middle of  text label
    }
    if (h_edit->style & ES_PW || h_edit->style & ES_LM_6) {
        blank = 0;    //add by wrm 20150302 for text in the middle of  setting pw label
    }	
    if ((h_edit->style & ES_FT_L) && (strlen(string) > 27)) {
        text_fill_canvas(h_edit->h_font, ENCODING, color, h_edit->chn_w-2, h_edit->chn_h-2 , \
                 0, 0, h_edit->x+1, h_edit->y+(h_edit->txt_h-h_edit->chn_h)/2+blank, string);
    } else {
        text_fill_canvas(h_edit->h_font, ENCODING, color, h_edit->chn_w, h_edit->chn_h , \
                 0, 0, h_edit->x+1, h_edit->y+(h_edit->txt_h-h_edit->chn_h)/2+blank, string);
    }
    osd_draw_canvas(h_edit->w, h_edit->h, h_edit->x, h_edit->y);   
    memset(h_edit->text, '\0', sizeof(h_edit->text));
    h_edit->index  = 0;
    edit_cur_flg   = 1;
    h_edit->prompt = 1;
    h_edit->empty  = 0;
}

void edit_text_clr(edit_handle h_edit)
{
    if (h_edit == NULL)  return;
    
    edit_put_rgb(h_edit);
    osd_draw_canvas(h_edit->w, h_edit->h, h_edit->x, h_edit->y);
    memset(h_edit->text, '\0', sizeof(h_edit->text));
    h_edit->index = 0;
    h_edit->prompt = 0;
    h_edit->empty = 1;
}

void edit_text_del(edit_handle h_edit)
{
    if (h_edit == NULL)  return;
    if (h_edit->prompt)  {
        edit_put_rgb(h_edit);
        osd_draw_canvas(h_edit->w, h_edit->h, h_edit->x, h_edit->y);
//add by wrm 20150330 for * del one char the original is * del all so not need		
    	h_edit->prompt = 0;
        return;
    }

    if (h_edit->index == 0)  {
	h_edit->empty = 1;
    }
    if (h_edit->index >= 1) {
        h_edit->text[h_edit->index-1] = '\0';
        if (h_edit->index > 0) h_edit->index--;
    }

    edit_cur_flg = 1;
    edit_flash_suspend(TIME_1S(1));
    edit_text_show(h_edit);
}

void edit_text_add(edit_handle h_edit, char ch)
{
    if (h_edit == NULL)  return;
    if (h_edit->index >= h_edit->limit) return;
    
    h_edit->text[h_edit->index] = ch;
    h_edit->index++;
    h_edit->empty = 0;
    edit_cur_flg = 1;
    edit_flash_suspend(TIME_1S(1));
    edit_text_show(h_edit);
}

static int edit_char_count(edit_handle h_edit, char c)
{
    if (h_edit == NULL)  return -1;
    int i, cnt = 0;
    for (i = 0; i < h_edit->index; i++)
    {
        if (h_edit->text[i] == c)
            cnt++;
    }

    return cnt;
}

void edit_text_edit(int key)
{
    int key_code = key;
    if (h_edit_focus == NULL) return;

    if (key == KEY_X ) {
        edit_text_del(h_edit_focus);
        return;
    }
    //if (key == KEY_F1 ) { //add by wrm 20150316
   //     edit_text_clr(h_edit_focus);
   //     return;
   // }	
    else
    if ((key == KEY_Y)) {
        if (h_edit_focus->style & ES_IP) {
            if (edit_char_count(h_edit_focus, '.') >= 3)
                return;
            key_code = '.';
        } 
        else
        if (h_edit_focus->style & ES_DATE) {
            if (edit_char_count(h_edit_focus, '-') >= 2)
                return;
            key_code = '-';
        } 
        else
        if (h_edit_focus->style & ES_TIME) {
            if (edit_char_count(h_edit_focus, ':') >= 2)
                return;
            key_code = ':';
        }
        else
            return;
        edit_text_add(h_edit_focus, key_code);
    }
    else
    if ((key >= KEY_0) && (key <= KEY_9)) {
	//printf("-------------key code is %c-------------------\n",key);
        if (h_edit_focus->style & ES_BOOL) {
            if((key != KEY_0)&&(key != KEY_1))
                return;
        }
        edit_text_add(h_edit_focus, key_code);
    }
}

int edit_get_length(edit_handle h_edit)
{
    if (h_edit == NULL)  return 0;
    return h_edit->index;
}

int edit_is_full(edit_handle h_edit)
{
    if (h_edit == NULL)  return 0;
   // printf("index = %d,limit = %d \n",h_edit->index,h_edit->limit);
    return (h_edit->index>=h_edit->limit);
}

int edit_is_empty(edit_handle h_edit)
{
    if (h_edit == NULL)  return 0;
    //return ((h_edit->index == 0) && (h_edit->prompt == 0));
    //return  ((h_edit->empty != 0) && (h_edit->prompt == 0));
    return  h_edit->empty;
}

int edit_want_return(edit_handle h_edit)
{
    if (h_edit == NULL)     return 0;
    if (h_edit->index == 0) return 0;

    if (h_edit->style & ES_IP) {
        if ((edit_char_count(h_edit, '.') >= 3) && (h_edit->text[h_edit->index-1]!='.'))
            return 1;
    } 
    else
    if (h_edit->style & ES_DATE) {
        if ((edit_char_count(h_edit, '-') >= 2) && (h_edit->text[h_edit->index-1]!='-'))
            return 1;
    } 
    else
    if (h_edit->style & ES_TIME) {
        if ((edit_char_count(h_edit, ':') >= 2) && (h_edit->text[h_edit->index-1]!=':'))
            return 1;
    }
    return 0;
}

char* edit_get_text(edit_handle h_edit)
{
    if (h_edit == NULL)  return NULL;
    return h_edit->text;
}

void edit_get_bcd(edit_handle h_edit, unsigned char bcd[])
{
    unsigned char num[TEXT_MAX_LEN]={0};
    unsigned char i, j;

    if (h_edit == NULL)  return;
    
    for (i = 0; i < 20; i++)
        num[i]  = h_edit->text[i]-'0';

    for (i = 0,j = 0; i < h_edit->limit/2; i++,j+=2)
        bcd[i] = (num[j]<<4)|num[j+1];
}

void edit_free(edit_handle h_edit)
{
    if (h_edit == NULL) return;

    if (h_edit->rgb) {
        free(h_edit->rgb);
        h_edit->rgb = NULL;
    }
}

void edit_delete(edit_handle h_edit)
{
    if (h_edit == NULL)  return; 

    if (h_edit->rgb) {
        free(h_edit->rgb);
    }
    if (h_edit->h_img) {
        img_delete(h_edit->h_img);
    }

    free(h_edit);
}


