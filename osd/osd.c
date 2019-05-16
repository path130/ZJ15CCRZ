/*
 ============================================================================
 Name        : osd.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : DM365 OSD部分
  ============================================================================
 */
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
// #include <video/davincifb_ioctl.h>
// #include <video/davinci_osd.h>
#include "dpplatform.h"
#include "dpgpio.h"

#include "common.h"
#include "osd.h"

struct fbdev_attrs  osd0_attrs = {          
    .xpos   	= 0,
    .ypos   	= 0,
    .width  	= OSD0_WIDTH,
    .height 	= OSD0_HEIGHT,
    .bpp        = 16,
    .xoffset    = 0,
    .yoffset    = 0,
    .vmode      = OSD0_VMODE,
    .buf_num    = OSD0_BUFNUM,
};

struct fbdev_attrs  osd1_attrs = {
    .xpos   	= 0,
    .ypos   	= 0,
    .width  	= OSD0_WIDTH,
    .height 	= OSD0_HEIGHT,
    .bpp        = 4,
    .xoffset    = 0,
    .yoffset    = 0,
    .vmode      = OSD0_VMODE,
    .buf_num    = 1,
};

struct fbdev_object osd0, osd1;
struct rectangle    osd0_rect;

fbdev_handle h_osd;
fbdev_handle h_attr;
pixel_u     *canvas;


const int MUL255[] = {    
        0,   255,  510,    765,  1020,  1275,  1530,  1785,  2040,  2295,  2550,  2805,  3060,  3315,  3570,  3825,
     4080,  4335,  4590,  4845,  5100,  5355,  5610,  5865,  6120,  6375,  6630,  6885,  7140,  7395,  7650,  7905, 
     8160,  8415,  8670,  8925,  9180,  9435,  9690,  9945, 10200, 10455, 10710, 10965, 11220, 11475, 11730, 11985, 
    12240, 12495, 12750, 13005, 13260, 13515, 13770, 14025, 14280, 14535, 14790, 15045, 15300, 15555, 15810, 16065, 
    16320, 16575, 16830, 17085, 17340, 17595, 17850, 18105, 18360, 18615, 18870, 19125, 19380, 19635, 19890, 20145, 
    20400, 20655, 20910, 21165, 21420, 21675, 21930, 22185, 22440, 22695, 22950, 23205, 23460, 23715, 23970, 24225, 
    24480, 24735, 24990, 25245, 25500, 25755, 26010, 26265, 26520, 26775, 27030, 27285, 27540, 27795, 28050, 28305, 
    28560, 28815, 29070, 29325, 29580, 29835, 30090, 30345, 30600, 30855, 31110, 31365, 31620, 31875, 32130, 32385, 
    32640, 32895, 33150, 33405, 33660, 33915, 34170, 34425, 34680, 34935, 35190, 35445, 35700, 35955, 36210, 36465, 
    36720, 36975, 37230, 37485, 37740, 37995, 38250, 38505, 38760, 39015, 39270, 39525, 39780, 40035, 40290, 40545, 
    40800, 41055, 41310, 41565, 41820, 42075, 42330, 42585, 42840, 43095, 43350, 43605, 43860, 44115, 44370, 44625, 
    44880, 45135, 45390, 45645, 45900, 46155, 46410, 46665, 46920, 47175, 47430, 47685, 47940, 48195, 48450, 48705, 
    48960, 49215, 49470, 49725, 49980, 50235, 50490, 50745, 51000, 51255, 51510, 51765, 52020, 52275, 52530, 52785, 
    53040, 53295, 53550, 53805, 54060, 54315, 54570, 54825, 55080, 55335, 55590, 55845, 56100, 56355, 56610, 56865, 
    57120, 57375, 57630, 57885, 58140, 58395, 58650, 58905, 59160, 59415, 59670, 59925, 60180, 60435, 60690, 60945,
    61200, 61455, 61710, 61965, 62220, 62475, 62730, 62985, 63240, 63495, 63750, 64005, 64260, 64515, 64770, 65025, 65280};

/* 
 * OSD初始化
 */
void osd_init(void)
{
	osd0_rect.xpos   = osd0_attrs.xpos;
    osd0_rect.ypos   = osd0_attrs.ypos;

    h_osd  = &osd0;
    h_attr = &osd1;

    fbdev_reg(h_osd, OSD0_DEVICE);
    if (fbdev_open(h_osd, &osd0_attrs) < 0) {
		ERR("Failed open framebuf osd0!\n");
		exit(0);
	}
    
	/*fbdev_reg(h_attr, OSD1_DEVICE);
    if (fbdev_open(h_attr, &osd1_attrs) < 0) {
		ERR("Failed open framebuf osd1!\n");
		exit(0);
	}*/

    osd0_rect.width  = h_osd->var_info.xres;
    osd0_rect.height = h_osd->var_info.yres;
    
    fbdev_clear(h_osd);
    osd_set_transparent(&osd0_rect, 0xff);
    
    canvas = calloc(h_osd->var_info.xres * (h_osd->var_info.yres+8), 4);
    if (canvas == NULL) {
        ERR("Failed to allocate memory for canvas\n");
        exit(-1);
    }
}

/* 设置OSD的一个矩形区域相对于视频图层(fb1/fb3)的透明度
 * rect     矩形区域
 * value    透明度
 */
void osd_set_transparent(struct rectangle *rect, unsigned char value)
{return;
    int   y;
    int   rect_line_bytes = rect->width / 2;
    int   fb_line_bytes   = h_attr->fix_info.line_length; 
    int   y_max = MIN((h_attr->var_info.yres - rect->ypos), rect->height);
	
#if 0
    char *dst   = (char*)h_attr->buffer[h_attr->buf_index] + h_attr->fix_info.line_length * rect->ypos + rect->xpos / 2;
    for (y = 0; y < y_max; y++) {
		memset(dst, value, rect_line_bytes);
		dst += fb_line_bytes;
	}
#else
    char *dst     = (char*)h_attr->buffer[h_attr->buf_index];
    int  fb_bytes = h_attr->fix_info.line_length*h_attr->var_info.yres;
    unsigned char tmpval[1024*678];
    unsigned char *dsttmp  = tmpval+h_attr->fix_info.line_length * rect->ypos + rect->xpos / 2;
    memset(tmpval, 0xFF, fb_bytes);
    if (value != 0xFF) {
        for (y = 0; y < y_max; y++) {
		    memset(dsttmp, value, rect_line_bytes);
		    dsttmp += fb_line_bytes;
	    }
    }
    memcpy(dst, tmpval, fb_bytes);
#endif
}

/* 移动整个OSD到另一个物理位置
 * xpos     要移动到的坐标x
 * ypos 	起移动到的坐标y
 */
void osd_move(int xpos, int ypos)
{
// 	/* Set window position */
//     if (ioctl(h_osd->fd, FBIO_SETPOSX, xpos) < 0) {
//         ERR("Failed  FBIO_SETPOSX for %s, xpos:%d\n", h_osd->device, xpos);
//         return;
//     }
//     if (ioctl(h_osd->fd, FBIO_SETPOSY, ypos) < 0) {
//         ERR("Failed  FBIO_SETPOSY for %s, ypos:%d\n", h_osd->device, ypos);
//         return;
//     }
// 
// 	if (ioctl(h_attr->fd, FBIO_SETPOSX, xpos) < 0) {
//         ERR("Failed  FBIO_SETPOSX for %s, xpos:%d\n", h_attr->device, xpos);
//         return;
//     }
//     if (ioctl(h_attr->fd, FBIO_SETPOSY, ypos) < 0) {
//         ERR("Failed  FBIO_SETPOSY for %s, ypos:%d\n", h_attr->device, ypos);
//         return;
//     }
}

/* 在rect矩形区域内填充color色
 * rect     矩形区域
 * color	要填充的颜色
 */
void osd_fill(struct rectangle *rect, int color)
{
#if 0
    int  x, y;
    int  x_max  = MIN((h_osd->var_info.xres - rect->xpos), rect->width);
	int  y_max  = MIN((h_osd->var_info.yres - rect->ypos), rect->height);
    pixel_u *dst  = canvas + rect->ypos * h_osd->var_info.xres + rect->xpos;
    for (y = 0; y < y_max; y++) {
        for (x = 0; x < x_max; x++) {
            dst[x].rgba = color;
        }
        dst += h_osd->var_info.xres;
    }
#endif
    osd_fill_canvas(color, rect->width, rect->height, rect->xpos, rect->ypos);
    osd_draw_canvas(rect->width, rect->height, rect->xpos, rect->ypos);
}

/* 按alpha混合前背景色,实现透明效果
 * alpha	透明通道(0不透明~32全透明)
 * xpos     起始坐标x
 * ypos 	起始坐标y
 */
#define RGB_MASK	0x7E0F81F
inline void make_alpha(unsigned short *pback_color, unsigned short fore_color, unsigned char alpha)
{
#if 0
	register unsigned blend;
	register unsigned fore = *pfore_color;
	register unsigned back = *pback_color;
	register unsigned fore24 = (fore<<16|fore)&RGB_MASK;

	blend = ((((((back<<16|back)&RGB_MASK) - fore24) * alpha) >>5) + fore24) & RGB_MASK;
	
	*pback_color =  (blend >> 16) | (blend & 0xFFFF);

#else

	register unsigned int fore24 = (fore_color<<16  | fore_color)  & RGB_MASK;
	register unsigned int back24 = (*pback_color<<16|*pback_color) & RGB_MASK;
	register unsigned int blend = (((back24-fore24) * alpha)/32 + fore24) & RGB_MASK;

	*pback_color =  (blend >> 16) | (blend & 0xFFFF);
#endif
}

inline unsigned short alpha_color(unsigned short pback_color, unsigned short fore_color, unsigned char alpha)
{
    register unsigned int fore24 = (fore_color<<16  | fore_color)  & RGB_MASK;
	register unsigned int back24 = (pback_color<<16 | pback_color) & RGB_MASK;
	register unsigned int blend = (((back24-fore24) * alpha)/32 + fore24) & RGB_MASK;

	return (blend >> 16) | (blend & 0xFFFF);
}

void osd_fill_canvas(int rgba, int width, int height, int xpos, int ypos)
{
    int	 x, y;
	int  x_max  = MIN((h_osd->var_info.xres - xpos), width);
	int  y_max  = MIN((h_osd->var_info.yres - ypos), height);

    int  canvas_line_pixels = h_osd->var_info.xres;

    pixel_u *dst = canvas + ypos*canvas_line_pixels + xpos;
    
    pixel_u p;
    p.rgba = rgba;
    if ((p.c.r == p.c.g) && (p.c.r == p.c.b)) {
        int	 cpy_line_bytes = x_max * sizeof(pixel_u);
        for (y = 0; y < y_max; y++) {
            memset(dst, p.c.r, cpy_line_bytes);
            dst += canvas_line_pixels;
        }
    } else {
	    for (y = 0; y < y_max; y++) {
            for (x = 0; x < x_max; x++) {
                dst[x].rgba = rgba;
            }
		    dst += canvas_line_pixels;
	    }
    }
}

void osd_get_canvas(void *rgba, int width, int height, int xpos, int ypos)
{
    int	 y;
	int  x_max  = MIN((h_osd->var_info.xres - xpos), width);
	int  y_max  = MIN((h_osd->var_info.yres - ypos), height);
   
    int  canvas_line_pixels = h_osd->var_info.xres;
	int	 cpy_line_bytes     = x_max * sizeof(pixel_u);    //RGBA *4

    if (rgba == NULL) return;
    pixel_u *dst = (pixel_u*)rgba;
    pixel_u *src = canvas + ypos*canvas_line_pixels + xpos;

	for (y = 0; y < y_max; y++) {
		memcpy(dst, src, cpy_line_bytes);
		dst += x_max;
		src += canvas_line_pixels;
	}
}

void osd_put_canvas(void *rgba, int width, int height, int xpos, int ypos)
{
    int	 y;
	int  x_max  = MIN((h_osd->var_info.xres - xpos), width);
	int  y_max  = MIN((h_osd->var_info.yres - ypos), height);

    int  canvas_line_pixels = h_osd->var_info.xres;
	int	 cpy_line_bytes     = x_max * sizeof(pixel_u);    //RGBA *4

    if (rgba == NULL) return;

    pixel_u *dst = canvas + ypos*canvas_line_pixels + xpos;
    pixel_u *src = (pixel_u*)rgba;

	for (y = 0; y < y_max; y++) {
		memcpy(dst, src, cpy_line_bytes);
		dst += canvas_line_pixels;
		src += x_max;
	}
}

void osd_draw_canvas(int width, int height, int xpos, int ypos)
{
    int	 i, x, y;
	int  x_max  = MIN((h_osd->var_info.xres - xpos), width);
	int  y_max  = MIN((h_osd->var_info.yres - ypos), height);

    int  depth = h_osd->var_info.bits_per_pixel;
    int  fb_line_pixels = h_osd->var_info.xres;
	int	 cpy_line_bytes = x_max * depth / 8;
    //printf("----------x_max:%d y_max:%d cpy_line_bytes :%d depth:%d\n", x_max, y_max, cpy_line_bytes, depth);

    if (depth == 16) {
        unsigned short rgb565[1024*600];
        unsigned short *dst = (unsigned short*)h_osd->buffer[h_osd->buf_index] + fb_line_pixels * ypos + xpos;
        unsigned short *src = rgb565;
        pixel_u *piexl_src  = canvas;

        for (y = 0; y < y_max; y++) {
            i = h_osd->var_info.xres * (ypos+y) + xpos;
            for (x = 0; x < x_max; x++) {
                *src = (short)RGB888_TO_RGB565(piexl_src[i].c.r, piexl_src[i].c.g, piexl_src[i].c.b);
                src++;
                i++;
            }
        }
        src = rgb565;
	    for (y = 0; y < y_max; y++) {
		    memcpy(dst, src, cpy_line_bytes);
		    dst += fb_line_pixels;
		    src += x_max;
	    }
    } else if (depth == 32) {
        unsigned int *dst = (unsigned int*)h_osd->buffer[h_osd->buf_index] + fb_line_pixels * ypos + xpos;
        unsigned int *src = (unsigned int*)canvas + fb_line_pixels * ypos+ xpos;

        for (y = 0; y < y_max; y++) {
		    memcpy(dst, src, cpy_line_bytes);
		    dst += fb_line_pixels;
		    src += fb_line_pixels;
	    }
    }
        ioctl(h_osd->fd, IOCTL_DEV_SHOW, NULL);

}

/* 在OSD上输出RGBA数据
 * *rgba	    RGB数据指针
 * width    宽度
 * height   高度
 * xpos     起始坐标x
 * ypos 	起始坐标y
 */
void osd_draw_rgba(const int *rgba, int width, int height, int xpos, int ypos)
{
	int	 i, x, y;
	int  x_max  = MIN((h_osd->var_info.xres - xpos), width);
	int  y_max  = MIN((h_osd->var_info.yres - ypos), height);

    int  depth = h_osd->fix_info.line_length/h_osd->var_info.xres;
    int  fb_line_pixels = h_osd->var_info.xres;
	int	 cpy_line_bytes = x_max * depth / 8;

    if (depth == 16) {
        unsigned short rgb565[1024*600];
        unsigned short *dst = (unsigned short*)h_osd->buffer[h_osd->buf_index] + fb_line_pixels * ypos + xpos;
        unsigned short *src = rgb565;
        pixel_u *piexl_src  = (pixel_u*)rgba;

        for (y = 0; y < y_max; y++) {
            i = h_osd->var_info.xres * (ypos+y) + xpos;
            for (x = 0; x < x_max; x++) {
                *src = (short)RGB888_TO_RGB565(piexl_src[i].c.r, piexl_src[i].c.g, piexl_src[i].c.b);
                src++;
                i++;
            }
        }
        src = rgb565;
	    for (y = 0; y < y_max; y++) {
		    memcpy(dst, src, cpy_line_bytes);
		    dst += fb_line_pixels;
		    src += x_max;
	    }
    } else if (depth == 32) {
        unsigned int *dst = (unsigned int*)h_osd->buffer[h_osd->buf_index] + fb_line_pixels * ypos + xpos;
        unsigned int *src = (unsigned int*)rgba;

        for (y = 0; y < y_max; y++) {
		    memcpy(dst, src, cpy_line_bytes);
		    dst += fb_line_pixels;
		    src += x_max;
	    }
    }
}

void osd_get_rgb(void *rgb, int width, int height, int xpos, int ypos)
{
    int	 y;
    int  fb_line_pixels = h_osd->var_info.xres;
	int	 cpy_line_bytes = MIN((h_osd->var_info.xres - xpos), width) * 2;

	int  y_max  = MIN((h_osd->var_info.yres - ypos), height);
	unsigned short *dst  = (unsigned short*)rgb;
    unsigned short *src  = (unsigned short*)h_osd->buffer[h_osd->buf_index] + fb_line_pixels * ypos + xpos;

	for (y = 0; y < y_max; y++) {
		memcpy(dst, src, cpy_line_bytes);
		dst += width;
		src += fb_line_pixels;
	}
}

