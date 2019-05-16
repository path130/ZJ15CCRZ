/*
 ============================================================================
 Name        : fbdev.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : DM365 framebuf部分
  ============================================================================
 */
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
// #include <video/davinci_osd.h>
// #include <video/davincifb_ioctl.h>
#include "dpplatform.h"
#include "dpgpio.h"

#include "common.h"
#include "fbdev.h"


    /* frame for face detect */
int fd_face;
void *frame_face;


/* 注册一个framebuf设备到fbdev_handle
 * h_fb 	fbdev_handle
 * dev_file	framebuf设备
 */
void fbdev_reg(fbdev_handle h_fb, const char *dev_file)
{
    int i;
    h_fb->fd        = -1;
    h_fb->device    = dev_file;
    h_fb->buf_index = 0;
    for (i = 0; i < MAX_FBDEV_BUF_NUM; i++) {
        h_fb->buffer[i] = NULL;
    }
}

/* 打开一个framebuf设备,并设置参数,返回设备文件指针
 * h_fb 	fbdev_handle
 * param	framebuf设备参数
 */
int fbdev_open(fbdev_handle h_fb, const struct fbdev_attrs *param)
{
    int i;
    
    int fd = open(h_fb->device, O_RDWR);
    if (fd < 0) {
        if (h_fb->device == NULL) {
            ERR("Must provide h_fb->device, you should run fbdev_reg() first!");
        }
        ERR("Could not open %s\n", h_fb->device);
        return -1;
    }
    /*if (ioctl(fd, FBIOGET_FSCREENINFO, &h_fb->fix_info) < 0) {
        ERR("Could not get fix_info of %s\n", h_fb->device);
        close(fd);
        return -2;
    }
    if (ioctl(fd, FBIOGET_VSCREENINFO, &h_fb->var_info) < 0) {
        ERR("Could not get var_info of %s\n", h_fb->device);
        close(fd);
        return -3;
    }
    
    printf("h_fb->device:%s xres:%d,yres:%d, " \
          "bits_per_pixel:%d line_bytes:%d\n", h_fb->device,h_fb->var_info.xres,h_fb->var_info.yres,h_fb->var_info.bits_per_pixel,h_fb->fix_info.line_length);
	*/
h_fb->var_info.xres = 320;
h_fb->var_info.yres = 240;
h_fb->var_info.bits_per_pixel = 32;
h_fb->fix_info.line_length = 320*4;
    /*Set the parameters of fbdev*/
    h_fb->fd = fd;
    h_fb->buf_size = h_fb->fix_info.line_length * h_fb->var_info.yres; 
    h_fb->buf_num  = param->buf_num;
    h_fb->var_info.xoffset = 0;
    h_fb->var_info.yoffset = 0;
    h_fb->var_info.vmode = param->vmode;
    //h_fb->var_info.bits_per_pixel = param->bpp; //此处不要设置bpp，在UBOOT中设置以让内核适应RGB565/RGB888

#ifndef CFG_DEFAULT_FULL_SCREEN
    h_fb->var_info.xres  = param->width;
    h_fb->var_info.yres  = param->height;
    h_fb->var_info.xres_virtual   = param->width;
    h_fb->var_info.yres_virtual   = param->height * param->buf_num;
#endif

    /* Set  window format */
    /*if (ioctl(fd, FBIOPUT_VSCREENINFO, &h_fb->var_info) < 0) {
        ERR("Failed FBIOPUT_VSCREENINFO for %s\n", h_fb->device);
        return -4;
    }

    if (ioctl(fd, FBIO_SETPOSX, param->xpos) < 0) {
        ERR("Failed  FBIO_SETPOSX for %s, param->xpos:%d\n", h_fb->device, param->xpos);
        return -5;
    }
    if (ioctl(fd, FBIO_SETPOSY, param->ypos) < 0) {
        ERR("Failed  FBIO_SETPOSY for %s, param->ypos:%d\n", h_fb->device, param->ypos);
        return -6;
    }

    if (ioctl(fd, FBIOBLANK, 0)) {
        ERR("Error enabling %s\n", h_fb->device);
        return -7;
    }*/

    if (h_fb->buf_num > MAX_FBDEV_BUF_NUM || h_fb->buf_num < 1) {
        ERR("Error h_fb->buf_num\n");
        return -8;
    }

    /* Map the fb buffers to user space */
        Block_Req frame;
        int memsize;
        frame.lt.x = 0;
        frame.lt.y = 0;
        frame.win.cx = 320;
        frame.win.cy = 240;
//         memsize = frame.win.cx*frame.win.cy*4;
        memsize = (frame.win.cx * frame.win.cy * 4 + 0xfff) & 0xfffff000;
        ioctl(fd, IOCTL_BAR_REQUEST, &frame);
        ioctl(fd, IOCTL_BLK_SHOW, &frame);

	h_fb->buffer[0] = (char *)mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, h_fb->fd, 0);

	if (h_fb->buffer[0] == MAP_FAILED) {
		ERR("Failed mmap on %s\n", h_fb->device);
		return -9;
	}

	for (i = 0; i < h_fb->buf_num - 1; i++) {
		h_fb->buffer[i + 1] = h_fb->buffer[i] + h_fb->buf_size;
		//printf("Display buffer %d mapped to address %#lx\n", i + 1,(unsigned long)h_fb->buffer[i + 1]);
	}


    /* frame for face detect */
        Block_Req *framee = malloc(sizeof(Block_Req));//consider it never del
        framee->lt.x = 0;
        framee->lt.y = 0;
        framee->win.cx = 320;
        framee->win.cy = 240;

        ioctl(fd, IOCTL_SPR_REQUEST, framee);
	framee->viraddr = (char *)mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, h_fb->fd, 0);
	if (framee->viraddr == MAP_FAILED) {
		ERR("Failed mmap on %s\n", h_fb->device);
		return -9;
	}
	fd_face = fd;
	frame_face = framee;
    /* please don't be mad */


    return fd;
}

/* 关闭一个framebuf设备
 * h_fb 	fbdev_handle
 */
void fbdev_close(fbdev_handle h_fb)
{
    if(h_fb->buffer[0] != NULL)    
        munmap(h_fb->buffer[0], h_fb->buf_size * h_fb->buf_num);
    if(h_fb->fd)
        close(h_fb->fd);
}

/* 清除一个framebuf设备,主要用来让OSD0全黑 
 * h_fb 	fbdev_handle
 */
int fbdev_clear(fbdev_handle h_fb)
{
    if (h_fb->fd < 0) 
        return -1;

    if (h_fb->buf_index < h_fb->buf_num - 1) {
        if (h_fb->buffer[h_fb->buf_index]) {
            memset(h_fb->buffer[h_fb->buf_index], 0x00, h_fb->buf_size);
        }
    }
    return 0;
}

