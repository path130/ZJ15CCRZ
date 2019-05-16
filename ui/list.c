/*
 ============================================================================
 Name        : ui.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : UI list
  ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "osd.h"
#include "img.h"
#include "text.h"
#include "list.h"
#include "public.h"
#include "ui.h"
#include "file.h"

#define LIST_TEXT_HEAD_H    24
#define LIST_PIC_HEAD_H     19

FILE_ATTR   list_files_attr[FILE_LIST_CNT_MAX + 5];
FILE_ATTR   list_file_blank = {"", "", "null"};
FILE_ATTR   list_show_row[5];

static int  list_files_num  = 0;
static int  auto_slide_item = 0;
#ifdef ADD_UNLOCK_AD
FILE_ATTR   list_files_attr_ad[FILE_LIST_CNT_MAX + 5];
FILE_ATTR   list_file_blank_ad = {"", "", "null"};
FILE_ATTR   list_show_row_ad[5];
static int  list_files_num_ad  = 0;
static int  auto_slide_item_ad = 0;


int get_rand(int rand_val){
	//srand((int)time(0));
	struct timeval ts;
	gettimeofday(&ts,NULL);	
	srand((unsigned int)ts.tv_usec);
	return (0+(int)(1.0*rand_val*rand()/(RAND_MAX+1.0)));
}

#endif

list_handle list_create_ex(font_handle h_font, const char *f_img, int x, int y, unsigned int style)
{
    list_handle 	h_list;

	h_list = calloc(1, sizeof(list_object));
	if (h_list == NULL) {
		app_debug(DBG_ERROR, "Alloc failed: list object\n");
		return NULL;
	}

    h_list->h_img  = img_create(f_img, 1);
    h_list->h_font = h_font;
    h_list->x      = x;
    h_list->y      = y;
    h_list->w      = h_list->h_img->width;
    h_list->h      = h_list->h_img->height;
    h_list->style  = style;
    h_list->focus  = 0;
    h_list->start  = 0;
    if (style == LS_TXT) {
        h_list->row = 3;
        h_list->col = 3;
        h_list->c_x = h_list->x;
        h_list->c_w = h_list->w;
        h_list->c_y = h_list->y+LIST_TEXT_HEAD_H;
        h_list->c_h = h_list->h-LIST_TEXT_HEAD_H;
    }
    else {
        h_list->row = 5;
        h_list->col = 1;
        h_list->c_x = h_list->x;
        h_list->c_w = h_list->w;
        h_list->c_y = h_list->y+LIST_PIC_HEAD_H;
        h_list->c_h = h_list->h-LIST_PIC_HEAD_H;
    }

    return h_list;
}

static void list_get_rgb(list_handle h_list)
{
    if (h_list->rgb == NULL) {
        h_list->rgb = malloc(h_list->c_w * h_list->c_h * sizeof(pixel_u));
        app_debug(DBG_INFO, "h_list bg_rgb malloc\n");
        if (h_list->rgb == NULL) {
		    app_debug(DBG_ERROR, "Alloc failed: list rgb\n");
		    return;
	    }
    }
    osd_get_canvas(h_list->rgb, h_list->c_w, h_list->c_h, h_list->c_x, h_list->c_y);
}

static void list_put_rgb(list_handle h_list)
{
    osd_put_canvas(h_list->rgb, h_list->c_w, h_list->c_h, h_list->c_x, h_list->c_y);
}

void list_select_row(list_handle h_list, int row)
{
    if (h_list->style == LS_TXT) {
        osd_fill_canvas(0x4189A3, LT_TEXT_COL_1_W, LT_TEXT_ROW_H, h_list->x+1, h_list->c_y + row*LT_TEXT_ROW_H+row);
        osd_fill_canvas(0x4189A3, LT_TEXT_COL_2_W, LT_TEXT_ROW_H, h_list->x+LT_TEXT_COL_1_W+3, h_list->c_y + row*LT_TEXT_ROW_H+row);
        osd_fill_canvas(0x4189A3, LT_TEXT_COL_3_W, LT_TEXT_ROW_H, h_list->x+LT_TEXT_COL_1_W+LT_TEXT_COL_2_W+5, h_list->c_y + row*LT_TEXT_ROW_H +row);
/*
        if (row == 2) //not a standard image
            osd_fill_canvas(0x4189A3, LT_TEXT_COL_3_W, LT_TEXT_ROW_H+1, h_list->x+LT_TEXT_COL_1_W+LT_TEXT_COL_2_W+5, h_list->c_y + row*LT_TEXT_ROW_H +row - 1);
        else
            osd_fill_canvas(0x4189A3, LT_TEXT_COL_3_W, LT_TEXT_ROW_H, h_list->x+LT_TEXT_COL_1_W+LT_TEXT_COL_2_W+5, h_list->c_y + row*LT_TEXT_ROW_H +row);
*/
    }
    else {
        osd_fill_canvas(0x4189A3, LT_PIC_COL_W, LT_PIC_ROW_H, h_list->x+1, h_list->c_y + row*LT_PIC_ROW_H+row);
        /*if (row >= 2) 
            osd_fill_canvas(0x4189A3, LT_PIC_COL_W, LT_PIC_ROW_H-1, h_list->x+1, h_list->c_y + row*LT_PIC_ROW_H+row - 1);
        else
            osd_fill_canvas(0x4189A3, LT_PIC_COL_W, LT_PIC_ROW_H-1, h_list->x+1, h_list->c_y + row*LT_PIC_ROW_H+row);
       */
    }
}

void list_fill_row_col(list_handle h_list, int row, int col, const char *txt)
{
    int text_eare_x = 0, text_eare_y = 0, text_eare_w = 0, text_eare_h = 0;
    int txt_w = 12, txt_h = 12;
    int padding_x = 0, padding_y = 0;
    if (h_list->style == LS_TXT) {
        switch (col) {
            case COL_TITLE:
                padding_x   = 2;
                padding_y   = 5;
                txt_w=txt_h = 15;
                text_eare_w = LT_TEXT_COL_1_W - padding_x;
                text_eare_x = h_list->c_x + padding_x;
                break;
            case COL_TIME:
                padding_x   = 3;
                padding_y   = 1;
                txt_w=txt_h = 12;
                text_eare_w = LT_TEXT_COL_2_W - padding_x;
                text_eare_x = h_list->c_x + LT_TEXT_COL_1_W + padding_x;
                break;
            case COL_CONTENT:
                padding_x   = 4;
                padding_y   = 6;
                txt_w=txt_h = 14;
                text_eare_w = LT_TEXT_COL_3_W - padding_x;
                text_eare_x = h_list->c_x + LT_TEXT_COL_1_W + LT_TEXT_COL_2_W + padding_x;
                break;
        }
        text_eare_y = h_list->c_y + row*LT_TEXT_ROW_H + padding_y;
        text_eare_h = LT_TEXT_ROW_H - padding_y;
    }
    else {
        padding_x   = 1;
        padding_y   = 0;
        txt_w=txt_h = 12;
        text_eare_w = LT_PIC_COL_W - padding_x;
        text_eare_x = h_list->c_x + padding_x;
        text_eare_y = h_list->c_y + row*LT_PIC_ROW_H + row + padding_y;
        text_eare_h = LT_PIC_ROW_H - padding_y;
    }
    text_fill_canvas(h_list->h_font, 0, RGB_WHITE, txt_w, txt_h, text_eare_w, text_eare_h, text_eare_x, text_eare_y, txt);
}

void list_fill_text(list_handle h_list)
{
    int  row, img_ret = 0;
    if (h_list == NULL)      return;

    if (h_list->style == LS_TXT) {
        char time_s[20];
        char content[100];

        for (row = 0; row < MIN(list_files_num, h_list->row); row++) {
            memset(time_s,  '\0', sizeof(time_s));
            memset(content, '\0', sizeof(content));
            struct tm *tnow = localtime(&list_files_attr[h_list->start+row].ftime);
            sprintf(time_s, "%4d.%02d.%02d\n  %02d:%02d:%02d", 1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday, tnow->tm_hour, tnow->tm_min, tnow->tm_sec);
            FILE *fp  = fopen(list_files_attr[h_list->start+row].fpath, "r");
            if (fp != NULL) {
                fread(content, sizeof(content)-1, 1, fp); 
                fclose(fp);
            }
            else {
                if (h_list->start+row == list_files_num)
                    return;
            }
            list_fill_row_col(h_list, row, COL_TITLE,   list_files_attr[h_list->start+row].fname);
            list_fill_row_col(h_list, row, COL_TIME,    time_s);
            list_fill_row_col(h_list, row, COL_CONTENT, content);
        }
    }
    else {
        char fpath[FILE_PATH_LEN_MAX+12];  //len of ("./thumbnail")
        unsigned int pic_rgb[(IMG_FRAME_W+1)*IMG_FRAME_H] = {0};
        unsigned int item = h_list->start+h_list->focus;

        for (row = 0; row < MIN(list_files_num, h_list->row); row++) {   
            list_fill_row_col(h_list, row, 0, list_files_attr[h_list->start+row].fname);
        }
        if (0 != strcmp(list_files_attr[item].fexpand, "null")) {
            memset(fpath,  '\0', sizeof(fpath));
            strcpy(fpath, PATH_PIC);
            strcat(fpath, "/thumbnail/");
            strcat(fpath, list_files_attr[item].fname);
            strcat(fpath, ".");
            strcat(fpath, list_files_attr[item].fexpand);
            FILE *fp  = fopen(fpath, "r");
            if (fp != NULL)
                fclose(fp);
            else
                strcpy(fpath, list_files_attr[item].fpath);
            
            if (img_ret == img_zoom(fpath, pic_rgb, IMG_FRAME_W, IMG_FRAME_H)) {
                osd_put_canvas(pic_rgb, IMG_FRAME_W, IMG_FRAME_H, IMG_FRAME_X, IMG_FRAME_Y);
                return;
            }
        }
        osd_fill_canvas(RGB_BLACK, IMG_FRAME_W, IMG_FRAME_H, IMG_FRAME_X, IMG_FRAME_Y);
        if (0 == strcmp(list_files_attr[item].fexpand, "null")) return;
        //text_fill_canvas(h_list->h_font, 0, RGB_WHITE, 22, 22, IMG_FRAME_W, IMG_FRAME_H, IMG_FRAME_X, IMG_FRAME_Y, list_files_attr[item].fpath);
        char errmsg[100];
        strcpy(errmsg, "错误: ");
        switch(img_ret) {
            case 0:
                return;
            case IMG_FILE_ERR:
                strcat(errmsg, "打开出错");
                break;
            case IMG_FORM_ERR:
                strcat(errmsg, "格式不被支持或数据已损坏");
                break;
            case IMG_SIZE_ERR:
                strcat(errmsg, "图像过大");
                 break;
            case IMG_DATA_ERR:
                strcat(errmsg, "数据损坏");
                break;
            case IMG_MEMY_ERR:
                strcat(errmsg, "内存不足");
                break;
            default:
                strcat(errmsg, "内部错误");
                break;      
        }
        text_fill_canvas(h_list->h_font, 1, RGB_WHITE, 16, 16, IMG_FRAME_W, IMG_FRAME_H, IMG_FRAME_X, IMG_FRAME_Y, errmsg);
    }
}

void list_get_files_list(list_handle h_list)
{
    int i;
    if (h_list == NULL)  return;

/*
    if(h_list->style == LS_UNLOCK_AD){
   		file_list_in_dir(FILE_PIC, PATH_PIC_UNLOCK, list_files_attr_ad, &list_files_num_ad);
    	for (i = 0; i < h_list->row; i++)
        	memcpy(&list_files_attr_ad[list_files_num_ad + i], &list_file_blank_ad, sizeof(FILE_ATTR));
    }
*/
#ifdef ADD_UNLOCK_AD
   if(h_list->style == LS_UNLOCK_AD){
        int ad_num = 0;
   		file_list_in_dir(FILE_PIC, "./ajb", list_files_attr_ad, &list_files_num_ad);
   		file_list_in_dir(FILE_PIC, "./ad", &list_files_attr_ad[list_files_num_ad], &ad_num);
   		list_files_num_ad += ad_num;
    	for (i = 0; i < h_list->row; i++)
        	memcpy(&list_files_attr_ad[list_files_num_ad + i], &list_file_blank_ad, sizeof(FILE_ATTR));
    }

    else
#endif
    {
	    if (h_list->style == LS_TXT)
	        file_list_in_dir(FILE_TXT, PATH_TXT, list_files_attr, &list_files_num);
	    else 
	        file_list_in_dir(FILE_PIC, PATH_PIC, list_files_attr, &list_files_num);  
	    	
	    	
	    for (i = 0; i < h_list->row; i++)
	        memcpy(&list_files_attr[list_files_num + i], &list_file_blank, sizeof(FILE_ATTR));

    }
}

const char *list_get_title(list_handle h_list)
{
    if (h_list->start+h_list->focus >= list_files_num)
        return NULL;
    return list_files_attr[h_list->start+h_list->focus].fname;
}

void list_fill_canvas(list_handle h_list)
{
    if (h_list == NULL)  return;

    //list_get_files_list(h_list);
    if (h_list->detail_bg == NULL) {
        h_list->detail_bg = malloc(TXT_DETAIL_BG_W * TXT_DETAIL_BG_H * sizeof(pixel_u));
        app_debug(DBG_INFO, "h_list detail_bg malloc\n");
        if (h_list->detail_bg == NULL) {
            app_debug(DBG_ERROR, "Alloc failed: list detail_bg\n");
            return;
        }
    }
    osd_get_canvas(h_list->detail_bg, TXT_DETAIL_BG_W, TXT_DETAIL_BG_H, TXT_DETAIL_BG_X, TXT_DETAIL_BG_Y);
    img_fill_canvas(h_list->h_img, h_list->x, h_list->y);
    list_get_rgb(h_list);
    list_select_row(h_list, h_list->focus);
    list_fill_text(h_list);
}

void list_set_focus(list_handle h_list, int row)
{
    if (h_list == NULL)  return;
    if (row > h_list->row-1) return;
    h_list->focus = row;
    list_put_rgb(h_list);
    list_select_row(h_list, row);
    osd_draw_canvas(h_list->c_w, h_list->c_h, h_list->c_x, h_list->c_y);
}

void list_reset_index(list_handle h_list)
{
    if (h_list == NULL)  return;

    h_list->start  = 0; 
    h_list->focus  = 0;
    list_files_num = 0;
}

static void list_focus_move_up(list_handle h_list)
{
    if(h_list->focus > 0) {
        h_list->focus--;
    }
    else
    if (h_list->start > 0) {
        h_list->start--;
    }
}

static void list_focus_move_down(list_handle h_list)
{
    int start_max;
    if (h_list->focus < h_list->row-1) {
        h_list->focus++;
    }
    else {
        if (list_files_num >= h_list->row)
            start_max = list_files_num - h_list->row + 1;
        else
            start_max = 0;
            
        if (h_list->start < start_max) {
            h_list->start++;
        }
    }
}

void list_move_up(list_handle h_list)
{
    if (h_list == NULL)  return;

    list_put_rgb(h_list);
    list_focus_move_up(h_list);
    list_select_row(h_list, h_list->focus);
    list_fill_text(h_list);
    osd_draw_canvas(h_list->c_w, h_list->c_h, h_list->c_x, h_list->c_y);
    if (h_list->style == LS_PIC)
        osd_draw_canvas(IMG_FRAME_W, IMG_FRAME_H, IMG_FRAME_X, IMG_FRAME_Y);
}

void list_move_down(list_handle h_list)
{
    if (h_list == NULL)  return;

    list_put_rgb(h_list);
    list_focus_move_down(h_list);
    list_select_row(h_list, h_list->focus);
    list_fill_text(h_list);
    osd_draw_canvas(h_list->c_w, h_list->c_h, h_list->c_x, h_list->c_y);
    if (h_list->style == LS_PIC)
        osd_draw_canvas(IMG_FRAME_W, IMG_FRAME_H, IMG_FRAME_X, IMG_FRAME_Y);
}

int list_fill_detail(list_handle h_list)
{
    int title_h = 24;
    int title_w = TXT_DETAIL_BG_W;
    int title_y = TXT_DETAIL_BG_Y; 
    int title_x;
    int item = h_list->start+h_list->focus;
    unsigned int title_end_pos;

    if (h_list->style == LS_TXT) {
        const char *title = list_files_attr[item].fname;
        char content[250*2];
        if (list_files_num == 0)
            list_get_files_list(h_list);
        if (item >= list_files_num) {
            title = "End!";
        }
        else {
            memset(content, '\0', sizeof(content));
            FILE *fp  = fopen(list_files_attr[item].fpath, "r");
            if (fp != NULL) {
                fread(content, sizeof(content)-1, 1, fp); 
                fclose(fp);
            }
            else  {
                strcpy(content, "读取失败");
            }
        }
        
        if (title == NULL && strlen(content) == 0) return -1;
        title_end_pos = text_fill_canvas(h_list->h_font, 0, RGB_WHITE, 20, 20, title_w, title_h, TXT_DETAIL_BG_X, TXT_DETAIL_BG_Y, title);
        osd_put_canvas(h_list->detail_bg, TXT_DETAIL_BG_W, TXT_DETAIL_BG_H, TXT_DETAIL_BG_X, TXT_DETAIL_BG_Y);
        title_x = TXT_DETAIL_BG_X + (TXT_DETAIL_BG_X+TXT_DETAIL_BG_W-(title_end_pos&0xffff))/2;
        text_fill_canvas(h_list->h_font, 0, RGB_WHITE, 20, 20, TXT_DETAIL_BG_W, title_h, title_x, title_y, title);
        text_fill_canvas(h_list->h_font, 0, RGB_WHITE, 16, 16, TXT_DETAIL_BG_W, TXT_DETAIL_BG_H-title_h, TXT_DETAIL_BG_X, title_y+title_h, content);
    }
    else {
        unsigned int pic_rgb[UI_FULL_W*UI_FULL_H] = {0};
        if (list_files_num == 0)
            list_get_files_list(h_list);
        if (0 == strcmp(list_files_attr[item].fexpand, "null")) return -1;
        if (0 != img_zoom(list_files_attr[item].fpath, pic_rgb, UI_FULL_W, UI_FULL_H)) return -1;
        osd_put_canvas(pic_rgb, UI_FULL_W, UI_FULL_H, 0, 0);
        auto_slide_item = item;
    }
    return 0;
}

void list_show_next(list_handle h_list)
{
    if (h_list == NULL)  return;

    list_focus_move_down(h_list);
    list_fill_detail(h_list);
    osd_draw_canvas(TXT_DETAIL_BG_W, TXT_DETAIL_BG_H, TXT_DETAIL_BG_X, TXT_DETAIL_BG_Y);
}

void list_show_prev(list_handle h_list)
{
    if (h_list == NULL)  return;

    list_focus_move_up(h_list);
    list_fill_detail(h_list);
    osd_draw_canvas(TXT_DETAIL_BG_W, TXT_DETAIL_BG_H, TXT_DETAIL_BG_X, TXT_DETAIL_BG_Y);
}

const char *list_get_pic_path(list_handle h_list)
{
    if (h_list == NULL)  return NULL;
    if (0 == strcmp(list_files_attr[h_list->start+h_list->focus].fexpand, "null")) 
        return NULL;
    return list_files_attr[h_list->start+h_list->focus].fpath;
}


int list_auto_slide_show(list_handle h_list)
{
    int curr_item = -1;
    if (list_files_num == 0) {
        list_get_files_list(h_list);
        auto_slide_item = 0;
    }
    if (list_files_num == 0) return -1;
    
    if (h_list->style == LS_PIC) {
        unsigned int pic_rgb[UI_FULL_W*UI_FULL_H] = {0};
        if (0 == strcmp(list_files_attr[auto_slide_item].fexpand, "null")) return -1;
        //if (0 != img_zoom(list_files_attr[auto_slide_item].fpath, pic_rgb, UI_FULL_W, UI_FULL_H)) return;
        while (0 != img_zoom(list_files_attr[auto_slide_item].fpath, pic_rgb, UI_FULL_W, UI_FULL_H)) {
            if (curr_item == -1)                     curr_item = auto_slide_item;
            if (++auto_slide_item >= list_files_num) auto_slide_item = 0;
            if (curr_item == auto_slide_item)        return -1;
        }
        osd_put_canvas(pic_rgb, UI_FULL_W, UI_FULL_H, 0, 0);
        osd_draw_canvas(UI_FULL_W, UI_FULL_H, 0, 0);
    }
    if (++auto_slide_item >= list_files_num)
        auto_slide_item = 0;
    return 0;
}
#ifdef ADD_UNLOCK_AD

int list_auto_unlock_ad_show(list_handle h_list)
{
    int curr_item = -1; 
    //static int list_files_num_ad_pr;

    //system("chmod 777 /opt/app/ajb/");    
    //system("cp /opt/app/ajb/*AJB*.* /opt/app/ad/");
   // system("chmod 444 /opt/app/ajb/"); 
    //printf("------------get_rand()=%d =%d\n",list_files_num_ad,get_rand(list_files_num_ad));
    //if (list_files_num_ad == 0) {
        list_get_files_list(h_list);
        auto_slide_item_ad = 0;
   // }    

   // printf("list_files_num_ad=%d auto_slide_item_ad=%d\n",list_files_num_ad,auto_slide_item_ad);
    if (list_files_num_ad == 0) return -1;
    if(list_files_num_ad > 5 )
		auto_slide_item_ad =  get_rand(5);
	else	
   		auto_slide_item_ad = get_rand((list_files_num_ad));   		

   		//auto_slide_item_ad=4;

   // printf("list_files_num_ad=%d auto_slide_item_ad=%d\n",list_files_num_ad,auto_slide_item_ad);
   
    
    if (h_list->style == LS_UNLOCK_AD) {
        unsigned int pic_rgb[UI_FULL_W*UI_FULL_H] = {0};
        if (0 == strcmp(list_files_attr[auto_slide_item_ad].fexpand, "null")) return -1;
        //if (0 != img_zoom(list_files_attr[auto_slide_item].fpath, pic_rgb, UI_FULL_W, UI_FULL_H)) return;
        while (0 != img_zoom(list_files_attr_ad[auto_slide_item_ad].fpath, pic_rgb, UI_FULL_W, UI_FULL_H)) {
            if (curr_item == -1)                     curr_item = auto_slide_item_ad;
            
            //if (++auto_slide_item_ad >= list_files_num_ad) auto_slide_item_ad = 0;
            
            if (curr_item == auto_slide_item_ad)        return -1;
        }
        
        osd_put_canvas(pic_rgb, UI_FULL_W, UI_FULL_W, 0, 0);
        osd_draw_canvas(UI_FULL_W, UI_FULL_W, 0, 0);
    }

    //printf("list_files_num_ad=%d auto_slide_item_ad=%d\n",list_files_num_ad,auto_slide_item_ad);
    //if (++auto_slide_item_ad >= list_files_num_ad)
       // auto_slide_item_ad = 0;
    return 0;
}

#endif

void list_free(list_handle h_list)
{
    if (h_list == NULL) return;

    if (h_list->rgb) {
        free(h_list->rgb);
        h_list->rgb = NULL;
    }
    if (h_list->detail_bg) {
        free(h_list->detail_bg);
        h_list->detail_bg = NULL;
    }
    if (h_list->h_img) {
        img_delete(h_list->h_img);
        h_list->h_img = NULL;
    }
}

void list_delete(list_handle h_list)
{
    if (h_list == NULL)  return; 

    list_free(h_list);

    free(h_list);
}


