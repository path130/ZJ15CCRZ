/*
 ============================================================================
 Name        : video_send.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : 视频捕获, 编码, rtp发送
 ============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/mman.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "public.h"
#include "video_send.h"
#include "dev_config.h"
#include "cpld.h"
#include "key.h"
#include "ajb_net_video.h"
#include "sensor.h"
//#include "spi.h" //del by wrm 20141121

#define AVSERVER_RESTART

#define VID_COUNT_FILE          "/tmp/vid_cnt.cfg"
#define VID_PARAM_FILE          "/tmp/vid_prm.cfg"

#define RTP_TYPE_VIDEO          1

#define VID_RTP_PACKET_MAX      1024

#define VID_UDP_PORT            6670

#define VSS_STOPED              0
#define VSS_RUNNING             1
#define VSS_STOPING             (-1)
#define VSS_STARTING            (-2)

#define RTP_CMD_NONE            0
#define RTP_CMD_ADD_DEST        1
#define RTP_CMD_DEL_DEST        2
#define RTP_CMD_SINGLE_DEST     3

#define RTP_CTLBUF_SIZE         6
#define RTP_CTLBUF_LEN          10

#define LOCK_MP4_VOL            0
#define UNLOCK_MP4_VOL 	     1
#define LOCK_MP4		     2
#define LOCK_MP4_IFRAM          3
#define UNLOCK_MP4              4
#define GET_MPEG4_SERIAL        5
#define WAIT_NEW_MPEG4_SERIAL   6

//static global_data  gbl_vsstat   = GBL_DATA_INIT;
//static pthread_t    vid_snd_tid  = 0;
#ifdef CFG_NECESSARY_ADJUST
static pthread_t    vid_mode_tid = 0;
#endif

static int   rtp_dest_cnt = 0;
//static int   frame_cnt    = 0;
static int   interface    = 0;
static int   vid_mian_out = VID_MAIN_LCD;
static unsigned int VID_RTP_PORT = 9880;
static unsigned int rtp_cmd_top = 0, rtp_cmd_bot = 0;
static unsigned int rtp_cmd_buf[RTP_CTLBUF_SIZE][RTP_CTLBUF_LEN];
static pthread_mutex_t rtpctl_mutex = PTHREAD_MUTEX_INITIALIZER;


struct stct_vid_prm {
    int photo;
    int mode;
};
static struct stct_vid_prm *pprm = MAP_FAILED;
struct stct_vid_snd_arg {/*
    struct rtp_multidest_attrs  attrs;
    int                         snd_type;
    int                         rtp_type;
*/};
static struct stct_vid_snd_arg  arg_vid_snd_t;

static int interface_init(int msg_id) 
{
	printf("%s not define\n", __func__);return 0;
//     int qid;
// 
//     if (ApproDrvInit(msg_id)) {
//         printf("ApproDrvInit error!\n");
//         ApproDrvExit();
//         return -1;
//     }
//     printf("func_get_mem in\n");
//     if (func_get_mem(&qid)) {
//         ApproDrvExit();
//         printf("func_get_mem error\n");
//         return -1;
//     }
//     printf("func_get_mem out\n");
//     interface = 1;
//     return 0;
}

static void interface_clean(void) 
{
	printf("%s not define\n", __func__);return ;
//     //ApproDrvExit();
//     if (interface) {
//         ApproInterfaceExit();
//         interface = 0;
//     }
}

void proc_signal(int signo)
{
	printf("%s not define\n", __func__);return ;
    interface_clean();
#ifdef AVSERVER_RESTART
    system("killall -2 av_server.out");
#endif
    usleep(400*1000);
    printf("proc_signal:app exit\n");
    exit(1);
}

static void vid_signal_bind(void)
{
	printf("%s not define\n", __func__);return ;
    signal(SIGINT,  &proc_signal);
    signal(SIGQUIT, &proc_signal);
    signal(SIGTSTP, &proc_signal);
    signal(SIGHUP,  &proc_signal);
    signal(SIGSEGV, &proc_signal);
    signal(SIGTERM, &proc_signal);
}

static int rtp_put_cmd(unsigned int cmd, unsigned int lip, unsigned int port)
{
	printf("%s not define\n", __func__);return 0;
    if (lip == 0)
        return -1;

    pthread_mutex_lock(&rtpctl_mutex);
    rtp_cmd_buf[rtp_cmd_top][0] = cmd;
    rtp_cmd_buf[rtp_cmd_top][1] = lip;
    rtp_cmd_buf[rtp_cmd_top][2] = port;
    if (++rtp_cmd_top >= RTP_CTLBUF_SIZE)
        rtp_cmd_top = 0;
    pthread_mutex_unlock(&rtpctl_mutex);

    return 0;
}

static int rtp_get_cmd(unsigned int *cmd, unsigned int *lip, unsigned int *port)
{
	printf("%s not define\n", __func__);return 0;
    int ret = -1;

    pthread_mutex_lock(&rtpctl_mutex);
    if(rtp_cmd_top != rtp_cmd_bot) {
        *cmd  = rtp_cmd_buf[rtp_cmd_bot][0];
        *lip  = rtp_cmd_buf[rtp_cmd_bot][1];
        *port = rtp_cmd_buf[rtp_cmd_bot][2];
        if (++rtp_cmd_bot >= RTP_CTLBUF_SIZE)
            rtp_cmd_bot = 0;
        ret = 0;
    }
    pthread_mutex_unlock(&rtpctl_mutex);

    return ret;
}

static int rtp_proc_cmd(void)
{
	printf("%s not define\n", __func__);return 0;
    unsigned int cmd, lip, port;

    if (rtp_get_cmd(&cmd, &lip, &port))
        return -1;

    if (lip == 0)
        return -1;

    if(debug_level >= DBG_INFO) 
    {
        char *actstr[]={"add", "del", "single"};
        struct in_addr ia;
        char  ias[32];
        ia.s_addr = lip;
        strcpy(ias, inet_ntoa(ia));
        if ((cmd >= 1) && (cmd <= 3))
            printf("video %s rtp dest:%s \n", actstr[cmd-1], ias);
    }

    switch(cmd) {
        case RTP_CMD_ADD_DEST:
            if (++rtp_dest_cnt > RTP_MAXDEST) 
                return -1;
            //rtp_add_dest(RTP_TYPE_VIDEO, lip, port);
            break;
        case RTP_CMD_DEL_DEST:
         //   rtp_del_dest(RTP_TYPE_VIDEO, lip, port);
            if (rtp_dest_cnt > 0) 
                rtp_dest_cnt--;
            break;
        case RTP_CMD_SINGLE_DEST:
      //      rtp_single_dest(RTP_TYPE_VIDEO, lip, port);
            rtp_dest_cnt = 1;
            break;
        default:
            break;
    }

    return 0;
}

static void rtp_reset_cmd(void)
{
	printf("%s not define\n", __func__);return ;
    pthread_mutex_lock(&rtpctl_mutex);
    rtp_cmd_top  = 0;
    rtp_cmd_bot  = 0;
    pthread_mutex_unlock(&rtpctl_mutex);
}

static unsigned int cap_main_out = (-1);

void video_main_mode_set(int mode)
{printf("%s not define\n", __func__);return ;
    if (dev_cfg.cvbs_mode == 1) {
        vid_mian_out = VID_MAIN_CVBS;
    } else {
        vid_mian_out = (mode == VID_MAIN_CVBS) ? VID_MAIN_CVBS : VID_MAIN_LCD;
    }
}

void video_main_mode_adjust(void)
{
	printf("%s not define\n", __func__);return ;
    if ((vid_mian_out == VID_MAIN_CVBS) && (cap_main_out != VID_MAIN_CVBS)) {
//         SetBrightness(165);
//         SetSaturation(100);
        cap_main_out = VID_MAIN_CVBS;
        printf("cap_main_out : VID_MAIN_CVBS \n");
    } else if ((vid_mian_out == VID_MAIN_LCD) && (cap_main_out != VID_MAIN_LCD)) {
//         SetBrightness(128);
//         SetSaturation(128);
        cap_main_out = VID_MAIN_LCD;
        printf("cap_main_out : VID_MAIN_LCD \n");
    }
}

static void video_main_mode_reset(void)
{
	printf("%s not define\n", __func__);return ;
    cap_main_out = (-1);
    video_main_mode_set(VID_MAIN_LCD);
}

int video_del_rtpdest(unsigned long lip[])
{
	printf("%s not define\n", __func__);return 0;
    int dest_idx;
    for (dest_idx = 0; dest_idx < RTP_MAXDEST; dest_idx++) {
        rtp_put_cmd(RTP_CMD_DEL_DEST, lip[dest_idx], VID_RTP_PORT);
    }
    return 0;
}

int video_del_rtpdest_one(unsigned long lip)
{
	printf("%s not define\n", __func__);return 0;
    return rtp_put_cmd(RTP_CMD_DEL_DEST, lip, VID_RTP_PORT);
}

int video_add_rtpdest_one(unsigned long lip)
{
	printf("%s not define\n", __func__);return 0;
    return rtp_put_cmd(RTP_CMD_ADD_DEST, lip, VID_RTP_PORT);
}

int video_single_rtpdest(unsigned long lip)
{printf("%s not define\n", __func__);return 0;
    return rtp_put_cmd(RTP_CMD_SINGLE_DEST, lip, VID_RTP_PORT);
}


static int vid_snd_by_tcp(char *data, int len, int sock)
{printf("%s not define\n", __func__);return 0;
    if (send(sock, data, len, 0) < 0) {
        perror("sock_tcp_client send data error!");
        close(sock);
        return -1;
    }
    return 0;
}

static int vid_snd_by_udp(char *data, int len, int sock, struct sockaddr_in *p_sockaddr)
{printf("%s not define\n", __func__);return 0;
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

    if(len > 48*1024 || len <= 1024) return -1;
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
        usleep(2100);
        tmpcha = *(data + (i+1)*1024);
        *(data + (i+1)*1024) = i+1;
        ret = sendto(sock, data + i*1024, 1024+1, 0, (struct sockaddr *)p_sockaddr, sizeof(struct sockaddr_in));
        *(data + (i+1)*1024) = tmpcha;
    }
    usleep(2100);
    *(data + len) = i+1;
    sendto(sock, data + i*1024, len-i*1024+1, 0, (struct sockaddr *)p_sockaddr, sizeof(struct sockaddr_in));   
    return 0;
}

static inline void vid_snd_by_rtp(char *data, int len, int mark)
{
	printf("%s not define\n", __func__);return ;
    //printf("vid_snd_by_rtp encdata_bytes:%d\n", encdata_bytes);
    //rtp_send_packet(RTP_TYPE_VIDEO, data, len, 98, mark, 3600);
}

//is_bigframe含义:若大于30KB则发完此包需延时一会儿，因为分机端处理能力不够，会导致马赛克
static inline void vid_snd_by_rtp_ex(char *data, int len, int mark, int is_bigframe)
{   printf("%s not define\n", __func__);return ;
	//app_debug(DBG_INFO, "vid_snd_by_rtp %02X %02X %02X %02X %02X %02X %02X %02X encdata_bytes:%d\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], len); 
/*	rtp_send_packet(RTP_TYPE_VIDEO, data, len, 98, mark, 3600);
	if(1 == is_bigframe){   		
		usleep(20);//ruan_20150529_视频图像有马赛克问题     		
		//app_debug(DBG_INFO, "vid_snd_by_rtp %02X %02X %02X %02X %02X %02X %02X %02X encdata_bytes:%d\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], len);   		
		}*/
}
void *vid_mode_thread(void *arg)
{
	printf("%s not define\n", __func__);return NULL ;

#if 0
    void *status   = THREAD_SUCCESS;
    //cpld_ircut_sw(IR_CUT_DAY);
    cpld_io_clr(EIO_CAMERA_LIGHT);

    if ((pprm == MAP_FAILED) || (pprm == NULL))
        return THREAD_FAILURE;
    pprm->photo = VID_MODE_DAY;

#ifdef CFG_NECESSARY_ADJUST
    int ircut_stat = IR_CUT_UNKNOWN;
#else
    if (dev_cfg.camera_night) {
        if (cpld_io_voltage(EIO_PHOTOSENSITIVE)) {
            pprm->photo = VID_MODE_DAY;
            cpld_io_clr(EIO_CAMERA_LIGHT);
            cpld_ircut_sw(IR_CUT_DAY);
        } else {
            pprm->photo = VID_MODE_NIGHT;
            cpld_io_set(EIO_CAMERA_LIGHT);
            cpld_ircut_sw(IR_CUT_NIGHT);
        }
        #if 0
        if (pprm->mode == VID_MODE_DAY) {
            cpld_ircut_sw(IR_CUT_DAY);
        } else {
            cpld_ircut_sw(IR_CUT_NIGHT);
        }
        #endif
    }
#endif

    while (VSS_STOPING != gbl_data_get(&gbl_vsstat)) {
#ifdef CFG_NECESSARY_ADJUST
        if (dev_cfg.camera_night) {
            if (!cpld_io_voltage(EIO_PHOTOSENSITIVE)) {
                pprm->photo = VID_MODE_NIGHT;
                cpld_io_set(EIO_CAMERA_LIGHT);
            } else {
                pprm->photo = VID_MODE_DAY;
                cpld_io_clr(EIO_CAMERA_LIGHT);
            }

            if (pprm->mode == VID_MODE_DAY) {
                if (ircut_stat != VID_MODE_DAY) {
                    cpld_ircut_sw(IR_CUT_DAY);
                    ircut_stat  = IR_CUT_DAY;
                }
            } else {
                if (ircut_stat != IR_CUT_NIGHT) {
                    cpld_ircut_sw(IR_CUT_NIGHT);
                    ircut_stat  = IR_CUT_NIGHT;
                }
            }
        }
#endif
        usleep(250*1000);
    }
    cpld_io_clr(EIO_CAMERA_LIGHT);
    return status;
#endif
}


void *vid_snd_h264_thread(void *arg)
{
#if 0
	printf("%s not define\n", __func__);return ;
    void *status = THREAD_SUCCESS;

    int vid_tcp_sock = -1;
    struct stct_vid_snd_arg s_arg;
    struct stct_vid_snd_arg *p_arg = &s_arg;

    memcpy(p_arg, arg, sizeof(struct stct_vid_snd_arg));

//     AV_DATA        vol_data;
    int vType      = 0;
    int SerialBook = -1;
    int SerialLock = -1;
    int ret        = 0;
//     AV_DATA        av_data;

//     static int mpeg4_field[7][2] = 
//     { 
//         { AV_OP_LOCK_MP4_VOL, AV_OP_LOCK_MP4_CIF_VOL }, 
//         { AV_OP_UNLOCK_MP4_VOL,	AV_OP_UNLOCK_MP4_CIF_VOL }, 
//         { AV_OP_LOCK_MP4, AV_OP_LOCK_MP4_CIF },
//         { AV_OP_LOCK_MP4_IFRAME, AV_OP_LOCK_MP4_CIF_IFRAME },
//         { AV_OP_UNLOCK_MP4, AV_OP_UNLOCK_MP4_CIF }, 
//         { AV_OP_GET_MPEG4_SERIAL, AV_OP_GET_MPEG4_CIF_SERIAL },
//         { AV_OP_WAIT_NEW_MPEG4_SERIAL, AV_OP_WAIT_NEW_MPEG4_CIF_SERIAL } 
//     };

    if (p_arg->snd_type == VID_SND_RTP) {
        printf("rtp_create_multidest: dest_port %d \n", p_arg->attrs.dest_port);
        rtp_create_multidest(RTP_TYPE_VIDEO, &p_arg->attrs); 
    } else if (p_arg->snd_type == VID_SND_TCP) {
        vid_tcp_sock = (int)(p_arg->attrs.lip[0]);
    } else {
        return THREAD_FAILURE;
    }

    frame_cnt = 0;

//     if (RET_SUCCESS != GetAVData(mpeg4_field[LOCK_MP4_VOL][vType], -1, &vol_data)) {
//         printf("Error on Get Vol data\n");
//         goto RECORD_QUIT;
//     }

    int encdata_bytes;
    char *encdata_pointer;
    int is_bigframe = 0;
    if(vid_mian_out == VID_MAIN_CVBS)
		is_bigframe = 1;
    if (p_arg->snd_type == VID_SND_RTP) {
        if (p_arg->rtp_type == VID_RTP_PACK_ONE) {
            //vid_snd_by_rtp(encdata_pointer, encdata_bytes, 0);
            vid_snd_by_rtp_ex(encdata_pointer, encdata_bytes, 0,0);
        } else {
            if(encdata_bytes > 20*1024)
		is_bigframe = 1;
            while(encdata_bytes > VID_RTP_PACKET_MAX) {
	/*	if(encdata_bytes > 30*1024)		
                	vid_snd_by_rtp_ex(encdata_pointer, VID_RTP_PACKET_MAX, 0,1);
		else
			 vid_snd_by_rtp_ex(encdata_pointer, VID_RTP_PACKET_MAX, 0,0);*/
			 
		vid_snd_by_rtp_ex(encdata_pointer, VID_RTP_PACKET_MAX, 0,is_bigframe);
                encdata_bytes   -= VID_RTP_PACKET_MAX;
                encdata_pointer += VID_RTP_PACKET_MAX;
            }
            if (encdata_bytes > 0) {
                //vid_snd_by_rtp(encdata_pointer, encdata_bytes, 1);
                 /*vid_snd_by_rtp_ex(encdata_pointer, encdata_bytes, 1 ,0);*/
                 vid_snd_by_rtp_ex(encdata_pointer, encdata_bytes, 1 ,is_bigframe);
            }
        }
        rtp_proc_cmd();
    } else if (p_arg->snd_type == VID_SND_TCP) {
        vid_snd_by_tcp(encdata_pointer, encdata_bytes, vid_tcp_sock);
    } 

//     printf("av_data unlock vol data\n");
//     GetAVData(mpeg4_field[UNLOCK_MP4_VOL][vType], -1, &vol_data);

//     do {
//         GetAVData(mpeg4_field[GET_MPEG4_SERIAL][vType], -1, &av_data);
//     } while ((av_data.flags != AV_FLAGS_MP4_I_FRAME) && (VSS_STOPING != gbl_data_get(&gbl_vsstat)));

//     SerialBook = av_data.serial;

    printf("get SerialBook %d\n",SerialBook);
    int cnt = 20;

    while (VSS_STOPING != gbl_data_get(&gbl_vsstat)) {
	is_bigframe = 0;
//         ret = GetAVData(mpeg4_field[LOCK_MP4][vType], SerialBook, &av_data);

        if (ret/* == RET_SUCCESS*/) {
//             encdata_bytes   = av_data.size;
//             encdata_pointer = (char *)av_data.ptr;
            //printf("encdata_bytes:%d\n", encdata_bytes);
            if(vid_mian_out == VID_MAIN_CVBS)
			   	is_bigframe = 1;
            if (p_arg->snd_type == VID_SND_RTP) {
                if (p_arg->rtp_type == VID_RTP_PACK_ONE) {
                    //vid_snd_by_rtp(encdata_pointer, encdata_bytes, 0);
                    vid_snd_by_rtp_ex(encdata_pointer, encdata_bytes, 0,0); 
                } else {
	            if(encdata_bytes > 30*1024)
			is_bigframe = 1;
	            while(encdata_bytes > VID_RTP_PACKET_MAX) {
		/*	if(encdata_bytes > 30*1024)		
	                	vid_snd_by_rtp_ex(encdata_pointer, VID_RTP_PACKET_MAX, 0,1);
			else
				 vid_snd_by_rtp_ex(encdata_pointer, VID_RTP_PACKET_MAX, 0,0);*/
				 
			vid_snd_by_rtp_ex(encdata_pointer, VID_RTP_PACKET_MAX, 0,is_bigframe);
	                encdata_bytes   -= VID_RTP_PACKET_MAX;
	                encdata_pointer += VID_RTP_PACKET_MAX;
	            }
	            if (encdata_bytes > 0) {
	                //vid_snd_by_rtp(encdata_pointer, encdata_bytes, 1);
	                 /*vid_snd_by_rtp_ex(encdata_pointer, encdata_bytes, 1 ,0);*/
	                 vid_snd_by_rtp_ex(encdata_pointer, encdata_bytes, 1 ,is_bigframe);
	            }

                }
                rtp_proc_cmd();
                video_main_mode_adjust();
            } else if (p_arg->snd_type == VID_SND_TCP) {
                vid_snd_by_tcp(encdata_pointer, encdata_bytes, vid_tcp_sock);
            }

            if (SerialLock >= 0) {
//                 GetAVData(mpeg4_field[UNLOCK_MP4][vType], SerialLock, &av_data);
            }

            SerialLock = SerialBook;
            cnt++;
            SerialBook++;
        } /*else if (ret == RET_NO_VALID_DATA) {
            // wait new frame
            usleep(1000);
        } else {
            GetAVData(mpeg4_field[GET_MPEG4_SERIAL][vType], -1, &av_data);
            SerialBook = av_data.serial;
        }*/
    }

//     GetAVData(mpeg4_field[UNLOCK_MP4][vType], SerialLock, &av_data);

RECORD_QUIT:

    if (p_arg->snd_type == VID_SND_RTP) {
        rtp_delete(RTP_TYPE_VIDEO);
    }
    else if (p_arg->snd_type == VID_SND_TCP) {
        close(vid_tcp_sock);
        vid_tcp_sock = -1;
    }

    return status;
#else
    return NULL;
#endif
}

void *vid_snd_jpeg_thread(void *arg)
{
#if 0
	printf("%s not define\n", __func__);return ;
    void *status = THREAD_SUCCESS;
    
    struct sockaddr_in  vid_udp_sockaddr;
    
    struct rtp_multidest_attrs s_arg;
    struct rtp_multidest_attrs *p_arg = &s_arg;

    memcpy(p_arg, arg, sizeof(struct rtp_attrs));

//     AV_DATA        av_data;

    int  SerialLock = 0;
    int  ret        = 0;

    int   vid_udp_sock = -1;
    int   encdata_bytes;
    char *encdata_pointer;

    if ((vid_udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket() error\n");
        exit(EXIT_FAILURE);
    }

    bzero((void *)(&vid_udp_sockaddr), sizeof(vid_udp_sockaddr));
    vid_udp_sockaddr.sin_family      = AF_INET;
    vid_udp_sockaddr.sin_port        = htons(VID_UDP_PORT);
    vid_udp_sockaddr.sin_addr.s_addr = p_arg->lip[0];

    frame_cnt = 0;

    while (VSS_STOPING != gbl_data_get(&gbl_vsstat)) {
//         av_data.serial = -1;
//         GetAVData(AV_OP_GET_MJPEG_SERIAL, -1, &av_data);

//         if (av_data.serial <= SerialLock)
//             continue;
        video_main_mode_adjust();
//         if( (int)av_data.serial < 0 ) {
//             printf("Stream %d is not avaliable~~~~~~~~\n", 2);
//             status = THREAD_FAILURE;
//             goto RECORD_QUIT;
//         }

//         ret = GetAVData(AV_OP_LOCK_MJPEG, av_data.serial, &av_data );

        if (ret/* == RET_SUCCESS*/) {
//             encdata_bytes   = av_data.size;
//             encdata_pointer = (char *)av_data.ptr;
//             if (av_data.serial & 0x01)
                vid_snd_by_udp(encdata_pointer, encdata_bytes, vid_udp_sock, &vid_udp_sockaddr);
        }
//         SerialLock = av_data.serial;
        //printf("AV_OP_UNLOCK_MJPEG SerialLock = %d\n", SerialLock);
//         GetAVData(AV_OP_UNLOCK_MJPEG, SerialLock, NULL); 
    }
	
RECORD_QUIT:
    close(vid_udp_sock);
    vid_udp_sock = -1;

    return status;
#else
    return NULL;
#endif
}

int video_day_night_set(void)
{
#if 0
	printf("%s not define\n", __func__);return ;
#ifndef CFG_NECESSARY_ADJUST
/*
   printf("dev_cfg.camera_night=%d,day_night_set %d\n",dev_cfg.camera_night,cpld_io_voltage(EIO_PHOTOSENSITIVE));
    if ((dev_cfg.camera_night) && (cpld_io_voltage(EIO_PHOTOSENSITIVE))) { //modified by wrm 20141121 !cpld_io_voltage(EIO_PHOTOSENSITIVE)
        pprm->photo = VID_MODE_NIGHT;
        cpld_io_set(EIO_CAMERA_LIGHT);
        cpld_ircut_sw(IR_CUT_NIGHT);
    } else {
        pprm->photo = VID_MODE_DAY;
        cpld_io_clr(EIO_CAMERA_LIGHT);
        cpld_ircut_sw(IR_CUT_DAY);
    }
    */
     printf("dev_cfg.camera_night=%d,day_night_set %d\n",dev_cfg.camera_night,cpld_io_voltage(EIO_PHOTOSENSITIVE));
      if (dev_cfg.camera_night) {  
        if (cpld_io_voltage(EIO_PHOTOSENSITIVE)) {
            //pprm->photo = VID_MODE_NIGHT;
            cpld_io_set(EIO_CAMERA_LIGHT);
            //cpld_ircut_sw(IR_CUT_NIGHT);           
        } else {
        //pprm->photo = VID_MODE_DAY;
        cpld_io_clr(EIO_CAMERA_LIGHT);
        //cpld_ircut_sw(IR_CUT_DAY);
        }
    } else {
        //pprm->photo = VID_MODE_DAY;
        cpld_io_clr(EIO_CAMERA_LIGHT);
        //cpld_ircut_sw(IR_CUT_DAY);
    }
#endif
#else
        if (get_photo_sence()) {
            usleep(10000);
            if(get_photo_sence()){
                Set_CCD_Light(1);//cpld_io_set(EIO_CAMERA_LIGHT); 
                return 1;
            }
        } 
        else {
            usleep(10000);
            if(!get_photo_sence()){        
                Set_CCD_Light(0);
                return 0;
            }
        }
#endif
}

void video_day_night_clr(void)
{
#if 0
    printf("%s not define\n", __func__);return ;
#else    
    Set_CCD_Light(0);
    //ccd_led_ctrl(0); //cpld_io_clr(EIO_CAMERA_LIGHT);
#endif
}

int video_send_init(void)
{
#if 0
	printf("%s not define\n", __func__);return 0;
    vid_signal_bind();
    pthread_mutex_init(&gbl_vsstat.mutex, NULL);
    pthread_mutex_init(&rtpctl_mutex,     NULL);
    gbl_data_set(&gbl_vsstat, VSS_STOPED);

    int fd_vid_prm = open(VID_PARAM_FILE, O_RDWR, 0x664);
    if (fd_vid_prm < 0) {
        fd_vid_prm = open(VID_PARAM_FILE, O_RDWR|O_TRUNC|O_CREAT, 0x664);
        if (fd_vid_prm < 0) {
            app_debug(DBG_FATAL, "Cannot opn config file:%s for write\n", VID_PARAM_FILE);
            return -1;
        } else {
            struct stct_vid_prm vid_prm;
            vid_prm.photo = VID_MODE_DAY;
            vid_prm.mode  = VID_MODE_DAY;
            write(fd_vid_prm, &vid_prm, sizeof(vid_prm));
        }
    }

    pprm = (struct stct_vid_prm *)mmap(NULL, sizeof(struct stct_vid_prm), PROT_READ|PROT_WRITE, MAP_SHARED, fd_vid_prm, 0); 

    if (pprm == MAP_FAILED) {
        perror("mmap");
    }
    close(fd_vid_prm);
    video_day_night_clr();
    video_main_mode_reset();
    return 0;
#else
    return 0;
#endif
}

int video_send_start(unsigned long lip[], enum vid_type vid_t, enum vid_res vid_r)
{
	int cnt=0;
	dest_ip_t dest[3];
	char is_night=0;

	memset(dest,0,sizeof(dest_ip_t)*3);
	dest[0].dest_port=dest[1].dest_port=dest[2].dest_port=9880;
	if(lip[1]!=0)
	{
		dest[cnt].dest_ip=lip[1];
		cnt++;
	}

	if(lip[2]!=0)
	{
		dest[cnt].dest_ip=lip[2];
		cnt++;
	}

	if(lip[3]!=0)
	{
		dest[cnt].dest_ip=lip[3];
		cnt++;
	}
#if 1
    struct in_addr ia[4];
    char  ias[4][32];
    ia[0].s_addr = lip[0];
    ia[1].s_addr = lip[1];
    ia[2].s_addr = lip[2];
    ia[3].s_addr = lip[3];

    strcpy(ias[0], inet_ntoa(ia[0]));
    strcpy(ias[1], inet_ntoa(ia[1]));
    strcpy(ias[2], inet_ntoa(ia[2]));
    strcpy(ias[3], inet_ntoa(ia[3]));

    printf("vid ip:%s %s %s %s vid_t :%d, vid_r:%d\n",\
           ias[0], ias[1], ias[2], ias[3], vid_t, vid_r);
#endif	
          
	ajb_net_video_service_attr_t ajb_net_video_service_attr;
	ajb_net_video_service_attr.net_video_mode=VIDEO_SEND;
	ajb_net_video_service_attr.net_video_type=VIDEO_H264;
	ajb_net_video_service_attr.trans_protocol=RTP_NET;
	ajb_net_video_service_attr.ajb_video_attr.vid_t=ENC_H264;
	ajb_net_video_service_attr.ajb_video_attr.vid_x=0;
	ajb_net_video_service_attr.ajb_video_attr.vid_y=0;
	ajb_net_video_service_attr.ajb_video_attr.vid_w=640;//544;
	ajb_net_video_service_attr.ajb_video_attr.vid_h=480;//368;	
	ajb_net_video_service_attr.ajb_video_trans_attrs=default_ajb_video_trans_attrs;	

    if (vid_t == VID_MOBILE) {
        //ajb_net_video_service_attr.net_video_type=VIDEO_H264; //arg_vid_snd_t.snd_type = VID_SND_TCP;
        vid_t  = VID_H264;
        printf("not supported, vid_t is %d\n",vid_t);
        return -1;
    } else if (vid_t == VID_JPEG) {
		ajb_net_video_service_attr.net_video_type=VIDEO_JPEG;    
		ajb_net_video_service_attr.trans_protocol=UDP_NET;
		ajb_net_video_service_attr.ajb_video_attr.vid_t=ENC_JPEG;
		ajb_net_video_service_attr.ajb_video_trans_attrs.dest_port = 6670;
		dest[0].dest_port=dest[1].dest_port=dest[2].dest_port=6670;          
    } else if (vid_t == VID_SERVER) {
        vid_t  = VID_H264;
        ajb_net_video_service_attr.net_video_type=VIDEO_H264; //arg_vid_snd_t.snd_type = VID_SND_RTP;
        ajb_net_video_service_attr.ajb_video_trans_attrs.dest_port = 6670;
    }
	else {
        ajb_net_video_service_attr.net_video_type=VIDEO_H264;
        if (vid_r == VID_DF2100) {
            ajb_net_video_service_attr.ajb_video_trans_attrs.dest_port =5000;
        } else {
            ajb_net_video_service_attr.ajb_video_trans_attrs.dest_port =9880;
        }
    }	
   
    switch(vid_r) {
        case VID_PAL:
        case VID_DF2100:
            ajb_net_video_service_attr.ajb_video_attr.vid_w = VID_PAL_W;
            ajb_net_video_service_attr.ajb_video_attr.vid_h = VID_PAL_H;
            break;
        case VID_VGA:
            ajb_net_video_service_attr.ajb_video_attr.vid_w = VID_VGA_W;
            ajb_net_video_service_attr.ajb_video_attr.vid_h = VID_VGA_H;          
            break;
        case VID_QVGA:
            ajb_net_video_service_attr.ajb_video_attr.vid_w = VID_QVGA_W;
            ajb_net_video_service_attr.ajb_video_attr.vid_h = VID_QVGA_H;
            break;
        case VID_CIF:
            ajb_net_video_service_attr.ajb_video_attr.vid_w = VID_CIF_W;
            ajb_net_video_service_attr.ajb_video_attr.vid_h = VID_CIF_H;
            break;
        case VID_CIF5:
            ajb_net_video_service_attr.ajb_video_attr.vid_w = VID_CIF_W;
            ajb_net_video_service_attr.ajb_video_attr.vid_h = VID_CIF_H;            
            break;
        case VID_WSVGA:
            ajb_net_video_service_attr.ajb_video_attr.vid_w = VID_WSVGA_W;
            ajb_net_video_service_attr.ajb_video_attr.vid_h = VID_WSVGA_H;
            break;
        default:
            printf("res not supported ,vid_r is %d\n",vid_r);
            return -1;
    }	

    printf(">>>vid_w= %d,vid_h= %d <<<\n",ajb_net_video_service_attr.ajb_video_attr.vid_w,ajb_net_video_service_attr.ajb_video_attr.vid_h);

	struct in_addr in;
	in.s_addr=lip[0];

	ajb_net_video_service_attr.ajb_video_trans_attrs.dest_ip=inet_ntoa(in);


	if(new_ajb_net_video_service(ajb_net_video_service_attr)<0)
	{
		app_debug(DBG_FATAL, "new_ajb_net_video_service failed,return  error\n");
		return -1;
	}
    if(cnt > 0){
    	if(ajb_net_video_service_add_destination(dest,cnt)<0){
    		app_debug(DBG_FATAL, "ajb_net_video_service_add_destination failed,return  error,cnt=%d\n",cnt);
    	}
	}
	is_night=video_day_night_set();
    if(vid_r == VID_PAL){	
        ae_rule_algo_init(is_night,1,1);
    }
    else{
        ae_rule_algo_init(is_night,0,1);
    }
	return 	 start_ajb_net_video_service();
    
#if 0
    rtp_reset_cmd();
    rtp_dest_cnt = 0;
    for (idx = 0; idx < RTP_MAXDEST; idx++) {
        if (lip[idx] != 0) rtp_dest_cnt++;
    }
    if (rtp_dest_cnt == 0) return -1;

    gbl_data_set(&gbl_vsstat, VSS_STARTING);
#ifdef AVSERVER_RESTART
    //system("killall -2 av_server.out");
    switch (vid_r) {
        case VID_VGA:
			if(vid_t == VID_H264_926)
				system("./av_server.out DM365 PAL TI2A AEWB VGA H264 2000000 VBR AUTO MAXBR 3000000 BP MJPEG 75 AUTO MENUOFF &");
			else
            	system("./av_server.out DM365 PAL TI2A AEWB VGA H264 2000000 VBR AUTO MAXBR 3000000 MJPEG 75 AUTO MENUOFF &");
            break;
        case VID_PAL:
        case VID_DF2100:
            system("./av_server.out DM365 PAL TI2A AEWB PAL_D1 H264 1200000 VBR AUTO MAXBR 2000000 AUTO MJPEG 75 AUTO MENUOFF &");
            //system("./av_server.out DM365 PAL TI2A AEWB PAL_D1 H264 2000000 VBR MAXBR 3000000 AUTO MJPEG 75 AUTO MENUOFF &");
            break;
        case VID_720P:
            system("./av_server.out DM365 PAL TI2A AEWB 720P H264 3376000 VBR AUTO MAXBR 6000000 MJPEG 75 AUTO MENUOFF &");
            break;
        case VID_CIF:
            system("./av_server.out DM365 PAL TI2A AEWB CIF H264 500000 VBR AUTO MAXBR 3000000 MJPEG 75 AUTO MENUOFF &");
            break;
	 case VID_CIF5:
            system("./av_server.out DM365 PAL TI2A AEWB CIF5 H264 500000 VBR AUTO MJPEG 75 AUTO MENUOFF &");
            break;
        default:
            system("./av_server.out DM365 PAL TI2A AEWB  WSVGA H264 3000000 VBR  AUTO MAXBR 4500000 MJPEG 75 AUTO MENUOFF &");
            break;
    }

//     stream_key = (vid_t == VID_JPEG)? STREAM1_OK_KEY : STREAM0_OK_KEY;
    stream_cnt = 80;
    while(stream_cnt--) {
   // while (1) {
        usleep(100*1000);
        if (msgget(stream_key, 0) >= 0) break;
    }
    printf("Streaming start time: %dms\n", (80-stream_cnt)*100);
    if (stream_cnt <= 0) 
        goto avserver_timout;
#endif
    stream_cnt = 10;
    while(stream_cnt--) {
        if (interface_init(3) >= 0) break;
        usleep(100*1000);
    }
    printf("Interface start time: %dms\n", (9-stream_cnt)*100);
    if (stream_cnt <= 0) 
        goto avserver_clean;

    memcpy(&arg_vid_snd_t.attrs,  &rtp_video_multidest_attrs, sizeof(struct rtp_multidest_attrs));
    memcpy(arg_vid_snd_t.attrs.lip, lip, RTP_MAXDEST*sizeof(unsigned long));

    if (vid_t == VID_MOBILE) {
        arg_vid_snd_t.snd_type = VID_SND_TCP;
        vid_t  = VID_H264;
    } else if (vid_t == VID_JPEG) {
        arg_vid_snd_t.snd_type = VID_SND_UDP;
    } else if (vid_t == VID_SERVER) {
        vid_t  = VID_H264;
        arg_vid_snd_t.snd_type = VID_SND_RTP;
        arg_vid_snd_t.rtp_type = VID_RTP_PACK_ONE;
        arg_vid_snd_t.attrs.dest_port = VID_RTP_PORT = 6670;
    }
	else {
        arg_vid_snd_t.snd_type = VID_SND_RTP;
        if (vid_r == VID_DF2100) {
            arg_vid_snd_t.rtp_type = VID_RTP_PACK_ONE;
            VID_RTP_PORT           = 5000;
        } else {
            arg_vid_snd_t.rtp_type = VID_RTP_PACK_MULTI;
            VID_RTP_PORT           = 9880;
        }
        arg_vid_snd_t.attrs.dest_port = VID_RTP_PORT;
    }
#ifdef CFG_NECESSARY_ADJUST
    pthread_create(&vid_mode_tid, NULL, vid_mode_thread, NULL);
#endif
    if (vid_t == VID_H264 ||vid_t == VID_H264_926)
        pthread_create(&vid_snd_tid, NULL, vid_snd_h264_thread, &arg_vid_snd_t);
    else 
        pthread_create(&vid_snd_tid, NULL, vid_snd_jpeg_thread, &arg_vid_snd_t);

    usleep(50000);

    gbl_data_set(&gbl_vsstat, VSS_RUNNING);

    return 0;

    printf("App start avserver timout \n");
avserver_clean:
    interface_clean();
avserver_timout:
    system("killall -2 av_server.out");
    usleep(450*1000);
    gbl_data_set(&gbl_vsstat, VSS_STOPED);
    video_main_mode_reset();
    return -1;
#else
    return 0;
#endif
}

//#include "ajb_net_video.h"

int video_send_stop(void)
{
	delete_ajb_net_video_service();
    ae_rule_algo_init(0,0,1);//针对呼叫信息存储器的情况，恢复镜头的色差值
#if 0
	printf("%s not define\n", __func__);return 0;
    switch (gbl_data_get(&gbl_vsstat)) {
        case VSS_RUNNING:
            break;
        case VSS_STOPED:
        case VSS_STOPING:
        case VSS_STARTING:
            return -1;
            break;
    }

    rtp_reset_cmd();
    gbl_data_set(&gbl_vsstat, VSS_STOPING);
#ifdef CFG_NECESSARY_ADJUST
    if (vid_mode_tid) {
        pthread_join(vid_mode_tid, NULL);
        vid_mode_tid = 0;
    }
#endif
    if (vid_snd_tid) {
        pthread_join(vid_snd_tid, NULL);
        vid_snd_tid = 0;
    }

    interface_clean();
#ifdef AVSERVER_RESTART
    system("killall -2 av_server.out");
    usleep(450*1000);
    printf("av_server.out killed finished \n");
#endif
    gbl_data_set(&gbl_vsstat, VSS_STOPED);
    video_main_mode_reset();

    pprm->photo = VID_MODE_DAY;
    cpld_io_clr(EIO_CAMERA_LIGHT);
    cpld_ircut_sw(IR_CUT_DAY);
    return 0;
#else
    return 0;
#endif
}

