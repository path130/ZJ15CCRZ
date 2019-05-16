/*
 ============================================================================
 Name        : 11bh.c
 Author      : gyt
 Version     :
 Copyright   :
 Description : 15AH主机IP型 
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>


#include "public.h"
#include "osd.h"

#include "ui.h"
#include "tim.h"
#include "msg.h"
//#include "spi.h"
#include "key.h"
#include "uart.h"
#include "cpld.h"
#include "audio.h"

#include "global_def.h"
#include "ajb_bus.h"
#include "cmd_def.h"
#include "net_run.h"
#include "net_init.h"
#include "net_event.h"
#include "tcp_task.h"
#include "ui_pro.h"
#include "data_com_ui.h"
#include "video_send.h"
#include "dev_config.h"

#include "wave_record.h"
#include "auto_test.h"
#include "call_event.h"
//#include "alsa_route.h"
#include "speech_def.h"

#include "sql_pro.h"
#include "info_container.h"
#include "vlan_iptable.h"
#include "sqlite_data.h"

#include "aes.h"
#include "my_debug.h"
#include "ajb_net_video.h"

////////////////////////////////////////////////////////////////add by mkq 20170916
//#include "dpdevice.h"
#include "sys/mman.h"
#include "rtc.h"
#include "json_msg.h"  //added by hgj 180529
#include "face.h"

#ifndef IOCTRL_SET_IRQ_MODE
#define IOCTRL_SET_IRQ_MODE                     0x4c434b09//add by mkq 20170906
#endif

//////////////////////////////////////////////////////////////////
#include "finger.h"//by mkq finger
#define   KEYS_ONE      0xA350C9
#define   KEYS_TWO     0x49D613E7

static signed char *leave_buf = NULL;

#define 		BLE
#define 	       QR

#ifdef		BLE
//ble data
#define   OR_DATA			0XAF
#define   WIFI_DATA			0x9F 
#define	BLTH_CELLPHONE	0x7F //蓝牙数据

#define	AES_VERSION		0X01
#define   TEA_VERSION		0X02
#define   XXTEA_VERSION		0X03
#define   DATA_END			0X01

#define   VERSION_1			0X10
#define   VERSION_2			0x20
#define   VERSION_3                 0x30

#define   VERSION_SEED           0xD0
#define   VERSION_VI                0xE0
#define   VERSION_TIME           0xF0

#define   MATCHING_OK		1
#define   MATCHING_TIMEOUT  0
#define   MATCHING_ERROR      -1

unsigned char  default_seed[16] ="guangdonganjubao";
unsigned char  seed_key[16] = {'\0'};
static unsigned char seed_key1[16]={0}; //此版本为最初始版本加入了楼栋号加密
static unsigned char seed_key2[16]={0};  //此版本为最初始版本去除了楼栋号加密
static unsigned char seed_key3[16] = {0}; //此版本是不再使用固定 default_seed  ，seed 采用动态。楼栋号也不参与
static unsigned char seed_key_updata[16] = {0}; //此版本作为更新小区号密钥使用
#pragma pack(1)
struct recv_uart1_data{
	char type;
	char village_code[4];
	char my_code[3];
	char room_nu[2];
	char dead_line[5];
	char sum;
};
#pragma pack()
struct recv_uart1_data decrypt_data;

#endif

#pragma pack(1)
typedef struct DoorCardInfo{
	uint8_t dev_no[4];		//主机设备编码
	uint8_t cardtype;		//卡类型，参见表2“卡类型定义”
	uint8_t reserved0;		//预留，填充0
	uint16_t reserved1;		//预留
	uint8_t cardnumber[8];	//卡号,最大8字节，只有4字节时[0~3]填充0
	uint8_t result;			//开锁结果，	0：成功，1：黑名单，2：过期，3：本栋楼未授权，4：冻结
	uint8_t time[6];			//刷卡时间，[0~6]依次：年、月、日、时、分、秒
	uint8_t reserved2;		//预留
}door_card_info_t;
/**add for photo upload**/
typedef struct {
	uint8_t dev_no[4];	//主机设备编码
	uint8_t result;		//验证结果
	uint8_t household_no[4];	//住户编码
	uint8_t type;		//类型
}check_result_t;
/**end**/
#pragma pack()

#define MSG_CPLD_DAEMON         0x01
#define MSG_WATCH_TIMEOUT       0x02
#define MSG_LVMSG_TIMEOUT       0x03
#define MSG_WAIT_ACK_TIMEOUT    0x04
#define MSG_WAIT_MSG_TIMEOUT    0x05
#define MSG_DEST_ONLINE         0x06

#define MSG_MOBILE_WATCH        0x0A

#define DEST_OFFLINE            0x01
#define DEST_ONLINE             0x02
#define DEST_BUSY               0x03

unsigned char need_opencheck=0;  //by mkq 20170907

#ifdef CALL_3G

static int          tim_try_call_server; //add by wrm 20150306 for cloud 
static int           dst_server = 0; //1 :WAIT_ACK to call server 2:Calling indoor to call server

#define 			MSG_TRY_CALL_SERVER     0x07
static int           dest_state = DEST_OFFLINE;
static int           cloud_server_state = DEST_OFFLINE;

static int           mobile_hold_on=0;

#endif

#if 0

typedef enum {
    PROC_NORMAL,
    PROC_WAIT_ACK,
    PROC_CALLING,
    PROC_TALKING,
    PROC_LEAVE_MSG,
} PROC_STATE;

typedef enum {
    WATCH_START,
    WATCH_STOP,
    WATCH_BREAK,
    WATCH_TIMEOUT,
} WATCH_ACTION;

#else

typedef enum {
    PROC_NORMAL,
    PROC_WAIT_ACK,
    PROC_CALLING,
    PROC_TALKING,
    PROC_LEAVE_MSG,
} PROC_STATE;

typedef enum {
    WATCH_TRY,
    WATCH_START,
    WATCH_STOP,
    WATCH_BREAK,
    WATCH_STOPALL,
    WATCH_TIMEOUT,
    WATCH_MOBILE_START,
} WATCH_ACTION;
#endif

static pipe_handle  proc_pipe_proc;

static PROC_STATE   proc_state = PROC_NORMAL;
static ip_tongh_t   ipmulti[TONG_HAOIP_MAX];

static unsigned char watch_host[4],watch_expect[4];
static unsigned int  dst_lip    = 0;
static int           dst_type   = TYPE_JDM365;
static int           dst_vres   = VIDEO_640;

static int          tim_proc_timeout;
static int          tim_sec2_timeout;
static int          tim_ssic_timeout;
static int          tim_wactch_timeout;
static int          tim_wait_tc_pw_timeout;

static global_data  gbl_sic_subunit = GBL_DATA_INIT;
static global_data  gbl_wait_ssic   = GBL_DATA_INIT;
static global_data  gbl_watching    = GBL_DATA_INIT;
global_data  gbl_cloud_show   = GBL_DATA_INIT;
static global_data  gbl_mb_watching    = GBL_DATA_INIT;

/**add for photo upload**/
static int QR_code[7]={0},g_ble_buf[16]={0};
/**end**/


static int is_this_dev(unsigned char *addr)
{
    int i;
    for (i = 0; i < 4; i++) {
        if (addr[i] != dev_cfg.my_code[i])
            return 0;
    }
    return 1;
}

static int is_this_floor(unsigned char *addr)
{
    int i;
    for (i = 0; i < 2; i++) {
        if (addr[i] != dev_cfg.my_code[i])
            return 0;
    }
    return 1;
}

static enum vid_res video_cap_send_mode_set(enum vid_res vid_r)
{
    enum vid_res ret = vid_r;
    printf("is_call_kzq:%d dst_type:%d vid_r:%d \n", is_call_kzq, dst_type, vid_r);
    if (proc_state == PROC_CALLING) {
        if (is_call_kzq) {
            video_main_mode_set(VID_MAIN_CVBS);
        } else {
            if (dst_type == TYPE_JDM365) {
                if (is_have_infostore() && (get_code_from_container_data((char*)&target[2]) > 0)) {
                    ret = VID_PAL;
                    video_main_mode_set(VID_MAIN_CVBS);
                } else if (IS_ADMIN(target)  && (dst_vres&VIDEO_720)) {
                    video_main_mode_set(VID_MAIN_CVBS);
                } 
                /* else if (dst_vres & 0x80000000) {
                    video_main_mode_set(VID_MAIN_CVBS);
                } */
            }
        }
    } else {
        if (dst_type == TYPE_JDM365) {
            if (is_have_infostore() && (get_code_from_container_data((char*)&watch_host[2]) > 0)) {
                ret = VID_PAL;
                video_main_mode_set(VID_MAIN_CVBS);
            } else if (IS_ADMIN(watch_host)  && (dst_vres&VIDEO_720)) {
                video_main_mode_set(VID_MAIN_CVBS);
            } else if (dst_vres & 0x80000000) {
                video_main_mode_set(VID_MAIN_CVBS);
            } 
        }
    }
    return ret;
}

#if 0
static int video_cap_send_to_server_start(unsigned long lip0)
{

	ajb_net_video_service_attr_t ajb_net_video_service_attr;
	ajb_net_video_service_attr.net_video_mode=VIDEO_SEND;
	ajb_net_video_service_attr.net_video_type=VIDEO_H264;
	ajb_net_video_service_attr.trans_protocol=RTP_NET;
	ajb_net_video_service_attr.ajb_video_attr.vid_t=ENC_H264;
	ajb_net_video_service_attr.ajb_video_attr.vid_x=0;
	ajb_net_video_service_attr.ajb_video_attr.vid_y=0;
	ajb_net_video_service_attr.ajb_video_attr.vid_w=VID_CIF_W;
	ajb_net_video_service_attr.ajb_video_attr.vid_h=VID_CIF_H;
	ajb_net_video_service_attr.ajb_video_trans_attrs=default_ajb_video_trans_attrs;	
	ajb_net_video_service_attr.ajb_video_trans_attrs.dest_port = 6670;
	
    printf(">>>vid_w= %d,vid_h= %d,dest_port=%d <<<\n",ajb_net_video_service_attr.ajb_video_attr.vid_w,ajb_net_video_service_attr.ajb_video_attr.vid_h,ajb_net_video_service_attr.ajb_video_trans_attrs.dest_port);
	struct in_addr in;
	in.s_addr=lip0;

	ajb_net_video_service_attr.ajb_video_trans_attrs.dest_ip=inet_ntoa(in);
	
	if(new_ajb_net_video_service(ajb_net_video_service_attr)<0)
	{
		app_debug(DBG_FATAL, "new_ajb_net_video_service failed,return  error\n");
		return -1;
	}

	return 	 start_ajb_net_video_service();
}
#endif

static int video_cap_send_start(unsigned long lip0, unsigned long lip1, unsigned long lip2, unsigned long lip3)
{
    enum vid_type vid_t;
    enum vid_res  vid_r;
    unsigned long dst_ip[4];

    switch(dst_type) {
        case TYPE_JDM365:
            vid_t = VID_H264;
            if (dst_vres & VIDEO_720) {
                vid_r = VID_PAL;
            } else if (dst_vres & VIDEO_1024) {
                vid_r = VID_WSVGA;
            } else if (dst_vres & VIDEO_720P) {
                vid_r = VID_720P;
            } else {
                vid_r = VID_VGA;
            }
            break;
        case TYPE_D2100:
            vid_t = VID_H264;
            vid_r = VID_DF2100;
            break;
       case TYPE_J926:
            vid_t = VID_H264_926;
            if (dst_vres & VIDEO_720)
                vid_r = VID_PAL;
            else
                vid_r = VID_VGA;
            break;	
        default:
            vid_t = VID_JPEG;
            vid_r = VID_QVGA;
            break;
    }

    dst_ip[0] = lip0;
    dst_ip[1] = lip1;
    dst_ip[2] = lip2;
    dst_ip[3] = lip3;
    printf("-------video type is %d-----------\n ",vid_t);
    return video_send_start(dst_ip, vid_t, vid_r);
}

static void net_send_to_node(unsigned char dest[], int net_cmd, unsigned char data[], int len)
{
    proc_msg msg_send;

    msg_send.msg_type = MSG_FROM_PROC;
    ajb_glj_msg_build(&msg_send.msg, net_cmd, data[0], dest, &data[1], len, MEM_GLJ);    
    send_data_to_net((unsigned char*)&msg_send, MSG_LEN_NET);
#ifdef APP_DEBUG
    unsigned char *c_data = (unsigned char *)data;
    int i;
    printf("net_send_to_node: cmd %03X, dest %02X%02X%02X%02X data ",  net_cmd, dest[0], dest[1], dest[2], dest[3]);
    for (i = 0; i < len; i++) {
        printf("%02X ", c_data[i]);
    }
    printf("\n");
#endif
}
/**
 *
 *发送TCP数据到18020端口
 * @param [in] net_cmd 网络命令字
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度,不能大于10
 * @return 成功返回0，失败返回-1
*/

int send_unlock_data_to_server(unsigned int net_cmd,void *buf,int len)
{
	proc_msg msg_send;
	net_data_msg msg;
	memcpy(msg.buf,buf,len);
	msg.net_cmd=net_cmd;
	msg.flag_addr=SERVER_DATA;
	msg.data_len=len;
	msg_send.msg_type=MSG_FROM_PROC;
	memcpy(&msg_send.msg,&msg,sizeof(net_data_msg));
	if(send_data_to_net(&msg_send, MSG_LEN_NET)<0){
		app_debug(DBG_INFO, "send_data_to_pc cmd:0x%x error\n",net_cmd);
		return -1;
	}
	return 0;
}
static void net_respond_node(int link_num, int net_cmd, int allow, unsigned int oper_type,int resstatus)
{
    proc_msg_ret msg_ack;
    
    msg_ack.msg_type  = MSG_FROM_PROC; 
    msg_ack.msg.num   = link_num;

    ajb_msg_ret_build((ajb_msg_ret *)&msg_ack.msg, net_cmd, 0x88, allow, oper_type, resstatus, MEM_DATA);
    if (net_cmd == 0x705)
        msg_ack.msg.data.dev_type = dst_type;
    send_data_to_net((unsigned char*)&msg_ack, MSG_LEN_NET); 
#ifdef APP_DEBUG
    printf("net_respond_node: link_num %d, allow %d oper_type %X resstatus %X\n", msg_ack.msg.num, allow, oper_type, resstatus);
#endif
}

static void net_retdata_node(int link_num, unsigned int net_cmd, void *data, int len)
{
#define NODE_DATA_RET PC_DATA_RET
    proc_msg msg_ack;
    net_data_msg msg;

    msg_ack.msg_type = MSG_FROM_PROC; 
    msg.flag_addr    = NODE_DATA_RET;
    msg.net_cmd      = net_cmd;
    msg.data_len     = len;
    msg.num          = link_num;

    memcpy(msg.buf,data, len);
    memcpy(&msg_ack.msg, &msg, sizeof(net_data_msg));

    send_data_to_net((unsigned char*)&msg_ack, MSG_LEN_NET); 
#ifdef APP_DEBUG
    unsigned char *c_data = (unsigned char *)data;
    int i;
    printf("net_retdata_node: cmd %03X, data ", net_cmd);
    for (i = 0; i < len;i++) {
        printf("%02X ", c_data[i]);
    }
    printf("\n");
#endif
}

#if 0
static void net_respond_node(int link_num, int net_cmd, int allow)
{
    proc_msg msg_ack;
    
    msg_ack.msg_type  = MSG_FROM_PROC; 
    msg_ack.msg.num   = link_num;

    ajb_msg_ret_build((ajb_msg_ret *)&msg_ack.msg, net_cmd, 0x88, allow, 0, 0, MEM_DATA);
    send_data_to_net((unsigned char*)&msg_ack, MSG_LEN_NET); 
#ifdef APP_DEBUG
    printf("net_respond_node: link_num %d, allow %d\n", msg_ack.msg.num, allow);
#endif
}
#endif

static void net_query_to_node(unsigned char dest[], int net_cmd, unsigned int oper_type, int resstatus)
{
    proc_msg msg_send;

    msg_send.msg_type = MSG_FROM_PROC;

    ajb_msg_build_whole(&msg_send.msg, net_cmd, 0x00, dest, dev_cfg.my_code, oper_type, resstatus, MEM_DATA);
    send_data_to_net((unsigned char*)&msg_send, MSG_LEN_NET);
#ifdef APP_DEBUG
    printf("net_query_to_node: dest %02X%02X%02X%02X, netcmd:%03X oper_type %X resstatus %X\n", dest[0], dest[1], dest[2], dest[3], net_cmd, oper_type, resstatus);
#endif
}

static void net_query_to_node_cmd(unsigned char dest[], int net_cmd, unsigned char cmd, unsigned int oper_type, int resstatus)
{
    proc_msg msg_send;

    msg_send.msg_type = MSG_FROM_PROC;

    ajb_msg_build_whole(&msg_send.msg, net_cmd, cmd, dest, dev_cfg.my_code, oper_type, resstatus, MEM_DATA);
    send_data_to_net((unsigned char*)&msg_send, MSG_LEN_NET);
#ifdef APP_DEBUG
    printf("net_query_to_node_cmd: dest %02X%02X%02X%02X, netcmd:%03X, cmd:%2x, oper_type %X resstatus %X\n", dest[0], dest[1], dest[2], dest[3], net_cmd, cmd, oper_type, resstatus);
#endif
}

 void net_send_to_server(unsigned int net_cmd, void *data, int len) //by mkq 20170907
{
    if (len > 10)
        send_addr_to_server(net_cmd, data, len);
    else
        send_data_to_server(net_cmd, data, len);
#ifdef APP_DEBUG
    unsigned char *c_data = (unsigned char *)data;
    int i;
    printf("net_send_to_pc:   cmd %03X, data ", net_cmd);
    for (i = 0; i < len;i++) {
        printf("%02X ", c_data[i]);
    }
    printf("\n");
#endif
}

static void net_respond_server(int link_num, unsigned int net_cmd, void *data, int len)
{
    if (len > 10)
        respond_addr_to_server(net_cmd, data, len, link_num);
    else
        respond_data_to_server(net_cmd, data, len, link_num);
        
#ifdef APP_DEBUG
    unsigned char *c_data = (unsigned char *)data;
    int i;
    printf("net_respond_pc:   cmd %03X, data ", net_cmd);
    for (i = 0; i < len;i++) {
        printf("%02X ", c_data[i]);
    }
    printf("\n");
#endif
}

#define ALARM_MC        0x20    //门磁报警
#define ALARM_FC        0x50    //防拆报警
static void net_send_alarm(int alarm_type)
{
    unsigned char dest[4];
    unsigned char data[10];
    unsigned char i;
    
    memset(dest, 0x00, 4);
    memset(data, 0x00, 10);

    data[0] = SPCL;
    data[1] = dev_cfg.my_code[0];
    data[2] = dev_cfg.my_code[1];
    data[7] = alarm_type;

    if (alarm_type == ALARM_MC) 
        data[4] = dev_cfg.my_code[3];
    else
    if (alarm_type == ALARM_FC)
        data[4] = data[6] = dev_cfg.my_code[3]; //new admin

    for (i = 0; i < 9; i++)
        data[9] += data[i];
    //向管理处报警

    net_send_to_node(dest, GDATC, data, 10);
 
    //向PC报警
    if (alarm_type == ALARM_MC)
        net_send_to_server(ZALRC, dev_cfg.my_code, 4);
    else
    if (alarm_type == ALARM_FC)
        net_send_to_server(FCBJC, dev_cfg.my_code, 4);
}

static void net_send_call_records(unsigned char dest[], unsigned char type)
{
    unsigned char data[10];
    
    memcpy(data,     dev_cfg.my_code, 4);
    memcpy(&data[4], dest, 4);
    memcpy(&data[8], &type, 1);
    net_send_to_server(0x916, data, 9);
}


//static void net_send_unlock_records(unsigned char dest[], unsigned char cardtype,uint8_t cardnumber[8],uint8_t result,uint8_t time[6])
static void net_send_unlock_records(unsigned char dest[], unsigned char cardtype)
{
    unsigned char data[27]={0};  


    data[0]=0x01;   
    memcpy(&data[1],     dev_cfg.my_code, 2);    
    memcpy(&data[3],     dev_cfg.my_code, 4);
    //memcpy(&data[11],     dev_cfg.my_code, 2);
    printf("--------dest:%02X %02X %02X %02X \n",dest[0],dest[1],dest[2],dest[3]);
    memcpy(&data[11], dest, 4);    

    memcpy(&data[7], &cardtype, 1); 


    time_t now;
    struct tm *tnow;
    time(&now);
    tnow = localtime(&now);      

    data[17+3] = 1900+tnow->tm_year - 2000;   
    data[18+3] = tnow->tm_mon+1;     
    data[19+3] = tnow->tm_mday;     
    data[20+3] = tnow->tm_hour;     
    data[21+3] = tnow->tm_min;      
    data[22+3] = tnow->tm_sec;    
    net_send_to_server(0x764, data, 27);
}

void send_face_unlock_to_server(uint64_t id)
{
    door_card_info_t door_card_info={};
    char *buf=NULL;    
    door_card_info.cardtype='F';
    buf=(char *)malloc(sizeof(door_card_info_t)+3);
    buf[0]=1;
    memcpy(buf+1,dev_cfg.my_code,2);
   	door_card_info.cardnumber[0]=id&0xff;
   	door_card_info.cardnumber[1]=(id>>8)&0xff;
   	door_card_info.cardnumber[2]=(id>>16)&0xff;
   	door_card_info.cardnumber[3]=(id>>24)&0xff;
   	door_card_info.cardnumber[4]=(id>>32)&0xff;
   	door_card_info.reserved0 = 1;
	time_t now;
    struct tm *tnow;
    time(&now);
    tnow = localtime(&now);      
   
  	door_card_info.time[0] = 1900+tnow->tm_year - 2000;   
  	door_card_info.time[1] = tnow->tm_mon+1;     
  	door_card_info.time[2] = tnow->tm_mday;     
  	door_card_info.time[3] = tnow->tm_hour;     
    door_card_info.time[4] = tnow->tm_min;      
    door_card_info.time[5] = tnow->tm_sec;   	
   	memcpy(&door_card_info.dev_no[0],&dev_cfg.my_code[0],sizeof(door_card_info.dev_no)); 
   	memcpy(buf+3,&door_card_info,sizeof(door_card_info));
    net_send_to_server(UNMJKC1, buf, sizeof(door_card_info_t)+3);
    free(buf);
}				

//added by hgj 180302
int send_lift_data_to_kzq(unsigned char dest[],unsigned int net_cmd,void *buf,int len)//by mkq 20171017 netlift
{
	proc_msg msg_send;
	net_data_msg msg;
	memcpy(msg.buf,buf,len);
	msg.net_cmd=net_cmd;
	msg.flag_addr=KZQ_LIFT_DATA;
	msg.data_len=len;
	msg_send.msg_type=MSG_FROM_PROC;
	memcpy(&msg_send.msg,&msg,sizeof(net_data_msg));
	if(send_data_to_net(&msg_send, MSG_LEN_NET)<0){
		app_debug(DBG_INFO, "send_data_to_pc cmd:0x%x error\n",net_cmd);
		return -1;
	}
	return 0;
}

//added by hgj 180302
void kzq_send_lift_data(unsigned sub_code ,unsigned char *data) //by mkq 20171025 netlift
{
    unsigned char dest[4]={0};
    int sum=0,i;
    dest[0]=  dev_cfg.my_code[0];
    dest[1]=  dev_cfg.my_code[1];
    dest[3]=  sub_code;
    printf("YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY \n");
    //net_send_to_node(dest, GDATC, data, 10);
    if(!((data[3]==0)&&(data[5]==0)&&(data[6]==0)&&(data[7]==0)&&(data[8]==0)))
    {
        for (i = 0; i < 9; i++) {
    		sum  += *(data + i);
        }
        data[9] = sum;
        
        send_lift_data_to_kzq(dest,NTLFTC, data, 10);
        printf("NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN \n");  
    }


    unsigned char *c_data = (unsigned char *)data;

    printf("net_send_to_node: cmd %03X, dest %02X%02X%02X%02X data ",  NTLFTC, dest[0], dest[1], dest[2], dest[3]);
    for (i = 0; i < 10; i++) {
        printf("%02X ", c_data[i]);
    }
    printf("\n");

}


#if 0
int proc_watch_sw(WATCH_ACTION act, const char *dst_aip)
{
    unsigned char msg[MSG_LEN_PROC];
    static char   aip[30];
    
    if ((proc_state == PROC_CALLING) || (proc_state == PROC_TALKING))
        return -1;

    switch (act) {
        case WATCH_START:
            if (gbl_data_get(&gbl_watching)) return -1;
            //usleep(150*1000);
            strcpy(aip, dst_aip);
            video_day_night_set();
            printf("watching dst_aip:%s\n", (char*)dst_aip);
            video_cap_send_start(inet_addr(dst_aip), 0, 0, 0);
            tim_reset_time(tim_wactch_timeout, TIME_500MS(33));
            gbl_data_set(&gbl_watching, 1); 
            break;
        case WATCH_STOP:
            if (!gbl_data_get(&gbl_watching)) return -1;
            video_send_stop();
            video_day_night_clr();
            gbl_data_set(&gbl_watching, 0);
            break;
        case WATCH_BREAK:
            //有呼叫才会打断监视，视频会被呼叫占用，无需关闭视频通道
            //正在监视的终端被呼叫打断，应先挂监视，而因呼叫别的终端打断正在监视的终端需等待被呼终端应答后才挂监视
            if (!gbl_data_get(&gbl_watching)) return -1;
            if (proc_state == PROC_WAIT_ACK) {
                if (0 == memcmp(target, watch_host, 4)) break;
            } else {
                //if (0 != memcmp(target, watch_host, 4)) break;
                if (0 != memcmp(target, watch_host, IS_ADMIN(watch_host)?3:4)) break;
            }
            if (dev_get_type(watch_host) == DEV_ADMIN)
                net_query_to_node(watch_host, NENDC, MAMOTOR, VIDEORES);
            else
                net_query_to_node(watch_host, NENDC, FJMOTOR, VIDEORES);         
            video_send_stop();
            gbl_data_set(&gbl_watching, 0);
            usleep(600*1000);
            break;
        case WATCH_TIMEOUT:
            msg[0] = MSG_FROM_PROC;
            msg[1] = MSG_WATCH_TIMEOUT;
            pipe_put(proc_pipe_proc, msg, MSG_LEN_PROC);
            break;
    }
    return 0;
}
#else

int proc_watch_sw(WATCH_ACTION act, unsigned long lip)
{
    unsigned char msg[MSG_LEN_PROC];
    static unsigned long watch_lip = 0;
    static int           watch_num = 0;

    if ((proc_state == PROC_CALLING) || (proc_state == PROC_TALKING))
        return -1;

    switch (act) {
        case WATCH_TRY:
            //if (!gbl_data_get(&gbl_watching)) {
             if ((!gbl_data_get(&gbl_watching)) ||(!gbl_data_get(&gbl_mb_watching))){
                if((get_cur_scene() == SCENE_FACE_DETECT) && (dst_type != TYPE_OTHER)){//jpeg码率输出时可以不关闭face_det 
                    printf("now FR mode exiting...\n");
                    ui_action(UI_SW_FR_TO_MAIN,0); 
                    usleep(10*1000);
                }                
                return 0;
            } 
            if (is_have_infostore() && (watch_lip == lip)) {
                return 0;
            }
            return -1;
        case WATCH_MOBILE_START:
           if (gbl_data_get(&gbl_watching)||gbl_data_get(&gbl_mb_watching)) return -1;       	
            gbl_data_set(&gbl_watching, 1);
            gbl_data_set(&gbl_mb_watching, 1);
            unsigned long lip1[4]={0};
            lip1[0] =lip;
            watch_lip = lip;
            memcpy(watch_host, watch_expect, 4);
            //video_day_night_set();
            video_send_start(lip1, VID_SERVER, VID_CIF5);
            //video_send_start(lip1, VID_MOBILE, VID_CIF);
            tim_reset_time(tim_wactch_timeout, TIME_500MS(40));
            break;
        case WATCH_START:
            if (is_have_infostore()) {
                //if (!gbl_data_get(&gbl_watching)) {
                if ((!gbl_data_get(&gbl_watching)) ||(!gbl_data_get(&gbl_mb_watching))) {
                    watch_num = 1;
                    printf("WATCH_START: watch_num: %d \n", watch_num);
                } else if (watch_lip == lip) {
                    tim_reset_time(tim_wactch_timeout, TIME_500MS(33));
                    watch_num++;
                    printf("WATCH_START: watch_num: %d \n", watch_num);
                    return 0;
                }
            }
            //if (gbl_data_get(&gbl_watching)) return -1;3
            if (gbl_data_get(&gbl_watching)||gbl_data_get(&gbl_mb_watching)) return -1;
	     
            gbl_data_set(&gbl_watching, 1);            
            watch_lip = lip;
            memcpy(watch_host, watch_expect, 4);
            //video_day_night_set();
            video_cap_send_start(lip, 0, 0, 0);
            tim_reset_time(tim_wactch_timeout, TIME_500MS(33));
            break;
        case WATCH_STOP:
            //if (is_have_infostore() && gbl_data_get(&gbl_watching)) {
            if (is_have_infostore() && ((!gbl_data_get(&gbl_watching)) ||(!gbl_data_get(&gbl_mb_watching)))) {

                if (get_code_from_container_data((char*)&watch_host[2]) > 0) {
                    printf("WATCH_STOP: watch_num: %d \n", watch_num);
                    if (--watch_num > 0) break;
                }
            }
            if (!gbl_data_get(&gbl_watching)) return -1;
            video_send_stop();
            video_day_night_clr();
            gbl_data_set(&gbl_watching, 0);
	     gbl_data_set(&gbl_mb_watching, 0);
            break;
        case WATCH_STOPALL:
            if (!gbl_data_get(&gbl_watching)) return -1;
            video_send_stop();
            video_day_night_clr();
            gbl_data_set(&gbl_watching, 0);
	     gbl_data_set(&gbl_mb_watching, 0);
            break;
        case WATCH_BREAK:
            //if (!gbl_data_get(&gbl_watching)) return -1;
            if (!gbl_data_get(&gbl_watching)&&!gbl_data_get(&gbl_mb_watching)) return -1;

            if (dev_get_type(watch_host) == DEV_ADMIN)
                net_query_to_node(watch_host, NENDC, MAMOTOR, VIDEORES);
            else{
                //net_query_to_node(watch_host, NENDC, FJMOTOR, VIDEORES); 
            	if (gbl_data_get(&gbl_mb_watching)) 
			        net_query_to_node(watch_host, NENDC3, FJMOTOR, VIDEORES);
                else
                     net_query_to_node(watch_host, NENDC, FJMOTOR, VIDEORES);
		} 
            video_send_stop();
            gbl_data_set(&gbl_watching, 0);
	     gbl_data_set(&gbl_mb_watching, 0);
            sleep(1);//usleep(600*1000);
            break;
        case WATCH_TIMEOUT:
            msg[0] = MSG_FROM_PROC;
            msg[1] = MSG_WATCH_TIMEOUT;
            pipe_put(proc_pipe_proc, msg, MSG_LEN_PROC);
            break;
    }
    return 0;
}
#endif

int quit_monitor_mode()
{
    printf("%s running\n",__func__);
    proc_watch_sw(WATCH_BREAK, 0);
}

static void proc_switch_to(PROC_STATE state)
{   
    PROC_STATE state_pre = proc_state;
    struct in_addr ip_in_addr;
    static char dst_aip[30];
    static int  leave_bytes;

    proc_state = state; 
    ip_in_addr.s_addr = dst_lip;
    strcpy(dst_aip, inet_ntoa(ip_in_addr));
    printf("proc_switch: %d to %d\n", state_pre, state);
    switch(state) {
        case PROC_NORMAL:
            tim_suspend_event(tim_proc_timeout);
	     gbl_data_set(&gbl_cloud_show, 0);
            video_day_night_clr();
//       key_light_delay_off(30);  //del by wrm 20141121 for i2c
            dest_state = DEST_OFFLINE;
            cloud_server_state = DEST_OFFLINE;
            dst_server = 0;
            mobile_hold_on=0;
            if (state_pre == PROC_CALLING) {
                audio_stop();
                video_send_stop();
            }
            else
            if (state_pre == PROC_TALKING) {
                speech_stop();
                video_send_stop();
            }
            else
            if (state_pre == PROC_LEAVE_MSG) {
		  printf("PROC_LEAVE_MSG......\n");
                memcpy(leave_buf,  target, 4);
                memcpy(leave_buf+4, &leave_bytes, 4);
                stop_local_leave_msg();
                if(dst_type == TYPE_D2100)
                    send_addr_to_server_bydev_NO(0x528, target, leave_buf, 8 + leave_bytes);
                else
                    send_addr_to_tml(0x528, target, leave_buf, 8 + leave_bytes);
            } 
            break;
        case PROC_WAIT_ACK:
		dest_state = DEST_OFFLINE;
#ifdef FRONT_DOOR
            tim_reset_time(tim_proc_timeout, ((dst_type == TYPE_JDM365)||(dst_type == TYPE_J926)) ? TIME_1S(8) : TIME_1S(10));
#else
            printf("reset 6s again\n");
            tim_reset_time(tim_proc_timeout, ((dst_type == TYPE_JDM365)||(dst_type == TYPE_J926)) ? TIME_500MS(12) : TIME_1S(8));
#endif
            break;
        case PROC_CALLING:
            //tim_reset_time(tim_proc_timeout, TIME_1S(31-1));    
            //tim_reset_time(tim_proc_timeout, dst_server?TIME_1S(25):TIME_1S(5)); 
            printf("dst_server=%d\n",dst_server);
            if(dst_server==0){
                tim_reset_time(tim_proc_timeout, TIME_500MS(30*2+1));//29
            }else{
                tim_reset_time(tim_proc_timeout, TIME_500MS(60*2+1));//59
            }
            proc_watch_sw(WATCH_BREAK, 0);
            if (gbl_data_get(&gbl_watching)) {
                video_send_stop();
                gbl_data_set(&gbl_watching, 0);
            }
	    if (dst_server != 2) {  //正在呼分机，超时呼到手机，振铃声音不变
	        printf("dst_server=%d\n",dst_server);
               audio_play(PMT_ECHORING, DEV_VOL_PLAY);
            }
            printf("calling dst_aip:%s\n", (char*)dst_aip);
            //audio_play(PMT_ECHORING, DEV_VOL_PLAY);
            //key_light_ctrl(1); //del by wrm 20141121
            usleep(100*1000);
            //video_day_night_set();

#ifndef RecallTime           
            if (dst_server) {
                unsigned long lip[4] = {0};
                lip[0] = get_server_ip();
                if (dest_state == DEST_ONLINE) {
                    video_send_start(lip, VID_SERVER, VID_CIF5);
                }//dst_server=2即承接若分机没有提机，主机会超时呼叫服务器。而终止时间变为59/2-13/2=46/2
                if (dst_server == 2) 
                    tim_reset_time(tim_proc_timeout, TIME_500MS(23*2+1)); //正在呼分机，超时呼到手机，等待时间缩短
            } else { 
            //在呼叫过程的13/2s后如果分机没有提机即主机没有转换成talking模式，则会转呼服务器
                tim_reset_time(tim_try_call_server, TIME_500MS(13));
                video_cap_send_start(ipmulti[0].ip_addr, ipmulti[1].ip_addr, ipmulti[2].ip_addr, ipmulti[3].ip_addr);
                printf("calling dst_aip:%s\n", (char*)dst_aip);
            }

#else
	    if (dst_server) {
                unsigned long lip[4] = {0};
                lip[0] = get_server_ip();
                if (dest_state == DEST_ONLINE) {
                    video_send_start(lip, VID_SERVER, VID_CIF5);
                }
            } else{
               if(cloud_server_state==DEST_ONLINE)
                tim_reset_time(tim_try_call_server, TIME_500MS(dev_cfg.recall_time*2));
                video_cap_send_start(ipmulti[0].ip_addr, ipmulti[1].ip_addr, ipmulti[2].ip_addr, ipmulti[3].ip_addr);
                printf("calling dst_aip:%s\n", (char*)dst_aip);
            } 
#endif            
           

		break;
        case PROC_TALKING:
            tim_suspend_event(tim_try_call_server);
            tim_reset_time(tim_proc_timeout, TIME_1S(60));//60 //mkq 20170907
            audio_stop();
            if (dst_server) {
                struct in_addr ip;
                ip.s_addr=get_server_ip();
                printf("speech_start serverip:%s\n", inet_ntoa(ip));
                cloud_speech_start(inet_ntoa(ip),DEV_VOL_TX,DEV_VOL_RX,0);
            //    speech_start_with_volume_pcm(inet_ntoa(ip), (double)1.0/8000.0, DEV_VOL_TX, DEV_VOL_RX, UDP_SPEECH);
                break;
                }
            if (dst_type == TYPE_JDM365||(dst_type == TYPE_J926)) {
                speech_start((char*)dst_aip,DEV_VOL_TX,DEV_VOL_RX,0);
              //  speech_start_with_volume((char*)dst_aip, (double)1.0/8000.0, DEV_VOL_TX, DEV_VOL_RX, UDP_SPEECH);
                }else{
                    speech_start((char*)dst_aip,DEV_VOL_TX,DEV_VOL_RX,0);
          //      speech_start_with_volume((char*)dst_aip, (double)1.0/8000.0, DEV_VOL_TX_2, DEV_VOL_RX_2, UDP_SPEECH);
                } 
            printf("speech_start dst_aip:%s\n", (char*)dst_aip);
            break;
        case PROC_LEAVE_MSG:
	     #if 0
            printf("leave msg dst_aip:%s\n", (char*)dst_aip);
            tim_reset_time(tim_proc_timeout, TIME_1S(16));
            if (dst_type == TYPE_D2100) {
                start_audio_capture_local_with_volume(leave_buf+8, &leave_bytes, MSG_VOL(DEV_VOL_TX_2));
            } else  if (dst_type == TYPE_JDM365||(dst_type == TYPE_J926)) {
            start_audio_capture_local_with_volume(leave_buf+8, &leave_bytes, MSG_VOL(DEV_VOL_TX));
            }
            break;
	     #endif
            printf("leave msg dst_aip:%s\n", (char*)dst_aip);
            tim_reset_time(tim_proc_timeout, TIME_1S(16));
            if (dst_type == TYPE_D2100) {
                start_audio_capture_local_with_volume(leave_buf+8, &leave_bytes, MSG_VOL(DEV_VOL_TX_2));
            } else  if (dst_type == TYPE_JDM365||(dst_type == TYPE_J926)) {
            start_audio_capture_local_with_volume(leave_buf+8, &leave_bytes, MSG_VOL(DEV_VOL_TX));
            }else         //ANKAY TYPE
             start_audio_capture_local_with_volume(leave_buf+8, &leave_bytes, MSG_VOL(DEV_VOL_TX));
            break;
        default:
            break;
    }
    usleep(100*1000);
}

static unsigned char sub_card[4];

static unsigned int card_hex2int(unsigned char *hex)
{
    return (hex[0]<<24)|(hex[1]<<16)|(hex[2]<<8)|hex[3];
}
static unsigned int card_bcd2hex(unsigned char *bcd, unsigned char *hex)
{
    unsigned long i, multi = 100000000;
    unsigned long ret = 0;
    for (i = 0; i < 5; i++) {
        ret += multi * ((bcd[i]/16)*10 + (bcd[i]%16));
        multi /= 100;
    }
    if (hex != NULL) {
    multi = 24;
    for (i = 0; i < 4; i++) {
        hex[i] = (ret&(0xFF<<multi)) >> multi;
        multi -= 8;
    }
    }
    return (unsigned int)ret;
}


static void card_hex2bcd(unsigned char *hex, unsigned char *bcd)
{
    unsigned int cardno = card_hex2int(hex);

    unsigned int i, multi = 100000000;
    for (i = 0; i < 5; i++) {
        unsigned char uchar_data = cardno/multi;
        bcd[i] = ((uchar_data/10)*16) + (uchar_data%10);
        cardno = cardno-uchar_data*multi;
        multi /= 100;
    }
}
/*
static unsigned int card_bcd2int(unsigned char *bcd)
{
    unsigned long i, multi = 1000000;
    unsigned long ret = 0;
    for (i = 0; i < 4; i++) {
        ret += multi * ((bcd[i]/16)*10 + (bcd[i]%16));
        multi /= 100;
    }
    return (unsigned int)ret;
}
*/
static void card_bcdcopy(unsigned char *data, unsigned char *buf)
{
    unsigned char dat[5];
    memcpy(dat, data, 5);
    buf[0] = dat[1];
    buf[1] = dat[0];
    buf[2] = dat[2];
    buf[3] = dat[3];
    buf[4] = dat[4];
}
#ifdef	BLE
static void printf_hex(uint8_t* str,uint8_t length)
{
    unsigned char i;
    for(i = 0; i < length; ++i)
        printf("%.2X ", str[i]);
    printf("\n");
}

unsigned char CharToHex(unsigned char bHex){
    if((bHex>=0)&&(bHex<=9))
        bHex += 0x30;
    else if((bHex>=10)&&(bHex<=15))//大写字母
        bHex += 0x37;
    else bHex = 0xff;
    return bHex;
}
unsigned char HexToChar(unsigned char bChar){
    if((bChar>=0x30)&&(bChar<=0x39))
        bChar -= 0x30;
    else if((bChar>=0x41)&&(bChar<=0x46))//大写字母
        bChar -= 0x37;
    else if((bChar>=0x61)&&(bChar<=0x66))//小写字母
        bChar -= 0x57;
    else bChar = 0xff;
    return bChar;
}

void build_key(unsigned char *default_data,unsigned char *new_seed){
	unsigned char i = 0;
	unsigned char buf[16] = {0};

	buf[0] = vs_cfg.my_village_code[0] /16;
	buf[1] = vs_cfg.my_village_code[0] %16;
	buf[2] = vs_cfg.my_village_code[1] /16;
	buf[3] = vs_cfg.my_village_code[1] %16;
	buf[4] = vs_cfg.my_village_code[2] /16;
	buf[5] = vs_cfg.my_village_code[2] %16;
	buf[6] = dev_cfg.my_code[0] /16;
	buf[7] = dev_cfg.my_code[0] %16;
	buf[8] = dev_cfg.my_code[1] /16;
	buf[9] = dev_cfg.my_code[1] %16;
	memset(&buf[10],0,6);
	printf("The key: ");
	for(i =0 ;i<16;i++){
		buf[i]=CharToHex(buf[i]);
		//printf("%c ",buf[i]);
		}
	printf("\n");
	
//	printf("The device decrypt key: ");
	AES128_ECB_encrypt(buf,default_data,seed_key1);
	//printf_hex(seed_key1, 16);
       buf[6] = 0X30;
       buf[7] = 0X30;
	buf[8] = 0x30;
	buf[9] = 0x30;
		printf("The key2: ");
	for(i =0 ;i<16;i++){
		//buf[i]=CharToHex(buf[i]);
		//printf("%c ",buf[i]);
		}
	printf("\n");
	printf("default_data :%02x %02x %02x %02x \n",default_data[0],default_data[1],default_data[2],default_data[3]);
	AES128_ECB_encrypt(buf,default_data,seed_key2);   //for version 2 key
	//printf_hex(seed_key2, 16);
	AES128_ECB_encrypt(buf,new_seed,seed_key3);   //for version 3key
	//printf_hex(seed_key3, 16);
	
	memset(buf,'F',6); //default village------------FFFFFF
	AES128_ECB_encrypt(buf,default_data,seed_key_updata);   //for version 3key
	//printf_hex(seed_key_updata, 16);
}

void  send_door_status_ble(unsigned char version,unsigned char status){
	    unsigned char send_blebuf[8] = {0};
	    send_blebuf[0] = 0X8F;
	    send_blebuf[1] = version;
	    send_blebuf[2] = status;
	    memset(&send_blebuf[3],0,5);
	    send_blebuf[5] = 0x00;
	    send_blebuf[6] = dev_cfg.my_code[3];
	    uart_send_data(UART_1, send_blebuf);
}


static int check_time_now(unsigned char *time_buf,unsigned char  version){
	unsigned char nowtime[20]   ={0};
	unsigned int 	recvtime[20]  ={0};
	unsigned int 	deadtime[20]  ={0};
	int 	ret = 0;
	
	time_t now;
	struct tm *tnow;
	time(&now);
	tnow = localtime(&now);
	sprintf((char *)nowtime, "%4d.%02d.%02d %02d:%02d:%02d", \
		1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday, tnow->tm_hour, tnow->tm_min,tnow->tm_sec);
	printf("nowtime: %s\n",nowtime);

	if(version != VERSION_3){
		recvtime[0] = (time_buf[0]/16)*1000+(time_buf[0]%16)*100+(time_buf[1]/16)*10+time_buf[1]%16;
		recvtime[1] = (time_buf[2]/16)*10+time_buf[2]%16;
		recvtime[2] = (time_buf[3]/16)*10+time_buf[3]%16;
		recvtime[3] = (time_buf[4]/16)*10+time_buf[4]%16;
		sprintf((char *)deadtime,"%4d.%02d.%02d %02d:%02d:%02d",recvtime[0],recvtime[1],\
			recvtime[2],recvtime[3],recvtime[4],recvtime[5]);
	}
	else{
		recvtime[0] = (time_buf[0]/16)*10+time_buf[0]%16;
		recvtime[1] = (time_buf[1]/16)*10+time_buf[1]%16;
		recvtime[2] = (time_buf[2]/16)*10+time_buf[2]%16;
		recvtime[3] = (time_buf[3]/16)*10+time_buf[3]%16;
		recvtime[4] = (time_buf[4]/16)*10+time_buf[4]%16;
		sprintf((char *)deadtime,"%4d.%02d.%02d %02d:%02d:%02d",2000+recvtime[0],recvtime[1],\
			recvtime[2],recvtime[3],recvtime[4],recvtime[5]);	
	}
	
	
	printf("deadtime:%s\n",(char *)deadtime);

	ret = strcmp((char *)deadtime,(char *)nowtime);
	printf("ret = %d\n",ret);

	return ret;
}

void updata_dev_village(unsigned char *data){
	
	vs_cfg.my_village_code[0] = data[0];
	vs_cfg.my_village_code[1] = data[1];
	vs_cfg.my_village_code[2] = data[2];
	//vs_cfg.has_set = 1;
	my_new_village_code_to_Ble();
       dev_config_save();
       usleep(200);
	dev_sv_get();
}

void updata_dev_seed(unsigned char *data){
	printf("updata_dev_seed\n");
	vs_cfg.has_set = 1;
	memcpy(vs_cfg.dev_seed_data,data,16);
       dev_config_save();
       usleep(200);
	dev_sv_get();
}
 int decrypt_up_time_data(const uint8_t *data,unsigned char *key){	
	uint8_t buf_out[16] = {0};
       time_t now;
       struct tm *tnow;
	char time_s[20] = {'\0'};
	char time_new[20]= {'\0'};
	int i = 0,ret = 0;
	AES128_ECB_decrypt((uint8_t*)data,key,buf_out);

       printf("buf_out time hex: ");
	for(i = 0 ;i<16;i++)
		printf("%02X ", buf_out[i]);
	printf("\n");
	printf("buf_out time int: ");
	for(i = 0 ;i<16;i++){
		buf_out[i] =  (buf_out[i] /16)*10+buf_out[i]%16;
		printf("%02X ", buf_out[i]);}
	printf("\n");
	
	time(&now);
	tnow = localtime(&now);
	sprintf(time_s, "%4d.%02d.%02d %02d:%02d:%02d", 1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday, tnow->tm_hour, tnow->tm_min,tnow->tm_sec);
	printf("dev time is %s\n",time_s);

	sprintf(time_new, "%4d.%02d.%02d %02d:%02d:%02d", 2000+buf_out[1], buf_out[2],buf_out[3],buf_out[4],buf_out[5],buf_out[6]);
	printf("rev time is %s\n",time_new);

	ret = strcmp((char *)time_new,(char *)time_s);
	/*
	if(ret >=0){
		dev_set_time(2000+buf_out[1], buf_out[2], buf_out[3], buf_out[4], buf_out[5], buf_out[6]);
		printf("OK!Set time success!\n");
	}
	else 
		printf("Sorry!Set time fail!ret <0\n");
	*/
	dev_set_time(2000+buf_out[1], buf_out[2], buf_out[3], buf_out[4], buf_out[5], buf_out[6]);
	printf("OK!Set time success!\n");
	ui_action(UI_STBAR_REFRESH, 0);
	return 0;
}

 int decrypt_up_village_or_seed_data(const uint8_t *data,unsigned char *key,unsigned char type){	
	uint8_t buf_out[16] = {0};
	int i = 0;
	AES128_ECB_decrypt((uint8_t*)data,key,buf_out);

       printf("village hex: ");
	for(i = 0 ;i<16;i++)
		printf("%02X ", buf_out[i]);
	printf("\n");
	if(type == 0)
		updata_dev_village(&buf_out[1]);
	else if(type == 0X01)
		updata_dev_seed(buf_out);
	return 0;
}
 int decrypt_check_unlock_data(const uint8_t *data,unsigned char *key,unsigned char  version){	
	uint8_t buf_out[16] = {0};	

	AES128_ECB_decrypt((uint8_t*)data,key,buf_out);

	
	memcpy(&decrypt_data,buf_out,sizeof(decrypt_data));

	printf("The decrypt data:%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n",\
		decrypt_data.type,decrypt_data.village_code[0],decrypt_data.village_code[1],decrypt_data.village_code[2],\
		decrypt_data.village_code[3],decrypt_data.my_code[0],decrypt_data.my_code[1],decrypt_data.my_code[2],\
		decrypt_data.room_nu[0],decrypt_data.room_nu[1],decrypt_data.dead_line[0],decrypt_data.dead_line[1],\
		decrypt_data.dead_line[2],decrypt_data.dead_line[3],decrypt_data.dead_line[4],decrypt_data.sum);

	printf("This Device:%02X %02X %02X %02X %02X %02X %02X \n",vs_cfg.my_village_code[0],vs_cfg.my_village_code[1] ,vs_cfg.my_village_code[2],\
		dev_cfg.my_code[0],dev_cfg.my_code[1],dev_cfg.my_code[2],dev_cfg.my_code[3]);
		
#ifdef FRONT_DOOR
	//check village and room
	if(decrypt_data.village_code[0] == vs_cfg.my_village_code[0] && decrypt_data.village_code[1] == vs_cfg.my_village_code[1] &&\
		decrypt_data.village_code[2] == vs_cfg.my_village_code[2])
#else

	if(decrypt_data.village_code[0] == vs_cfg.my_village_code[0] && decrypt_data.village_code[1] == vs_cfg.my_village_code[1] &&\
		decrypt_data.village_code[2] == vs_cfg.my_village_code[2]&&decrypt_data.my_code[0] == dev_cfg.my_code[0]&&\
		decrypt_data.my_code[1] == dev_cfg.my_code[1])
#endif
		{
		printf("The room is ok!\n");
		if( check_time_now(&buf_out[10],version) >= 0){
			printf("OK !The time is valid\n");
			return 1;
		}
		else{
			printf("Sorry!The time is dead!\n");
			return 0;
		}
	}else{
		printf("Sorry!Village or room error!\n");
		return -1;
	}
}


static int check_ble_data(unsigned char *data, unsigned char frame_num,unsigned char *key,unsigned char version){
	int ret = 0,i = 0,j=0;
	int result = -1;
	uint8_t data_xor[16] = {0};
	memset(data_xor, 0, 16);
	static uint8_t XOR_KEY[16]={0x67,0x75,0x61,0x6e,0x67,0x64,0x6f,0x6e,0x67,0x61,0x6e,0x6a,0x75,0x62,0x61,0x6f};
	
	printf("check data: \n");
	for(i = 0;i<=(frame_num+1)*16;i++)
		printf("%02X ",data[i]);
	printf("\n");
	for(i =0;i <= frame_num ;i++){
		printf("%d Decrypting data...... \n",i);
		printf_hex(&data[i*16],16);
		for(j = 0;j < 16;j++)
			data_xor[j] = data[i*16+j]^XOR_KEY[j];
		printf("XOR DATA.....\n");
		printf_hex(data_xor, 16);
		if(version == VERSION_TIME)
			ret = decrypt_up_time_data(data_xor,key);
		else if(version == VERSION_VI)
			ret = decrypt_up_village_or_seed_data(data_xor,key,0x00);
		else if(version == VERSION_SEED)
			ret = decrypt_up_village_or_seed_data(data_xor,key,0x01);
		else
			ret = decrypt_check_unlock_data(data_xor,key,version);
		if(ret == 1||ret == 0){
			result = ret;
/**add for photo upload**/
			//memcpy(g_ble_buf,&data[i*16],16);
/**end**/			
			break;
		}
		else
			result = ret;			
	}
	return result;	
}


#endif

int load_local_time(time_t *time)
{
	int fd=open("/opt/app/local_time.log",O_RDONLY);
	if(fd<0)
	{
		printf("open local_time.log filed:%s\n",strerror(errno));
		return -1;
	}
	if(read(fd,time,sizeof(time_t))!=sizeof(time_t))
	{
		printf("read time filed:%s\n",strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int save_local_time(time_t time)
{
	int fd=open("/opt/app/local_time.log",O_WRONLY|O_CREAT,0664);
	if(fd<0)
	{
		printf("open local_time.log filed:%s\n",strerror(errno));
		return -1;
	}
	if(write(fd,&time,sizeof(time_t))!=sizeof(time_t))
	{
		printf("write time filed:%s\n",strerror(errno));
		close(fd);
		return -1;
	}
    fdatasync(fd);	
	close(fd);
	return 0;
}

int set_local_time(time_t time)
{
    printf("%s: set time\n",__func__);
	if(stime(&time)!=0)
	{
		perror("stime local error:");
		return -1;
	}
	return 0;
}

void *save_time_thread_process(void*arg)
{
	printf("save_time_thread_process start!\n");
	time_t now;
	struct tm *tnow;
	int save_flag=1;

	if(load_local_time(&now)==0)
	{
		set_local_time(now);
	}
	sleep(300);
	while(1)
	{
		sleep(10);
		time(&now);
		tnow = localtime(&now);
		if(tnow->tm_hour==0)
		{
			if((tnow->tm_min==10)&&(tnow->tm_sec>10)&&(tnow->tm_sec<40))
			{
				if(save_flag==1)
				{
					save_local_time(now);
					//system("/opt/app/S25save-rtc.sh");
					save_flag=0;
					sleep(60);
				}
			}
			else
			{
				save_flag=1;
			}
		}
	}
	printf("save_time_thread_process exit!\n");
}

int save_time_start(void *arg)
{
	static pthread_t save_time_tid;
    pthread_create(&save_time_tid, NULL, save_time_thread_process, NULL);
    return 0;
}

void uart_send_lift_data(unsigned char *data)
{
#ifndef FRONT_DOOR
    unsigned char buf[16];
    printf("uart_send_lift_data!\n");
    buf[0] = data[0];
    switch(data[0]) {
        case SSIC:
            buf[1] = buf[5] = dev_cfg.my_code[0];
            buf[2] = buf[6] = dev_cfg.my_code[1];
            buf[3] = data[2];
            buf[4] = data[3];
            buf[7] = dev_cfg.my_code[3];
            buf[8] = 0x00;
            break;
        case SSCS:      //8 bytes
            buf[1] = dev_cfg.my_code[0];
            buf[2] = dev_cfg.my_code[1];
            buf[3] = data[2];
            buf[4] = data[3];
            buf[5] = dev_cfg.my_code[3];
            buf[6] = dev_cfg.my_code[1];
            buf[7] = data[4];
            buf[8] = data[5];
            break;
        case STEL:      //UI的呼叫动作, 非标准8bytes AJB数据
        case SONE:
        case 0x7C:
        case 0x7F:
            buf[0] = (buf[0] == 0x7F) ? SEND: STEL;
            buf[1] = dev_cfg.my_code[0];
            buf[2] = dev_cfg.my_code[1];
            buf[3] = data[3];
            buf[4] = data[4];
            buf[5] = dev_cfg.my_code[0];
            buf[6] = dev_cfg.my_code[1];
            buf[7] = 0x00;
            buf[8] = dev_cfg.my_code[3];
            break;
        case SSSK:      //10 bytes
        case SKEY:
        case SANS:
        case SEND:
        case SSINT:
            memcpy(buf, data, 10);
        #if 0
            buf[1] = dev_cfg.my_code[0];
            buf[2] = dev_cfg.my_code[1];
            buf[3] = data[2];
            buf[4] = data[3];
            buf[5] = dev_cfg.my_code[0];
            buf[6] = dev_cfg.my_code[1];
            buf[7] = data[4];
            buf[8] = data[5];
        #endif
            break;
        case 0xED:
            buf[1] = dev_cfg.my_code[0];
            buf[2] = dev_cfg.my_code[1];
            buf[3] = data[1];
            buf[4] = dev_cfg.my_code[3];
            buf[5] = data[4];
            buf[6] = data[2];
            buf[7] = data[5];
            buf[8] = data[6];
            break;
        default:
            return;
    }
    #if 1
    kzq_send_lift_data(UART_0, buf);
    #else    
    pause_fp_polling();
    uart_send_data(UART_0, buf);
    #endif
#endif
}

#ifdef DOOR_ACCESS_CARD_LIST
// date>current_data返回1  ，date《current_data返回-1 ，date==current_data返回0
int compare_date(uint8_t date[6],uint8_t current_data[6])
{
	int i=0;
	for(i=0;i<6;i++)
	{
		if(date[i]>current_data[i])
		{
			return 1;
		}
		else if(date[i]<current_data[i])
		{
			return -1;
		}
		else
		{
			continue;
		}
	}
	return 0;
}
#if 0
//把BCD分成十位和个位
unsigned char Make2TODec(unsigned char Bcd,unsigned char *Dec0,unsigned char *Dec1)
{
	*Dec0 = (Bcd>>4)&0x0f;
	*Dec1 = Bcd&0x0f;
	return 0;
}

unsigned char MakeDec(unsigned char buf0)
{
	unsigned char Dec = 0;
	Dec = ((buf0>>4)&0x0f)*10+(buf0&0x0f);
	return Dec;
}

unsigned char AddTotal(unsigned char *Data,unsigned char Len)
{
	unsigned char i;
	unsigned char result=0;

	for( i = 0 ; i < Len; i++ )
		result+=*(Data+i);
	return result;
}

unsigned char MemCmpInt( const unsigned char * SrcAddr,const unsigned char * DstAddr,unsigned short Count )
{
	unsigned short i;

	for( i = 0; i < Count; i++ )
	{
		if( SrcAddr[i] != DstAddr[i] )
			return i+1;
	}
	return 0;
}


unsigned char MakeBcd(unsigned char buf0,unsigned char buf1)
{
	unsigned char Bcd = 0;

	Bcd = ( buf0-'0') ;
	Bcd = ((Bcd <<4) & 0xf0)+(buf1-'0');
	return Bcd;
}


unsigned char Bcd2Dec(unsigned char buf0)
{
	unsigned char Dec = 0;
	Dec = ((buf0>>4)&0x0f)*10+(buf0&0x0f);
	return Dec;
}

unsigned char Dec2Bcd(unsigned char dec)
{
	return (unsigned char)(((dec/10)<<4) + dec%10) ;
}

#endif

void Uint32to5Bcd(uint32_t  dec,uint8_t *hex)
{
	int i=0;
	uint32_t tmp=100;
	uint8_t hex_tmp[5]={0};
	for(i=0;i<5;i++)
	{
		switch(i)
		{
		case 0:
			hex_tmp[i]=(dec%(100));
			break;
		case 1:
			tmp=100;
			hex_tmp[i]=((dec/tmp)%(100));
			break;
		case 2:
			tmp=10000;
			hex_tmp[i]=((dec/tmp)%(100));
			break;
		case 3:
			tmp=1000000;
			hex_tmp[i]=((dec/tmp)%(100));
			break;
		case 4:
			tmp=100000000;
			hex_tmp[i]=((dec/tmp)%(100));
			break;

		}
		hex[4-i]=Dec2Bcd(hex_tmp[i]);
	}
}

/**modified for photo upload**/
static int proc_sfz8or4_cardreader(unsigned char *data,int num)
{
	unsigned char dest[4]={0};
	unsigned char buf[10]={0};
	unsigned char send_card_buf[25]={0};   
	unsigned char card_hex[8], card_bcd[8],temp_buf[10];
	//unsigned char room[4]={0};
	uint8_t tty_buff[10] = {0x00};
	unsigned char tty_buff_r[10] = {0};
	int ret = 1,i;
	uint64_t tmp_number = 0;
 	
	printf("proc_sfz8or4_cardreader:  ");
    for (i = 0; i < UART1_FRAME_LEN+3; i++)
        printf("%02X ", data[i]);
    printf("\n");

	memcpy(card_hex, &data[0], num);		

	for(i=7;i>=0;i--){
		tty_buff[i]=data[7-i];
	}
	
	uint8_t devno[4];
	uint8_t tmp_cardnumber[8];
	memset(tmp_cardnumber,0,8);
	
	
	dcard_list_t door_card = {0,};
	door_card_info_t card_info;
	
	memcpy(tty_buff_r,tty_buff,10);	
	memset(door_card.cardnumber,0,8);
	memset(door_card.dev_no,0,4);

	card_info.reserved0=0;
	card_info.reserved1=0;
	card_info.reserved2=0;


	if (num==8){
		memcpy((uint8_t*)&tmp_number, tty_buff, num);
		memcpy(door_card.cardnumber, tty_buff, num);

		card_info.cardtype='B';
		}
	else{
		memcpy((uint8_t*)&tmp_number, &tty_buff[4], num);
		memcpy(door_card.cardnumber, &tty_buff[4], num);
		card_info.cardtype='A';
		}

	printf("door_card.cardnumber:  ");
    for (i = 0; i < 8; i++)
        printf("%02X ", door_card.cardnumber[i]);
    printf("\n");

	ret =query_door_card_list_number_8byte(&door_card, 0);

	printf("door_card.dev_no=%2x %2x %2x %2x \n",door_card.dev_no[0],door_card.dev_no[1],door_card.dev_no[2],door_card.dev_no[3]);
	printf("door_card.cardnumber= %llu \n",*(uint64_t *)door_card.cardnumber);
	printf("ret= %d,blacklist= %d,card_type= %d\n",ret,door_card.blacklist,door_card.card_type);

			
	if(ret > 0)
	{
		//card_info.cardtype=door_card.card_type;
		memcpy(devno,door_card.dev_no,4);
		memcpy(card_info.dev_no,door_card.dev_no,4);
		time_t now;
		struct tm *tnow;
		time(&now);
		tnow = localtime(&now);
		uint8_t current_date[6];
		current_date[0]=tnow->tm_year-100;
		current_date[1]=tnow->tm_mon+1;
		current_date[2]=tnow->tm_mday;
		current_date[3]=tnow->tm_hour;
		current_date[4]=tnow->tm_min;
		current_date[5]=tnow->tm_sec;
		memcpy(card_info.time,current_date,6);			//刷卡时间，[0~6]依次：年、月、日、时、分、秒

	
		if(door_card.blacklist)
		{
			buf[0] = SSIC;
			buf[1] = dev_cfg.my_code[0];
			buf[2] = buf[3] =0;
			buf[3] = 0;
			buf[4] = buf[5] = 0x0d;
			buf[6] = dev_cfg.my_code[3];
			buf[7] = buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6];
			uart_send_data(UART_1, buf);
			//buzz_play(BUZZ_WARNING);	
			card_info.result=1;			//开锁结果，	0：成功，1：黑名单，2：过期，3：本栋楼未授权，4：冻结
		}
		//日期判断的问题
		//	else if(door_card.next->exdate != (-1) && test_now >door_card.next->exdate)
		else if((compare_date(door_card.init_date,current_date)>0)||(compare_date(door_card.exprirydate,current_date)<0))
		{
			buf[0] = SSIC;
			buf[1] = dev_cfg.my_code[0];
			buf[2] = buf[3] =0;
			buf[3] = 0;
			buf[4] = buf[5] = 0x0e;
			buf[6] = dev_cfg.my_code[3];
			buf[7] = buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6];
			uart_send_data(UART_1, buf);
			//buzz_play(BUZZ_WARNING);			

			card_info.result=2;			//开锁结果，	0：成功，1：黑名单，2：过期，3：本栋楼未授权，4：冻结
		}
		else
		{
			card_info.result=0;			//开锁结果，	0：成功，1：黑名单，2：过期，3：本栋楼未授权，4：冻结
			memcpy(dest,&door_card.dev_no[0],4);
			
			buf[0] = SSIC;
			buf[1] = dev_cfg.my_code[0];
			memcpy(buf+2,dest+2,2);
			buf[4] = buf[5] = 0;
			buf[6] = dev_cfg.my_code[3];
			buf[7] = buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6];
			uart_send_data(UART_1, buf);//回应卡头

#ifndef FRONT_DOOR					
			data[0] = SSIC;
			data[2] = dest[2];
			data[3] = dest[3];
			//buzz_play(BUZZ_BEEP);
			uart_send_lift_data(data);//呼梯
			//if(!dev_cfg.my_code[3]){ //在新卡头程序中，主机也不分主副

            if ((data[2] != 0x00) && (data[3] != 0x00)) {			
                dest[0] = dev_cfg.my_code[0];
                dest[1] = dev_cfg.my_code[1];
                dest[2] = data[2];
                dest[3] = data[3];
#if 1 //modify and add by wrm 20150515 for 支持信存的层房卡头的撤防			
		//data[2] = data[7];
			//data[3] = data[8];
			memcpy(temp_buf,&data[2],6);
			memcpy(&data[3],temp_buf,6);
			data[1] =  dev_cfg.my_code[0];
			data[2] =  dev_cfg.my_code[1];
			//buf[0] = //
#endif		
                net_send_to_node(dest, GDATC, data, 10); //8 wrm

	        }
#endif
//}
		}
	}
	else  //卡号不存在
	{
		buf[0] = SSIC;
		buf[1] = dev_cfg.my_code[0];
		buf[2] = buf[3] =0;
		buf[3] = 0;
		buf[4] = buf[5] = 0x0f;
		buf[6] = dev_cfg.my_code[3];
		buf[7] = buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6];
		uart_send_data(UART_1, buf);
		//buzz_play(BUZZ_WARNING);
		
		time_t now;
		struct tm *tnow;
		time(&now);
		tnow = localtime(&now);
		uint8_t current_date[6];
		current_date[0]=tnow->tm_year-100;
		current_date[1]=tnow->tm_mon+1;
		current_date[2]=tnow->tm_mday;
		current_date[3]=tnow->tm_hour;
		current_date[4]=tnow->tm_min;
		current_date[5]=tnow->tm_sec;
		memcpy(card_info.time,current_date,6);			//刷卡时间，[0~6]依次：年、月、日、时、分、秒
		card_info.result=3;			//开锁结果，	0：成功，1：黑名单，2：过期，3：本栋楼未授权，4：冻结
		//card_info.cardtype=0;
	}
	
    int  pic_len = 0;
    char *pic_data = NULL;
    if(fkly_backup(&pic_data, &pic_len)){
        printf("[err] shoot_visitor_photo fail!\n");
    }    
	if(card_info.result==0){
        ui_action(UI_CARD_UNLOCK, 0);
        ui_action(UI_LCD_TURN_ON, 0);
    }
    
    //upload unlock records
	printf("card_info.result=%d\n",card_info.result);
	if((card_info.result==0)||(card_info.result==1)||(card_info.result==2))
	{		
       if(num==4){
    		buf[0] = SIC;
    		buf[1] = dev_cfg.my_code[0];
    		buf[2] = dev_cfg.my_code[1];
    		buf[3] = dev_cfg.my_code[3];
    		card_hex2bcd(card_hex, card_bcd);
    		card_bcdcopy(card_bcd, &buf[4]);
    		//buf[9]=buf[4];
    		//buf[4]=buf[5];
    		//buf[5]=buf[9];
    		int i=0;
    		for(i=0;i<9;i++)
    		{
    			buf[9]+=buf[i];
    		}
    		memset(dest, 0x00, 4);
    		usleep(100*1000);

    		net_send_to_node(dest, GDATC, buf, 10);	//发送刷卡记录至后台及管理机
        }

        usleep(100*1000);
    	send_card_buf[0]=0x01;
    	send_card_buf[1]=dev_cfg.my_code[0];
    	send_card_buf[2]=dev_cfg.my_code[1];
    	if(num==4){		   			       			
   			card_info.cardtype='A';   			
		}
		else{			   			   			
   			card_info.cardtype='B';
		}
       	memcpy(&card_info.cardnumber[0],&door_card.cardnumber[0],sizeof(card_info.cardnumber));
       	memcpy(&card_info.dev_no[0],&dev_cfg.my_code[0],sizeof(card_info.dev_no));

    	memcpy(&send_card_buf[3],&card_info,sizeof(card_info));
    	printf("\n++++++++++++++++++++\n");
    	for(i=0;i<27;i++)
		{
			printf(" %2x ",send_card_buf[i]);
			if(i%10==9)
				printf("\n");
		}
			
    	net_send_to_server(0x764, send_card_buf, 27);

	}

    usleep(200*1000);
    //刷卡拍录上传
    char crendence_id[30]={0},usr_id[10]={0};//add for photo upload
    if(num == 4){
        snprintf(crendence_id,sizeof(crendence_id),"%llu",*(uint64_t *)door_card.cardnumber); 
    }
    else{
        snprintf(crendence_id,sizeof(crendence_id),"%llu",*(uint64_t *)door_card.cardnumber); 
    }
    snprintf(usr_id,sizeof(usr_id),"%02x%02x%02x%02x",door_card.dev_no[0],door_card.dev_no[1],door_card.dev_no[2],door_card.dev_no[3]);
    if(card_info.result == 0){
        send_enter_record_to_server(CARD_UNLOCK_RECORD,crendence_id,usr_id,pic_data, pic_len);
    }
    else{
        send_enter_record_to_server(CARD_UNLOCK_FAIL_RECORD,crendence_id,usr_id,pic_data, pic_len);
    }		
								
	return 0;
}

#endif

static void proc_new_cardreader(unsigned char *data)
{
	unsigned char dest[4]={0};
	unsigned char buf[10]={0};
	unsigned char card_hex[8], card_bcd[8],temp_buf[10];
	unsigned char room[4]={0};
	memcpy(card_hex, &data[3], 4);
	int ret = 1;
	ret = is_card_unlock_ex(card_hex2int(&data[3]), dest);
	memcpy(room,dest,4);
	printf("unlock_new_cardreader %d\n",ret);
	switch(ret){
		case 0: //开锁
			buf[0] = SSIC;
			buf[1] = dev_cfg.my_code[0];
			memcpy(buf+2,dest+2,2);
			buf[4] = buf[5] = 0;
			buf[6] = dev_cfg.my_code[3];
			buf[7] = buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6];
			uart_send_data(UART_1, buf);//回应卡头
			
			data[0] = SSIC;
			data[2] = dest[2];
			data[3] = dest[3];
			buzz_play(BUZZ_BEEP);
			ui_action(UI_CARD_UNLOCK, 0);
                    usleep(5);
			ui_action(UI_LCD_TURN_ON, 0);
#ifndef FRONT_DOOR
			uart_send_lift_data(data);//呼梯
			//if(!dev_cfg.my_code[3]){ //在新卡头程序中，主机也不分主副
	            if ((data[2] != 0x00) && (data[3] != 0x00)) {			
	                dest[0] = dev_cfg.my_code[0];
	                dest[1] = dev_cfg.my_code[1];
	                dest[2] = data[2];
	                dest[3] = data[3];
#if 1 //modify and add by wrm 20150515 for 支持信存的层房卡头的撤防			
					//data[2] = data[7];
					//data[3] = data[8];
					memcpy(temp_buf,&data[2],6);
					memcpy(&data[3],temp_buf,6);
					data[1] =  dev_cfg.my_code[0];
					data[2] =  dev_cfg.my_code[1];
					//buf[0] = //
#endif		
                	net_send_to_node(dest, GDATC, data, 10); //8 wrm

            	}
#endif
			//}
			
			break;
		case -1: //不在有效期内
			buf[0] = SSIC;
			buf[1] = dev_cfg.my_code[0];
			buf[2] = buf[3] =0;
			buf[3] = 0;
			buf[4] = buf[5] = 0x0e;
			buf[6] = dev_cfg.my_code[3];
			buf[7] = buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6];
			uart_send_data(UART_1, buf);
			buzz_play(BUZZ_WARNING);			
			break;
		case -2: //无此卡号
			buf[0] = SSIC;
			buf[1] = dev_cfg.my_code[0];
			buf[2] = buf[3] =0;
			buf[3] = 0;
			buf[4] = buf[5] = 0x0f;
			buf[6] = dev_cfg.my_code[3];
			buf[7] = buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6];
			uart_send_data(UART_1, buf);
			buzz_play(BUZZ_WARNING);				
			break;
		case -3: //黑名单卡
			buf[0] = SSIC;
			buf[1] = dev_cfg.my_code[0];
			buf[2] = buf[3] =0;
			buf[3] = 0;
			buf[4] = buf[5] = 0x0d;
			buf[6] = dev_cfg.my_code[3];
			buf[7] = buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6];
			uart_send_data(UART_1, buf);
			buzz_play(BUZZ_WARNING);	
			break;
		default:
			buzz_play(BUZZ_WARNING);				
			break;
		
	}


	if(ret == 0||ret == -3){
	
		buf[0] = SIC;
		buf[1] = dev_cfg.my_code[0];
		buf[2] = dev_cfg.my_code[1];
		buf[3] = dev_cfg.my_code[3];
		card_hex2bcd(card_hex, card_bcd);
		card_bcdcopy(card_bcd, &buf[4]);
		memset(dest, 0x00, 4);
		usleep(800*1000);//add by wrm 20150703 this delay is needed otherwise can not get the right ip 
			//can modify the get_ip_list like 15c，具体可参考15c中的相关代码		
		net_send_to_server(GDATC, buf, 10);
		net_send_to_node(dest, GDATC, buf, 10);	//发送刷卡记录至后台及管理机
	
	}
	/*if(ret == 0 && 加入对副主机的支持，副主机如何判断? 即返回1B){
		dest[0] = dev_cfg.my_code[0];
		dest[1] = dev_cfg.my_code[1];
		dest[2] = 0x00;
		dest[3] = data[?]-1;// -1恢复副门口机
		buf[0] = SSIC;
		buf[1] = dev_cfg.my_code[0];				
		buf[6] = dest[3];
		buf[7] = room[2];
		buf[8] = room[3];
		memcpy(&buf[2], &raw_data[3], 4);
		net_send_to_node(dest, GDATC, buf, 10);		
	}*/
}


/*串口数据处理函数*/
static int proc_data_uart0(unsigned char *data) 
{
    //unsigned char dest[4]={0};
#ifdef APP_DEBUG //by mkq 20170916
    /*int i;
	 printf("uart0_rcev_data:  ");
    for (i = 0; i < UART0_FRAME_LEN; i++)
        printf("%02X ", data[i]);
    printf("\n");
	  printf("gbl_finger_status is :%02x--------\n",fp_status_get());
	  */
#endif  //by mkq 20170916
#if 1
    switch(data[0]){
		case 0xEF:
			if(data[1] == 0x01)
				proc_fpdata_uart(data);
		break;
	}
#endif
#if 0
    switch(data[0]) {
        case SIC :      //0x44
            memset(dest, 0x00, 4);
            net_send_to_server(GDATC, data, 10);
            net_send_to_node(dest, GDATC, data, 10);
            break;
        case SSOK:
            net_send_to_server(GDATC, data, 10);
            break;
        default:
            break;
    }
#endif
    return 0;
}

static int proc_data_uart1(unsigned char *data)
{
    unsigned char dest[4]={0};
    unsigned char buf[MSG_LEN_PROC]={0};//,temp_buf[10]={0};
    static unsigned char card[4];
    unsigned char floor, room;
    unsigned char self_unlock = 0;	
    static unsigned char sicbuf[10];	
    int is_new =0,i=0;	

#ifdef	BLE
    static unsigned char ble_buf[320] ={0};
    static unsigned char frame_num; 
    static unsigned char version = 0x01;
    static unsigned int begin = 0;
    unsigned char liftdata[10];
    int diff_ms = 0;
    static struct timeval tv_pre, tv_now;  
    static unsigned char seed_key[16]={0};
    static unsigned char is_this_f_and_v = 0;
    int ble_check_result = 0;
    char usr_id[10]={0};//add for photo upload
#endif 

#ifdef		QR
    static unsigned char qr_data[6];
#endif

#ifdef APP_DEBUG //by mkq 20170916
    //int i;
    if(data[0] == 0x7F){
    	printf("uart1_rev_data: ");
    	for(i = 0;i < 21;i++)
    		printf("%02X ",data[i]);
    		printf("\n");
    }
    else{
        printf("uart1_rcev_data:  ");
        for (i = 0; i < UART1_FRAME_LEN+3; i++)
            printf("%02X ", data[i]);
        printf("\n");
    }
#endif  //by mkq 20170916
    switch(data[0])
    {     
#ifdef DOOR_ACCESS_CARD_LIST    
    	case 0x59:      //0x59
    		proc_sfz8or4_cardreader(&data[1],8);
    	break;
#endif
                    
        case SSIC:      //0x1B
            floor = data[2];
            room  = data[3];
            self_unlock = 0;			
            if (dev_cfg.my_code[3] == data[6]){
                ui_action(UI_CARD_UNLOCK, 0);
                usleep(150*1000);
                ui_action(UI_LCD_TURN_ON, 0);
                uart_send_lift_data(data);
    		  self_unlock = 1;			
    		 // printf("test..........\n");
    		  //net_send_to_node(dest, 0x564, data, 10);
            }
            
            else
			if ((dev_cfg.my_code[3] == 0x00)&&(data[6]==gbl_data_get(&gbl_sic_subunit))) {               
                //增加卡号让副门口校验 0x1B 楼 卡1 卡2 卡3 卡4 主副 层 房 校验和

                printf("+++++++++++++zhu to fu +++++++++++++++++++++\n");
                gbl_data_set(&gbl_sic_subunit, 0x00);                  //增加卡号让副门口校验 0x1B 楼 卡1 卡2 卡3 卡4 主副 层 房 校验和
                dest[0] = dev_cfg.my_code[0];
                dest[1] = dev_cfg.my_code[1];
                dest[2] = 0x00;
                dest[3] = data[6];

                data[7] = data[2];
                data[8] = data[3];
                memcpy(&data[2], card, 4);
                net_send_to_node(dest, GDATC, data, 10);
            }  


#ifndef FRONT_DOOR
            if ((floor != 0x00) && (room != 0x00)) {
                dest[0] = dev_cfg.my_code[0];
                dest[1] = dev_cfg.my_code[1];
                dest[2] = floor;
                dest[3] = room;
    #if 0 //modify and add by wrm 20150515 for ????????????????			
        	//	data[2] = data[7];
        	//	data[3] = data[8];
        		memcpy(temp_buf,&data[2],6);
        		memcpy(&data[3],temp_buf,6);
        		data[1] =  dev_cfg.my_code[0];
        		data[2] =  dev_cfg.my_code[1];
        		data[5] = 0x00;
    #endif

    #if  1//modify and add by XXXXX 20161008 		
        		//data[2] = data[7];
        		//data[3] = data[8];
        		//memset(&data[1],0,10);	
        		//memcpy(temp_buf,&data[2],6);
        		//memcpy(&data[3],temp_buf,6);
        		data[1] =  dev_cfg.my_code[0];
        		data[2] =  dev_cfg.my_code[1];
        		data[3] = floor;
                      data[4] = room;
        		data[5] = 0x00;
        		data[7] = floor;
                      data[8] = room;
        		for(i = 0;i<10;i++)
        			printf("%02X ",data[i]);
        		printf("\n");
    #endif
            
                net_send_to_node(dest, GDATC, data, 10); //8 wrm

            }
#endif

			memset(dest, 0x00, 4);
			if (self_unlock)
			{
					sicbuf[3] = dev_cfg.my_code[3];
			} 
			usleep(800*1000);//add by wrm 20150703 this delay is needed otherwise can not get the right ip 
			//can modify the get_ip_list like 15c
			net_send_to_server(GDATC, sicbuf, 10);
			net_send_to_node(dest, GDATC, sicbuf, 10);

            break;
        case SIC :      //0x44
  /*        buf[0] = data[0];
            buf[1] = dev_cfg.my_code[0];
            buf[2] = dev_cfg.my_code[1];
            buf[3] = gbl_data_get(&gbl_sic_subunit);
            card_bcdcopy(&data[2], &buf[4]);
            memset(dest, 0x00, 4);
            card_bcd2hex(&data[2], card);
            gbl_data_set(&gbl_sic_subunit, 0x00);
            net_send_to_server(GDATC, buf, 10);
            net_send_to_node(dest, GDATC, buf, 10);*/
            sicbuf[0] = data[0];            
			sicbuf[1] = dev_cfg.my_code[0];            
			sicbuf[2] = dev_cfg.my_code[1];
			if(gbl_data_get(&gbl_sic_subunit))
			sicbuf[3] = gbl_data_get(&gbl_sic_subunit);
			else
			sicbuf[3] = 0;
			sicbuf[4] = data[3]; 
			sicbuf[5] = data[2];
			sicbuf[6] = data[4];
			sicbuf[7] = data[5];
			sicbuf[8] = data[6]; 
			card_bcd2hex(&data[2], card);
			//gbl_data_set(&gbl_sic_subunit, 0x00);  
            break;
        case 0x1C:
		//if((data[1] == 0xAA) &&(data[2] == 0xBB))
		if(data[1] == 0xAA){
			if (data[2]==0x88) 
				is_new = 2;
			else
				is_new = 2;
		}
		else
			is_new = 0;
			
		if (is_new == 1){//判断是新卡头且是主门口机
			proc_new_cardreader(data);
		}
		else if (is_new==2)
			proc_sfz8or4_cardreader(&data[3],4);
		else{
			if (dev_cfg.card_in_flash){
				if (dev_cfg.my_code[3]) {
					dest[0] = dev_cfg.my_code[0];
					dest[1] = dev_cfg.my_code[1];
					dest[2] = 0x00;
					dest[3] = 0x00;
					net_send_to_node(dest, GDATC, data, 10);
					buzz_play(BUZZ_BEEP);
					gbl_data_set(&gbl_wait_ssic, 1);
					tim_reset_time(tim_ssic_timeout, TIME_1S(4));
				}
				else
				{
					unsigned char card_hex[8], card_bcd[8],temp_data[10];
					memcpy(card_hex, &data[3], 4);
					if (is_card_unlock(card_hex2int(&data[3]), dest)) {
						data[0] = SSIC;
						data[2] = dest[2];
						data[3] = dest[3];
						buzz_play(BUZZ_BEEP);
						ui_action(UI_CARD_UNLOCK, 0);
						ui_action(UI_LCD_TURN_ON, 0);
						uart_send_lift_data(data);
						//??????????????
					    for (i = 0; i < UART1_FRAME_LEN; i++)
           					printf("%02X ", data[i]);
						printf("\n");
#ifndef FRONT_DOOR
						if ((dest[2] != 0x00) && (dest[3] != 0x00)) {
							dest[0] = dev_cfg.my_code[0];
							dest[1] = dev_cfg.my_code[1];
							data[1] = dev_cfg.my_code[0];	//￥??
							data[6] = dev_cfg.my_code[3];	//????
    #if 1        //add by wrm 20150513 使发送数据变为 1B 楼号 单元号(添加单元号) ...为兼容信息存储器转发撤防             
	                        memcpy(temp_data,&data[2],6);
            				memcpy(&data[3],temp_data,6);			
            				data[2] = 	dev_cfg.my_code[1]; //单元号		
	                        printf("1C Lock On\n");			
    #endif						
							net_send_to_node(dest, GDATC, data, 10);//8 modify by wrm 20150513
						}
#endif
						buf[0] = SIC;
						buf[1] = dev_cfg.my_code[0];
						buf[2] = dev_cfg.my_code[1];
						buf[3] = dev_cfg.my_code[3];
						card_hex2bcd(card_hex, card_bcd);
						card_bcdcopy(card_bcd, &buf[4]);
						memset(dest, 0x00, 4);
						net_send_to_server(GDATC, buf, 10);
						net_send_to_node(dest, GDATC, buf, 10);
					}
					else {
						buzz_play(BUZZ_WARNING);
					}
				}
			}
			else{
				if (!dev_cfg.my_code[3])  break;
				dest[0] = dev_cfg.my_code[0];
				dest[1] = dev_cfg.my_code[1];
				dest[2] = 0x00;
				dest[3] = 0x00;
				memcpy(sub_card, &data[3], 4);
				net_send_to_node(dest, GDATC, data, 10);
				buzz_play(BUZZ_BEEP);
				gbl_data_set(&gbl_wait_ssic, 1);
				tim_reset_time(tim_ssic_timeout, TIME_1S(4));
			}
    	}
			break;
        case SSOK:
            if (dev_cfg.my_code[3])   break;
            buf[0] = data[0];
            buf[1] = dev_cfg.my_code[0];
            buf[2] = dev_cfg.my_code[1];
            buf[3] = data[2];
            buf[4] = data[3];
            buf[5] = 0x00;
            buf[6] = data[4];
            buf[7] = data[5];
            net_send_to_server(GDATC, buf, 10);
#ifdef CFG_USE_OLD_CARDREADER
            ui_action(UI_UART1_GOT_DATA, SSOK);
#endif
            break;
#if 0
	 case 0XA6:                    //by mkq for songjian 20170907
            buf[0] = data[0];
            buf[1] = dev_cfg.my_code[0];
            buf[2] = dev_cfg.my_code[1];
            buf[3] = dev_cfg.my_code[3];
            buf[4] = data[2];
            buf[5] = data[3];
            buf[6] = data[4];
            buf[7] = data[5];
            buf[8] = data[6];
            buf[9] = data[7]+buf[1]+buf[2]+buf[3];
            net_send_to_server(GDATC, buf, 10);
            
			break;

	case 0X42:         //by mkq for songjian 20170907
         if(data[6]==0x99)
          {
            buf[0] = data[0];
            buf[1] = dev_cfg.my_code[0];
            buf[2] = dev_cfg.my_code[1];
            buf[3] = data[1];
            buf[4] = data[2];
            buf[5] = data[3];
            buf[6] = data[4];
            buf[7] = data[5];
            buf[8] = data[6];
            buf[9] = data[7]+buf[1]+buf[2];             
            net_send_to_server(GDATC, buf, 10);
          }
          
         if(data[6]==0x9A)
          {
            buf[0] = data[0];
            buf[1] = dev_cfg.my_code[0];
            buf[2] = dev_cfg.my_code[1];
            buf[3] = data[1];
            buf[4] = data[2];
            buf[5] = data[3];
            buf[6] = data[4];
            buf[7] = data[5];
            buf[8] = data[6];
            buf[9] = data[7]+buf[1]+buf[2];             
            net_send_to_server(GDATC, buf, 10);
          }
			
			break;
#endif			
			
#ifdef QR
    	case OR_DATA:
    		printf("-----------QR data!\n");
    		buf[0] = MSG_FROM_UI;
    		buf[1] = UI_SEND_AJB_DATA;
    		buf[2] = GETUK;
    		qr_data[0] = HexToChar(data[2]);
    		qr_data[1] = HexToChar(data[3]);
    		qr_data[2] = HexToChar(data[4]);
    		qr_data[3] = HexToChar(data[5]);
    		qr_data[4] = HexToChar(data[6]);
    		qr_data[5] = HexToChar(data[7]);
    		buf[3] = qr_data[0]*16+qr_data[1];
    		buf[4] = qr_data[2]*16+qr_data[3];
    		buf[5] = qr_data[4]*16+qr_data[5];
    		buf[6] = 'Q';
    		//if(proc_state == PROC_NORMAL){
    		//ui_action(UI_TWO_CODE_UNLOCK, 1);
    		tim_reset_time(tim_wait_tc_pw_timeout, TIME_1S(3));
    		printf("QR:");
			for(i= 0;i<6;i++)
    			printf("%02X       %02X\n",buf[i],qr_data[i]);
    		printf("\n");
    		pipe_put(proc_pipe_proc,buf, MSG_LEN_PROC);
    		//}

            /**add for photo upload**/
            memcpy(&QR_code[0],&data[2],6);
            /**end**/			
    		break;
#endif

#ifdef	BLE
    	case BLTH_CELLPHONE:
    		printf("bluetooth data !\n");
    		gettimeofday(&tv_now, NULL);
    		 if(data[1] == 0x01)
    		 { 
    		 	printf("frame_num:%d\n",frame_num+1);   
    			diff_ms = (tv_now.tv_sec - tv_pre.tv_sec) * 1000 + (tv_now.tv_usec - tv_pre.tv_usec) / 1000;
    			printf("diff_ms :%d \n",diff_ms);
    			tv_pre.tv_sec  = tv_now.tv_sec;
            		tv_pre.tv_usec = tv_now.tv_usec;
    			
    			if(VERSION_TIME == (data [2] & 0xF0)||VERSION_VI == (data [2] & 0xF0)){
    				if(diff_ms >1500){
    					printf("diff_ms >1500 \n");
    					is_this_f_and_v = 0;
    					frame_num = 0;
    					memset(&ble_buf, 0, sizeof(ble_buf));
    				}
    			}
    			else{
    				if(diff_ms >600){
    					printf("diff_ms >600 \n");
    					is_this_f_and_v = 0;
    					frame_num = 0;
    					memset(&ble_buf, 0, sizeof(ble_buf));
    				}
    			}
    				
    			if(frame_num < (sizeof(ble_buf)/16))
    				memcpy(&ble_buf[frame_num*16],&data[3],16);
    			printf("frame_num:%d The buf: ",frame_num);
    			for(i = 0 ;i < (frame_num+1)*16;i++)
    				printf("%02X ",ble_buf[i]);
    			printf("\n");
    			printf("Version: %02X \n",data[2]);
    			if( VERSION_1 == (data [2] & 0xF0))
    				memcpy(&seed_key,seed_key1,sizeof(seed_key1));
    			else if ( VERSION_2 == (data [2] & 0xF0))
    				memcpy(&seed_key,seed_key2,sizeof(seed_key2));
    			else if( VERSION_3 == (data [2] & 0xF0))
    				memcpy(&seed_key,seed_key3,sizeof(seed_key3));

    			
    			else if( VERSION_TIME == (data [2] & 0xF0)){
    				//check time .....
    				printf("Start updata......time is_this_f_and_v = %d\n",is_this_f_and_v);
    				if(is_this_f_and_v == 0x01)
    					check_ble_data(ble_buf, frame_num,seed_key,VERSION_TIME);
    				is_this_f_and_v = 0;
    				break;
    			}
    			/*else if( VERSION_VI == (data [2] & 0xF0)){
    				if(vs_cfg.my_village_code[0] == 0xFF&&vs_cfg.my_village_code[1] == 0xFF\
    					&&vs_cfg.my_village_code[2] == 0xFF)
    					check_ble_data(ble_buf, frame_num,seed_key,VERSION_VI);
    				break;
    			}*/
    			else if( VERSION_SEED == (data [2] & 0xF0)){ 
    				//if(vs_cfg.my_village_code[0] == 0xFF&&vs_cfg.my_village_code[1] == 0xFF\
    					//&&vs_cfg.my_village_code[2] == 0xFF){
    					if(vs_cfg.has_set == 0){
    					printf("Start updata.....village and seed!\n");
    					check_ble_data(ble_buf,0,seed_key_updata,VERSION_VI);
    				       check_ble_data(&ble_buf[16], 0,seed_key_updata,VERSION_SEED);
    					send_door_status_ble(0xD0, 0X01);
    					build_key(default_seed,vs_cfg.dev_seed_data);
    					buzz_play(BUZZ_CONFIRM);
    				}
    				else{
    					printf("Sorry!The device has been set!\n");
    					buzz_play(BUZZ_WARNING);	
    					send_door_status_ble(0xD0, 0X00);
    				}
    				break;
    			}

    			ble_check_result = check_ble_data(ble_buf, frame_num,seed_key,(data [2] & 0xF0));
    			if(vs_cfg.has_set == 1){
    				if((data [2] & 0xF0) < VERSION_3){
    					ble_check_result = -1;
    					printf("Sorry! your version is invalid!\n");
    				}
    				else
    					printf("OK!your version is valid!\n");
    			}

                int  pic_len = 0;
                char *pic_data = NULL;
                if(fkly_backup(&pic_data, &pic_len)){
                    printf("[err] shoot_visitor_photo fail!\n");
                }       			
    			switch (ble_check_result)
    			{
    				case MATCHING_OK:
    					ui_action(UI_CARD_UNLOCK, 0);
    					ui_action(UI_LCD_TURN_ON, 0);
    					printf("data=%X,data1=%X,",decrypt_data.room_nu[0],decrypt_data.room_nu[1]);
    					liftdata[0]=SSIC;
    					liftdata[2]=decrypt_data.room_nu[0];
    					liftdata[3]=decrypt_data.room_nu[1];
    					uart_send_lift_data(liftdata);
    					liftdata[2]=decrypt_data.my_code[0];   //add 20161226
    					liftdata[3]=decrypt_data.my_code[1];
    					liftdata[4]=decrypt_data.room_nu[0];
    					liftdata[5]=decrypt_data.room_nu[1];				
    					send_door_status_ble(version,1);
                        /**add for photo upload**/
                        snprintf(usr_id,sizeof(usr_id),"%02x%02x%02x%02x",decrypt_data.my_code[0],decrypt_data.my_code[1],\
                            decrypt_data.room_nu[0],decrypt_data.room_nu[1]);                         
                        send_enter_record_to_server(BULETOOTH_UNLOCK_RECORD,&g_ble_buf[0],usr_id,pic_data,pic_len);                        
                        /**end**/
    					net_send_unlock_records(&liftdata[2],BLE_RECORD_OPENDOOR);
    					is_this_f_and_v = 1;
        #if 0						
            #ifndef FRONT_DOOR	//撤防				
    					
    		            if ((data[2] != 0x00) && (data[3] != 0x00)) {
    		            	data[2]=decrypt_data.room_nu[0];
    						data[3]=decrypt_data.room_nu[1];
    		                dest[0] = dev_cfg.my_code[0];
    		                dest[1] = dev_cfg.my_code[1];
    		                dest[2] = data[2];
    		                dest[3] = data[3];
    						memcpy(temp_buf,&data[2],6);
    						memcpy(&data[3],temp_buf,6);
    						data[1] =  dev_cfg.my_code[0];
    						data[2] =  dev_cfg.my_code[1];		
                			net_send_to_node(dest, GDATC, data, 10); //8 wrm

            			}
            #endif
        #endif
    						
						break;
					case MATCHING_TIMEOUT:
						is_this_f_and_v = 1;
						buzz_play(BUZZ_WARNING);
						send_door_status_ble(version,2);
                        snprintf(usr_id,sizeof(usr_id),"%02x%02x%02x%02x",decrypt_data.my_code[0],decrypt_data.my_code[1],\
                            decrypt_data.room_nu[0],decrypt_data.room_nu[1]); 						
						send_enter_record_to_server(BULETOOTH_UNLOCK_FAIL_RECORD,&g_ble_buf[0],usr_id,pic_data,pic_len); //add for photo upload 
						break;
					default:
						is_this_f_and_v = 0;
						buzz_play(BUZZ_WARNING);
						send_door_status_ble(version,0);
                        snprintf(usr_id,sizeof(usr_id),"%02x%02x%02x%02x",decrypt_data.my_code[0],decrypt_data.my_code[1],\
                            decrypt_data.room_nu[0],decrypt_data.room_nu[1]); 						
						send_enter_record_to_server(BULETOOTH_UNLOCK_FAIL_RECORD,&g_ble_buf[0],usr_id,pic_data,pic_len); //add for photo upload 
						break;
    			
				}
				memset(ble_buf,0,sizeof(ble_buf));
				frame_num = 0;
			}
    		else
    		{	
		 		if(begin == 0){
					memset(&tv_pre, 0, sizeof(tv_pre));
		 			gettimeofday(&tv_now, NULL);
					tv_pre.tv_sec  = tv_now.tv_sec;
					tv_pre.tv_usec = tv_now.tv_usec;
					begin = 1;
				}
				diff_ms = (tv_now.tv_sec - tv_pre.tv_sec) * 1000 + (tv_now.tv_usec - tv_pre.tv_usec) / 1000;
				printf("diff_ms :%d \n",diff_ms);
				tv_pre.tv_sec  = tv_now.tv_sec;
        			tv_pre.tv_usec = tv_now.tv_usec;
				if(diff_ms >600){
					memset(&ble_buf, 0, sizeof(ble_buf));
				       frame_num = 0;
					is_this_f_and_v = 0;
		 		}
		 		if(frame_num < (sizeof(ble_buf)/16))
					memcpy(&ble_buf[frame_num*16],&data[3],16);
				frame_num =frame_num + 1;
				//printf("---frame_num:%d\n",frame_num);
        	}
    	break;
#endif
        default:
            break;
    }
    return 0;
}

int  door_status = DOOR_UNKNOWN;
static void proc_data_net(unsigned char *data) 
{ 
    int             allow     = 0;                    //0允许     -1不允许
    int             resstatus = 0;
    short           respond   = 0;                    //0操作成功 1失败
    //unsigned char   respond_buf[6];
    //int ret;
    unsigned int    net_cmd, net_lip, link_num, link_type, oper_type;
    unsigned char   buf[32];
    unsigned char   liftdata1[10];
    unsigned char   *raw_data;
    struct in_addr  ip_in_addr;	

    ajb_msg_ret     *p_ret = (ajb_msg_ret *)data;   //net return 
    ajb_msg         *p_msg = (ajb_msg *)data;       //net msg, ajb data 518
    
    net_cmd   = p_ret->data.net_cmd;
    net_lip   = ip_in_addr.s_addr = get_current_connect_ip(p_msg->num);

    link_type = (0 == memcmp(&net_lip, dev_cfg.ip_server, 4)) ? 1 : 0; //1: server
    raw_data  = &p_msg->data.cmd;
    link_num  = p_msg->num;
    oper_type = p_msg->data.oper_type;
    resstatus = p_msg->data.resstatus;

    time_t now;
    struct tm *tnow;
    char now_date[20] = {'\0'};
    char now_time[20] = {'\0'};
    int i = 0;
#ifdef APP_DEBUG
    printf("net_receive_data: ");
    printf("cmd %03X, ", p_msg->data.net_cmd);
    printf("data %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", \
            raw_data[0], raw_data[1], raw_data[2], raw_data[3], raw_data[4], raw_data[5], raw_data[6], raw_data[7], raw_data[8], raw_data[9]);
#endif
    switch(p_msg->data.net_cmd) {
        	case RVNUMC:
		printf("net data %02X %02X %02X \n", \
            	raw_data[0], raw_data[1], raw_data[2]);
#if 0
		int i = 0;
		for(i = 0;i<6;i ++)
			buf[i]=HexToChar(raw_data[i]);
		buf [0] = buf[0]*16 +buf[1];
		buf [1] = buf[2]*16 +buf[3];
		buf [2] = buf[4]*16 +buf[5];
		printf("NET Recv buf: %02X %02X %02X\n",buf[0],buf[1],buf[2]);
#endif
		//memcpy(dev_cfg.my_village_code,raw_data,3);
             
		if(vs_cfg.has_set == 0){
			cloud_server_state = DEST_ONLINE;
			memcpy(vs_cfg.my_village_code,raw_data,3);
			dev_config_save();
			usleep(200);
			//dev_config_get();
			dev_sv_get();
			//build_key(seed_data);
			//my_village_code_to_Ble();
			build_key(default_seed,vs_cfg.dev_seed_data);
			my_new_village_code_to_Ble();
			printf("The village set sucess!\n");
		}
		else{
			printf("The village set faild!\n");
			if(vs_cfg.my_village_code[0] ==raw_data[0] &&vs_cfg.my_village_code[1] ==raw_data[1]\
				&&vs_cfg.my_village_code[2] ==raw_data[2] )
				cloud_server_state=DEST_ONLINE;
		}
		break;

	case GETUNLOCKC:
			printf("net data %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", \
            	raw_data[0], raw_data[1], raw_data[2], raw_data[3], raw_data[4], raw_data[5], raw_data[6], raw_data[7], raw_data[8], raw_data[9]);
/**modified for photo upload**/
            check_result_t check_result={};
            char usr_id[10]={0};            
            memcpy(&check_result,raw_data,sizeof(check_result_t));
            
            int  pic_len = 0;
            char *pic_data = NULL;
            if(fkly_backup(&pic_data, &pic_len)){
                printf("[err] shoot_visitor_photo fail!\n");
            }               
            
		    if(check_result.result == 0x01)
		    {       
                if(check_result.type == 'Q'){
                    printf("unlock_type is erweima!\n");
                    tim_suspend_event(tim_wait_tc_pw_timeout);
                    ui_action(UI_CARD_UNLOCK, 0);
                    //add for photo upload
                    snprintf(usr_id,sizeof(usr_id),"%02x%02x%02x%02x",check_result.household_no[0],check_result.household_no[1],\
                        check_result.household_no[2],check_result.household_no[3]);
                    send_enter_record_to_server(QR_UNLOCK_RECORD,&QR_code[0],usr_id,pic_data,pic_len); 
                }else{
                    ui_action(UI_SB_PW_UNLOCK,1);
                }
                liftdata1[0]=SSIC;
                liftdata1[2]=check_result.household_no[2];
                liftdata1[3]=check_result.household_no[3];
                uart_send_lift_data(liftdata1);
                printf("raw_data[9]:%02x %C\n",raw_data[9],raw_data[9]);
                net_send_unlock_records(&raw_data[5],raw_data[9]);
			}
	        else if(check_result.result == 0x00){
                if(raw_data[9]=='Q'){
                    tim_suspend_event(tim_wait_tc_pw_timeout);
                    ui_action(UI_WAIT_TC_PW_TIMEOUT,1);
                    //add for photo upload
                    snprintf(usr_id,sizeof(usr_id),"%02x%02x%02x%02x",check_result.household_no[0],check_result.household_no[1],\
                        check_result.household_no[2],check_result.household_no[3]);                    
                    send_enter_record_to_server(QR_UNLOCK_FAIL_RECORD,&QR_code[0],usr_id,pic_data,pic_len); 
                }else{
                    ui_action(UI_WAIT_SB_PW_TIMEOUT,1); 
                }
	        }  
/**end**/    
	break;
        case STASC:
            ui_action(UI_NET_LINK_STAT, p_ret->data.resstatus==0 ? NET_LINK_ON: NET_LINK_OFF);    
            break;
        case RFJJC: 
            dev_set_time(2000+raw_data[4], raw_data[5], raw_data[6], raw_data[7], raw_data[8], raw_data[9]);
            ui_action(UI_STBAR_REFRESH, 0);
            break;
         case CJINC:
            {
                net_addr_msg *ptr = (net_addr_msg*)data;
                char *tmp_buf = alloca(ptr->data_len+1024);
                if(tmp_buf != NULL)
                {
                    int bytes = get_list_data(ptr->list_num, tmp_buf);
                    save_info_container_data(tmp_buf, bytes);
                    key_buzz(BUZZ_CONFIRM);
                } else {
                    perror("alloca:");
                }
                respond = 0;
                net_respond_server(link_num, RCJIC, &respond, 2);
                //sleep(1);
                //dev_app_restart();
            }
            break;
            case HQFJC:
            {
                char *tmp_buf = alloca(50*1024);
                int tmp=0;
                int ret = get_all_info_container_data(tmp_buf, 50*1024);
                if(ret>=0)
                {
                	tmp= (ret-6)/4;
                	memcpy(tmp_buf, dev_cfg.my_code, 2);
                	memcpy(&tmp_buf[2], &tmp, 2);
	                memcpy(&tmp_buf[4], &tmp, 2);
                    net_respond_server(link_num, RHQFC, tmp_buf, ret);
                }
            }
            break;
#ifdef DOOR_ACCESS_CARD_LIST

#if 0

	case SNMJKC:// respond door card_list to server
		{
			
			net_addr_msg *ptr = (net_addr_msg*)data;    
			uint8_t *buf = NULL;
			int ret;
			ret = load_ndoor_card_inf(0x01, dev_cfg.my_code, &buf);
			dbg_lo("ret=%d, buf=%08X\n", ret, (size_t)buf);
			if(ret > 0 && NULL != buf) {				
				ack_net_to_server(RNMJKC, buf, ret, ptr->num);
				free(buf);
			} else {
				buf = alloca(4);
				ack_net_to_server(RNMJKC, buf, 0, ptr->num);
			}
		}
			break;
		
		

		case UNMJKC:
		{
		
				printf("+++++++++++++++762+++++++++++++++\n");
				#if 0
                net_addr_msg *ptr = (net_addr_msg*)data;
                char *tmp_buf = alloca(ptr->data_len+1024);
                if(tmp_buf != NULL)
                {
                    int bytes = get_list_data(ptr->list_num, tmp_buf);                    
                    store_ndoor_card_info(ptr->list_num, ptr->data_len+1024,1);
                    key_buzz(BUZZ_CONFIRM);
                } else {
                    perror("alloca:");
                }
                #else
                net_addr_msg *ptr = (net_addr_msg*)data; 
                int ret;
                ret=store_ndoor_card_info(ptr->list_num, ptr->data_len+1024,0);
                if(ret < 0) {
					ack_net_to_server(ANMJKC, &ret, 2, ptr->num);
					break;
				}
                #endif
                printf("+++++++++++++++762+++++++++++++++\n");
                respond =0;
				respond_buf[0]= 1;				
                memcpy(&respond_buf[1], dev_cfg.my_code, 2);
	            memcpy(&respond_buf[3], &respond, 2);
                net_respond_server(link_num, ANMJKC, &respond_buf,5);
               	
		}
		break;
#else
	case SNMJKC://新协议  进入门禁信息
		{
			printf("+++++++++++++++760+++++++++++++++\n");
			net_data_msg *ptr=(net_data_msg *)(data);
			uint32_t bytes=0;
			uint8_t *tmp_buf =do_load_door_card_number_8byte_inf(&bytes);
			if(tmp_buf!=NULL)
			{
				tmp_buf[0]=1;
				tmp_buf[1]=0;
				memcpy(tmp_buf+2, dev_cfg.my_code, 2);
				ack_net_to_server(RNMJKC, tmp_buf, bytes, ptr->num);
				free(tmp_buf);
			}
			else
			{
				uint8_t *tmp_buf=alloca(8);
				tmp_buf[0]=1;
				tmp_buf[1]=0;
				memcpy(tmp_buf+2, dev_cfg.my_code, 2);
				bytes=0;
				memcpy(tmp_buf+4, &bytes, 4);
				ack_net_to_server(RNMJKC, tmp_buf, 0, ptr->num);
			}
		}
		break;


		case UNMJKC://新的协议,注册卡号
		{
			printf("+++++++++++++++762+++++++++++++++\n");
			net_addr_msg *ptr = (net_addr_msg *)(data);
			unsigned int list_num;
			unsigned int num;
			int data_len;
			net_addr_msg_split(&net_cmd,&list_num,&num,&data_len,ptr);
			uint8_t tmp_buf[5];
			tmp_buf[0]=1;
			tmp_buf[1]=dev_cfg.my_code[0];
			tmp_buf[2]=dev_cfg.my_code[1];
			tmp_buf[3]=0;
			tmp_buf[4]=0;
			//store_door_card_info(tmp_buf+2, bytes);
			if(store_door_card_number_8byte_info(list_num, data_len)<0)
			{
			    uint16_t ret=1;
				memcpy(&tmp_buf[3],&ret,2);
			}
			ack_net_to_server(ANMJKC, tmp_buf, 5, num);
                    key_buzz(BUZZ_CONFIRM);
		}
		break;
#endif

	   

#endif

            //#if NEW_IPINSTALL //add by wrm 20150529 for newipinstall 
	       case SMJKCN:
		 {
                printf("+++++++++++++++750+++++++++++++++\n");
                char *tmp_buf = alloca(50*1024);
                int ret = do_load_card_unlock_ex(&tmp_buf[6], 50*1024);
                if(ret>=0) {
                    memcpy(tmp_buf, dev_cfg.my_code, 2);
                    memcpy(&tmp_buf[2], &ret, 4);
                    net_respond_server(link_num, RMJKCN, tmp_buf, ret*40+6);
                    }			
		/*	net_data_msg *ptr=(net_data_msg *)(data);
			uint8_t *buf = NULL;
		
			dbg_inf("respond door card_list to server\n");
			ret = do_load_door_card_inf(&buf);
			if(ret > 0 && NULL != buf) {
				memcpy(buf, gData.DoorNo, 2);
				ack_net_to_server(RMJKCN, buf, ret, ptr->num);
				free(buf);
			} else {
				buf = alloca(4);
				ack_net_to_server(RMJKCN, buf, 0, ptr->num);
			}*/
		}
		break;
//#else
        case GCRDC:
            {
                char *tmp_buf = alloca(50*1024);
                int ret = do_load_card_unlock(&tmp_buf[6], 50*1024);
                if(ret>=0) {
                    memcpy(tmp_buf, dev_cfg.my_code, 2);
                    memcpy(&tmp_buf[2], &ret, 4);
                    net_respond_server(link_num, RGCRD, tmp_buf, ret*40+6);
                }
            }
			break;
//#endif	
//#if NEW_IPINSTALL //add by wrm 20150610 for newipinstall
	case UMJKCN:
		{
		printf("+++++++++++++++752+++++++++++++++\n");
             net_addr_msg *ptr = (net_addr_msg*)data;
             char *tmp_buf = alloca(ptr->data_len+1024);
             if(tmp_buf != NULL)
                {
                    int bytes = get_list_data(ptr->list_num, tmp_buf);
                    do_save_card_unlock_ex(tmp_buf, bytes);
                    key_buzz(BUZZ_CONFIRM);
                } else {
                    perror("alloca:");
                }
                respond = 0;
                net_respond_server(link_num, AMJKCN, &respond, 2);				
                }
			break;
//#else	
        case SCRDC:
            {
                net_addr_msg *ptr = (net_addr_msg*)data;
                char *tmp_buf = alloca(ptr->data_len+1024);
                if(tmp_buf != NULL)
                {
                    int bytes = get_list_data(ptr->list_num, tmp_buf);
                    do_save_card_unlock(tmp_buf, bytes);
                } else {
                    perror("alloca:");
                }
                respond = 0;
                net_respond_server(link_num, RSCRD, &respond, 2);
            }
            break;
//#endif	
          case GCITC:
		{
                unsigned int list_num;
                unsigned int num;
                int len = 0;
                vlan_list_t dst_data = {0,};
                net_addr_msg_split(&net_cmd,&list_num,&num,&len,(net_addr_msg*)p_msg);
                uint32_t quantity=query_vlan_list(&dst_data, 2);
                unsigned char *buf=malloc(9+quantity*18);
                memset(buf,0,9);
                len = do_load_vlan_list_data(buf, len);
                memcpy(buf,&quantity,4);	
                /*printf("####quantity=%d\n",quantity);
                printf("####num=%d\n",num);
                printf("####len=%d\n",len);
                printf("####8+quantity*sizeof(vlanip_table_t)=%d\n",8+quantity*sizeof(vlanip_table_t));*/
                respond_addr_to_server(0x836,buf,8+quantity*sizeof(vlanip_table_t),num);
		//respond_addr_to_server(0x836,buf,8,num);
		free(buf);
        }
          break;
        case SCITC:
            {
                unsigned int list_num;
                unsigned int num;
                int len = 0;int i=0;
                net_addr_msg_split(&net_cmd,&list_num,&num,&len,(net_addr_msg*)p_msg);
                if(len>0)
                    {
                        unsigned char buf[len+1];
                        key_buzz(BUZZ_CONFIRM);
                        if(get_list_data(list_num,buf)>0)
                            {
                                unsigned char *ptr=buf;
                                uint32_t quantity;
                                uint32_t def;
                                system("rm vlan_list.bin");
                                int write_fd=-1;
                                write_fd=open("vlan_list.bin",O_WRONLY|O_CREAT,0664);
                                if(write_fd>-1)
                                    {
                                        write(write_fd,buf,len);
                                        close(write_fd);
                                        }
                                else
                                    {
                                        perror("open for vlan_list.bin:");
                                        }
                                i=0;
                                store_vlan_list_data(ptr,len);
                                memcpy(&quantity,ptr,4);
                                ptr+=4;
                                memcpy(&def,ptr,4);
                                ptr+=4;
                                //ui_refresh_vlan_iptable(ptr,quantity,def);
                                }
                        }

        }
        break;
        case NTELC: {
            unsigned char real_target[4];
            #if 0
            if (((proc_state == PROC_CALLING)||(proc_state == PROC_WAIT_ACK)) && \
                (p_ret->data.cmd == 0x88)) {  //可能被副管理机应答
                memcpy(real_target, &p_ret->data.oper_type, 4);
                printf("real_target:%02X%02X%02X%02X \n", real_target[0], real_target[1], real_target[2], real_target[3]);
                if (IS_ADMIN(real_target) && IS_ADMIN(target)) {
                    memcpy(target, real_target, 4);
                }
            }
            else   //呼叫的是DF2200 DF2002 系统控制器,不需要留言
            #endif
            if (((proc_state == PROC_CALLING)||(proc_state == PROC_WAIT_ACK)) && \
                (p_ret->data.cmd == 0x87)) {
                memcpy(real_target, &p_ret->data.oper_type, 4);
                if (IS_KZQ(real_target)) {
                    is_call_kzq = 1;
                    video_main_mode_set(VID_MAIN_CVBS);
                } else {
                    video_main_mode_set(VID_MAIN_LCD);
                }
            }
            else {
                //管理机呼找不到终端时会去找主机
                if (0 != memcmp(p_msg->data.dest_No, dev_cfg.my_code, 4)) {
                    //net_respond_node(p_msg->num, RTELC, 0, oper_type, resstatus);
                    break;
                }

                dst_lip  = net_lip;
                dst_type = p_msg->data.check;       //if dm365
                dst_vres = p_msg->data.resstatus;
                memcpy(watch_expect, p_msg->data.src_No, 4);           

        		allow = proc_watch_sw(WATCH_TRY, net_lip);
                net_respond_node(p_msg->num, RTELC, allow, oper_type, resstatus);
                if (!allow) {
                    proc_watch_sw(WATCH_START, net_lip);
                    //set_dest_dev_status(p_msg->data.src_No, net_lip, TONGHAO_CALLING);
                }
    		
                  if (!allow) {
                  
                    if (IS_ADMIN(p_msg->data.src_No) && (dst_type==TYPE_JDM365) && (dst_vres&VIDEO_720)) {
                        video_main_mode_set(VID_MAIN_CVBS);
                    } else {
                        if ((dst_type == TYPE_JDM365) && (resstatus & 0x80000000)) {
                            resstatus &=~0x80000000;
                            video_main_mode_set(VID_MAIN_CVBS);
                        } else {
                            video_main_mode_set(VID_MAIN_LCD);
                        }
                    }
                    set_dest_dev_status(p_msg->data.src_No, net_lip, TONGHAO_CALLING);
                    memcpy(watch_host, p_msg->data.src_No, 4);
                }      
            }
            }
            break;
        case NENDC:
        case  NENDC3:
            mobile_hold_on=0;
         	net_respond_node(link_num, p_msg->data.net_cmd==NENDC?RENDC:RENDC3, 0, oper_type, resstatus);
            //net_respond_node(link_num, RENDC, 0, oper_type, resstatus);
            if ((proc_state == PROC_CALLING)||(proc_state == PROC_TALKING)) {
                if((!IS_ADMIN(target)) || (!IS_ADMIN(p_msg->data.src_No))) {
                    if (memcmp(target, p_msg->data.src_No, 4)) break;
                }
                unsigned char send_cmd = (proc_state == PROC_TALKING) ? SEND_TA : SEND_AL;
                if (dev_get_type(p_msg->data.src_No) == DEV_ADMIN) {
                    net_query_to_node_cmd(target, NENDC, send_cmd, ZJCGJ, AUDIONRES|VIDEORES);
                } else {
                    #ifdef FRONT_DOOR
                    net_query_to_node_cmd(target, NENDC, send_cmd, DJCBFJ, AUDIONRES|VIDEORES);
                    #else
                    net_query_to_node_cmd(target, NENDC, send_cmd, ZJCFJ, AUDIONRES|VIDEORES);
                    #endif
                }
                set_dest_dev_status(target, net_lip, TONGHAO_DOWN);
                proc_switch_to(PROC_NORMAL);
                ui_action(UI_SCENE_SWITCH, SCENE_MAIN);
                uart_send_lift_data(raw_data);
            } else {
                if (memcmp(watch_host, p_msg->data.src_No, 4)) break;
                set_dest_dev_status(p_msg->data.src_No, net_lip, TONGHAO_DOWN);    
                proc_watch_sw(WATCH_STOP, 0);
            }
            break;
        case RTELC:
            printf("-------705 :%u \n", net_lip);
            if ((p_ret->data.cmd == 0x99) && IS_ADMIN(target) && p_ret->data.allow) {
                if (proc_state == PROC_WAIT_ACK) {
                    dest_state = DEST_BUSY;
                    set_dest_dev_status(target, net_lip, TONGHAO_BUSY);
                    tim_reset_time(tim_proc_timeout, TIME_250MS(1));
                }
            }
            if (!is_rtelc_to_current_call(target, net_lip)) break;
            if (p_ret->data.allow) {
                if (dest_state != DEST_ONLINE) dest_state = DEST_BUSY;
                set_dest_dev_status(target, net_lip, TONGHAO_BUSY);
            } else {
                set_dest_dev_status(target, net_lip, TONGHAO_CALLING); //20140224
                if (proc_state == PROC_WAIT_ACK) {
                    if (dest_state != DEST_ONLINE) {
                        dest_state = DEST_ONLINE;
                        dst_lip    = net_lip;
                        dst_type   = p_ret->data.dev_type;       //if dm365                                           
                        dst_vres   = p_ret->data.resstatus;
                        printf("---[%s_%d]: dst_type = 0x%08x,dst_vres = 0x%08x\n", __func__, __LINE__,dst_type,dst_vres); //by hgj                            
                        tim_reset_time(tim_proc_timeout, TIME_500MS(1));
                        if (IS_ADMIN(target) && (dst_type==TYPE_JDM365) && (dst_vres&VIDEO_720)) {
                            video_main_mode_set(VID_MAIN_CVBS);
                        }
                    }
                } else if (proc_state == PROC_CALLING) {
#if 1
            	dest_ip_t destip;

            	destip.dest_port=(dst_type==TYPE_JDM365)?9880:6670;
            	if(net_lip>0){
        		    destip.dest_ip=net_lip;
        		}
            	printf("add ip %08x port %d to list!\n",destip.dest_ip,destip.dest_port);

                if(ajb_net_video_service_add_destination(&destip,1)<0){
                    printf("fail to add video destination!!\n");
                }
#else
                    video_add_rtpdest_one(net_lip);
#endif                    
                }
            }
            break;
#ifdef CALL_3G
// START ----add by wrm 20150305 for cloud net线程tcp_process_task收到服务器804回复,转给proc，由proc_data_net处理   

	     case NTELC3:        //add  20161130
            {
            unsigned char real_target[4];          
            if (((proc_state == PROC_CALLING)||(proc_state == PROC_WAIT_ACK)) && \
                (p_ret->data.cmd == 0x87)) {
                memcpy(real_target, &p_ret->data.oper_type, 4);
                if (IS_KZQ(real_target)) {
                    is_call_kzq = 1;
                    video_main_mode_set(VID_MAIN_CVBS);
                } else {
                    video_main_mode_set(VID_MAIN_LCD);
                }
            }
            else {
                //管理机呼找不到终端时会去找主机
                if (0 != memcmp(p_msg->data.dest_No, dev_cfg.my_code, 4)) {
                    //net_respond_node(p_msg->num, RTELC, 0, oper_type, resstatus);
                    break;
                }

                dst_lip  = net_lip;
                dst_type = p_msg->data.check;       //if dm365
                dst_vres = p_msg->data.resstatus;
                memcpy(watch_expect, p_msg->data.src_No, 4);           

        		allow = proc_watch_sw(WATCH_TRY, net_lip);     		
                net_respond_node(p_msg->num, RTELC3, allow, oper_type, resstatus);
                if (!allow) {
                    proc_watch_sw(WATCH_MOBILE_START, net_lip);
                    //set_dest_dev_status(p_msg->data.src_No, net_lip, TONGHAO_CALLING);
                }
    		
                  if (!allow) {
                  
                    if (IS_ADMIN(p_msg->data.src_No) && (dst_type==TYPE_JDM365) && (dst_vres&VIDEO_720)) {
                        video_main_mode_set(VID_MAIN_CVBS);
                    } else {
                        if ((dst_type == TYPE_JDM365) && (resstatus & 0x80000000)) {
                            resstatus &=~0x80000000;
                            video_main_mode_set(VID_MAIN_CVBS);
                        } else {
                            video_main_mode_set(VID_MAIN_LCD);
                        }
                    }
                    set_dest_dev_status(p_msg->data.src_No, net_lip, TONGHAO_CALLING);
                    memcpy(watch_host, p_msg->data.src_No, 4);
                }  

                printf("------------WATCH_MOBILE_START--------------\n");
            
            }
            }
            break;
			
	  case RTELC3:
	  	printf("-------805  \n");
            if (proc_state == PROC_WAIT_ACK) {
                //ui_action(UI_CALL_SHOW, 0);
                if (p_ret->data.allow) {
                    if (dest_state != DEST_ONLINE) dest_state = DEST_BUSY;
                } else {
                    if (dest_state != DEST_ONLINE)
                        dest_state = DEST_ONLINE;
                }
                tim_reset_time(tim_proc_timeout, TIME_250MS(1));
            } else if (proc_state==PROC_CALLING){
            	  // ui_action(UI_CALL_SHOW, 0);
                net_query_to_node(target, NENDC,  ZJCFJ, AUDIONRES|VIDEORES);
                video_send_stop();
                usleep(100*1000);
                dst_server = 2;
 		  gbl_data_set(&gbl_cloud_show, 1);
                proc_switch_to(PROC_CALLING);
            }
            break;
        case NANSC3:
            if(0x88 != raw_data[0]) {
                set_dest_dev_status(target, net_lip, TONGHAO_TON); 
            }
            if (proc_state == PROC_CALLING) {
                net_respond_node(p_msg->num, RANSC3, 0, oper_type, resstatus);
                ui_action(UI_SCENE_SWITCH, SCENE_TALKING); 
                proc_switch_to(PROC_TALKING);
                printf("net data %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", \
            	   raw_data[0], raw_data[1], raw_data[2], raw_data[3], raw_data[4], raw_data[5], raw_data[6], raw_data[7], raw_data[8], raw_data[9]);
                uart_send_lift_data(raw_data);
            }
            break;
        case ULOCKC3:
             if ((proc_state == PROC_CALLING)||(proc_state == PROC_TALKING)) {
                net_respond_node(link_num, RLOCKC3, 0x22, oper_type, resstatus);
                audio_play(PMT_COME_IN, DEV_VOL_PLAY);
                ui_action(UI_TALKING_UNLOCK, 0);
                #ifndef FRONT_DOOR
                printf("net data %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", \
            	   raw_data[0], raw_data[1], raw_data[2], raw_data[3], raw_data[4], raw_data[5], raw_data[6], raw_data[7], raw_data[8], raw_data[9]);
                uart_send_lift_data(&raw_data[1]);//22 88 01 00 98 88 01 01 02 CF
                upload_record_pc(INTERCOM_UNLOCK_RECORD);
				printf("---------812-----\n");
                #endif
            }
            break;
      #endif

        case NANSC:
            if (proc_state == PROC_CALLING) {
                if(0x88 != raw_data[0]) {
                   set_dest_dev_status(target, net_lip, TONGHAO_TON); 
                }
                if ((dst_type == TYPE_JDM365) && (resstatus & 0x80000000)) {
                    resstatus &=~0x80000000;
                    video_main_mode_set(VID_MAIN_CVBS);
                }
#ifdef FRONT_DOOR
          /*      if ((resstatus & 0x98) == 0x98)
                    us_client_port_set(6669);
                else
                    us_client_port_set(6668);*/
#endif
			net_respond_node(p_msg->num, RANSC, 0, oper_type, resstatus);
			ip_tongh_t ip_a[TONG_HAOIP_MAX]={0};				 
			if (dev_get_type(p_msg->data.src_No) == DEV_ADMIN) {
				net_query_to_node_cmd(target, NENDC, SEND_NO, ZJCGJ, AUDIONRES|VIDEORES);
			} else {
				/*关闭其余同号分机的视频传输*/
				unsigned int count_th=get_iplist_from_call_list(target,SEND_NO,ip_a);
				printf("video delete destination, count_th=%d\n",count_th);
				dest_ip_t dest[3];
				char cnt=0,loop=0;
		
				memset(dest,0,sizeof(dest_ip_t)*3);
				dest[0].dest_port=dest[1].dest_port=dest[2].dest_port=(dst_type==TYPE_JDM365)?9880:6670;
				for(loop=0;loop<count_th;loop++)
				{
					if(ip_a[loop].ip_addr!=0)
					{
						dest[cnt].dest_ip=ip_a[loop].ip_addr;
						printf("delete ip %08x port %d from list!\n",dest[cnt].dest_ip,dest[cnt].dest_port);
						cnt++;
					}
				}
		
				if(ajb_net_video_service_delete_destination(dest,cnt)<0){
					printf("fail to delete video destination!!\n");
				}
				
				net_query_to_node_cmd(target, NENDC, SEND_NO, ZJCFJ, AUDIONRES|VIDEORES);
			}
		
			memset(ip_a, 0x00, sizeof(ip_a));				 
			get_iplist_from_call_list(target, SEND_TA, ip_a);
			dst_lip = ip_a[0].ip_addr;
			proc_switch_to(PROC_TALKING);
			usleep(500*1000);
			ui_action(UI_SCENE_SWITCH, SCENE_TALKING); 
			uart_send_lift_data(raw_data);											  
		}
		break;

    case GDSTC:
        //pc取门状态
        memcpy(buf, dev_cfg.my_code, 4);
        //buf[4] = door_status;
        buf[4] = (dev_cfg.en_mc_check==0) ? DOOR_UNKNOWN : door_status;
        if (link_type) {
            buf[3] = 0x00;  //带副号后台不认
            net_respond_server(link_num, RDSTC, buf, 5);
        } 
        else
            net_retdata_node(link_num, RDSTC, buf, 5);
        break;
		
        case FJJSC:

            time(&now);
            tnow = localtime(&now);
            sprintf(now_date, "%4d-%02d-%02d", 1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday);
            sprintf(now_time, "%02d:%02d:%02d", tnow->tm_hour, tnow->tm_min, tnow->tm_sec);
            printf("now_date %s  now_time %s \n",now_date,now_time);

    	 	buf[0] = raw_data[1];
    	 	buf[1] = raw_data[2];
    	 	buf[2] = raw_data[3];
    	 	buf[3] = raw_data[4];
                       
    	 	buf[4] = 1900+tnow->tm_year - 2000;  
    	 	buf[5] = tnow->tm_mon+1;
    	 	buf[6] = tnow->tm_mday;
    	 	buf[7] = tnow->tm_hour;
    	 	buf[8] = tnow->tm_min; 
    	 	buf[9] = tnow->tm_sec;
    		 printf("565 send buf: ");
    		 for(i = 0;i<10;i++){
    		 	printf("%02X ",buf[i]);
    	 	}
    		 printf("\n");

    		 net_retdata_node(link_num, RFJJC, buf, 0x0A);
		break;
        case SENDVC:
		memset(buf,0,sizeof(buf));
		buf[0] = vs_cfg.my_village_code[0];
		buf[1] = vs_cfg.my_village_code[1];
		buf[2] = vs_cfg.my_village_code[2];
		 printf("889 send buf: ");
		 for(i = 0;i<3;i++)
		 	printf("%02X ",buf[i]);
		 printf("\n");
		net_retdata_node(link_num, GETDVC, buf, 0x03);
		
        case GDATC:
        {
#ifndef FRONT_DOOR
            if (raw_data[0] != SSIC) uart_send_lift_data(raw_data); //网络SSIC有条件发送呼梯，下边有处理
#endif
            if (raw_data[0] == 0x1C) {      //其他主(副)主机
                net_respond_node(link_num, RGDAC, 0, oper_type, resstatus);
                if (!dev_cfg.my_code[3]) {
                    /*
			if (dev_cfg.card_in_flash) {//已开启存卡功能
                        unsigned char room[4];
                        if (is_card_unlock(card_hex2int(&raw_data[3]), room)) {//ADD_NewCard  
                            dest[0] = dev_cfg.my_code[0];
                            dest[1] = dev_cfg.my_code[1];
                            dest[2] = 0x00;
                            dest[3] = raw_data[2]-1;// -1恢复副门口机
                            buf[0] = SSIC;  //副机刷卡，主机开锁
                            buf[1] = dev_cfg.my_code[0];				
                            buf[6] = dest[3];
                            buf[7] = room[2];
                            buf[8] = room[3];
                            memcpy(&buf[2], &raw_data[3], 4);
                            net_send_to_node(dest, GDATC, buf, 10);
                        }
                    }/*/
		//else{
                    gbl_data_set(&gbl_sic_subunit, raw_data[2]);
                    uart_send_data(UART_1, raw_data);
                    tim_set_event(TIME_1S(1), (tim_callback)gbl_data_set, (int)&gbl_sic_subunit, 0, TIME_DESTROY);
		//	}
                }
            } 
            else if (raw_data[0] == SSIC)            
#if 0
            {      //其他主(副)主机
                net_respond_node(link_num, RGDAC, 0, oper_type, resstatus);
                if (raw_data[6] == dev_cfg.my_code[3]) {
                    if (gbl_data_get(&gbl_wait_ssic)) {
                        //if (0 != memcmp(sub_card, &raw_data[2], 4)) break; //兼容旧卡头的8位卡号
                        tim_suspend_event(tim_ssic_timeout);
                        ui_action(UI_CARD_UNLOCK, 0);
                        ui_action(UI_LCD_TURN_ON, 0);
                        // 恢复数据 0x1B 楼 卡1 卡2 卡3 卡4 主副 层 房 校验和
                        raw_data[2] = raw_data[7];
                        raw_data[3] = raw_data[8];
                        uart_send_lift_data(raw_data);
                    }
                }
            }            
#else
            {      //其他主(副)主机
			//unsigned char temp_data[10];  
                net_respond_node(link_num, RGDAC, 0, oper_type, resstatus);
                if (raw_data[6] == dev_cfg.my_code[3])
                {
                    if (gbl_data_get(&gbl_wait_ssic))
                    {
                        //if (0 != memcmp(sub_card, &raw_data[2], 4)) break; //兼容旧卡头的8位卡号
                        unsigned char card_hex[8];//, card_bcd[8];
                        tim_suspend_event(tim_ssic_timeout);         
        			    memcpy(card_hex, &raw_data[2], 4);
                        ui_action(UI_CARD_UNLOCK, 0);
                        ui_action(UI_LCD_TURN_ON, 0);				
                        // 恢复数据 0x1B 楼 卡1 卡2 卡3 卡4 主副 层 房 校验和
                        raw_data[2] = raw_data[7];
                        raw_data[3] = raw_data[8];
                        uart_send_lift_data(raw_data);	
#if 0                  
#ifndef FRONT_DOOR
	
				if ((raw_data[2] != 0x00) && (raw_data[3] != 0x00)) {
					dest[0] = dev_cfg.my_code[0];
					dest[1] = dev_cfg.my_code[1];
#if 1 //add and modify by wrm 20150513 使发送数据变为 1B 楼号 单元号(添加单元号) ...兼容信息存储器转发撤防 
				memcpy(temp_data,&raw_data[2],8);
				memcpy(&raw_data[3],temp_data,8);
				raw_data[1] = dev_cfg.my_code[0];//wgs+  
				raw_data[2] = dev_cfg.my_code[1];
                          dest[2] = raw_data[3];
                          dest[3] = raw_data[4];
#endif	
				raw_data[5] = 0; //wgs+
				raw_data[7]=0;//wgs+
				raw_data[6] = dev_cfg.my_code[3];	//????
				net_send_to_node(dest, GDATC, raw_data, 10);// 8 wrm
				}
#endif

					buf[0] = SIC;
					buf[1] = dev_cfg.my_code[0];
					buf[2] = dev_cfg.my_code[1];
					buf[3] = dev_cfg.my_code[3];
					card_hex2bcd(card_hex, card_bcd);
					card_bcdcopy(card_bcd, &buf[4]);
					memset(dest, 0x00, 4);
					net_send_to_server(GDATC, buf, 10);
					net_send_to_node(dest, GDATC, buf, 10);      

#endif
                	}
                }
            }
#endif
            else if (raw_data[0] == SKEY)
            {
                if (proc_state == PROC_TALKING) {
                    net_respond_node(link_num, RGDAC, 0, oper_type, resstatus);
                    ui_action(UI_TALKING_UNLOCK, 0);
					upload_record_pc(INTERCOM_UNLOCK_RECORD);
                }
            }
            else if (raw_data[0] == SSSK) {
                net_respond_node(link_num, RGDAC, 0, oper_type, resstatus);
                net_send_call_records(p_msg->data.src_No, CALL_RECORD_UNLOCK);//VLAN_RUAN_ADD
                if (gbl_data_get(&gbl_user_unlock)) {
                	gbl_data_set(&gbl_user_unlock,0);
                	net_send_unlock_records(&p_msg->data.src_No[0],USER_RECORD_OPENDOOR );                	
                }
                ui_action(UI_USER_PW_UNLOCK, 1);
            }
            else if (raw_data[0] == 0xED) {
				if (dev_cfg.my_code[3]) { break; }
                net_respond_node(link_num, RGDAC, 0, oper_type, resstatus);
            }
            else if ((raw_data[0] == SIC) || (raw_data[0] == 0xDB)) {
                if (is_this_floor(p_msg->data.dest_No)) {
                    if (dev_cfg.my_code[3] == 0x00) {   //主门口 
                        buf[0] = raw_data[0];
                        buf[1] = raw_data[1];
                        buf[2] = raw_data[5];
                        buf[3] = raw_data[4];
                        buf[4] = raw_data[6];
                        buf[5] = raw_data[7];
                        buf[6] = raw_data[8];
                        uart_send_data(UART_1, buf);
					} else { respond = 1; }
				}
				else {
			        pause_fp_polling();
				    uart_send_data(UART_0, raw_data);
				}
                net_respond_server(link_num, RGDAC, &respond, 2);
            }
            else if ((raw_data[0] == SWRI) || (raw_data[0] == SDEL) || \
                       (raw_data[0] == SRED) || (raw_data[0] == SFOR))
           {
                if (is_this_floor(p_msg->data.dest_No)) {
                    if (dev_cfg.my_code[3] == 0x00) {   //主门口 
                        buf[0] = raw_data[0];
                        buf[1] = raw_data[1];
                        buf[2] = raw_data[3];
                        buf[3] = raw_data[4];
                        buf[4] = raw_data[6];
                        buf[5] = raw_data[7];
                        buf[6] = raw_data[8];
                        uart_send_data(UART_1, buf);
					} else { respond = 1; }
				}
				else {
    				pause_fp_polling();
    				uart_send_data(UART_0, raw_data);
				}
                net_respond_server(link_num, RGDAC, &respond, 2);
            }
            else if (raw_data[0] == SSSS) {
                net_respond_node(link_num, RGDAC, 0, oper_type, resstatus);
              //  if(!is_this_floor(p_msg->data.dest_No)) break;
				if (!is_this_dev(p_msg->data.dest_No)) { break; }
                dev_cfg.day_night = raw_data[8]?0xFF:0x00;
                dev_config_save();
            }
            else if(raw_data[0] == R_UNLOCK)
            {
                printf("server unlock request!\n");
                if(is_this_dev(&raw_data[1]))
                {
                    printf("allow unlock!\n");
                    respond = 0;
                    net_respond_server(link_num, RGDAC, &respond, 2);
					ui_action(UI_CARD_UNLOCK, 0);
					ui_action(UI_LCD_TURN_ON, 0);    
                }
                else
                {
                    respond = 1;
                    net_respond_server(link_num, RGDAC, &respond, 2);
                }
            }                   



            if (is_this_dev(p_msg->data.dest_No))
            {
                switch(p_msg->data.cmd)
                {
                    case SCRT:
                    case SGLC:
#if 0
                        allow = proc_watch_sw(WATCH_START, inet_ntoa(ip_in_addr));
                        net_respond_node(link_num, RGDAC, allow, oper_type, resstatus);
#else                      
                        dst_lip  = net_lip;
                        dst_type = p_msg->data.check;       //if dm365
                        dst_vres = p_msg->data.resstatus;
                        memcpy(watch_expect, p_msg->data.src_No, 4);
                        allow = proc_watch_sw(WATCH_TRY, net_lip);
                        net_respond_node(p_msg->num, RTELC, allow, oper_type, resstatus);
                        if (!allow) {
                            proc_watch_sw(WATCH_START, net_lip);
                            set_dest_dev_status(p_msg->data.src_No, net_lip, TONGHAO_CALLING);
                        }
#endif
                        break;
                        break;
                    case SCRE:
                    case 0x26:
                        net_respond_node(link_num, RGDAC, 0, oper_type, resstatus);
                        proc_watch_sw(WATCH_STOP, 0);
                        break;
                    
                    //以下命令来自管理处
                    case SOK:
                        net_respond_node(link_num, RGDAC, 0, oper_type, resstatus);
                        ui_action(UI_NIGHT_TRANSMIT, 0);
                        break;
                    case SDIS:
                        net_respond_node(link_num, RGDAC, 0, oper_type, resstatus);
                        memcpy(&buf[1], p_msg->data.src_No, 4);
                        ui_get_target(&buf[5]);
                        buf[0] = SDIS;
                        net_send_to_node(p_msg->data.src_No, GDATC, buf, 10);
                        break;
                        
                    //以下命令来自PC
                    case 0xDD:
                        if (link_type)
						{ net_respond_server(link_num, RGDAC, &respond, 2); }
                        else
						{ net_retdata_node(link_num, RGDAC, &respond, 2); }
                        buf[0] = 0xDD;
                        buf[1] = 0x00;
                        buf[2] = 0x00;
                        buf[3] = 0x00;
                        buf[4] = raw_data[8];
                        buf[5] = dev_cfg.my_code[0];
                        buf[6] = dev_cfg.my_code[1];
                        buf[7] = strtoul(dev_cfg.pw_manage, NULL, 16)>>8;
                        buf[8] = strtoul(dev_cfg.pw_manage, NULL, 16)&0xFF;

                        if (link_type)
						{ net_send_to_server(GDATC, buf, 10); }
                        else
						{ net_send_to_node(p_msg->data.src_No, GDATC, buf, 10); }
                        break;
                    default:
                        break;
                }    //end of "switch(p_msg->data.cmd)"
            }
            break; //end of "case GDATC"
        }    
        default:
            break;
    }
}


//tim_callback
void proc_timeout(int arg1, int arg2)
{
    unsigned char msg[MSG_LEN_PROC];
    msg[0] = MSG_FROM_PROC;
    switch(proc_state) {
        case PROC_WAIT_ACK:
            msg[1] = MSG_WAIT_ACK_TIMEOUT;
            pipe_put(proc_pipe_proc, msg, MSG_LEN_PROC);
            break;
        case PROC_CALLING:      //由UI触发挂机动作
            ui_action(UI_CALLING_TIMEOUT, 0);
            break;
        case PROC_TALKING:      //由UI触发挂机动作
            ui_action(UI_TALKING_TIMEOUT, 0);
            break;
        case PROC_LEAVE_MSG:
            msg[1] = MSG_LVMSG_TIMEOUT;
            pipe_put(proc_pipe_proc, msg, MSG_LEN_PROC);
            break;
        default:
            break;
    }
}

void proc_fp_timeout(int arg1,int arg2){//by mkq finger

    int status = fp_status_get();
    app_debug(DBG_INFO,"###proc_fp_timeout start run, fp_status=0x%02x\n",status);
    switch(status){
	   case  status_reg_timeout:
		ui_action(UI_SCENE_SWITCH, SCENE_MAIN);
		break;
	   case  status_reg_second:
	   case  status_reg_first:
	   case  status_reg_ok:
	   	fp_status_set(status_index);
		fp_readcolist(0x00);
		break;
        default:
            break;
	}
}
static int timing_interval = 0;
//tim_callback
void sec2_timeout(int arg1, int arg2)
{
    if (timing_interval-- == 0) {
        net_send_to_server(FJJSC, dev_cfg.my_code, 4);
        timing_interval = 17745 + (rand() & 0x000001FF);
    }
    if ((dev_cfg.delay_doorala > 0) && (door_status == DOOR_OPEN)){
        int alarm_delay = gbl_data_get(&gbl_alarm_delay);
        if (alarm_delay > TIME_1S(0)) {
            alarm_delay = alarm_delay - TIME_1S(2);
            gbl_data_set(&gbl_alarm_delay, alarm_delay);
            if (alarm_delay <= TIME_1S(0)) {
                net_send_alarm(ALARM_MC);
		   //audio_play(PMT_ALARM, DEV_VOL_PLAY);                             //by  mkq 20170907
                gbl_data_set(&gbl_alarm_delay, TIME_1MIN(3));
            }
        }
    }

    static unsigned char daemon_msg[MSG_LEN_PROC] = {0x00};
    daemon_msg[0] = MSG_FROM_PROC;
    daemon_msg[1] = MSG_CPLD_DAEMON;
    daemon_msg[2] = !daemon_msg[2];
    pipe_put(proc_pipe_proc, daemon_msg, MSG_LEN_PROC);
}

void ssic_timeout(int arg1, int arg2)
{
    buzz_play(BUZZ_WARNING); //modified by wrm 20141223 del by wrm 20141121
    gbl_data_set(&gbl_wait_ssic, 0);
}


#ifdef CALL_3G
void try_call_server(int arg1, int arg2) //add by wrm 20150306 for cloud
{
    unsigned char msg[MSG_LEN_PROC]; 
    if (dev_get_type(target) == DEV_INDOOR&&(proc_state == PROC_CALLING)) {
        msg[0] = MSG_FROM_PROC;
        msg[1] = MSG_TRY_CALL_SERVER;
        pipe_put(proc_pipe_proc, msg, MSG_LEN_PROC);
    }
}
#endif
static void *proc_message(void *arg)
{
    unsigned char msg[MSG_LEN_PROC] = {0};
    unsigned char dest[4];
    unsigned char data[10];
    unsigned char  unlock_pw[20];
    unsigned char dat[4];
    struct msg_t *p_msg = (struct msg_t *)arg;
   
    proc_pipe_proc = p_msg->pipe_proc;
    
    proc_msg msg_send;      //net
    gbl_data_set(&gbl_alarm_delay, 0);
    gbl_data_set(&gbl_watching,    0); 
    gbl_data_set(&gbl_user_unlock,    0); 
    gbl_data_set(&gbl_mb_watching, 0); 

    timing_interval = 1 + (rand()& 0x0000000F); // 0x0000001F  modify by wrm 20150429 for fast check time
    printf("interval : %d\n",timing_interval);
    leave_buf = malloc(SPEECH_MAX_LEN+10*1024);
    
    tim_proc_timeout   = tim_set_event(TIME_1S(0), (tim_callback)proc_timeout, 0, 0, TIME_ONESHOT);
    tim_sec2_timeout   = tim_set_event(TIME_1S(2), (tim_callback)sec2_timeout, 0, 0, TIME_PERIODIC);
    tim_ssic_timeout   = tim_set_event(TIME_1S(0), (tim_callback)ssic_timeout, 0, 0, TIME_ONESHOT);
    tim_wactch_timeout = tim_set_event(TIME_1S(0), (tim_callback)proc_watch_sw, WATCH_TIMEOUT, 0, TIME_ONESHOT);
    tim_try_call_server = tim_set_event(TIME_1S(0), (tim_callback)try_call_server, 0, 0, TIME_ONESHOT); //add by wrm 20150306 for cloud
    tim_wait_tc_pw_timeout=tim_set_event(TIME_1S(0), (tim_callback)ui_action, UI_WAIT_TC_PW_TIMEOUT, 0, TIME_ONESHOT);
    fp_time_int();//by mkq finger
    //build_key(seed_data);
    build_key(default_seed,vs_cfg.dev_seed_data);
    sleep(2);
    ui_get_village();

    while(1)
    {
        if (pipe_get(proc_pipe_proc, msg, MSG_LEN_PROC) < 0) {
            printf("proc pipe get error\n");
            usleep(100*1000);
            continue;
        }
        //app_debug(DBG_INFO, "proc get msg:%02x %02x %02x %02x %02x %02x %02x\n", msg[0], msg[1], msg[2], msg[3], msg[4], msg[5], msg[6]);
        switch (msg[0]) {
            case MSG_FROM_UI:
            {
                msg_send.msg_type = MSG_FROM_PROC;
                if (msg[1] == UI_SEND_AJB_DATA) {
                    switch (msg[2]) {
                     case GETVI:
				app_debug(DBG_INFO, "ui get village\n");
				memcpy(&dat,dev_cfg.my_code,4);
				//build_key(seed_data);
				build_key(default_seed,vs_cfg.dev_seed_data);
				if(proc_state == PROC_NORMAL)
                            send_get_village_data_to_server(GVNUMC,dat,4);

				break;

			case GETUK:
				app_debug(DBG_INFO, "ui get unlock\n");
				int temp_i;
				
				//memcpy(&unlock_pw[0],dev_cfg.my_village_code,3);
				memcpy(&unlock_pw[0],vs_cfg.my_village_code,3);
			       #ifndef FRONT_DOOR						
					memcpy(&unlock_pw[3],dev_cfg.my_code,4);
                            #else
                            unlock_pw[3]=0xEF;
                            unlock_pw[4]=0xEF;
                            unlock_pw[5]=0x00;
                            unlock_pw[6]=0x00;
                            #endif
				memcpy(&unlock_pw[7],&msg[3],4);
				for(temp_i=0;temp_i<11;temp_i++)
					printf("unlock_pw=%x\n",unlock_pw[temp_i]);
				//if(proc_state == PROC_NORMAL)
					send_unlock_data_to_server(SEDUNLOCKC, unlock_pw, 11);
				break;
                        case STEL:
                        case SONE:
                        case 0x7C:
                            app_debug(DBG_INFO, "ui make call\n");
                            if (proc_state == PROC_NORMAL) {
                                atest_cnt_inc(AT_CNT_CALL);
                                proc_watch_sw(WATCH_BREAK, 0);
                                is_call_kzq = 0;

                                if (msg[2] == 0x7C)
                         			usleep(((dst_type == TYPE_JDM365)||(dst_type == TYPE_J926)) ? 600*1000 : 2800*1000);
                                proc_switch_to(PROC_WAIT_ACK);
                                if (dev_get_type(&msg[3]) == DEV_ADMIN) {
                                    net_query_to_node(&msg[3], NTELC, ZJCGJ, AUDIONRES|VIDEORES);
                                } else {
                                 dst_server = 0; //add by wrm 20150305 for cloud 确保再一次呼叫时dst_server的值为0
#ifdef FRONT_DOOR
                                    net_query_to_node(&msg[3], NTELC, DJCBFJ, AUDIONRES|VIDEORES);
#else
                            	net_query_to_node(&msg[3], NTELC,  ZJCFJ, AUDIONRES|VIDEORES);
#endif
                                }
                                uart_send_lift_data(&msg[2]); 
                                net_send_call_records(&msg[3], CALL_RECORD_CALL);//VLAN_ADD_RUAN
                            }
                            break;
                        case SEND:
                        case SSINT:	
                            app_debug(DBG_INFO, "ui make end\n");  
                            mobile_hold_on=0;
                            if ((proc_state == PROC_WAIT_ACK) || (proc_state == PROC_CALLING) || (proc_state == PROC_TALKING))
                            {
               
                				printf("----dst_server = %d msg[3] = %02X\n",dst_server,msg[3]);
            				    if(dst_server){
                				   	net_query_to_node_cmd(&msg[3], NENDC3, SEND_AL, ZJCFJ, AUDIONRES|VIDEORES);
            				   	}else{
                                    if (dev_get_type(&msg[3]) == DEV_ADMIN) {
                                        net_query_to_node_cmd(&msg[3], NENDC, SEND_AL, ZJCGJ, AUDIONRES|VIDEORES);
                                    } else {
                                        #ifdef FRONT_DOOR
                                        net_query_to_node_cmd(&msg[3], NENDC, SEND_AL, DJCBFJ, AUDIONRES|VIDEORES);
                                        #else
                                        net_query_to_node_cmd(&msg[3], NENDC, SEND_AL, ZJCFJ, AUDIONRES|VIDEORES);
                                        #endif
                                	}
            				   }
                                tim_suspend_event( tim_try_call_server );
                                proc_switch_to(PROC_NORMAL);
                                data[1] = 0x7F; //UI的呼叫动作, 非标准 AJB数据，用0x7F来中转
                                uart_send_lift_data(&data[1]); 
                            }
                            break;
                        case SSCS:
                            dest[0] = dev_cfg.my_code[0];
                            dest[1] = dev_cfg.my_code[1];
                            dest[2] = msg[3];
                            dest[3] = msg[4];

                            data[0] = SSCS;
                            data[1] = dev_cfg.my_code[0];
                            data[2] = msg[3];
                            data[3] = msg[4];
                            data[4] = msg[5];
                            data[5] = msg[6];
                            data[6] = dev_cfg.my_code[3];
                            net_send_to_node(dest, GDATC, data, 8);
                            uart_send_lift_data(data);
                            //net_send_unlock_records(dest,USER_RECORD_OPENDOOR);
                            break;				
                    }
                }
                else if (msg[1] == UI_SEND_CALL_RECORDS)
                {
                	net_send_call_records(&msg[2], msg[6]);
        		}
                else if (msg[1] == UI_SEND_CALL_TRANSFER)
                {
        		net_query_to_node(target, NENDC,  ZJCFJ, AUDIONRES|VIDEORES);
        		video_send_stop();
        		usleep(100*1000);
        		dst_server = (proc_state == PROC_CALLING) ? 2 : 1;
        		proc_switch_to(PROC_WAIT_ACK);
        		//net_query_to_node(target, NTELC3, ZJCFJ, AUDIONRES|VIDEORES);
    			}
                else
                if (msg[1] == UI_SEND_LEAVE_MSG) {
                    if (msg[2]) {
                        if (proc_state == PROC_NORMAL) {
                            proc_switch_to(PROC_LEAVE_MSG);
                            ui_action(UI_START_LEAVE_MSG, 0);
                        }
                    }
                    else {
                        if (proc_state == PROC_LEAVE_MSG) {
                            proc_switch_to(PROC_NORMAL);
                        }
                    }
                }
#ifdef APP_AUTO_TEST
                else
                if (msg[1] == UI_SEND_AUTO_TEST) {
                    memcpy(dest, &msg[3], 4);
                    memcpy(&data[1], &msg[3], 4);
                    data[0] = 0xEE;
                    data[5] = 0xAA;
                    data[6] = 0xBB;
                    if (msg[2]) {
                        data[7] = 0xCC;
                        data[8] = 0xDD;
                    }
                    else {
                        data[7] = 0xDD;
                        data[8] = 0xEE;
                    }
                    net_send_to_node(dest, GDATC, data, 10);
                }
#endif
                break;
            }                
            case MSG_FROM_KEY:
                switch (msg[1]) {
                    case KEY_MC:
		      printf("MSG_FROM_KEY   KEY_MC++++++++++++\n");
                        door_status = (msg[2]==DOOR_OPEN)?DOOR_OPEN:DOOR_CLOSE;
                        if (msg[2] == DOOR_OPEN) {
                            gbl_data_set(&gbl_alarm_delay, TIME_1MIN(dev_cfg.delay_doorala));
                        }
                        else {
                            gbl_data_set(&gbl_alarm_delay, TIME_1MIN(0));
                        }
                        if (!dev_cfg.en_mc_check) break;
                        data[0] = SIC;
                        data[1] = dev_cfg.my_code[0];
                        data[2] = dev_cfg.my_code[1];
                        data[3] = 0xEE;
                        data[4] = data[5] = data[6] = 0x00;
                        data[7] = dev_cfg.my_code[3];
                        data[8] = door_status;
                        net_send_to_server(GDATC, data, 10);
                        break;
                    case KEY_FC:
		      printf("KEY_FC................\n");
                        if (!dev_cfg.en_fc_alarm) break;
                        net_send_alarm(ALARM_FC);
                        break;
                    default:
                        break;
                }
                break;
 //del by wrm 20141121
            case MSG_FROM_NET:
                proc_data_net(&msg[1]);
                break;

            case MSG_FROM_UART:
                if (msg[1] == UART_0)
                    proc_data_uart0(&msg[2]); 
                else
                    proc_data_uart1(&msg[2]);
                break;

            case MSG_FROM_PROC:
            {            
                if (msg[1] == MSG_CPLD_DAEMON) {                  
                    /*if (msg[2]) cpld_io_set(IOLed);
                    else        cpld_io_clr(IOLed);*/
                }
                else 
                if (msg[1] == MSG_WATCH_TIMEOUT) {
        			if (is_have_infostore()) 
                        proc_watch_sw(WATCH_STOPALL, 0);
                    else
                    proc_watch_sw(WATCH_STOP, 0);
                }
                else
                if (msg[1] == MSG_LVMSG_TIMEOUT) {
                    proc_switch_to(PROC_NORMAL);
                }
                else
                if (msg[1] == MSG_WAIT_ACK_TIMEOUT) {
                    if (dest_state == DEST_ONLINE) {
                        proc_watch_sw(WATCH_BREAK, 0);
                        memset(ipmulti, 0x00, sizeof(ipmulti));
                        //get_iplist_from_call_list(target, SEND_AL, ipmulti); //20140224
                        get_iplist_from_call_list(target, SEND_RE, ipmulti);
                        ui_action(UI_CALLING_GOTASK, 0);
                        atest_cnt_inc(AT_CNT_ASK);
                        proc_switch_to(PROC_CALLING);
                    }
                    else if (dest_state == DEST_BUSY) {
        			   ui_action(UI_CALLING_FAIL, dst_server?SERVER_BUSY:TARGET_BUSY);
        			   proc_switch_to(PROC_NORMAL);
        			} //分机不在线情况的处理
                    else if(cloud_server_state ==DEST_ONLINE){
            			printf("-------------try call server-------------\n");
                        if((!dst_server) && (dev_get_type(target) == DEV_INDOOR)){
                            dst_server = 1;
                            net_query_to_node(target, NTELC3, ZJCFJ, AUDIONRES|VIDEORES);
                            ui_action(UI_CALL_SHOW, 0);
                            gbl_data_set(&gbl_cloud_show, 1);
                            proc_switch_to(PROC_WAIT_ACK);
        				}
        				else{
        				ui_action(UI_CALLING_FAIL, SERVER_OFFLINE);
                        	proc_switch_to(PROC_NORMAL);
                        	//ui_action(UI_CALLING_FAIL, TARGET_OFFLINE);
            			}
                    }
                    else {
                           ui_action(UI_CALLING_FAIL, SERVER_OFFLINE);
                           proc_switch_to(PROC_NORMAL);
                    } 
                }
                else
                if (msg[1] == MSG_WAIT_MSG_TIMEOUT) {
                    proc_switch_to(PROC_NORMAL);
                    ui_action(UI_WAIT_MSG_TIMEOUT, 0);
                }
	            else
                if ((msg[1] == MSG_TRY_CALL_SERVER) &&(!mobile_hold_on)){
			printf("-------------try NTELC3 -------------\n");
                    net_query_to_node(target, NTELC3, ZJCFJ, AUDIONRES|VIDEORES);
                    ui_action(UI_CALL_SHOW, 0);
                    gbl_data_set(&gbl_cloud_show, 1);
                }
                break;
            }
            case MSG_FROM_IGNORE:
                if (msg[1] == MSG_MOBILE_WATCH) {
                    int video_snd_sock;
                    unsigned char house[4];

                    mobile_hold_on=1;
                    memcpy(&video_snd_sock, &msg[2], sizeof(video_snd_sock));
                    memcpy(house, &msg[6], sizeof(house));
                    printf("video_snd_sock:%d house:%02X%02X%02X%02X\n", video_snd_sock,
                           msg[6], msg[7], msg[8], msg[9]);
                    if (video_snd_sock > 0) {
                        if ((proc_state == PROC_CALLING || proc_state == PROC_TALKING) && (0==memcmp(house, target, 4))) {
                            if(dst_server){
                                net_query_to_node_cmd(target, NENDC3, SEND_AL, ZJCFJ, AUDIONRES|VIDEORES);
                                //net_query_to_node(target,NENDC3,ZJCFJ,AUDIONRES|VIDEORES);
                            }else {
#ifdef FRONT_DOOR
                            net_query_to_node(target, NENDC, DJCBFJ, AUDIONRES|VIDEORES);
#else
                            net_query_to_node(target, NENDC, ZJCFJ, AUDIONRES|VIDEORES);
#endif
                            }
                            audio_stop();
                            speech_stop();
                            video_send_stop();
                            send(video_snd_sock, "CTP/1.0 200 OK", sizeof("CTP/1.0 200 OK"), 0);
                            tim_reset_time(tim_proc_timeout, TIME_1S(61));
                            ui_action(UI_FORCE_SWITCH, SCENE_TALKING);
                            //video_day_night_set();
                            proc_state = PROC_TALKING;
                            unsigned long lip[4] = {0};
                            lip[0] = video_snd_sock;
                            usleep(100*1000);
                            video_send_start(lip, VID_MOBILE, VID_CIF);
                                
                        } 
                        else {
                            send(video_snd_sock, "CTP/1.0 404 notfound", sizeof("CTP/1.0 404 notfound"), 0);
                            close(video_snd_sock);
                        }
                    }
                }
                break;
            default:
                break;
        }
    }
    exit(-1);
}

#include "ansi_char.h"
static void set_pdu_head(void)
{
    unsigned int tcp_port = TCP_PC_PORT;
    unsigned char i;
    memset(&udp_echo, 0x00, sizeof(udp_echo));
    udp_echo.dhcp_ebl = 0;
    udp_echo.equ_type = 2;	                                 //设备类型
    
    memcpy(udp_echo.http_port,        &tcp_port,       4);
    memcpy(udp_echo.ip_addr,          dev_cfg.ip_addr, 4);
    memcpy(udp_echo.mac_addr,         dev_cfg.ip_mac,  6);
    memcpy(udp_echo.subnet_mask,      dev_cfg.ip_mask, 4);
    memcpy(udp_echo.gateway_addr,     dev_cfg.ip_gate, 4);

    memcpy(udp_echo.dns1,      		  dev_cfg.dns1, 4);
    memcpy(udp_echo.dns2,     		  dev_cfg.dns2, 4);
    for(i=0;i<4;i++) printf(" dev_cfg.dns1[%d] = %X \n ",i,dev_cfg.dns1[i]);
	for(i=0;i<4;i++) printf(" dev_cfg.dns2[%d] = %X \n ",i,dev_cfg.dns2[i]);
    
    strcpy(udp_echo.equ_name,         PDU_EQU_NAME);        //设备名称
    strcpy(udp_echo.prod_name,        PDU_PROD_NAME);       //生产商名称
    strcpy(udp_echo.type_name,        PDU_TYPE_NAME);       //类型名称
    strcpy(udp_echo.equ_description,  PDU_EQU_DESC);        //设备描述
    strcpy(udp_echo.hardware_version, PDU_HW_VER);
    strcpy(udp_echo.software_version, dev_software_ver());  //"11B-"__DATE__);
    strcpy(udp_echo.position,         PDU_POSITION);        //位置
    strcpy(udp_echo.description,      PDU_DESCRIPTION);     //描述
}

void net_start(void *arg)
{
    net_para my_net_para;
    system("ifconfig lo 127.0.0.1 netmask 255.255.255.255");
    memcpy(&my_net_para.board_ip, dev_cfg.ip_bcast, 4);
    memcpy(my_net_para.dev_No,    dev_cfg.my_code,  4);
    my_net_para.load_global_data_fun = global_data_load;
    my_net_para.save_global_data_fun = global_data_save;
    my_net_para.save_eth_para_fun    = eth_para_save;
    set_pdu_head();
    net_com_create(&my_net_para, arg);
    register_dev_to_pc();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main( int argc, char *argv[] )
{
    app_debug_init();
    osd_init();
    cpld_init();
    sql_init();

     dev_sv_get();
    dev_config_get();
//     video_send_init();

    struct msg_t msg_t; 
    
    msg_t.pipe_ui    = pipe_create();
    msg_t.pipe_net   = pipe_create();
    msg_t.pipe_spi   = pipe_create();
    msg_t.pipe_buzz  = pipe_create();
    msg_t.pipe_uart  = pipe_create();
    msg_t.pipe_proc  = pipe_create();
    msg_t.pipe_audio = pipe_create();
    msg_t.pipe_finger= pipe_create();//by mkq finger

    tim_start(NULL);    
    face_start(&msg_t);   
    net_start(&msg_t);
    key_start(&msg_t);//add and modified by wrm 20141121
    uart_start(&msg_t);
    audio_start(&msg_t);
    syn_with_rtc_time();
    
    ui_start(&msg_t);    
    finger_start(&msg_t);//by mkq finger
    proc_message(&msg_t);

    cpld_close();
    net_com_delete();
    
    return 0;
}
