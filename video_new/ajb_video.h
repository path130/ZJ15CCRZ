/*
 * ajb_video.h
 *
 *  Created on: 2016年10月17日
 *      Author: chli
 */

#ifndef NET_INC_AJB_VIDEO_H_
#define NET_INC_AJB_VIDEO_H_

#if defined (__cplusplus)
extern "C" {
#endif


#define DL_VIDEO_NAME "./ajb_video_arm.so"

enum ajb_vid_type {
        DEC_H264=1<<0,
        DEC_JPEG=1<<1,
        ENC_H264=1<<2,
        ENC_JPEG=1<<3,
};

/*采集参数结构*/
typedef struct  STCT_CAPTURE_ARG_T
{
}stct_capture_arg_t;

/*接收显示部分参数*/
typedef struct STCT_DISPLAY_ARG
{
}stct_display_arg_t;


typedef struct AJB_VIDEO_SERVICE_ATTR
{
}ajb_video_service_attr_t;

typedef struct AJB_VIDEO_ATTR
{
        enum ajb_vid_type vid_t;
        int vid_x;
        int vid_y;
        int vid_w;
        int vid_h;
}ajb_video_attr_t;

#define BUFSIZE 60*1024
#define BUFNUM 3
typedef struct dp_buf {
	char *buf;
	int len;
	int used;
} dp_buf_t;

typedef struct AJB_VIDEO_HANDLE
{
	ajb_video_attr_t attr;
	int bufsize;
	char *(*get_buf)(void*ajb_video_handle);
	void (*put_buf)(void*ajb_video_handle,void *buf);
	int photo;
	void *photo_buf;
	int photo_len;
	int play_pipe[2];
	pthread_t process_thread;
	int run;
	int video_fd;
	char *video_buf;
	dp_buf_t dp_buf[BUFNUM];
	//pthread_mutex_t lock;
	/*
 unsigned int dest_ip;
unsigned short dest_port;
enum cap_input cap_in;
enum vid_type vid_t;
enum vid_res vid_r;*/
}ajb_video_handle_t;


typedef int (*ajb_video_service_create_t)(const ajb_video_service_attr_t*attr);

typedef ajb_video_handle_t* (*ajb_create_ajb_video_handle_t)(ajb_video_attr_t*attr);

typedef void (*ajb_delete_ajb_video_handle_t)(ajb_video_handle_t*handle);

typedef int (*ajb_video_capture_t)(ajb_video_handle_t* handle,void*buf,int length);

typedef int (*ajb_video_playback_t)(ajb_video_handle_t* handle,void*buf,int length);

typedef int (*ajb_video_take_photo_t)(ajb_video_handle_t* handle,void*buf,int length);


typedef struct{
	void*dl_handle;
	ajb_video_service_create_t ajb_video_service_create_ptr;
	ajb_create_ajb_video_handle_t ajb_create_ajb_video_handle_ptr;
	ajb_delete_ajb_video_handle_t ajb_delete_ajb_video_handle_ptr;
	ajb_video_capture_t ajb_video_capture_ptr;
	ajb_video_playback_t ajb_video_playback_ptr;
	ajb_video_take_photo_t ajb_video_take_photo_ptr;
}ajb_dl_video_handle_t;


void delete_ajb_dl_video_handle(ajb_dl_video_handle_t*ajb_dl_video_handle);
ajb_dl_video_handle_t*new_ajb_dl_video_handle(char*dl_name);



/**
 *
 * 创建视频服务
 * @param [in] attr 视频服务属性信息
 * @return 成功返回0，否则返回-1
*/
int ajb_video_service_create(const ajb_video_service_attr_t*attr);


/**
 *
 * 创建ajb_video_handle
 * @param [in] attr  video属性
 * @return 成功返回创建的handle，失败返回NULL
*/
ajb_video_handle_t*ajb_create_ajb_video_handle(ajb_video_attr_t*attr);

void ajb_delete_ajb_video_handle(ajb_video_handle_t*handle);


/**
 *
 * 视频采集
 * @param [in] handle 视频句柄
 * @param [out] buf 采集视频输出
 * @param [in] length 采集视频最大长度
 * @return 成功返回实际采集字节数，失败返回-1
*/
int ajb_video_capture(ajb_video_handle_t* handle,void*buf,int length);

/**
 *
 * 视频播放
 * @param [in] handle 视频句柄
 * @param [in] buf 待播放视频数据
 * @param [in] length 视频数据长度
 * @return 成功返回实际播放字节数，失败返回-1
*/
int ajb_video_playback(ajb_video_handle_t* handle,void*buf,int length);


/**
 *
 * 拍录
 * @param [in] handle 拍录句柄
 * @param [in] length允许最大字节数
 * @param [out] buf拍录数据存储地址
 * @return 成功返回拍录数据的字节数，失败返回-1
*/
int ajb_video_take_photo(ajb_video_handle_t* handle,void*buf,int length);

int init_ajb_video_dl(char*dl_name);

#if defined (__cplusplus)
}
#endif

#endif /* NET_INC_AJB_VIDEO_H_ */
