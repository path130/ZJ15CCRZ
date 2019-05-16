#ifndef _CATALOG_H_
#define _CATALOG_H_

#if defined (__cplusplus)
extern "C" {
#endif
#include "text.h"
#define CATALOG_ROWS            6
#define CATALOG_COLS            2

#define CLG_ROW_H               24
#define CLG_TITLE_H             23
#define CLG_RIM_SIZE            1
#define CLG_SEL_COLOR           0x4189A3
#define CLG_COL_ROOM            1
#define CLG_COL_NAME            2

#define CATALOG_X               18
#define CATALOG_Y               38
#define CATALOG_W               (320-CATALOG_X*2)
#define CATALOG_H               (CATALOG_ROWS*(CLG_ROW_H+CLG_RIM_SIZE)+CLG_TITLE_H+CLG_RIM_SIZE)
#define CLG_COL_ROOM_W          100//wrm 20141112 50
#define CLG_COL_NAME_W          (CLG_WIDTH-3*CLG_RIM_SIZE-CLG_COL_ROOM_W)
#define CLG_WIDTH               CATALOG_W
#define CLG_HEIGHT              CATALOG_H

#define CLG_COL_X_OFT_ROOM      CLG_RIM_SIZE
#define CLG_COL_X_OFT_NAME      (CLG_COL_X_OFT_ROOM+CLG_COL_ROOM_W+CLG_RIM_SIZE)

typedef struct catalog_object_struct {  
    int             x, y, w, h;
    int             c_x, c_y, c_w, c_h;
    int             txt_w, txt_h;
    int             col, row;   
    int             focus, start;
    void            *rgb;
    font_handle     h_font;
} catalog_object, *catalog_handle;

struct catalog_s
{
    unsigned char room[9];//wrm 20141112 unsigned char room[4]
    char name[36];
};

extern struct catalog_s const catalog_table[];
extern catalog_handle   catalog_create_ex(font_handle h_font, int w, int h, int x, int y);
extern void             catalog_get_files_catalog(catalog_handle h_catalog);
extern void             catalog_fill_canvas(catalog_handle h_catalog);
extern void             catalog_move_up(catalog_handle h_catalog);
extern void             catalog_move_down(catalog_handle h_catalog);
extern void             catalog_page_up(catalog_handle h_catalog);
extern void             catalog_page_down(catalog_handle h_catalog);
extern void             catalog_reset_index(catalog_handle h_catalog);
extern int              catalog_get_bcd(catalog_handle h_catalog, unsigned char bcd[]);
extern void             catalog_free(catalog_handle h_catalog);
extern void             catalog_delete(catalog_handle h_catalog);
#if defined (__cplusplus)
}
#endif
 
#endif
