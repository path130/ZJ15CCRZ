/*
 ============================================================================
 Name        : text.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : 文字显示
 ============================================================================
 */

#include <stdio.h>
#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "osd.h"
#include "text.h"
#include "public.h"
#include "utf2uni.h"
#include "ccic2uni.h"

#ifdef TEXT_LOAD_INDEX_ADVANCE
#define TEXT_INDEX_NUM  0xFFFF
static unsigned short text_index[TEXT_INDEX_NUM+1];
#endif

font_handle           text_font;

/*装载一个字库*/
font_handle text_font_load(const char *file)
{
	font_handle h_font;
    
	h_font = calloc(1, sizeof(font_object));

	if (h_font == NULL) {
		app_debug(DBG_ERROR, "Alloc failed: font Object\n");
		return NULL;
	}

	if (FT_Init_FreeType((FT_Library *) (void *) &h_font->library)) {
		app_debug(DBG_ERROR, "Failed to intialize freetype library\n");
		free(h_font);
		return NULL;
	}

	if (FT_New_Face((FT_Library) (void *) h_font->library, file, 0,
					(FT_Face *) (void *) &h_font->face)) {
		app_debug(DBG_ERROR, "Failed to load font %s\n", file);
		free(h_font);
		return NULL;
	}

	if (FT_Select_Charmap((FT_Face) h_font->face, FT_ENCODING_UNICODE)) {
		app_debug(DBG_ERROR, "Invalid charmap [%d]\n", FT_ENCODING_UNICODE);
		free(h_font);
		return NULL;
	}
#ifdef TEXT_LOAD_INDEX_ADVANCE
    int     i;
    FT_Face face = (FT_Face) h_font->face;

    for (i = 1; i < TEXT_INDEX_NUM; i++) {
        text_index[i] = FT_Get_Char_Index(face, i);
    }
#endif
	return h_font;
}

/*不再使用时, 释放这个字体的font_object*/
int text_font_free(font_handle h_font)
{
    if (h_font) {
        free(h_font);
    }

    return 0;
}

int text2canvas(unsigned int color, unsigned char *alpha, int width, int height, int xpos, int ypos)
{
    int  i = 0;
    int  x = 0, y = 0;
    int  x_max  = MIN((osd0_rect.width  - xpos), width);
	 int  y_max  = MIN((osd0_rect.height - ypos), height);

    unsigned char pixel_alpha;
    unsigned char pixel_src_r = ((pixel_u)color).c.r;
    unsigned char pixel_src_g = ((pixel_u)color).c.g;
    unsigned char pixel_src_b = ((pixel_u)color).c.b;
    unsigned char *a  = (unsigned char *)alpha;
    pixel_u *dst  = canvas + ypos*osd0_rect.width + xpos;
    pixel_u pixel_dst;

    for (y = 0; y < y_max; y++) {
        for (x = 0; x < x_max; x++) {
            pixel_dst   = dst[x];
            pixel_alpha = a[i];
            //dst[x].c.r = (pixel_src_r * pixel_alpha + pixel_dst.c.r * (255-pixel_alpha)) >> 8;
            //dst[x].c.g = (pixel_src_g * pixel_alpha + pixel_dst.c.g * (255-pixel_alpha)) >> 8;
            //dst[x].c.b = (pixel_src_b * pixel_alpha + pixel_dst.c.b * (255-pixel_alpha)) >> 8;

            dst[x].c.r = ((pixel_src_r - pixel_dst.c.r) * pixel_alpha + MUL255[pixel_dst.c.r]) >> 8;
            dst[x].c.g = ((pixel_src_g - pixel_dst.c.g) * pixel_alpha + MUL255[pixel_dst.c.g]) >> 8;
            dst[x].c.b = ((pixel_src_b - pixel_dst.c.b) * pixel_alpha + MUL255[pixel_dst.c.b]) >> 8;
	        i++;
        }
        dst += osd0_rect.width;
        a   += width;
        i   = 0;
    }

    return 0;
}

unsigned int text_fill_canvas(font_handle h_font, int utf8, unsigned int color, int font_w, int font_h, \
                                         int area_w, int area_h, int x, int y, const char *txt)
{
    FT_Error error;
	int x_start = x + 2;          //+2为了不让文字太靠边
	int x_area  = x + 2;
	int y_start = y + font_h;
	int index, idx_inc;
	int width, height, left, top;
    int line_gap_pixel = font_h / 5;
	unsigned char *grays;
	unsigned short uni = 0;

    int x_max = area_w == 0 ? osd0_rect.width  : MIN(area_w + x, osd0_rect.width) - 2;
    int y_max = area_h == 0 ? osd0_rect.height : MIN(area_h + y, osd0_rect.height)- 2;

	if (h_font == NULL) return -1;
    if (txt == NULL)    return -1;

    FT_Face		face = (FT_Face) h_font->face;

	FT_Set_Pixel_Sizes(face, font_w, font_h);

	while (*txt) {
		idx_inc = 1;
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

		if ((uni == 0) || (uni == '\r'))
			goto NEXT_TXT;
        else
        if (uni == '\t')
		    uni = ' ';
        else
        if (uni == '\n') {
			x_start = x_area;
			y_start += font_h + line_gap_pixel;//LINE_GAP_PIXEL;
			goto NEXT_TXT;
		}
#ifdef TEXT_LOAD_INDEX_ADVANCE
		index = text_index[uni];
#else
        index = FT_Get_Char_Index(face, uni);
#endif
        error = FT_Load_Glyph(face, index, 0);
        if (error) {
            app_debug(DBG_ERROR, "Failed to load glyph for character %c\n", *txt);
            break;
        }

        if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
            error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
            if (error) {
                app_debug(DBG_ERROR, "Failed to render glyph for character %c\n", *txt);
                break;
            }
        }

		width  = face->glyph->bitmap.width;
        height = face->glyph->bitmap.rows;

        left   = face->glyph->bitmap_left;
        top    = face->glyph->bitmap_top;
        grays  = face->glyph->bitmap.buffer;

		x = x_start + left;
		y = y_start - top;
		if (x < 0) x = 0;
		if (y < 0) y = 0;

		if (x + width > x_max) { 
            if ((y_start+font_h+line_gap_pixel) >= y_max) {
                app_debug(DBG_WARNING, "Text out of the area %dx%d\n", area_w, area_h);
                return (y_start<<16|x_start);
            }
            y_start += font_h + line_gap_pixel;//LINE_GAP_PIXEL;
			x = x_start = x_area;
			y = y_start - top;
		}

        if (y_start > y_max) {
            app_debug(DBG_WARNING, "Text out of the area %dx%d\n", area_w, area_h);
            return (y_start<<16|x_start);
        }

        text2canvas(color, grays, width, height, x, y);

		//x_start += (face->glyph->advance.x >> 6) + 1;
        x_start += (face->glyph->advance.x >> 6);
NEXT_TXT:
		txt += idx_inc;
	}

    return (y<<16|x_start);
}

/* 在OSD上输出字符,
 * h_font   使用的字体handle
 * utf8		输入字符串是否是UTF8编码
 * color	文字颜色,RGB565
 * txt      需要显示的字符串指针
 * area_w   显示区域宽度,达到后自动换行
 * area_w   显示区域高度
 * font_w	字体宽度
 * font_h   字体高度
 * x, y     起始坐标
 */
unsigned int text_show(font_handle h_font, int utf8, unsigned int color, int font_w, int font_h, int area_w, int area_h, int x, int y, const char *txt)
{
    
    if (text_fill_canvas(h_font, utf8, color, font_w, font_h, area_w, area_h, x, y, txt) < 0)
        return -1;
    area_w = area_w == 0 ? osd0_rect.width : area_w;
    area_h = area_h == 0 ? osd0_rect.height: area_h;
    osd_draw_canvas(area_w, area_h, x, y);
    return 0;
}

/* 输出字符灰度数据,
 * h_font   使用的字体handle
 * utf8     输入字符串是否是UTF8编码

 * buf		需要填充的灰度数据指针
 * txt      需要显示的字符串指针
 * area_w   显示区域宽度,达到后自动换行
 * font_w	字体宽度

 * font_h   字体高度
 * x, y     起始坐标
 */
unsigned int text_grays2buf(font_handle h_font, unsigned int utf8, unsigned char *buf, int font_w, int font_h, int area_w, int area_h, int x, int y, const char *txt)
{
    FT_Error error;
    int i = 0, j = 0, g = 0;
	int x_start = x + 2;
	int x_area  = x + 2;
	int y_start = y + font_h;
	int index, idx_inc;
	int width, height, left, top;
    int line_gap_pixel = font_h / 5;
	unsigned char *grays;
	unsigned short uni = 0;

	if (h_font == NULL) return -1;
    if (txt == NULL)    return -1;

    int x_max = area_w == 0 ? osd0_rect.width  : MIN(area_w + x, 736) - 2;
    int y_max = area_h == 0 ? osd0_rect.height : MIN(area_h + y, 480)- 2;

    FT_Face		face = (FT_Face) h_font->face;

	FT_Set_Pixel_Sizes(face, font_w, font_h);

	while (*txt) {
		idx_inc = 1;
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

		if ((uni == 0) || (uni == '\r'))
			goto NEXT_TXT;
        else
        if (uni == '\t')
		    uni = ' ';
        else
		if (uni == '\n') {
			x_start = x_area;
			y_start += font_h + line_gap_pixel;
			goto NEXT_TXT;
		}

#ifdef TEXT_LOAD_INDEX_ADVANCE
		index = text_index[uni];
#else
        index = FT_Get_Char_Index(face, uni);
#endif
        error = FT_Load_Glyph(face, index, 0);
        if (error) {
            app_debug(DBG_ERROR, "Failed to load glyph for character %c\n", *txt);
            break;
        }

        if (face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
            error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
            if (error) {
                app_debug(DBG_ERROR, "Failed to render glyph for character %c\n", *txt);
                break;
            }
        }

		width  = face->glyph->bitmap.width;
        height = face->glyph->bitmap.rows;

        left   = face->glyph->bitmap_left;
        top    = face->glyph->bitmap_top;
        grays  = face->glyph->bitmap.buffer;

		x = x_start + left;
		y = y_start - top;
		if (x < 0) x = 0;
		if (y < 0) y = 0;

 
		if (x + width > x_max) { 
            if ((y_start+font_h+line_gap_pixel) >= y_max) {
                app_debug(DBG_WARNING, "Text out of the area %dx%d\n", area_w, area_h);
                return (y_start<<16|x_start);
            }
            y_start += font_h + line_gap_pixel;//LINE_GAP_PIXEL;
			x = x_start = x_area;
			y = y_start - top;
		}

		g = 0;

		for (j = y; j < height+y; j++) {
			for (i = x; i < width+x; i ++) {
				buf[j*area_w + i] = grays[g++];
			}
		}

		//x_start += (face->glyph->advance.x >> 6) + 1;
        x_start += (face->glyph->advance.x >> 6);
NEXT_TXT:
		txt += idx_inc;
	}
    return (y<<16|x_start);
}


