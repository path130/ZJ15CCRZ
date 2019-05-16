#ifndef _OSD_H_
#define _OSD_H_

#include "fbdev.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define CFG_RGBA_ENTIRE

/******************* OSD config *******************/ 
#define OSD0_WIDTH  		320
#define OSD0_HEIGHT 		240
#define OSD0_VMODE  		FB_VMODE_INTERLACED
#define OSD0_BUFNUM 		1

#if (OSD0_BUFNUM > MAX_FBDEV_BUF_NUM || OSD0_BUFNUM < 1)
#error "OSD0_BUFNUM must between in 1 and MAX_BUFFERS_NUM"
#endif

#define RGB_BLACK           0x000000
#define RGB_WHITE           0xFFFFFF
#define RGB_RED             0xFF0000
#define RGB_GREEN           0x00FF00
#define RGB_BULE            0x0000FF

#define BRG888_TO_RGB565(B ,G ,R)   ( ((char)(B)>>3) | ((short)((char)(G)>>2))<<5 | ((short)((char)(R)>>3))<<11 )
#define RGB888_TO_RGB565(R ,G ,B)   ( ((char)(B)>>3) | ((short)((char)(G)>>2))<<5 | ((short)((char)(R)>>3))<<11 )

/* AARRGGBB */
typedef struct {
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
} __attribute__((packed)) rgba_t;

typedef union {
    unsigned int  rgba;
    rgba_t        c;//unsigned char c[4];
} pixel_u;

struct rectangle {  
    unsigned int xpos;
    unsigned int ypos;
    unsigned int width;
    unsigned int height;
};

extern pixel_u *canvas;
extern const int MUL255[];
extern struct fbdev_attrs  osd0_attrs;
extern struct fbdev_attrs  osd1_attrs;
extern struct rectangle    osd0_rect;

extern void osd_init(void);
extern void osd_move(int xpos, int ypos);
extern void osd_fill(struct rectangle *rect, int color);
extern void osd_set_transparent(struct rectangle *rect, unsigned char value);
extern void osd_draw_canvas(int width, int height, int xpos, int ypos);
extern void osd_get_canvas(void *rgba, int width, int height, int xpos, int ypos);
extern void osd_put_canvas(void *rgba, int width, int height, int xpos, int ypos);
extern void osd_fill_canvas(int rgba, int width, int height, int xpos, int ypos);
extern void osd_draw_rgba(const int *rgba, int width, int height, int xpos, int ypos);
extern int  osd_draw_text(unsigned short color, const unsigned char *grays, int width, int height, int xpos, int ypos);


    /* frame for face detect */
extern int fd_face;
extern void *frame_face;




#if defined (__cplusplus)
}
#endif

#endif
