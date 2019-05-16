/*
 ============================================================================
 Name        : img.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : OSD显示image
  ============================================================================
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "osd.h"
#include "img.h"
#include "./libjpeg/jpeglib.h"
#include "./libpng/png.h"

#define getdata_out(x)  \
{                       \
    status = x;         \
    goto getret;        \
}

struct user_error_mgr {
  struct  jpeg_error_mgr mgr;
  jmp_buf setjmp_buffer;
};
typedef struct user_error_mgr *usr_error_mgr;

img_object img_dummy = {
    .width  = 0,
    .height = 0,
    .rgba   = NULL,
    .type   = IMG_UNSUPPORTED,
};

img_handle h_img_dummy = &img_dummy;

static unsigned int img_data[IMG_MAX_WIDTH * IMG_MAX_HEIGHT];

void user_error_exit(j_common_ptr cinfo)
{
    usr_error_mgr usr_err = (usr_error_mgr) cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    longjmp(usr_err->setjmp_buffer, 1);
}

static inline int is_too_large(img_handle h_img)
{
	//printf("%s\tfd:0x%x\tres:%dx%d\n", h_img->fname, (unsigned int)h_img->fp, h_img->width, h_img->height);
	if(h_img->width > osd0_rect.width || h_img->height > osd0_rect.height) {
		ERR("%s is large than %dx%d\n", h_img->fname, osd0_rect.width, osd0_rect.height);
		if (h_img->fp) {
			fclose(h_img->fp);
			h_img->fp = NULL;
		}
		return 1;
	}
	return 0;
}

static int bmp_get_data(img_handle h_img, int only_info)
{
	int   	wptr, hptr;  
    int   	ptr         = 0;
	int		align_bytes = 0;	
    int     status      = 0;

	struct  bmp_file_header file_header;
    struct  bmp_info_header info_header;

	if (h_img == NULL)     return -1;
	if (h_img->fp == NULL) return -1;

	fseek(h_img->fp, sizeof(struct bmp_file_header), SEEK_SET);
	fread(&info_header, sizeof(struct bmp_info_header), 1, h_img->fp); 
	if (info_header.bit_count != 24) {
		ERR("%s is not a bmp24 file\n", h_img->fname);
		getdata_out(IMG_FORM_ERR);
	}

	h_img->width  = info_header.width;
	h_img->height = info_header.height;
	
	if (is_too_large(h_img))  {
        getdata_out(IMG_SIZE_ERR);
    }

    if (only_info) {
        getdata_out(0);
    }
	if (h_img->rgba == NULL) {
        h_img->rgba = malloc(h_img->width * h_img->height * sizeof(pixel_u));
        //h_img->rgba = img_data;     //位图数据不常驻内存
	    if (h_img->rgba == NULL) {
	        ERR("Failed to allocate memory for bmp image\n");
	        getdata_out(IMG_MEMY_ERR);
	    }
    }

    fseek(h_img->fp, 0, SEEK_SET);
    fread(&file_header, sizeof(file_header), 1, h_img->fp);
    fread(&info_header, sizeof(info_header), 1, h_img->fp); 
    fseek(h_img->fp, file_header.data_offset, SEEK_SET);

    align_bytes = (info_header.width*3) % 4;
    if (align_bytes != 0) align_bytes = 4-align_bytes;

    char rgb888_buf[(IMG_MAX_WIDTH+1) * IMG_MAX_HEIGHT * 3]; //最大内存已知，生存周期仅在函数内，使用栈内存
    int  read_bytes = info_header.width * info_header.height * 3 + info_header.height*align_bytes;
    fread(rgb888_buf, 1, read_bytes, h_img->fp);
	
    pixel_u *dst = (pixel_u *)h_img->rgba; 

    for(hptr = info_header.height - 1; hptr>=0; hptr--) {
        wptr = 0;
        dst  = (pixel_u*)h_img->rgba + hptr * info_header.width;
        while (++wptr <= info_header.width) {
            (*dst).c.b = rgb888_buf[ptr];     //bmp:b g r
            (*dst).c.g = rgb888_buf[ptr+1];
            (*dst).c.r = rgb888_buf[ptr+2];
            ptr+=3;
            dst++;
        }
		ptr+=align_bytes;
    }
getret:
    if (h_img->fp) {
		fclose(h_img->fp);
		h_img->fp = NULL;
	}
    return status;
}

int jpeg_get_data(img_handle h_img, int only_info)
{
printf("%s %d not define", __func__, __LINE__); return -1;
 	int x, pos, row_stride;
    int status = 0;
	JSAMPARRAY buf;
 	struct jpeg_decompress_struct cinfo;
	//struct jpeg_error_mgr jerr;
    struct user_error_mgr jerr;

    if (h_img == NULL)     return -1;
	if (h_img->fp == NULL) return -1;

    cinfo.err = jpeg_std_error(&jerr.mgr);
    jerr.mgr.error_exit = user_error_exit;

    if (setjmp(jerr.setjmp_buffer)) {
        h_img->width  = cinfo.output_width  = 100;
        h_img->height = cinfo.output_height = 100;
        getdata_out(IMG_DATA_ERR);
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, h_img->fp);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

	h_img->width  = cinfo.output_width;
    h_img->height = cinfo.output_height;

	if (is_too_large(h_img)) {
        getdata_out(IMG_SIZE_ERR);
	}

    if (only_info) {
        getdata_out(0);
    }

	row_stride = cinfo.output_width * cinfo.output_components;
    buf = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	if (h_img->rgba == NULL) {
        h_img->rgba = malloc(row_stride * cinfo.output_height * sizeof(pixel_u));
        if (h_img->rgba == NULL) {
	        ERR("Failed to allocate memory for jpeg image\n");
		    getdata_out(IMG_MEMY_ERR);
	    }
    }

    pixel_u *dst = (pixel_u *)h_img->rgba; 

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, (JSAMPARRAY) buf, 1);

        for (x = 0; x < cinfo.output_width; x++) {
            if (cinfo.output_components == 1) { // Grayscale
                (*(dst+x)).c.r = buf[0][x];
                (*(dst+x)).c.g = buf[0][x];
                (*(dst+x)).c.b = buf[0][x];
            }
            else {                              // RGB
                pos = x * cinfo.output_components;
                (*(dst+x)).c.r = buf[0][pos + 0];
                (*(dst+x)).c.g = buf[0][pos + 1];
                (*(dst+x)).c.b = buf[0][pos + 2];
            }
        }
        dst += cinfo.output_width;
    }

getret:
    //jpeg_finish_decompress(&cinfo); //JPEG损坏时有可能会异常exit，不要也不会泄漏内存(jpeg_destroy_decompress释放了)，干脆不要
	jpeg_destroy_decompress(&cinfo);
    if (h_img->fp) {
        fclose(h_img->fp);
        h_img->fp = NULL;
    }
    return status;
}

static int png_get_data(img_handle h_img, int only_info)
{
	int bit_depth, color_type, interlace_type;
    int x, y;
	int status = 0;
    png_bytep   	row_pointers[800]; 
    png_structp		png_ptr;
    png_infop   	info_ptr;
    png_uint_32 	width, height;

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        ERR("Failed to create png read struct\n");
        getdata_out(IMG_OTHR_ERR);
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        ERR("Failed to create png info struct\n");
        getdata_out(IMG_OTHR_ERR);
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        ERR("Failed png_jmpbuf\n");
        getdata_out(IMG_DATA_ERR);
    }

    png_init_io(png_ptr, h_img->fp);
    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
                 &interlace_type, int_p_NULL, int_p_NULL);

	h_img->width  = width;
    h_img->height = height;
//printf("png:width%d height%d\n", width, height);
	if (is_too_large(h_img)) {
		getdata_out(IMG_SIZE_ERR);
	}

    if ((color_type & PNG_COLOR_MASK_ALPHA) == 0) {
        h_img->type = IMG_PNG_24;
    }

    if (only_info) {
        getdata_out(0);
    }

	if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_packing(png_ptr);
        png_set_palette_to_rgb(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    if (bit_depth == 16)
        png_set_strip_16(png_ptr);

    if ((color_type & PNG_COLOR_MASK_ALPHA) == 0) 
        png_set_strip_alpha(png_ptr);

    png_set_packswap(png_ptr);
    
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_gray_1_2_4_to_8(png_ptr);

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) 
		png_set_tRNS_to_alpha(png_ptr);

    if (h_img->rgba == NULL) {
		h_img->rgba = malloc(width * height * sizeof(pixel_u));
        if (h_img->rgba == NULL) {
            ERR("Failed to allocate memory for png image\n");
            getdata_out(IMG_MEMY_ERR);
        }
    }

    for (y = 0; y < height; y++) {
        row_pointers[y] = png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));
    }

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, info_ptr);
    
    pixel_u *dst = h_img->rgba;
    if (color_type == PNG_COLOR_TYPE_RGBA) {
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                dst = (pixel_u *)h_img->rgba + y * width + x;
                (*dst).c.r = row_pointers[y][x * 4 + 0];
                (*dst).c.g = row_pointers[y][x * 4 + 1];
                (*dst).c.b = row_pointers[y][x * 4 + 2];
                (*dst).c.a = row_pointers[y][x * 4 + 3];
            }
        }
    }
    else {
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                dst = (pixel_u *)h_img->rgba + y * width + x;
                (*dst).c.r = row_pointers[y][x * 3 + 0];
                (*dst).c.g = row_pointers[y][x * 3 + 1];
                (*dst).c.b = row_pointers[y][x * 3 + 2];
            }
        }
    }

    for (y = 0; y < height; y++) {
        png_free(png_ptr, row_pointers[y]);
    }

getret:
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
    if (h_img->fp) {
        fclose(h_img->fp);
        h_img->fp = NULL;
    }
    return status;
}

/* 判断image类型
 * BMP:  0x42 0x4D
 * PNG:  0x89 0x50 0x4E 0x47 0x0D 0x0A 0x1A 0x0A
 * JPEG: 0XFF 0xD8
 */
static enum img_type img_get_type(img_handle h_img)
{
	unsigned char header[8] = {0x00};
	
	fread(header, sizeof(header), 1, h_img->fp);
	fseek(h_img->fp, 0, SEEK_SET);

	if ((header[0] == 0x42)	&& (header[1] == 0x4D))
		return IMG_BMP;
	else
	if ((header[0] == 0XFF)	&& (header[1] == 0xD8))
		return IMG_JPEG;
	else
	if ((header[0] == 0x89)	&& (header[1] == 0x50)) {
		if ((header[6] == 0x1A)	&& (header[7] == 0x0A))
			return IMG_PNG_32;
	}
	
	return IMG_UNSUPPORTED;
}

int img_get_data(img_handle h_img, int only_info)
{
    int ret = 0;

	if (h_img->fp == NULL) {
		FILE *fp  = fopen(h_img->fname, "r");
		if(fp == NULL) {
            ERR("Image %s open failed!\n", h_img->fname);
            return IMG_FILE_ERR;
		}
		h_img->fp = fp;
	} 

	switch (h_img->type) {
		case IMG_BMP:
			ret = bmp_get_data(h_img, only_info);
			break;
		case IMG_PNG_32:
        case IMG_PNG_24:
			ret = png_get_data(h_img, only_info);
			break;
		case IMG_JPEG:
			ret = jpeg_get_data(h_img, only_info); 
			break;
		default:
			break;
	}

	if (h_img->fp != NULL) {
		fclose(h_img->fp);
		h_img->fp  = NULL;
	}

	return ret;
}

img_handle img_create(const char *img_file, int ram)
{
	FILE	    *fp;
	img_handle 	h_img;

	h_img = calloc(1, sizeof(img_object));

	if (h_img == NULL) {
		ERR("Failed to allocate space for image object\n");
		return h_img_dummy;
	}

    if (img_file == NULL)
        return h_img_dummy;

	fp = fopen(img_file, "r");
	if (fp == NULL) {
	    ERR("Image %s open failed!\n", img_file);
	    return h_img_dummy;
	}

	h_img->fp    = fp;
	h_img->fname = img_file;
	h_img->type  = img_get_type(h_img);
    h_img->rgba  = NULL;
    h_img->ram   = (ram==0) ? 0 : 1;

    if (h_img->type == IMG_BMP)
         h_img->ram = 0;

    if (h_img->ram == 0) {
        h_img->rgba = img_data;
    }
    
    if (img_get_data(h_img, !h_img->ram) < 0) {
	    return h_img_dummy;
    }

	return h_img;
}

void img_delete(img_handle h_img)
{
    if (h_img == h_img_dummy) return;
    if (h_img->ram) {
	    if (h_img->rgba) {
            if (h_img->rgba != img_data) {
		        free(h_img->rgba);
		        h_img->rgba = NULL;
            }
	    }
    }

	if (h_img) {
		free(h_img);
		h_img = NULL;
	}
}

int img_fill_canvas(img_handle h_img, int xpos, int ypos)
{
    int ret = 0;

	if (h_img == NULL)                  return -1;
	if (h_img->type == IMG_UNSUPPORTED) return -1;

    if (h_img->ram == 0) {
	    if ((ret = img_get_data(h_img, 0)) < 0) {
		    return ret;
	    }
    }

	if (h_img->rgba == NULL) return IMG_FILE_ERR;

    int  i = 0;
    int  x, y;
    int  x_max  = MIN((osd0_rect.width  - xpos), h_img->width);
	int  y_max  = MIN((osd0_rect.height - ypos), h_img->height);

    pixel_u pixel_src;
    pixel_u pixel_dst;
    pixel_u *src = (pixel_u *)h_img->rgba;
    pixel_u *dst = canvas + ypos*osd0_rect.width + xpos;

    if(h_img->type == IMG_PNG_32) {
        for (y = 0; y < y_max; y++) {
	        for (x = 0; x < x_max; x++) {
                pixel_dst  = *(dst+x);
                pixel_src  = *(src+i);
                //dst[x].c.r = (pixel_src.c.r * pixel_src.c.a + pixel_dst.c.r * (255-pixel_src.c.a)) >> 8;
                //dst[x].c.g = (pixel_src.c.g * pixel_src.c.a + pixel_dst.c.g * (255-pixel_src.c.a)) >> 8;
                //dst[x].c.b = (pixel_src.c.b * pixel_src.c.a + pixel_dst.c.b * (255-pixel_src.c.a)) >> 8;
                dst[x].c.r = ((pixel_src.c.r - pixel_dst.c.r) * pixel_src.c.a + MUL255[pixel_dst.c.r]) >> 8;
                dst[x].c.g = ((pixel_src.c.g - pixel_dst.c.g) * pixel_src.c.a + MUL255[pixel_dst.c.g]) >> 8;
                dst[x].c.b = ((pixel_src.c.b - pixel_dst.c.b) * pixel_src.c.a + MUL255[pixel_dst.c.b]) >> 8;
		        i++;
	        }
	        dst += osd0_rect.width;
	        src += h_img->width;
	        i   = 0;
        }
    }
    else {
        for (y = 0; y < y_max; y++) {
	        memcpy(dst, src, x_max*4);
	        dst += osd0_rect.width;
	        src += h_img->width;
        }
    }

    return 0;
}

int img_fill_canvas_ex(const char *img_file, int xpos, int ypos)
{
    unsigned int rgba[IMG_MAX_WIDTH * IMG_MAX_HEIGHT];
    img_object img_obj;
    img_handle h_img = &img_obj;

	FILE *fp = fopen(img_file, "r");
	if (fp == NULL) {
	    ERR("Image %s open failed!\n", img_file);
	    return IMG_FILE_ERR;
	}

	h_img->fp    = fp;
	h_img->fname = img_file;
	h_img->type  = img_get_type(h_img); 
    h_img->rgba  = rgba;
    h_img->ram   = 0;

    return img_fill_canvas(h_img, xpos, ypos);
}

void img_show(img_handle h_img, int xpos, int ypos)
{printf("----------------------------img_show\n");
	if (img_fill_canvas(h_img, xpos, ypos) < 0)
        return;

    osd_draw_canvas(h_img->width, h_img->height, xpos, ypos);
}

void img_show_ex(const char *img_file, int xpos, int ypos)
{
    unsigned int rgba[IMG_MAX_WIDTH * IMG_MAX_HEIGHT];
	img_object img_obj;
    img_handle h_img = &img_obj;

	FILE *fp = fopen(img_file, "r");
	if (fp == NULL) {
	    ERR("Image %s open failed!\n", img_file);
	    return;
	}

	h_img->fp    = fp;
	h_img->fname = img_file;
	h_img->type  = img_get_type(h_img); 
    h_img->rgba  = rgba;
    h_img->ram   = 0;

    img_fill_canvas(h_img, xpos, ypos);

    osd_draw_canvas(h_img->width, h_img->height, xpos, ypos);
}

int img_zoom(const char *img_file, unsigned int *dst_rgb, int dst_w, int dst_h)
{
    int          ret = 0;
    unsigned int rgba[IMG_MAX_WIDTH * IMG_MAX_HEIGHT];
    img_object img_obj;
    img_handle h_img = &img_obj;

    if(dst_w > osd0_rect.width || dst_h > osd0_rect.height) return -1;

	FILE *fp = fopen(img_file, "r");
	if (fp == NULL) {
	    ERR("Image %s open failed!\n", img_file);
	    return IMG_FILE_ERR;
	}

	h_img->fp    = fp;
	h_img->fname = img_file;
	h_img->type  = img_get_type(h_img); 
    h_img->rgba  = rgba;
    h_img->ram   = 0;

    if ((ret = img_get_data(h_img, 0)) < 0) {
	    return ret;
    }

    if ((dst_w == h_img->width) && (dst_h == h_img->height)) {
        memcpy(dst_rgb, h_img->rgba, dst_w*dst_h*sizeof(int));
        return 0;
    }

    unsigned int x, y;  //src_y;
    
    unsigned int x_16 = (h_img->width<<16)/dst_w + 1;
    unsigned int y_16 = (h_img->height<<16)/dst_h + 1;
#if 0
    unsigned int src_x;
    unsigned int src_line_start, dst_line_start;
    for (y = 0; y < dst_h; y++) {
         //src_y = (h*y_16) >> 16;
         src_line_start = ((y*y_16) >> 16) * h_img->width;
         dst_line_start = y * dst_w;
         for (x = 0; x < dst_w; x++) {
            src_x = (x*x_16) >> 16;
            dst_rgb[dst_line_start+x] = *((unsigned int *)h_img->rgba + src_line_start+src_x);
        }
    }
#else
    unsigned int *p_dst_line = dst_rgb;
    unsigned int *p_src_line;
    unsigned int srcx_16 = 0;
    unsigned int srcy_16 = 0;
    for (y = 0; y < dst_h; y++) {
         p_src_line = (unsigned int *)h_img->rgba+h_img->width * (srcy_16>>16);
         srcx_16 = 0;
         for (x = 0; x < dst_w; x++) {
            p_dst_line[x] = p_src_line[srcx_16>>16];
		    srcx_16+=x_16;
        }
        srcy_16+=y_16;
        p_dst_line+=dst_w;
    }
#endif
    return 0;
}

