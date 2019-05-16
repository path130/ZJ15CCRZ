#ifndef _FBDEV_H_
#define _FBDEV_H_

#include <linux/fb.h>

#if defined (__cplusplus)
extern "C" {
#endif

#define OSD0_DEVICE			"/dev/frame"
// #define OSD1_DEVICE			"/dev/fb2"
// #define VID0_DEVICE			"/dev/fb1"
// #define VID1_DEVICE			"/dev/fb3"

#define MAX_FBDEV_BUF_NUM	3
#define BYTES_PER_PIXEL		2

#define CFG_DEFAULT_FULL_SCREEN

struct fbdev_object {
    int    fd;
    const  char *device;
    char   *buffer[MAX_FBDEV_BUF_NUM];
    struct fb_fix_screeninfo fix_info;
	struct fb_var_screeninfo var_info;
    unsigned int  buf_num; 
    unsigned int  buf_size;
    unsigned int  buf_index;
};

struct fbdev_attrs {
	unsigned int xpos;
    unsigned int ypos;
    unsigned int width;
    unsigned int height;
    unsigned int bpp;
    unsigned int xoffset;
    unsigned int yoffset;
    unsigned int vmode;
    unsigned int buf_num;
};

typedef struct fbdev_object *fbdev_handle;

extern void fbdev_reg(fbdev_handle h_fb, const char *dev_file);
extern int  fbdev_open(fbdev_handle h_fb, const struct fbdev_attrs *param);
extern void fbdev_close(fbdev_handle h_fb);
extern int  fbdev_clear(fbdev_handle h_fb);

#if defined (__cplusplus)
#endif

#endif
