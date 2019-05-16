/*
 * data_com_ui.h
 *
 *  Created on: 2012-6-28
 *
 */

#ifndef DATA_COM_UI_H_
#define DATA_COM_UI_H_
#include <stdlib.h>
#include "global_def.h"
#include "data_com.h"
#include "msg.h"
#include "net_parameter.h"
#include "dev_config.h"

#define MEM_ADDR  	(0x41)
#define MEM_DATA  	(0x40)
#define MEM_GLJ   	(0x43)
#define GLJ_RQ_ADDR  (0x44)//请求数据
#define GLJ_RP_ADDR  (0x45)//回复数据
#define GLJ_RQ_RET   (0x46)//请求数据回复

#define PC_ADDR   	(0x11)
#define PC_DATA   	(0x10)
#define PC_ADDR_RET  (0x12)
#define PC_DATA_RET  (0x13)
#define PC_ADDR_DEF (0x14)
#define DATA_MASK 	(0x10)

#define SERVER_DATA (0x15)
//added by hgj 180302
#define KZQ_LIFT_DATA (0x16)//by mkq 20171017 netlift
#define PC_ADDR_JSON (0x17)

#pragma pack(1)
typedef struct{
	unsigned int  net_cmd;
	unsigned char cmd;
	unsigned char dest_No[4];
	unsigned char src_No[4];
	char check;
	unsigned int oper_type;
	int resstatus;//音视频资源占用
}ajb_data;

typedef struct{
	unsigned int net_cmd;
	unsigned char cmd;
	int allow;
	unsigned int oper_type;
	char res;
	int resstatus;//音视频资源占用
	int dev_type;
}ajb_data_ret;

typedef struct{
	char flag_addr;
	ajb_data data;//22
	unsigned int num;
}ajb_msg;

typedef struct{
	char flag_addr;   		   //标志位
	unsigned int net_cmd;      //网络命令
	unsigned int list_num;     //链表头编号
	int data_len;              //数据长度
	char tmp[sizeof(ajb_data)-2*sizeof(unsigned int)-sizeof(int)];//填充位，无作用
	unsigned int num;          //网络编号
}net_addr_msg;

typedef struct{
	char flag_addr;            //标志位
	unsigned int net_cmd;      //网络命令
	char buf[sizeof(ajb_data)-sizeof(int)-sizeof(unsigned int)];//传地址时此位前四个字节表示地址 10字节
	int data_len;              //数据长度
	unsigned int num;          //网络编号
}net_data_msg;

typedef struct{
	unsigned char msg_type;
	ajb_msg msg;
}proc_msg;

typedef struct{
	char flag_addr;
	ajb_data_ret data;
	unsigned int num;
}ajb_msg_ret;

typedef struct{
	unsigned char msg_type;
	ajb_msg_ret msg;
}proc_msg_ret;

#pragma pack()


typedef struct{
	char dev_no[4]; 			//设备编码
	char dev_ip[4]; 			//IP(不用)
	char dev_level[2]; 		//管理机级别
	char dev_function[2]; 	//管理机功能
	char dev_count[2]; 		//管理的楼栋数
	char dev_table[4]; 		//楼栋编码表
}glj_list_t;

typedef struct{
	char dev_no[4]; 			//设备编码
	char dev_ip[4]; 			//IP(不用)
	char dev_level[2]; 		//管理机级别
	char dev_function[2]; 	//管理机功能
	char dev_count[2]; 		//管理的楼栋数
	char dev_table[4]; 		//楼栋编码表
}ip_code_list_t;



/*
 * NET 创建初始化
 */

void net_com_creat(net_para *net_para_data);
void net_com_delete();
void msg_rev_thread();
void net_com_detect();

void set_net_para(unsigned char DevNo[4],unsigned char DevIp[4],unsigned short DevType);//设置本机消息

//创建AJB数据 被叫
void ajb_send_data_build(ajb_data *data,const unsigned int net_cmd,const unsigned char cmd,\
		const unsigned char dest_No[4],const unsigned char src_No[4]);

//创建AJB数据 主叫 返回数据
void ajb_ret_data_build(ajb_data_ret *data,const unsigned int net_cmd,const unsigned char cmd,\
		const short Allow,const unsigned int oper_type,int resstatus);

//解析AJB数据 被叫
int ajb_send_data_split(unsigned int *net_cmd,unsigned char *cmd,unsigned char **dest_No,\
		unsigned char **src_No,const ajb_data *data);


//解析AJB数据 主叫
void ajb_ret_data_split(unsigned int *net_cmd, unsigned char *cmd,short *Allow,\
		unsigned int *oper_type,int *resstatus,ajb_data_ret *data);




//创建ajb_msg数据
inline void ajb_msg_build(ajb_msg *msg,const unsigned int net_cmd,const unsigned char cmd,\
		const unsigned char dest_No[4],const unsigned char src_No[4],char flag_addr);
//创建ajb_msg数据，带oper_type和resstatus
void ajb_msg_build_whole(ajb_msg *msg,const unsigned int net_cmd,const unsigned char cmd,\
		const unsigned char dest_No[4],const unsigned char src_No[4],unsigned int oper_type,\
		int resstatus,char flag_addr);

//解析ajb_msg数据
void ajb_msg_split(unsigned int *net_cmd,unsigned char *cmd,unsigned char **dest_No,unsigned char **src_No,\
		char *flag_addr,const ajb_msg *msg);
//解析ajb_msg数据，带oper_type和resstatus
void ajb_msg_split_whole(unsigned int *net_cmd,unsigned char *cmd,unsigned char **dest_No,unsigned char **src_No,\
		unsigned int *oper_type,int *resstatus,char *flag_addr,const ajb_msg *msg);

void ajb_glj_msg_build(ajb_msg *msg,const unsigned int net_cmd,const unsigned char cmd,\
		const unsigned char dest_No[4],const void *data,char data_len,char flag_addr);
void ajb_glj_respond_msg_build(ajb_msg *msg,const unsigned int net_cmd,const unsigned char cmd,\
		const unsigned char dest_No[4],const void *data,char data_len,char flag_addr,unsigned int num);

//创建ajb_msg_ret数据
void ajb_msg_ret_build(ajb_msg_ret *ret_msg,const unsigned int net_cmd,const unsigned char cmd,\
		const short allow,const unsigned int oper_type,int resstatus,char flag_addr);

//解析ajb_msg_ret数据
void ajb_msg_ret_split(unsigned int *net_cmd, unsigned char *cmd,short *allow,\
		unsigned int *oper_type,int *resstatus,char *flag_addr,ajb_msg_ret *msg);
void ajb_msg_ret_split_whole(unsigned int *net_cmd, unsigned char *cmd,short *allow,\
		unsigned int *oper_type,int *resstatus,int *dev_type,char *flag_addr,ajb_msg_ret *msg);

void net_addr_msg_split(unsigned int *net_cmd,unsigned int *list_num,unsigned int *num,int *,net_addr_msg*msg);

#if 1
int init_pipe_udp_ui();
int send_ui_data_to_udp(void*buf,int len);//UI发送数据到 UDP
int get_udp_data_from_ui(void*buf,int len);//UDP接收UI发送到的数据
int get_ui_data_from_udp(void*buf,int len);//UI接收UDP发送的数据
int send_udp_data_to_ui(void*buf,int len);//UDP发送数据到 UI
#endif
void get_msg_handle(struct msg_t*);
int send_data_to_proc(void*buf,int len);//发送数据到 PROC

int send_data_to_net(void*buf,int len);//发送数据到 NET
int get_data_from_proc(void*buf,int len);

int get_data_from_uipro_thread(void*buf,int len);
int send_data_to_uipro_cache(void*buf,int len);
int init_pipe_uipro2cache(void);

int send_addr_to_server(unsigned int net_cmd,void *buf,int len);
int send_addr_to_server_bydev_NO(unsigned int net_cmd,unsigned char devdest[4],void *buf,int len);
int send_addr_to_tml(unsigned int net_cmd,void *dest_NO,void *buf,int len);
int send_data_to_server(unsigned int net_cmd,void *buf,int len);
int respond_to_server(unsigned int net_cmd,void *buf,int len,int);
int send_to_server(unsigned int net_cmd,void *buf,int len);
int send_data_to_udp(void *buf,int len);
int respond_addr_to_server(unsigned int net_cmd,void *buf,int len,int num);
int respond_data_to_server(unsigned int net_cmd,void *buf,int len,int num);

int send_data_to_pc();

//获取传输内存地址数据
int get_list_size(int num);
int get_list_data(int num,void *buf);
int free_list_data(int num);

void set_devstatus(unsigned int val);
unsigned int get_devstatus(void);
void get_devNo(unsigned char *devNo);

/*void set_coon_status(int val);
int  get_coon_status();*/

#endif /* DATA_COM_UI_H_ */
