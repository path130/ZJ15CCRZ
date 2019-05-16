#ifndef AJB_RTPC_H
#define AJB_RTPC_H

#define SENDER      0
#define RECEIVER    1
#define RTP_VOICE   0
#define RTP_VIDEO   1
#if defined (__cplusplus)
extern "C" {
#endif
#include <stdint.h>


#define RTP_MAX_PACKAGESIZE  (60*1024)
typedef struct rtp_attrs {
    double ts_unit;             //时间戳
    const char*  dest_ip;             //目的IP
    unsigned int  accept;       //标识
    unsigned int  dest_port;    //目的端口
    unsigned int  local_port;    //本地端口
    unsigned char payload;      //负载类型   
    unsigned long ts_inc;       //时戳增量
    int  max_package_size;//拆包发送，每包的最大值，为-1时表示不拆包
    char *eth_name;
}ajb_rtp_attrs_t;

typedef struct{
	void*handle;
	int  max_package_size;//拆包发送，每包的最大值，为-1时表示不拆包
}ajb_rtphandle_t;


typedef struct{
   uint32_t dest_ip;             //目的IP
   uint32_t  dest_port;    //目的端口
}dest_ip_t;

ajb_rtphandle_t* new_ajb_rtphandle( const struct rtp_attrs *attrs);
void delete_ajb_rtphandle(ajb_rtphandle_t*handle);



 int  rtp_set_attrs(ajb_rtphandle_t*ajb_rtphandle,const struct rtp_attrs *attrs);
 int rtp_send_packet(ajb_rtphandle_t*ajb_rtphandle, const void *buffer,unsigned int len, unsigned char pt,int mark, unsigned long ts_inc);
 int rtp_begin_data_access(ajb_rtphandle_t*ajb_rtphandle);
 int rtp_end_data_access(ajb_rtphandle_t*ajb_rtphandle);
 int  rtp_goto_first_source(ajb_rtphandle_t*ajb_rtphandle);
 int  rtp_goto_next_source(ajb_rtphandle_t*ajb_rtphandle);
 int rtp_get_packet(ajb_rtphandle_t*ajb_rtphandle, char *data, unsigned int *seq, unsigned int *len, int *mark);

 int rtp_add_destination(ajb_rtphandle_t*ajb_rtphandle,dest_ip_t *dest_ip,int size);
 int rtp_delete_destination(ajb_rtphandle_t*ajb_rtphandle,dest_ip_t *dest_ip,int size);

/*
 extern int video_del_rtpdest(unsigned long lip[]);
 extern int video_del_rtpdest_one(unsigned long lip);
 extern int video_add_rtpdest_one(unsigned long lip);
 extern int video_single_rtpdest(unsigned long lip);
*/


 void rtp_sess_poll(ajb_rtphandle_t*ajb_rtphandle);
 void rtp_wait(unsigned int second, unsigned int microsec);
 void rtp_delete(ajb_rtphandle_t*ajb_rtphandle);

#if defined (__cplusplus)
}
#endif
#endif
