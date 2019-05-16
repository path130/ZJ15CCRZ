/*
 * global_def.h
 *
 *  Created on: 2012-6-21
 *
 */

#ifndef GLOBAL_DEF_H_
#define GLOBAL_DEF_H_

#include "data_struct.h"
#include "net_parameter.h"

#define DEV_TERMINAL 0	  //终端
#define DEV_GATE 		1    //主机
#define DEV_CONTROL	0	  //主机控制器
#define DEV_GLJKZQ 	0	  //管理机控制器
#define DEV_TSGLJKZQ 0


#define DEFAULT_JDM365 0x4000
#define RES_VAL         '*'
#define RES_2100        '-'

#define TYPE_J926	    0x00000004 //简化版926系统
#define RES_926  '+'

#define DEV_TYPE 0

#define DEV_BUSY 1
#define DEV_IDLE 0


#define HAVE_SPEECH 0
#define DEF_PC

#define CALL_ON 1
#define CALL_OFF 2

#define NET_START  1
#define NET_STOP 0


#define MSG_NET2UI 100
#define MSG_UI2NET 102

#define COON_QUANTITY 6
#define LIST_QUANTITY COON_QUANTITY*6
#define COON_LIST_QUANTITY (COON_QUANTITY*8)

//#define ARP_DELAY (50000) //us
#define ARP_DELAY (70000) //us
#define ARP_RETYR_TIMES (3)
#define CONNECT_TIMEOUT (1)  //sec 3 //减少超时重连的时间

#define ITEM_ONE (0x01)
#define ITEM_TWO (0x02)

/*
 * UDP数据包
 */

#define MAX_LEN 1024//1300
#define COUNT_ITEM(x) (((x)/MAX_LEN)+(((x)%MAX_LEN)?1:0))
#define DATA_LEN 1024
#define UDP_LEN 1024
#define UDP_PORT 6672
#define UDP_PC_PORT 6666
#define UDP_SPEECH_PORT 6668
#define HAVE_ECHO_CANCEL 0


#define TCP_MAX_SIZE 1024
#define TCP_PC_PORT	18020
#define TCP_PORT 18022
#define TCP_PC_JSON_PORT	18026

/*
 * PIPE读写端口
 */
#define PIPE_WRITE 1
#define PIPE_READ 0

/*
 * NET
 */
#define DETECT_TIME 1                        //网络状态检测间隔，单位为秒
extern net_para *dev_info;
extern DevList my_devlist;
extern int msg_id;

/*
 * 同号
 */
#define HAVE_TONGHAO
#define TONG_HAOIP_MAX 8

#define SEND_AL SEND       //挂断所有
#define SEND_NO (SEND|0xf0)  //挂断未提机的
#define SEND_TA (SEND|0xe0)   //挂断提机的
#define SEND_BS (SEND|0xc0)   //清空
#define SEND_NC (SEND|0xc0)   //未发送704


#endif /* GLOBAL_DEF_H_ */
