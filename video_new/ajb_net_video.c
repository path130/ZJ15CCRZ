/*
 * ajb_net_video.c
 *
 *  Created on: 2016年10月17日
 *      Author: chli
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "ajb_net_video.h"
#include"public.h"

/**
 *
 * 视频数据发送线程函数，不同的端口使用以端口命名的不同的程序名分开处理,protocol用RTP或UDP登录表示，port表示端口
 * @param [in] handle对讲句柄
*/
void ajb_net_video_send_process_task(ajb_net_video_handle_t*handle)
{

}




#define VID_RTP_PACKET_MAX 1024

typedef struct{
	ajb_net_video_handle_t*ajb_net_video_handle;
}ajb_net_video_thread_arg_t;





static struct rtp_attrs rtp_video_attrs = {
		1.0/90000.0, 	//时间戳
		"127.0.0.1", 	//目的IP
		1,      		//标识
		9880,    		//目的端口
		9880,    		//本地端口
		98,      		//负载类型
		3600,      		//时戳增量
		VID_RTP_PACKET_MAX,
		NULL,

};

ajb_video_trans_attrs_t default_ajb_video_trans_attrs=
{
		.ts_unit=-1,
		.ts_inc=0,
		.dest_ip=NULL,
		.local_ip=NULL,
		.dest_port=0,
		.local_port=0,
		.max_package_size=VID_RTP_PACKET_MAX,
};




static ajb_udp_video_attrs_t ajb_udp_video_attrs={
		1.0/90000.0, 	//时间戳
			"127.0.0.1", 	//目的IP
			6670,    		//目的端口
			6670,    		//本地端口
			3600,      		//时戳增量
};


/*
x=32;y=66;
#define VID_7CZD_W              544
#define VID_7CZD_H              368
*/
//static
ajb_video_attr_t ajb_video_attr={
		DEC_H264,
		32,
		66,
		544,
		368,
};
static ajb_net_video_handle_t*  ajb_net_video_handle;

ajb_net_video_handle_t*ajb_create_ajb_net_video_handle(ajb_net_video_attr_t*attr);
int ajb_delete_ajb_net_video_handle(ajb_net_video_handle_t*ajb_net_video_handle);
int ajb_net_video_stop(ajb_net_video_handle_t* handle);

static int vid_snd_by_udp(char *data, int len, int sock, struct sockaddr_in *p_sockaddr)
{
#if 1
    const unsigned char head[] = {
            0xff, 0xd8, 0xff, 0xe1, 0x00, 0x08, 0x45, 0x78, \
            0x69, 0x66, 0x00, 0x00, 0xff, 0xe2, 0x00, 0x17, 0x00};
#else
    const unsigned char head[18] = {
            0x00};
#endif
    unsigned char pack1[1100];
    unsigned char tmpcha;
    int  i = 0,   ret, len_half;

    if(len > 48*1024 || len <= 1024)
    {
    	return -1;
    }
    if ((len&0x00000001) != 0) {
        *(data + len) = 0x00;
        len++;
    }
    len_half = len/2;

    memcpy(pack1,    &len_half, 4);
    memset(pack1+4,  0x00,      28);
    memcpy(pack1+32, head,      17);
    memcpy(pack1+49, data,      975);
    *(pack1 + 1024) = 0;
    sendto(sock, pack1, 1024+1, 0, (struct sockaddr *)p_sockaddr, sizeof(struct sockaddr_in));
    data += 975;
    len  -= 975;
    for (i = 0; i < len/1024; i++) {
        usleep(2000);
        tmpcha = *(data + (i+1)*1024);
        *(data + (i+1)*1024) = i+1;
        ret = sendto(sock, data + i*1024, 1024+1, 0, (struct sockaddr *)p_sockaddr, sizeof(struct sockaddr_in));
        *(data + (i+1)*1024) = tmpcha;
    }
    usleep(2000);
    *(data + len) = i+1;
    sendto(sock, data + i*1024, len-i*1024+1, 0, (struct sockaddr *)p_sockaddr, sizeof(struct sockaddr_in));
    return 0;
}



void* ajb_net_video_send_task_udp_6670(void*arg)
{
	ajb_net_video_handle_t* ajb_net_video_handle=(ajb_net_video_handle_t*)arg;
#define MAX_JPEG_SIZE (70*1024)
	char *buf=alloca(MAX_JPEG_SIZE);
	while(ajb_net_video_handle->ajb_udp_video_handle->run)
	{
		if(ajb_net_video_handle->ajb_dl_video_handle->ajb_video_capture_ptr==NULL)
		{
			usleep(5000);
			continue;
		}

		int length=ajb_net_video_handle->ajb_dl_video_handle->ajb_video_capture_ptr(ajb_net_video_handle->ajb_video_handle,buf,MAX_JPEG_SIZE);
		int i=0;
		for(i=0;i<ajb_net_video_handle->ajb_udp_video_handle->client_count;i++)
		{
			if(ajb_net_video_handle->ajb_udp_video_handle->client_dest[i].sin_addr.s_addr!=0)
			{
				vid_snd_by_udp(buf, length,ajb_net_video_handle->ajb_udp_video_handle->socket_handle_send.socked_fd,\
					(struct sockaddr_in *)&ajb_net_video_handle->ajb_udp_video_handle->client_dest[i]);
			}
		}
		usleep(40000);
	}
}

/**
 *
 * UDP 6670端口　视频数据接收回调函数
 * @param [in] ajb_epoll_task_ptr　传入UDP接收线程的ajb_epoll_task_t
 * @param [in] ajb_epoll_task_data 传入UDP接收线程的ajb_net_video_handle_t
 *
*/

/*void *thread_jpeg_dec(void *arg)
{
	ajb_video_handle_t* handle = (ajb_video_handle_t *)arg;
	int i;
	struct dp_buf rcv_buf;
        //JPGDecStart(100, 30, 640, 480);
	JPGDecStart(handle->attr.vid_x, handle->attr.vid_y, handle->attr.vid_w, handle->attr.vid_h);
	while (1) {
		read(vid_pipe[0], &rcv_buf, sizeof(rcv_buf));
		if (rcv_buf.len < 0) break;

		int i;
		for (i = 0; i < BUFNUM; i++) {
			if (db[i].buf == rcv_buf.buf) {
				H264WriteData(rcv_buf.buf, rcv_buf.len, 0);
				if (handle->photo && handle->photo_len > rcv_buf.len) {
					printf("memcpy(handle->photo_buf, rcv_buf.buf, rcv_buf.len)\n");
					handle->photo = 0;
				}
				db[i].used = 0;
			}
		}
	}

        H264DecStop();
}*/

inline int ajb_net_recv_from(const socket_handle_t handle,void *buf, size_t len, int flags,struct sockaddr *src_addr, socklen_t *addrlen)
{
	 if((buf!=NULL)&&(len>0))
	 {
		 return recvfrom(handle.socked_fd,buf,len,flags,src_addr,addrlen);
	 }
	 return -1;
}


#define PACKET_SIZE_JPEG 1025
int ajb_net_video_recv_task_udp_6670(void*ajb_epoll_task_ptr,void*ajb_epoll_task_data)
{

	if (ajb_epoll_task_data == NULL)
	{
		return 0;
	}
	//	ajb_epoll_task_t*ajb_epoll_task = (ajb_epoll_task_t*) ajb_epoll_task_ptr;
	ajb_net_video_handle_t*ajb_net_video_handle =(ajb_net_video_handle_t*) ajb_epoll_task_data;

	static void *dec_buf = NULL;
	char rcv_buf[PACKET_SIZE_JPEG];
	static int rcv_len;
	static int frame_len = -1;
	static char seq;
	int len;
	int count=0;
	static socklen_t addrlen=sizeof(struct sockaddr);
	len=  ajb_net_recv_from(ajb_net_video_handle->ajb_udp_video_handle->socket_handle_recv,rcv_buf,
			PACKET_SIZE_JPEG, 0,(struct sockaddr *)&ajb_net_video_handle->ajb_udp_video_handle->client_recv, &addrlen);
	if (len <= 0)
	{
		goto err;

	}
	seq++;
	if (rcv_buf[len - 1] == 0)
	{
		memcpy(&frame_len, rcv_buf, 4);
		if (frame_len * 2 > ajb_net_video_handle->ajb_video_handle->bufsize)
		{
			printf("drop one frame len %d\n", frame_len * 2);
		}
		else
		{
			dec_buf = ajb_net_video_handle->ajb_video_handle->get_buf(ajb_net_video_handle->ajb_video_handle);
			if (dec_buf == NULL)
			{
				goto err;
			}
			frame_len *= 2;
			seq = 0;
			memcpy(dec_buf, rcv_buf + 49, len - 49 - 1);
			rcv_len = len - 49 - 1;
		}
	}
	else if (seq == rcv_buf[len - 1])
	{
		if (rcv_len + len - 1< ajb_net_video_handle->ajb_video_handle->bufsize)
		{
			memcpy(dec_buf + rcv_len, rcv_buf, len - 1);
			rcv_len += (len - 1);
		}
		if (rcv_len == frame_len)
		{
			ajb_net_video_handle->ajb_dl_video_handle->ajb_video_playback_ptr(ajb_net_video_handle->ajb_video_handle, dec_buf, frame_len);
			//if (take photo) save file
/*			count++;
			if (count % 50 == 0)
			{
				printf("------------------take photo\n");
				ajb_net_video_handle->ajb_dl_video_handle->ajb_video_take_photo_ptr(ajb_net_video_handle->ajb_video_handle, dec_buf,frame_len * 2);
			}*/
			rcv_len = 0;
			frame_len = -1;
			dec_buf = NULL;
		}
	}
	else
	{
		if (frame_len > 0)
		{
			frame_len = -1;
			ajb_net_video_handle->ajb_video_handle->put_buf(ajb_net_video_handle->ajb_video_handle,dec_buf);
		}
	}
	return 0;
err:
	return 0;
}



ajb_udp_video_handle_t*new_ajb_udp_video_handle(ajb_udp_video_attrs_t*ajb_udp_video_attrs)
{
	ajb_udp_video_handle_t*handle=calloc(sizeof(ajb_udp_video_handle_t),1);
	if(handle==NULL)
	{
		app_debug(DBG_FATAL,"calloc for  ajb_speech_handle failed !\n");
		return NULL;
	}
	connect_handle_attr_t connect_handle_attr;
	connect_handle_attr.domain=AF_INET;

	//if(ajb_net_video_handle->net_video_mode&VIDEO_RECV)
	{
		connect_handle_attr.type=SOCK_DGRAM|SOCK_NONBLOCK;
		connect_handle_attr.protocol=0;
		handle->socket_handle_recv=ajb_new_net_connect_handle(&connect_handle_attr);
		if(handle->socket_handle_recv.socked_fd<0)
		{
			app_debug(DBG_FATAL," ajb_new_net_connect_handle for socket_handle_recv failed !\n");
			goto err_socket_handle_recv;
		}
		bzero(&handle->server,sizeof(handle->server));
		handle->server.sin_family = AF_INET;
		handle->server.sin_port =htons(ajb_udp_video_attrs->local_port);
		handle->server.sin_addr.s_addr=htonl (INADDR_ANY);

		bzero(&handle->client_recv,sizeof(handle->server));



		if( ajb_bind(handle->socket_handle_recv,(struct sockaddr *) &handle->server,sizeof(struct sockaddr))<0)
		{
			app_debug(DBG_FATAL," ajb_bind UDP port %d failed:%s  !\n",ntohs(handle->server.sin_port),strerror(errno));
			goto bind_error;
		}
		handle->ajb_epoll_task_handle=ajb_create_ajb_epoll_task_handle(2,10);
		//handle->ajb_epoll_task_handle=ajb_create_ajb_epoll_task_handle(2);
		if(handle->ajb_epoll_task_handle==NULL)
		{
			app_debug(DBG_FATAL,"ajb_create_ajb_epoll_task_handle failed  !\n");
			goto err_ajb_epoll_task_handle;
		}
	}

	//if(ajb_net_video_handle->net_video_mode&VIDEO_SEND)
	{
		connect_handle_attr.type=SOCK_DGRAM;
		handle->socket_handle_send=ajb_new_net_connect_handle(&connect_handle_attr);
		if(handle->socket_handle_send.socked_fd<0)
		{
			app_debug(DBG_FATAL," ajb_new_net_connect_handle for socket_handle_send failed !\n");
			goto err_socket_handle_send;
		}
		bzero(&handle->client_dest,sizeof(handle->client_dest));
		bzero(&handle->client_dest[0],sizeof(handle->server));
		handle->client_dest[0].sin_family = AF_INET;
		handle->client_dest[0].sin_port =htons(ajb_udp_video_attrs->dest_port);
		handle->client_dest[0].sin_addr.s_addr=inet_addr(ajb_udp_video_attrs->dest_ip);
		handle->client_count=4;
	}

	return handle;
err_socket_handle_send:
	if(handle->socket_handle_recv.socked_fd>-1)
	{
		ajb_delete_connect_handle(handle->socket_handle_recv);
		handle->socket_handle_recv.socked_fd=-1;
	}
err_ajb_epoll_task_handle:
bind_error:
err_socket_handle_recv:
	if(handle!=NULL)
	{
		free(handle);
		handle=NULL;
	}
	return NULL;

}


void delete_ajb_udp_video_handle(ajb_udp_video_handle_t*ajb_udp_video_handle)
{
	if(ajb_udp_video_handle!=NULL)
	{
		ajb_udp_video_handle->run=0;
		if(ajb_udp_video_handle->ajb_epoll_task_handle!=NULL)
		{
			ajb_delete_ajb_epoll_task_handle(ajb_udp_video_handle->ajb_epoll_task_handle);
			ajb_udp_video_handle->ajb_epoll_task_handle=NULL;
		}

		if(ajb_udp_video_handle->send_thread)
		{
			pthread_join(ajb_udp_video_handle->send_thread,NULL);
			ajb_udp_video_handle->send_thread=0;
		}

		if(ajb_udp_video_handle->socket_handle_recv.socked_fd>-1)
		{
			ajb_delete_connect_handle(ajb_udp_video_handle->socket_handle_recv);
			ajb_udp_video_handle->socket_handle_recv.socked_fd=-1;
		}

		
		if(ajb_udp_video_handle->socket_handle_send.socked_fd>-1)
		{
			ajb_delete_connect_handle(ajb_udp_video_handle->socket_handle_send);
			ajb_udp_video_handle->socket_handle_send.socked_fd=-1;
		}
		
		
		free(ajb_udp_video_handle);
		ajb_udp_video_handle=NULL;
	}
}


int ajb_udp_video_start(ajb_net_video_handle_t* ajb_net_video_handle)
{

	if(ajb_net_video_handle==NULL)
	{
		app_debug(DBG_FATAL,"ajb_net_video_handle is NULL return failed !\n");
		return -1;
	}

	if(ajb_net_video_handle->trans_protocol!=UDP_NET)
	{
		app_debug(DBG_FATAL,"trans_protocol is not UDP_NET, return failed !\n");
		return -1;
	}

	if(ajb_net_video_handle->net_video_mode&VIDEO_RECV)
	{
		ajb_udp_video_handle_t* ajb_udp_video_handle=ajb_net_video_handle->ajb_udp_video_handle;
		if(ajb_udp_video_handle!=NULL)
		{
			ajb_udp_video_handle->ajb_epoll_task.ajb_epoll_socket.socket_fd=ajb_udp_video_handle->socket_handle_recv.socked_fd;
			ajb_udp_video_handle->ajb_epoll_task.ajb_epoll_task_callback_fun=ajb_net_video_recv_task_udp_6670;
			ajb_udp_video_handle->ajb_epoll_task.data=ajb_net_video_handle;
			ajb_udp_video_handle->ajb_epoll_task.index=1;
			ajb_add_epoll_task(ajb_udp_video_handle->ajb_epoll_task_handle,&ajb_udp_video_handle->ajb_epoll_task);
		}

		if( ajb_start_ajb_epoll_task(ajb_udp_video_handle->ajb_epoll_task_handle)<0)
		{
			app_debug(DBG_FATAL,"ajb_start_ajb_epoll_task failed !\n");
			goto err_ajb_start_ajb_epoll_task;
		}
	}

	if(ajb_net_video_handle->net_video_mode&VIDEO_SEND)
	{

		ajb_udp_video_handle_t* ajb_udp_video_handle=ajb_net_video_handle->ajb_udp_video_handle;
		if(ajb_udp_video_handle!=NULL)
		{
			ajb_udp_video_handle->run=1;
			if(pthread_create(&ajb_udp_video_handle->send_thread,NULL,ajb_net_video_send_task_udp_6670,ajb_net_video_handle)<0)
			{
				app_debug(DBG_FATAL,"pthread_create for ajb_udp_video_handle->send_thread  failed !\n");
				return -1;
			}
		}



	}

	return 0;

err_ajb_start_ajb_epoll_task:
	return -1;
}

int ajb_udp_video_handle_add_destination(ajb_udp_video_handle_t*ajb_udp_video_handle,dest_ip_t *dest_ip,int size)
{
	int i=0;
	int count=0;
	for(i=0;i<size;i++)
	{
		int j=0;
		for(j=0;j<ajb_udp_video_handle->client_count;j++)
		{
			if(ajb_udp_video_handle->client_dest[j].sin_addr.s_addr==0)
			{
				bzero(&ajb_udp_video_handle->client_dest[j],sizeof(	struct sockaddr_in));
				ajb_udp_video_handle->client_dest[j].sin_family = AF_INET;
				ajb_udp_video_handle->client_dest[j].sin_port =htons(dest_ip[i].dest_port);
				ajb_udp_video_handle->client_dest[j].sin_addr.s_addr=dest_ip[i].dest_ip;
				count++;
				break;
			}
		}
	}
	return count;
}

int ajb_udp_video_handle_delete_destination(ajb_udp_video_handle_t*ajb_udp_video_handle,dest_ip_t *dest_ip,int size)
{
	int i=0;
	for(i=0;i<size;i++)
	{
		if(ajb_udp_video_handle->client_dest[i].sin_addr.s_addr==dest_ip[i].dest_ip)
		{
			bzero(&ajb_udp_video_handle->client_dest[i],sizeof(	struct sockaddr_in));
		}
	}
	return i;
}

int new_ajb_udp_video_service(ajb_net_video_service_attr_t ajb_net_video_service_attr)
{

	if(ajb_net_video_service_attr.net_video_mode&VIDEO_SEND)
	{
		if(	ajb_net_video_service_attr.ajb_video_trans_attrs.dest_ip==NULL)
		{
			app_debug(DBG_FATAL,"destip  is NULL,return failed !\n");
			return -1;
		}
	}

	ajb_net_video_attr_t attr;
	ajb_udp_video_attrs_t tmp_ajb_udp_video_attrs;
	memcpy(&tmp_ajb_udp_video_attrs,&ajb_udp_video_attrs,sizeof(ajb_udp_video_attrs_t));
	attr.ajb_udp_video_attrs=&tmp_ajb_udp_video_attrs;
	attr.ajb_udp_video_attrs->dest_ip=ajb_net_video_service_attr.ajb_video_trans_attrs.dest_ip;
	attr.ajb_net_video_service_attr=ajb_net_video_service_attr;
	attr.ajb_video_attr=&ajb_net_video_service_attr.ajb_video_attr;

	if(ajb_net_video_service_attr.net_video_type&VIDEO_JPEG)
	{
		if(ajb_net_video_service_attr.net_video_mode&VIDEO_RECV)
		{
			attr.ajb_video_attr->vid_t=DEC_JPEG;
		}
		else if(ajb_net_video_service_attr.net_video_mode&VIDEO_SEND)
		{
			attr.ajb_video_attr->vid_t=ENC_JPEG;
		}
	}
	else if(ajb_net_video_service_attr.net_video_type&VIDEO_H264)
	{

		if(ajb_net_video_service_attr.net_video_mode&VIDEO_RECV)
		{
			attr.ajb_video_attr->vid_t=DEC_H264;
		}
		else if(ajb_net_video_service_attr.net_video_mode&VIDEO_SEND)
		{
			attr.ajb_video_attr->vid_t=ENC_H264;
		}
	}

	ajb_net_video_handle=ajb_create_ajb_net_video_handle(&attr);
	if(ajb_net_video_handle==NULL)
	{
		app_debug(DBG_FATAL,"ajb_create_ajb_net_video_handle  for ajb_net_video_handle failed !\n");
		return -1;
	}

	return 0;

}



/*int delete_ajb_udp_video_service(void)
{
	if(ajb_net_video_handle!=NULL)
	{
		ajb_net_video_stop(ajb_net_video_handle);
		ajb_delete_ajb_net_video_handle(ajb_net_video_handle);
		ajb_net_video_handle=NULL;
	}
	return 0;
}*/

int start_ajb_udp_video_service(void)
{
	return ajb_udp_video_start(ajb_net_video_handle);
}


void ajb_net_video_send_task_rtp_9880(void*arg)
{

	if(arg==NULL)
	{
		app_debug(DBG_FATAL,"arg is NULL return failed !\n");
		return ;
	}

	ajb_net_video_thread_arg_t*ajb_net_video_thread_arg=(ajb_net_video_thread_arg_t*)arg;
	ajb_net_video_handle_t*ajb_net_video_handle=ajb_net_video_thread_arg->ajb_net_video_handle;
	if(ajb_net_video_handle==NULL)
	{
		app_debug(DBG_FATAL,"ajb_net_video_thread_arg->ajb_net_video_handle is NULL return failed !\n");
		return ;
	}

	if(ajb_net_video_handle->ajb_dl_video_handle==NULL)
	{
		app_debug(DBG_FATAL,"ajb_net_video_handle->ajb_dl_video_handle is NULL return failed !\n");
		return ;
	}
	int len = 0;

#define BUF_SIZE 512*1024
	char *send_buf=alloca(BUF_SIZE);
	if(send_buf == NULL){
		fprintf(stderr, "%s(%d): Cannot alloc send_buf memory\n",  __FILE__, __LINE__);
		return ;
	}
	char *encdata_pointer=send_buf;
	while(ajb_net_video_handle->run)
	{
		//capture一帧数据后必须sleep一段时间后再capture。sleep 50*1000 us左右
		if(ajb_net_video_handle->ajb_dl_video_handle->ajb_video_capture_ptr==NULL)
		{
			usleep(5000);
			continue;
		}
		len = 	ajb_net_video_handle->ajb_dl_video_handle->ajb_video_capture_ptr(ajb_net_video_handle->ajb_video_handle, send_buf, BUF_SIZE);
		if(len>0)
		{		
            if(ajb_net_video_handle->ajb_video_handle->attr.vid_w==352 && ajb_net_video_handle->ajb_video_handle->attr.vid_h==288){
                rtp_send_packet(ajb_net_video_handle->rtp_handle,send_buf, len, 98,0, 3600);
                usleep(200000);
            }
            else{
    			encdata_pointer=send_buf;
    			if(ajb_net_video_handle->rtp_handle->max_package_size>0)
    			{
    				while(len > ajb_net_video_handle->rtp_handle->max_package_size) {
    					rtp_send_packet(ajb_net_video_handle->rtp_handle,encdata_pointer, ajb_net_video_handle->rtp_handle->max_package_size, 98,0, 3600);
    					len   -= ajb_net_video_handle->rtp_handle->max_package_size;
    					encdata_pointer += ajb_net_video_handle->rtp_handle->max_package_size;
    				}
    				if (len > 0)
    				{
    					rtp_send_packet(ajb_net_video_handle->rtp_handle,encdata_pointer, len, 98,1, 3600);
    				}
    			}
    			else
    			{
    				rtp_send_packet(ajb_net_video_handle->rtp_handle,encdata_pointer, len, 98,1, 3600);
    			}            
			    usleep(30000);
			}
		}
		/*if(len == -1)
			{
				//视频数据没准备好
			}
		 */
		//int rtp_send_packet(ajb_rtphandle_t*ajb_rtphandle, const void *buffer,unsigned int len, unsigned char pt,int mark, unsigned long ts_inc);
	}

	free(arg);

}

/**
 *
 * RTP 9880端口　视频数据接收线程函数
 * @param [in] arg　传入ＲＴＰ接收线程的ajb_net_video_thread_arg_t
*/

void ajb_net_video_recv_task_rtp_9880(void*arg)
{
	if(arg==NULL)
	{
		app_debug(DBG_FATAL,"arg is NULL return failed !\n");
		return ;
	}

	ajb_net_video_thread_arg_t*ajb_net_video_thread_arg=(ajb_net_video_thread_arg_t*)arg;
	ajb_net_video_handle_t*ajb_net_video_handle=ajb_net_video_thread_arg->ajb_net_video_handle;
	if(ajb_net_video_handle==NULL)
	{
		app_debug(DBG_FATAL,"ajb_net_video_thread_arg->ajb_net_video_handle is NULL return failed !\n");
		return ;
	}

	if(ajb_net_video_handle->ajb_dl_video_handle==NULL)
	{
		app_debug(DBG_FATAL,"ajb_net_video_handle->ajb_dl_video_handle is NULL return failed !\n");
		return ;
	}

	/*	 if(ajb_net_video_handle->ajb_video_handle==NULL)
	 {
			app_debug(DBG_FATAL,"ajb_net_video_handle->ajb_dl_video_handle is NULL return failed !\n");
			return ;
	 }*/



	// 	char *recv_buf=malloc(RTP_MAX_PACKAGESIZE);
	void *recv_buf=ajb_net_video_handle->ajb_video_handle->get_buf(ajb_net_video_handle->ajb_video_handle);
	if(recv_buf==NULL)
	{
		app_debug(DBG_FATAL,"malloc for  recv_buf failed !\n");
		return;
	}

	unsigned int SequenceNumber=0;
	unsigned int data_length=0;
	int mark=0;
	int i = 0, len = 0;
	//	, seq = 0
//	static int ct = 0;
	while(ajb_net_video_handle->run)
	{
		if(ajb_net_video_handle->rtp_handle==NULL)
		{
			usleep(5000);
			continue;
		}

		rtp_begin_data_access(ajb_net_video_handle->rtp_handle);
		if (rtp_goto_first_source(ajb_net_video_handle->rtp_handle))
		{
			do
			{
				if(rtp_get_packet(ajb_net_video_handle->rtp_handle,recv_buf+i*1024,&SequenceNumber,&data_length,&mark)>0)
				{
					/*if (seq == 0) seq = SequenceNumber;
					 if (seq != SequenceNumber) printf("---------------------------------------------seq err\n");
					 // else printf("%d\n", seq);
					 seq++;*/
					i++;
					len+=data_length;
					if (i >= (ajb_net_video_handle->ajb_video_handle->bufsize >> 10)-1)
					{
						len = 0;
						i--;
						printf("drop one packet\n");
					}

					if(mark)
					{
						if(ajb_net_video_handle->ajb_dl_video_handle->ajb_video_playback_ptr!=NULL && len)
						{
							ajb_net_video_handle->ajb_dl_video_handle->ajb_video_playback_ptr(ajb_net_video_handle->ajb_video_handle,recv_buf,len);
							//if (take photo) save file
							/*ct++;
							if (ct%50 == 0)
								ajb_net_video_handle->ajb_dl_video_handle->ajb_video_take_photo_ptr(ajb_net_video_handle->ajb_video_handle,recv_buf,len);*/
						}
						i = 0;
						len = 0;
						usleep(1000);
					}
				}
			}while(rtp_goto_next_source(ajb_net_video_handle->rtp_handle)&&(ajb_net_video_handle->run));
		}
		rtp_end_data_access(ajb_net_video_handle->rtp_handle);
		rtp_sess_poll(ajb_net_video_handle->rtp_handle);
	}
	/*	printf("recv_data_bytes =%d \n",data_bytes);
	printf("send_data_bytes=%d \n",send_data_bytes);*/
	// 	free(recv_buf);
	free(arg);
}


/**
 *
 * 查询视频工作模式
 * @param [in] ajb_net_video_handle  带查询的ajb_net_video_handle
 * @return 返回ajb_net_video_handle的　net_video_mode_t
*/

net_video_mode_t ajb_get_net_video_mode(ajb_net_video_handle_t*ajb_net_video_handle)
{
	return ajb_net_video_handle->net_video_mode;
}

/**
 *
 * 创建ajb_net_video_handle_t
 * @param [in] attr  net_video属性
 * @return 成功返回创建的handle，失败返回NULL
*/
ajb_net_video_handle_t*ajb_create_ajb_net_video_handle(ajb_net_video_attr_t*attr)
{
	ajb_net_video_handle_t *ajb_net_video_handle=(ajb_net_video_handle_t*)calloc(1,sizeof(ajb_net_video_handle_t));
	if(ajb_net_video_handle==NULL)
	{
		app_debug(DBG_FATAL,"malloc for  ajb_net_video_handle failed !\n");
		return NULL;
	}
	ajb_net_video_handle->trans_protocol=attr->ajb_net_video_service_attr.trans_protocol;
	switch(ajb_net_video_handle->trans_protocol)
	{
	case RTP_NET:
	{
		if(attr->ajb_rtp_attrs)
		{
			ajb_net_video_handle->rtp_handle=new_ajb_rtphandle(attr->ajb_rtp_attrs);
			if(ajb_net_video_handle->rtp_handle==NULL)
			{
				app_debug(DBG_FATAL,"new_ajb_rtphandle for  rtp_handle failed !\n");
				goto err_rtp_handle;
			}
		}
	}
	break;
	case UDP_NET:
	{

		ajb_net_video_handle->ajb_udp_video_handle=new_ajb_udp_video_handle(attr->ajb_udp_video_attrs);
		if(ajb_net_video_handle->ajb_udp_video_handle==NULL)
		{
			app_debug(DBG_FATAL,"new_ajb_udp_video_handle for  ajb_udp_video_handle failed !\n");
			goto err_ajb_udp_video_handle;
		}
	}
	break;
	}
	if(attr->ajb_video_attr)
	{
		ajb_net_video_handle->ajb_dl_video_handle=new_ajb_dl_video_handle(DL_VIDEO_NAME);
		if(ajb_net_video_handle->ajb_dl_video_handle==NULL)
		{
			app_debug(DBG_FATAL,"new_ajb_dl_video_handle for  local_handle failed !\n");
			goto err_new_ajb_dl_video_handle;
		}

		ajb_net_video_handle->ajb_video_handle=ajb_net_video_handle->ajb_dl_video_handle->ajb_create_ajb_video_handle_ptr(attr->ajb_video_attr);
		if(ajb_net_video_handle->ajb_video_handle==NULL)
		{
			app_debug(DBG_FATAL,"ajb_create_ajb_video_handle for  ajb_video_handle failed !\n");
			goto err_ajb_create_ajb_video_handle;
		}
		printf("####ajb_net_video_handle->ajb_video_handle=%d\n",ajb_net_video_handle->ajb_video_handle->video_fd);
	}

	ajb_net_video_handle->net_video_mode=attr->ajb_net_video_service_attr.net_video_mode;

	return ajb_net_video_handle;

err_ajb_create_ajb_video_handle:
	if(ajb_net_video_handle->ajb_dl_video_handle!=NULL)
	{
		delete_ajb_dl_video_handle(ajb_net_video_handle->ajb_dl_video_handle);
		ajb_net_video_handle->ajb_dl_video_handle=NULL;
	}
err_new_ajb_dl_video_handle:
	switch(ajb_net_video_handle->trans_protocol)
	{
	case RTP_NET:
	{
		if(ajb_net_video_handle->rtp_handle!=NULL)
		{
			delete_ajb_rtphandle(ajb_net_video_handle->rtp_handle);
			ajb_net_video_handle->rtp_handle=NULL;
		}

		break;
	}
	case UDP_NET:
	{
		if(ajb_net_video_handle->ajb_udp_video_handle!=NULL)
		{
			delete_ajb_udp_video_handle(ajb_net_video_handle->ajb_udp_video_handle);
			ajb_net_video_handle->ajb_udp_video_handle=NULL;
		}

		break;
	}
	}
err_rtp_handle:
err_ajb_udp_video_handle:
	if(ajb_net_video_handle==NULL)
	{
		free(ajb_net_video_handle);
	}
	return NULL;
}


/**
 *
 *删除建ajb_net_video_handle_t
 * @param [in] handle 句柄
 * @return 成功返回创建的0，失败返回-1
*/

int ajb_delete_ajb_net_video_handle(ajb_net_video_handle_t*ajb_net_video_handle)
{
	if(ajb_net_video_handle!=NULL)
	{

		switch(ajb_net_video_handle->trans_protocol)
		{
		case RTP_NET:
		{
			if(ajb_net_video_handle->rtp_handle!=NULL)
			{
				delete_ajb_rtphandle(ajb_net_video_handle->rtp_handle);
				ajb_net_video_handle->rtp_handle=NULL;
			}
		}
		break;
		case UDP_NET:
		{
			if(ajb_net_video_handle->ajb_udp_video_handle!=NULL)
			{
				delete_ajb_udp_video_handle(ajb_net_video_handle->ajb_udp_video_handle);
				ajb_net_video_handle->ajb_udp_video_handle=NULL;
			}
		}
		break;
		}

		if(ajb_net_video_handle->ajb_video_handle)
		{
			if(ajb_net_video_handle->ajb_dl_video_handle!=NULL)
			{
				if(ajb_net_video_handle->ajb_dl_video_handle->ajb_delete_ajb_video_handle_ptr!=NULL)
				{
					ajb_net_video_handle->ajb_dl_video_handle->ajb_delete_ajb_video_handle_ptr(ajb_net_video_handle->ajb_video_handle);
					ajb_net_video_handle->ajb_video_handle=NULL;
				}
			}
		}

		if(ajb_net_video_handle->ajb_dl_video_handle!=NULL)
		{
			delete_ajb_dl_video_handle(ajb_net_video_handle->ajb_dl_video_handle);
			ajb_net_video_handle->ajb_dl_video_handle=NULL;
		}

		free(ajb_net_video_handle);
	}
	return 0;
}

int ajb_net_video_handle_add_destination(ajb_net_video_handle_t*ajb_net_video_handle,dest_ip_t *dest_ip,int size)
{
	switch(ajb_net_video_handle->trans_protocol)
		{
		case RTP_NET:
		{
			return rtp_add_destination(ajb_net_video_handle->rtp_handle,dest_ip, size);
		}
		break;
		case UDP_NET:
		{
			return  ajb_udp_video_handle_add_destination(ajb_net_video_handle->ajb_udp_video_handle,dest_ip, size);
		}
		break;
		}
}

int ajb_net_video_handle_delete_destination(ajb_net_video_handle_t*ajb_net_video_handle,dest_ip_t *dest_ip,int size)
{
	switch(ajb_net_video_handle->trans_protocol)
		{
		case RTP_NET:
		{
			return rtp_delete_destination(ajb_net_video_handle->rtp_handle,dest_ip, size);
		}
		break;
		case UDP_NET:
		{
			return  ajb_udp_video_handle_delete_destination(ajb_net_video_handle->ajb_udp_video_handle,dest_ip, size);
		}
		break;
		}
}




/**
 *
 * 终止视频传输
 * @param [in] handle 视频传输句柄
 * @return 成功返回0，失败返回-1
*/

int ajb_net_video_stop(ajb_net_video_handle_t* handle)
{
	if(handle)
	{
		handle->run=0;
		if(handle->thread_recv)
		{
			pthread_join(handle->thread_recv,NULL);
			handle->thread_recv=0;
		}

		if(handle->thread_send)
		{
			pthread_join(handle->thread_send,NULL);
			handle->thread_send=0;
		}
	}
	return 0;
}

/**
 *
 * 设置对讲属性
 * @param [in] handle 视频传输句柄
 * @param [in] attr  net_video属性
 * @return 成功返回0，失败返回-1
*/
int ajb_net_video_set_attr(ajb_net_video_handle_t* handle,ajb_net_video_attr_t*attr)
{
	return 0;
}

int ajb_rtp_video_start(ajb_net_video_handle_t* ajb_net_video_handle)
{
	int ret=0;

	if(ajb_net_video_handle!=NULL)
	{
		if(ajb_net_video_handle->trans_protocol!=RTP_NET)
		{
			app_debug(DBG_FATAL,"trans_protocol is not RTP_NET, return failed !\n");
			return -1;
		}

		ajb_net_video_thread_arg_t*ajb_net_video_thread_arg=(ajb_net_video_thread_arg_t*)calloc(sizeof(ajb_net_video_thread_arg_t),1);
		if(ajb_net_video_thread_arg==NULL)
		{
			app_debug(DBG_FATAL,"calloc  for ajb_net_video_thread_arg  failed !\n");
			return -1;
		}
		ajb_net_video_thread_arg->ajb_net_video_handle=ajb_net_video_handle;

		if((ajb_net_video_handle->net_video_mode&VIDEO_RECV)&&(ajb_net_video_handle->thread_recv==0))
		{
			ajb_net_video_handle->run=1;
			if(pthread_create(&ajb_net_video_handle->thread_recv,NULL,(void*)&ajb_net_video_recv_task_rtp_9880,(void*)ajb_net_video_thread_arg)<0)
			{
				free(ajb_net_video_thread_arg);
				ajb_net_video_thread_arg=NULL;
				app_debug(DBG_FATAL,"pthread_create for  thread_recv failed !\n");
				ret=-1;
			}
		}

		if((ajb_net_video_handle->net_video_mode&VIDEO_SEND)&&(ajb_net_video_handle->thread_send==0))
		{
			ajb_net_video_handle->run=1;
			if(pthread_create(&ajb_net_video_handle->thread_send,NULL,(void*)&ajb_net_video_send_task_rtp_9880,(void*)ajb_net_video_thread_arg)<0)
			{
				free(ajb_net_video_thread_arg);
				ajb_net_video_thread_arg=NULL;
				app_debug(DBG_FATAL,"pthread_create for  thread_send failed !\n");
				ret=-1;
			}
		}
	}
	return ret;
}


int new_ajb_rtp_video_service(ajb_net_video_service_attr_t ajb_net_video_service_attr)
{
	if(ajb_net_video_service_attr.net_video_mode&VIDEO_SEND)
	{
		if(ajb_net_video_service_attr.ajb_video_trans_attrs.dest_ip==NULL)
		{
			app_debug(DBG_FATAL,"destip  is NULL,return failed !\n");
			return -1;
		}
	}

	ajb_net_video_attr_t attr;
	struct rtp_attrs tmp_rtp_video_attrs =rtp_video_attrs;
	attr.ajb_rtp_attrs=&tmp_rtp_video_attrs;

	attr.ajb_rtp_attrs->dest_ip=ajb_net_video_service_attr.ajb_video_trans_attrs.dest_ip;
	if(ajb_net_video_service_attr.ajb_video_trans_attrs.max_package_size>-1)
	{
		attr.ajb_rtp_attrs->max_package_size=ajb_net_video_service_attr.ajb_video_trans_attrs.max_package_size;
	}

	if(ajb_net_video_service_attr.ajb_video_trans_attrs.local_port>0)
	{
		attr.ajb_rtp_attrs->local_port=ajb_net_video_service_attr.ajb_video_trans_attrs.local_port;
	}

	if(ajb_net_video_service_attr.ajb_video_trans_attrs.dest_port>0)
	{
	attr.ajb_rtp_attrs->dest_port=ajb_net_video_service_attr.ajb_video_trans_attrs.dest_port;
	}

	if(ajb_net_video_service_attr.ajb_video_trans_attrs.ts_unit>-1)
	{
		attr.ajb_rtp_attrs->ts_unit=ajb_net_video_service_attr.ajb_video_trans_attrs.ts_unit;
	}
	if(ajb_net_video_service_attr.ajb_video_trans_attrs.ts_inc>0)
	{
		attr.ajb_rtp_attrs->ts_inc=ajb_net_video_service_attr.ajb_video_trans_attrs.ts_inc;
	}

	attr.ajb_net_video_service_attr=ajb_net_video_service_attr;
	attr.ajb_video_attr=&ajb_net_video_service_attr.ajb_video_attr;

	ajb_net_video_handle=ajb_create_ajb_net_video_handle(&attr);
	if(ajb_net_video_handle==NULL)
	{
		app_debug(DBG_FATAL,"ajb_create_ajb_net_video_handle  for ajb_net_video_handle failed !\n");
		return -1;
	}

	return 0;
}


int start_ajb_rtp_video_service(void)
{
	return ajb_rtp_video_start(ajb_net_video_handle);
}



int delete_ajb_rtp_video_service(void)
{
	if(ajb_net_video_handle!=NULL)
	{
		ajb_net_video_stop(ajb_net_video_handle);
		ajb_delete_ajb_net_video_handle(ajb_net_video_handle);
		ajb_net_video_handle=NULL;
	}
	return 0;
}


int new_ajb_net_video_service(ajb_net_video_service_attr_t ajb_net_video_service_attr)
{

	switch(ajb_net_video_service_attr.trans_protocol)
	{
	case RTP_NET:
	{
		return  new_ajb_rtp_video_service( ajb_net_video_service_attr);
	}
	break;
	case UDP_NET:
	{

		return new_ajb_udp_video_service(ajb_net_video_service_attr);
	}
	break;
	}
	return -1;
}

int ajb_net_video_service_add_destination(dest_ip_t *dest_ip,int size)
{
	if(ajb_net_video_handle!=NULL)
	{
		return  ajb_net_video_handle_add_destination(ajb_net_video_handle,dest_ip, size);
	}
	return -1;
}


int ajb_net_video_service_delete_destination(dest_ip_t *dest_ip,int size)
{
	if(ajb_net_video_handle!=NULL)
	{
		return  ajb_net_video_handle_delete_destination(ajb_net_video_handle,dest_ip, size);
	}
	return -1;
}

int start_ajb_net_video_service(void)
{

	if(ajb_net_video_handle!=NULL)
	{
		switch(ajb_net_video_handle->trans_protocol)
		{
		case UDP_NET:
		{
			return start_ajb_udp_video_service();
		}
		break;
		case RTP_NET:
		{
			return  start_ajb_rtp_video_service();
		}
		break;
		}
	}
	return -1;
}

void delete_ajb_net_video_service(void)
{
	if(ajb_net_video_handle!=NULL)
	{
		ajb_net_video_stop(ajb_net_video_handle);
		ajb_delete_ajb_net_video_handle(ajb_net_video_handle);
		ajb_net_video_handle=NULL;
	}
}

