/*
 ============================================================================
 Name        : contacts.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : UI contacts
 ============================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "osd.h"
#include "img.h"
#include "text.h"
#include "public.h"
#include "catalog.h"
//#include "sqlite_ui.h"
//#include "sqlite_data.h"
#if 0 // modified by wrm 20141112
struct catalog_s const catalog_table[] = {
    {"00008550", "楼栋编码"},
    {"00008556", "键盘密码修改"},
    {"00008031", "门禁格式化"},
    {"00008054", "注册卡(刷卡形式)"},
    {"00008053", "删除卡"},
    {"00008051", "注册卡(卡号输入形式)"},
    {"00009182", "门磁检测功能"},
    {"00008553", "门磁延时报警设置"},
    {"00008990", "防拆报警功能"},
    {"00009003", "电锁/磁锁切换"},
    {"00009005", "彩转黑功能"},
    {"00009009", "IP/屏幕/音量等设置"},
    {"00008083", "通话送话音量设置"},
    {"00008084", "通话受话音量设置"},
    {"00009185", "用户密码开锁功能"},

};
#endif

#if 1 // modified by wrm 20141112
struct catalog_s const catalog_table[] = {
    {"0101", "德安居"},
    {"0102", "美安居"},
    {"0201", "馨安居"},
    {"0202", "丰安居"},
    {"0301", "益安居"},
    {"0302", "美安居"},
    {"0401", "瑞安居"},

};
#endif

//struct catalog_s const catalog_table[1024];
static int  catalog_item_num  = 15;//modified wrm 20141112 20

catalog_handle catalog_create_ex(font_handle h_font, int w, int h, int x, int y)
{
    catalog_handle h_catalog;

	h_catalog = calloc(1, sizeof(catalog_object));
	if (h_catalog == NULL) {
		app_debug(DBG_ERROR, "Alloc failed: catalog object\n");
		return NULL;
	}

    h_catalog->h_font = h_font;
    h_catalog->w      = w;
    h_catalog->h      = h;
    h_catalog->x      = x;
    h_catalog->y      = y;
    h_catalog->focus  = 0;
    h_catalog->start  = 0;

    h_catalog->row    = CATALOG_ROWS;
    h_catalog->col    = CATALOG_COLS;
    h_catalog->c_x    = h_catalog->x;
    h_catalog->c_w    = h_catalog->w;
    h_catalog->c_y    = h_catalog->y+CLG_RIM_SIZE+CLG_TITLE_H;
    h_catalog->c_h    = h_catalog->h-CLG_RIM_SIZE-CLG_TITLE_H;

    return h_catalog;
}

static void catalog_get_rgb(catalog_handle h_catalog)
{
    if (h_catalog->rgb == NULL) {
        h_catalog->rgb = malloc(h_catalog->c_w * h_catalog->c_h * sizeof(pixel_u));
        app_debug(DBG_INFO, "h_catalog bg_rgb malloc\n");
        if (h_catalog->rgb == NULL) {
		    app_debug(DBG_ERROR, "Alloc failed: catalog rgb\n");
		    return;
	    }
    }
    osd_get_canvas(h_catalog->rgb, h_catalog->c_w, h_catalog->c_h, h_catalog->c_x, h_catalog->c_y);
}

static void catalog_put_rgb(catalog_handle h_catalog)
{
    osd_put_canvas(h_catalog->rgb, h_catalog->c_w, h_catalog->c_h, h_catalog->c_x, h_catalog->c_y);
}

void catalog_select_row(catalog_handle h_catalog, int row)
{
    osd_fill_canvas(CLG_SEL_COLOR, CLG_COL_ROOM_W, CLG_ROW_H, h_catalog->c_x+CLG_COL_X_OFT_ROOM, h_catalog->c_y+CLG_RIM_SIZE+row*(CLG_ROW_H+CLG_RIM_SIZE));
    osd_fill_canvas(CLG_SEL_COLOR, CLG_COL_NAME_W, CLG_ROW_H, h_catalog->c_x+CLG_COL_X_OFT_NAME, h_catalog->c_y+CLG_RIM_SIZE+row*(CLG_ROW_H+CLG_RIM_SIZE));
}

void catalog_fill_row_col(catalog_handle h_catalog, int row, int col, const char *txt)
{
    int text_eare_x = 0, text_eare_y = 0, text_eare_w = 0, text_eare_h = 0;
    int txt_w = 12, txt_h = 12;
    int padding_x = 0, padding_y = 0;
    switch (col) {
        case CLG_COL_ROOM:
            padding_x   = 6;
            padding_y   = 3;
            txt_w=txt_h = 14;
            text_eare_w = CLG_COL_ROOM_W - padding_x;
            text_eare_x = h_catalog->c_x + CLG_COL_X_OFT_ROOM + padding_x;
            break;
        case CLG_COL_NAME:
            padding_x   = 10;//modify wrm 2014 3
            padding_y   = 3;
            txt_w=txt_h = 14;
            text_eare_w = CLG_COL_NAME_W - padding_x;
            text_eare_x = h_catalog->c_x + CLG_COL_X_OFT_NAME + padding_x;
            break;
    }

    text_eare_y = h_catalog->c_y + row*(CLG_ROW_H+CLG_RIM_SIZE) + padding_y;
    text_eare_h = CLG_ROW_H - padding_y;
    text_fill_canvas(h_catalog->h_font, ENCODING, RGB_WHITE, txt_w, txt_h, text_eare_w, text_eare_h, text_eare_x, text_eare_y, txt);
}

void catalog_fill_text(catalog_handle h_catalog)
{
    int row;
    char strroom[16], charroom[16];
    if (h_catalog == NULL)      return;
    for (row = 0; row < MIN(catalog_item_num, CATALOG_ROWS); row++) {
        if (h_catalog->start+row >= catalog_item_num) return;
        //memcpy(charroom, catalog_table[h_catalog->start+row].room, 4);
       // snprintf(strroom, 5, "%02d%02d", charroom[2], charroom[3]);
        catalog_fill_row_col(h_catalog, row, CLG_COL_ROOM, (char *)catalog_table[h_catalog->start+row].room);
        catalog_fill_row_col(h_catalog, row, CLG_COL_NAME,  (char *)catalog_table[h_catalog->start+row].name);
    }
}

void catalog_get_items(catalog_handle h_catalog)
{
    if (h_catalog == NULL)  return;
}

static inline void contact_fill_rim_horizontal(int x, int y, int length)
{
    osd_fill_canvas(RGB_WHITE, CLG_WIDTH, CLG_RIM_SIZE, x, y);
}

static inline void contact_fill_rim_vertical(int x, int y, int length)
{
    osd_fill_canvas(RGB_WHITE, CLG_RIM_SIZE, CLG_HEIGHT, x, y);
}

void catalog_fill_canvas(catalog_handle h_catalog)
{
    int i;
    if (h_catalog == NULL)  return;
    //catalog_item_num  = do_load_catalog((char*)catalog_table, 50*1024);
    catalog_reset_index(h_catalog);
    contact_fill_rim_horizontal(h_catalog->x, h_catalog->y, CLG_WIDTH);
    text_fill_canvas(h_catalog->h_font, 1, RGB_WHITE, 16, 16, CLG_COL_ROOM_W, CLG_ROW_H, h_catalog->x+CLG_COL_X_OFT_ROOM+6,  h_catalog->y+CLG_RIM_SIZE+2, "楼栋号");
    text_fill_canvas(h_catalog->h_font, 1, RGB_WHITE, 16, 16, CLG_COL_NAME_W, CLG_ROW_H, h_catalog->x+CLG_COL_X_OFT_NAME+80, h_catalog->y+CLG_RIM_SIZE+2, "所属楼栋");
    
    for (i = 0; i <= CATALOG_ROWS; i++) {
        contact_fill_rim_horizontal(h_catalog->x, h_catalog->c_y+i*(CLG_ROW_H+CLG_RIM_SIZE), CLG_WIDTH);
    }
    contact_fill_rim_vertical(h_catalog->x, h_catalog->y, CLG_HEIGHT);
    contact_fill_rim_vertical(h_catalog->x+CLG_COL_ROOM_W+CLG_COL_X_OFT_ROOM, h_catalog->y, CLG_HEIGHT);
    contact_fill_rim_vertical(h_catalog->x+CLG_COL_NAME_W+CLG_COL_X_OFT_NAME, h_catalog->y, CLG_HEIGHT);
    catalog_get_rgb(h_catalog);
    catalog_select_row(h_catalog, h_catalog->focus);
    catalog_fill_text(h_catalog);
}

void catalog_set_focus(catalog_handle h_catalog, int row)
{
    if (h_catalog == NULL)      return;
    if (row > CATALOG_ROWS-1) return;
    h_catalog->focus = row;
    catalog_put_rgb(h_catalog);
    catalog_select_row(h_catalog, row);
    osd_draw_canvas(h_catalog->c_w, h_catalog->c_h, h_catalog->c_x, h_catalog->c_y);
}

void catalog_reset_index(catalog_handle h_catalog)
{
    if (h_catalog == NULL)  return;

    h_catalog->start  = 0; 
    h_catalog->focus  = 0;
    //catalog_item_num = 0;
}

static void catalog_focus_move_up(catalog_handle h_catalog)
{
    if(h_catalog->focus > 0) {
        h_catalog->focus--;
    }
    else
    if (h_catalog->start > 0) {
        h_catalog->start--;
    }
}

static void catalog_focus_move_down(catalog_handle h_catalog)
{
    int start_max;
    if (h_catalog->focus < CATALOG_ROWS-1) {
        h_catalog->focus++;
    }
    else {
        if (catalog_item_num >= CATALOG_ROWS)
            start_max = catalog_item_num-CATALOG_ROWS+1;//catalog_item_num-1
        else
            start_max = 0;
        if (h_catalog->start < start_max) {
            h_catalog->start++;
        }
    }
}

static void catalog_focus_page_up(catalog_handle h_catalog)
{
    if (h_catalog->start >= CATALOG_ROWS) {
        h_catalog->start-=CATALOG_ROWS;
    } else if (h_catalog->start == 0) {
        h_catalog->focus = 0;
    } else {
        h_catalog->start = 0;
    }
}

static void catalog_focus_page_down(catalog_handle h_catalog)
{
    int start_max;

    if (CATALOG_ROWS <= catalog_item_num) {
        start_max = catalog_item_num-CATALOG_ROWS+1;//catalog_item_num-1
    } else {
        start_max = 0;
    }

    if (h_catalog->start >= start_max) {
        h_catalog->focus = CATALOG_ROWS-1;
    } else if ((h_catalog->start+CATALOG_ROWS-1) >= start_max) {
        h_catalog->start = start_max;    
    } else {
        h_catalog->start += CATALOG_ROWS;
    }
}

void catalog_move_up(catalog_handle h_catalog)
{
    if (h_catalog == NULL)  return;

    catalog_put_rgb(h_catalog);
    catalog_focus_move_up(h_catalog);
    catalog_select_row(h_catalog, h_catalog->focus);
    catalog_fill_text(h_catalog);
    osd_draw_canvas(h_catalog->c_w, h_catalog->c_h, h_catalog->c_x, h_catalog->c_y);
}

void catalog_move_down(catalog_handle h_catalog)
{
    if (h_catalog == NULL)  return;

    catalog_put_rgb(h_catalog);
    catalog_focus_move_down(h_catalog);
    catalog_select_row(h_catalog, h_catalog->focus);
    catalog_fill_text(h_catalog);
    osd_draw_canvas(h_catalog->c_w, h_catalog->c_h, h_catalog->c_x, h_catalog->c_y);
}

void catalog_page_up(catalog_handle h_catalog)
{
    if (h_catalog == NULL)  return;

    catalog_put_rgb(h_catalog);
    catalog_focus_page_up(h_catalog);
    catalog_select_row(h_catalog, h_catalog->focus);
    catalog_fill_text(h_catalog);
    osd_draw_canvas(h_catalog->c_w, h_catalog->c_h, h_catalog->c_x, h_catalog->c_y);
}

void catalog_page_down(catalog_handle h_catalog)
{
    if (h_catalog == NULL)  return;

    catalog_put_rgb(h_catalog);
    catalog_focus_page_down(h_catalog);
    catalog_select_row(h_catalog, h_catalog->focus);
    catalog_fill_text(h_catalog);
    osd_draw_canvas(h_catalog->c_w, h_catalog->c_h, h_catalog->c_x, h_catalog->c_y);
}

int catalog_get_bcd(catalog_handle h_catalog, unsigned char bcd[])
{
    unsigned char num[16]={0};
    int i, j, len;
    int item = h_catalog->start+h_catalog->focus;
    if (item >= catalog_item_num) return -1;
    char strroom[16]={0};
    snprintf(strroom, 5, "%02d%02d", catalog_table[item].room[2], catalog_table[item].room[3]);

    len = strlen(strroom);
    if (len == 0) {
        return -1;
    }
    if (len & 0x01) {
        len++;
        num[0] = 0;
        for (i = 1; i < 8; i++) {
            num[i]  = strroom[i-1]-'0';
        }
    } else {
        for (i = 0; i < 8; i++) {
            num[i]  = strroom[i]-'0';
        }
    }
    for (i = 0,j = 0; i < MIN(4,len/2); i++,j+=2) {
        bcd[i] = (num[j]<<4)|num[j+1];
    }
    return 0;
}

int catalog_get_code(catalog_handle h_catalog, unsigned char code[])
{
    int item = h_catalog->start+h_catalog->focus;
    if (item >= catalog_item_num) return -1;
    //snprintf(strroom, 5, "%02d%02d", catalog_table[item].room[2], catalog_table[item].room[3]);
    strcpy((char*)code, (char*)catalog_table[item].room);
    return 0;
}

void catalog_free(catalog_handle h_catalog)
{
    if (h_catalog == NULL) return;

    if (h_catalog->rgb) {
        free(h_catalog->rgb);
    }
}

void catalog_delete(catalog_handle h_catalog)
{
    if (h_catalog == NULL)  return; 

    catalog_free(h_catalog);

    free(h_catalog);
}


