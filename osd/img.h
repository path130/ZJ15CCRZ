#ifndef _IMG_H_
#define _IMG_H_

#if defined (__cplusplus)
extern "C" {
#endif

//#define CFG_NO_RESIDENT

#define IMG_FILE_ERR    (-1)    //文件错误
#define IMG_FORM_ERR    (-2)    //格式不被支持
#define IMG_SIZE_ERR    (-3)    //图像过大(超过OSD实际分辨率)
#define IMG_DATA_ERR    (-4)    //数据损坏或读取错误
#define IMG_MEMY_ERR    (-5)    //内存不足
#define IMG_OTHR_ERR    (-6)    //其他错误

enum img_type { 
	IMG_UNSUPPORTED = 0,
	IMG_BMP,
	IMG_PNG_24,
    IMG_PNG_32,
	IMG_JPEG,
};

typedef struct img_object_struct {
    enum img_type 	type;
    int             ram;
    int 			width;
    int 			height;
	const char      *fname;
	FILE 			*fp;
    void 			*rgba;
} img_object, *img_handle;

struct bmp_file_header {
    char	file_type[2];
    long	file_size;
    long	reserved;
    long	data_offset;
}__attribute__((packed));

struct bmp_info_header {
    long	size_self;
    long	width;
    long	height;
    short	planes;
    short	bit_count;
    long	compression;
    long	size_image;
    long	x_pels_per_merer;
    long	y_pels_per_merer;
    long	clr_used;
    long	Clr_important;
}__attribute__((packed));

struct bmp16_info {
    long	size;
    long	width;
    long	height;
    void 	*data;
};
extern img_object img_dummy;
extern img_handle img_create(const char *img_file, int ram);
extern int        img_get_data(img_handle h_img, int only_info);
extern int        img_fill_canvas(img_handle h_img, int xpos, int ypos);
extern int        img_fill_canvas_ex(const char *img_file, int xpos, int ypos);
extern void       img_show(img_handle h_img, int xpos, int ypos);
extern void       img_show_ex(const char *img_file, int xpos, int ypos);
extern int        img_zoom(const char *img_file, unsigned int *dst_rgb, int dst_w, int dst_h);
extern void       img_delete(img_handle h_img);

#define IMG_MAX_WIDTH   800
#define IMG_MAX_HEIGHT  480

#if defined (__cplusplus)
}
#endif

#endif


