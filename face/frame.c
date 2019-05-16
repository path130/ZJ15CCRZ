#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "dpplatform.h"
#include "dpgpio.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "utf2uni.h"
#include "ccic2uni.h"

#include "text.h"
#include "osd.h"

#define SCR_WIDTH 320
#define SCR_HEIGH 240


Block_Req *frame;
static int memsize;
static int fd = -1;

#define SQAR_SIZE 10
void frame_draw_sqar(int col, int row, int width, int height)
{
	int i;
	if (fd < 0) return;

	memset(frame->viraddr+row*SCR_WIDTH+col, 0xff, SQAR_SIZE*4);
	memset(frame->viraddr+row*SCR_WIDTH+col+width-SQAR_SIZE, 0xff, SQAR_SIZE*4);
	memset(frame->viraddr+(row+height)*SCR_WIDTH+col, 0xff, SQAR_SIZE*4);
	memset(frame->viraddr+(row+height)*SCR_WIDTH+col+width-SQAR_SIZE, 0xff, SQAR_SIZE*4);
	for (i = row; i < row+SQAR_SIZE; i++) {
		memset(frame->viraddr+i*SCR_WIDTH+col, 0xff, 4);
		memset(frame->viraddr+i*SCR_WIDTH+col+width, 0xff, 4);
	}
	for (i = row+height-SQAR_SIZE; i < row+height; i++) {
		memset(frame->viraddr+i*SCR_WIDTH+col, 0xff, 4);
		memset(frame->viraddr+i*SCR_WIDTH+col+width, 0xff, 4);
	}

	ioctl(fd, IOCTL_DEV_SHOW, NULL);

}

void _frame_draw_block(int x, int y, int w, int h, unsigned int c)
{
	if (fd < 0) return;
	int i, j;
// printf("%d %d %d %d %x\n", x, y, w, h, c);
	for (j = y; j < y+h; j++) {
		for (i = x; i < x+w; i++) {
			memcpy(frame->viraddr+320*j+i, &c, 4);
		}
	}

// 	ioctl(fd, IOCTL_DEV_SHOW, NULL);
}

void frame_draw_block(int x, int y, int w, int h, unsigned int c)
{
	int i, j;
	if (fd < 0) return;
// printf("%d %d %d %d %x\n", x, y, w, h, c);
	for (j = y; j < y+h; j++) {
		for (i = x; i < x+w; i++) {
			memcpy(frame->viraddr+320*j+i, &c, 4);
		}
	}

	ioctl(fd, IOCTL_DEV_SHOW, NULL);
}

void frame_draw_text(const char *txt, int utf8, unsigned int color, int font_w, int font_h, int x, int y)
{
    FT_Error error;
	int x_start = x + 2;          //+2为了不让文字太靠边
	int y_start;
	int index, idx_inc;
	int width, height, left, top;
	unsigned char *grays;
	unsigned short uni = 0;
unsigned int *fontaddr;
int i, j;

	if (fd < 0) return;
	if (text_font == NULL) return;
    if (txt == NULL)    return;
	_frame_draw_block(0, 220, 320, 240-220, 0xff000000);//TODO dead code

    FT_Face		face = (FT_Face) text_font->face;

	FT_Set_Pixel_Sizes(face, font_w, font_h);

	while (*txt) {
		if (utf8) {
			idx_inc = get_utf8_bytes((unsigned char*)txt);
			uni = utf8_trans_unicode(txt, idx_inc);
		}
		else {
			if (is_cn_char(txt)) { 
				idx_inc = 2;
				uni = ccic_trans_unicode(txt);
			}
			else
				uni = (unsigned short) *txt;
		}

		if ((uni == '\t') || (uni == '\r') || (uni == '\n'))
		    uni = ' ';

// #ifdef TEXT_LOAD_INDEX_ADVANCE
// 		index = text_index[uni];
// #else
        index = FT_Get_Char_Index(face, uni);
// #endif
        error = FT_Load_Glyph(face, index, 0);
        if (error) {
//             app_debug(DBG_ERROR, "Failed to load glyph for character %c\n", *txt);
            break;
        }

        if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
            error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
            if (error) {
//                 app_debug(DBG_ERROR, "Failed to render glyph for character %c\n", *txt);
                break;
            }
        }

	width  = face->glyph->bitmap.width;
        height = face->glyph->bitmap.rows;

        left   = face->glyph->bitmap_left;
        top    = face->glyph->bitmap_top;
        grays  = face->glyph->bitmap.buffer;

		x_start += left;
		y_start = y + font_h - top;
		height = SCR_HEIGH-y_start > height ? height : SCR_HEIGH-y_start;
		if ((x_start+width) > SCR_WIDTH)
			break;
//         text2canvas(color, grays, width, height, x, y);
	fontaddr = frame->viraddr + y_start*SCR_WIDTH + x_start;
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			if (*(grays+j*width+i) == 0)
				*(fontaddr+j*SCR_WIDTH+i) = 0xff000000;
			else
				*(fontaddr+j*SCR_WIDTH+i) = (*(grays+j*width+i) & color) + ((*(grays+j*width+i) << 8) & color) + ((*(grays+j*width+i) << 16) & color) + 0xff000000;
		}
	}

		//x_start += (face->glyph->advance.x >> 6) + 1;
        x_start += (face->glyph->advance.x >> 6);
	txt+=idx_inc;

	}
	ioctl(fd, IOCTL_DEV_SHOW, NULL);

}

void frame_start()
{
	frame = (Block_Req *)frame_face;
	fd = fd_face;

	ioctl(fd, IOCTL_BLK_SHOW, frame);

	memset(frame->viraddr, 320*240*4, 0);
	frame_draw_block(0, 220, 320, 240-220, 0xff000000);//information line TODO dead code
	ioctl(fd, IOCTL_DEV_SHOW, NULL);

}

void frame_stop()
{
	memset(frame->viraddr, 320*240*4, 0x00);
	ioctl(fd, IOCTL_BLK_HIDE, frame);
 	ioctl(fd, IOCTL_DEV_SHOW, NULL);
 	
	frame = NULL;
	fd = -1;

}

