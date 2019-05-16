/*
 * ajb_net_video.h
 *
 *  Created on: 2016年10月17日
 *      Author: chli
 */

#ifndef NET_INC_AJB_NET_VIDEO_H_
#define NET_INC_AJB_NET_VIDEO_H_
#if defined (__cplusplus)
extern "C" {
#endif
#include <pthread.h>
#include "ajb_video.h"
#include "ajb_rtpc.h"
#include "ajb_epoll_task.h"
//#include "ajb_net.h"
#include "net_com.h"


#define NET_VIDEO_BUF_SIZE (1025)

typedef enum{
	VIDEO_SEND=1<<0,     //发送视频
	VIDEO_RECV=1<<1,      //接收视频
	VIDEO_SEND_RECV=VIDEO_SEND|VIDEO_RECV,//同时在本地显示和转发到移动APP上
	VIDEO_DISP=1<<3,
}net_video_mode_t;

typedef enum{
	VIDEO_LOCAL=1<<0,        //本地显示视频
	VIDEO_MOBILE=1<<1,     //转发到移动设备APP上
	VIDEO_LOCAL_MOBILE=VIDEO_LOCAL|VIDEO_MOBILE,//同时在本地显示和转发到移动APP上
}net_video_pair_mode_t;


typedef enum{
	VIDEO_JPEG=1<<0,
	VIDEO_H264=1<<1,
}net_video_type_t;


typedef enum{
	UDP_NET,
	RTP_NET,
}trans_protocol_t;
//传输协议



typedef struct
{
	double ts_unit;                             //时间戳
	const char*  dest_ip;                 //目的IP
	unsigned short dest_port;    //目的端口
	unsigned short local_port;    //本地端口
	unsigned long ts_inc;                //时戳增量
}ajb_udp_video_attrs_t;

typedef struct
{
	double ts_unit;                             //时间戳,-1表示使用默认值
	unsigned long ts_inc;                //时戳增量，0表示使用默认值
	char *dest_ip;                                 //目标ＩＰ，接收视频是为NULL
	char*local_ip;                                //本地ＩＰ，默认为NULL
	unsigned short dest_port;    //目的端口,0表示使用默认值
	unsigned short local_port;    //本地端口,0表示使用默认值
	int  max_package_size;//拆包发送，每包的最大值，为-1时表示不拆包
}ajb_video_trans_attrs_t;


typedef struct{

	net_video_mode_t net_video_mode;   //视频工作模式
	net_video_type_t net_video_type;        //视频编码格式
	trans_protocol_t trans_protocol;          //视频传输网络协议
	ajb_video_attr_t ajb_video_attr;           //视频显示的位置大小等参数
	ajb_video_trans_attrs_t ajb_video_trans_attrs;

}ajb_net_video_service_attr_t;


//视频网络传输模块
//网络部分基于应用协议层实现
typedef struct AJB_NET_VIDEO_ATTR
{
	union{
	ajb_rtp_attrs_t *ajb_rtp_attrs;                                                                 //rtp视频参数
	ajb_udp_video_attrs_t*ajb_udp_video_attrs;                               //udp视频参数
	};
	ajb_video_attr_t*ajb_video_attr;                                                          //视频参数
	ajb_net_video_service_attr_t ajb_net_video_service_attr;
//指定协议类型，IP和端口
}ajb_net_video_attr_t;



typedef struct
{
	int run;
	socket_handle_t socket_handle_send;
	socket_handle_t socket_handle_recv;
	ajb_epoll_task_t ajb_epoll_task;
	ajb_epoll_task_handle_t*ajb_epoll_task_handle;
	struct sockaddr_in server;
	struct sockaddr_in client_dest[4];
	int client_count;
	struct sockaddr_in client_recv;
	pthread_t send_thread;
}ajb_udp_video_handle_t;



typedef struct AJB_NET_VIDEO_HANDLE
{
	ajb_dl_video_handle_t*ajb_dl_video_handle;
	ajb_video_handle_t *ajb_video_handle;
	union
	{
		ajb_rtphandle_t *rtp_handle;
		ajb_udp_video_handle_t*ajb_udp_video_handle;
	};
	net_video_mode_t net_video_mode;
	trans_protocol_t trans_protocol;
	pthread_t thread_recv;
	pthread_t thread_send;
	int run;
}ajb_net_video_handle_t;


extern ajb_video_trans_attrs_t default_ajb_video_trans_attrs;


/*
 * *********************视频服务使用方法*****************************
 *
 *                         创建视频服务                                                 启动视频传输服务                                                删除视频传输服务
 *　　　　　　new_ajb_net_video_service();------------->start_ajb_net_video_service();--------------->delete_ajb_net_video_service();
 *
 */


/*
* 创建视频传输服务
* @param [in] destip 视频发生的目标ＩＰ地址，若只接收视频则设置为NULL
*  @param [in] ajb_net_video_service_attr　　视频参数
* @return 成功返回0，失败返回-1
*/

int new_ajb_net_video_service(ajb_net_video_service_attr_t ajb_net_video_service_attr);

/*
* 启动视频传输服务
* @return 成功返回0，失败返回-1
*/
int start_ajb_net_video_service(void);

/*
* 添加视频发送的目标ＩＰ和端口
* @param [in] dest_ip 视频发送的目标ＩＰ地址和端口，RTP默认是9880，ＵＤＰ默认是6670
*  @param [in] size　　dest_ip的数量
* @return 成功返回0或者大于０，失败返回-1
*/
int ajb_net_video_service_add_destination(dest_ip_t *dest_ip,int size);

/*
* 删除视频发送的目标ＩＰ和端口
* @param [in] dest_ip 视频发送的目标ＩＰ地址和端口，RTP默认是9880，ＵＤＰ默认是6670
*  @param [in] size　　dest_ip的数量
* @return 成功返回0或者大于０，失败返回-1
*/
int ajb_net_video_service_delete_destination(dest_ip_t *dest_ip,int size);

/*
*删除视频传输服务
*/
void delete_ajb_net_video_service(void);



#if defined (__cplusplus)
}
#endif

#endif /* NET_INC_AJB_NET_VIDEO_H_ */
