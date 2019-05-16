#ifndef _LIST_H_
#define _LIST_H_

#if defined (__cplusplus)
extern "C" {
#endif

#define LS_PIC         0x0001  
#define LS_TXT         0x0002
#define LS_UNLOCK_AD         0x0003

#define LT_TEXT_ROW_H       30
#define LT_TEXT_COL_1_W     68
#define LT_TEXT_COL_2_W     68
#define LT_TEXT_COL_3_W     164

#define LT_PIC_COL_W        78
#define LT_PIC_ROW_H        15

#define TXT_DETAIL_BG_W     308
#define TXT_DETAIL_BG_H     160
#define TXT_DETAIL_BG_X     6
#define TXT_DETAIL_BG_Y     36

#define COL_TITLE           0
#define COL_TIME            1
#define COL_CONTENT         2
#define ROW_1               0
#define ROW_2               1
#define ROW_3               2
#define ROW_4               3
#define ROW_5               4
#define ROW_6               5

typedef struct list_object_struct { 
    int             style;    
    int             x, y, w, h;
    int             c_x, c_y, c_w, c_h;
    int             txt_w, txt_h;
    int             col, row;   
    int             focus, start;
    void            *rgb;
    void            *detail_bg;
    img_handle      h_img;
    font_handle     h_font;
} list_object, *list_handle;

extern list_handle  list_create_ex(font_handle h_font, const char *f_img, int x, int y, unsigned int style);
extern void         list_get_files_list(list_handle h_list);
extern void         list_fill_canvas(list_handle h_list);
extern void         list_move_up(list_handle h_list);
extern void         list_move_down(list_handle h_list);
extern int          list_fill_detail(list_handle h_list);
extern void         list_show_next(list_handle h_list);
extern void         list_show_prev(list_handle h_list);
extern void         list_reset_index(list_handle h_list);
extern const char  *list_get_pic_path(list_handle h_list);
extern const char  *list_get_title(list_handle h_list);
extern int          list_auto_slide_show(list_handle h_list);
extern void         list_free(list_handle h_list);
extern void         list_delete(list_handle h_list);
#if defined (__cplusplus)
}
#endif
 
#endif
