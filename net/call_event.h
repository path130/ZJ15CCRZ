/*
 * call_event.h
 *
 *  Created on: 2012-9-19 上午11:05:41
 *  
 */

#ifndef CALL_EVENT_H_
#define CALL_EVENT_H_
#include "net_com_struct.h"
#include "event_num.h"
#include "ajb_bus.h"
#include "global_def.h"

#if defined (__cplusplus)
extern "C" {
#endif


#ifndef HAVE_TONGHAO

typedef struct{
	unsigned char dest_No[4];
	unsigned int  ip_addr;
	unsigned int  event_num;							//表示网络事件在链表中的存储序列号
}call_item;

void add_dest_dev_type_by_ip(unsigned int ip_addr,unsigned int type);
unsigned int get_dest_dev_type(unsigned char dest_No[4]);
unsigned int get_dest_dev_type_by_ip(unsigned int ip_addr);
unsigned int get_dest_dev_type_by_NO(unsigned char dest_No[4]);

#else

typedef struct IP_HEAD_T{
	unsigned int  ip_addr;
	unsigned int  ip_talk;//同号对讲使用中
	struct IP_HEAD_T *next;
}ip_list_t;

typedef struct IP_TH_T{
	unsigned int  ip_addr;
	unsigned int  ip_talk;//同号对讲使用中
}ip_tongh_t;

typedef struct{
	unsigned char dest_No[4];
	ip_list_t     *ip_head;
	unsigned int  event_num;							//表示网络事件在链表中的存储序列号
	int ip_count; //同号IP数量
	int ip_max;   //同号IP最大数量
}call_item;

#define SEND_RE (SEND|0xa0)   //已经收到705回复
typedef enum
{
	TONGHAO_TOOFF, //未提机的设备
	TONGHAO_TON,   //正在对讲的设备
	TONGHAO_BUSY,  //返回忙的设备，挂机时不操作这些设备
	TONGHAO_DOWN,  //本机已经发过挂机的设备
	TONGHAO_CALLON, //已发送或收到704
	TONGHAO_CALLING, // 添加到呼叫列表
	TONGHAO_ARP,
}th_status_t;

unsigned int get_talkip_from_call_list(unsigned char dest_No[4],ip_tongh_t ip_addr[TONG_HAOIP_MAX]);
unsigned int get_iplist_from_call_list(unsigned char dest_No[4],unsigned char flag,ip_tongh_t ip_addr[TONG_HAOIP_MAX]);
void clear_talkip_from_call_list(unsigned char dest_No[4]);
void clear_tonghaoip_from_call_list(unsigned char dest_No[4]);
void set_cur_talk_dev_ip(unsigned char dest_No[4],unsigned int dev_ip);
void set_dest_dev_status(unsigned char dest_No[4],unsigned int dev_ip,th_status_t status);
int is_rtelc_to_current_call(unsigned char dest_No[4],unsigned int ip_addr);

#endif


void create_call_list();
unsigned int get_ip_from_call_list(unsigned char dest_No[4],unsigned int *ip_addr);
void add_call_item(unsigned char dest_No[4],unsigned int ip_addr);



#if defined (__cplusplus)
}
#endif

#endif /* CALL_EVENT_H_ */
