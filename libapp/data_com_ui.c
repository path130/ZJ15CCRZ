/*
 * data_com_ui.c
 *
 *  Created on: 2012-6-28
 *
 */

/*
 * AJB数据处理
 */
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include "data_com_ui.h"
#include "cmd_def.h"
#include "ajb_bus.h"
#include "global_def.h"

DevList my_devlist;
pipe_handle net2proc,proc2net,uipro2cache;

//#if DEV_GLJKZQ|DEV_TSGLJKZQ              
#ifdef CFG_SUPPORT_INFO_STORE     
pipe_handle udp2ui,ui2udp;

int init_pipe_udp_ui()
{
	udp2ui=pipe_create();
	if(udp2ui==NULL){
		app_debug(DBG_FATAL,"udp2ui create FAILURE.\n");
		goto err;
	}
	ui2udp=pipe_create();
	if(ui2udp==NULL){
		app_debug(DBG_FATAL,"ui2udp create FAILURE.\n");
		goto err;
	}
	return 0;
err:
	app_debug(DBG_FATAL,"init_pipe_udp_ui FAILURE.\n");
	return (-1);
}

int send_ui_data_to_udp(void*buf,int len)//UI发送数据到 UDP
{
	return pipe_put(ui2udp,buf,len);
}

int get_udp_data_from_ui(void*buf,int len)//UDP接收UI发送到的数据
{
	return pipe_get(ui2udp,buf,len);
}

int get_ui_data_from_udp(void*buf,int len)//UI接收UDP发送的数据
{
	return pipe_get(udp2ui,buf,len);
}

int send_udp_data_to_ui(void*buf,int len)//UDP发送数据到 UI
{
	return pipe_put(udp2ui,buf,len);
}
#endif

int init_pipe_uipro2cache(void)
{
	uipro2cache=pipe_create();
	if(uipro2cache==NULL){
		fprintf(stderr,"uipro2cache create FAILURE.\n");
		goto err;
	}
	return 0;
err:
	fprintf(stderr,"init uipro2cache FAILURE.\n");
	return (-1);
}


/**
 *
 * 查询设备状态
 * @return 返回设备状态值
*/
unsigned int get_devstatus(void)
{
return my_devlist.DevStatus;
}

/**
 *
 * 获取本机房号
 * @param [out] devNo 返回四位房号
*/

void get_devNo(unsigned char *devNo)
{
	memcpy(devNo,my_devlist.DevNo,4);
}

/**
 *
 * 设置设备状态
 * @param [in] val 设备状态值
*/

void set_devstatus(unsigned int val)
{
	printf("set_devstatus!!!!!!!!!!!!\n");
	my_devlist.DevStatus=val;
}

void get_msg_handle(struct msg_t*p_msg)
{
	proc2net = p_msg->pipe_net;
	net2proc = p_msg->pipe_proc;
}

/**
 *
 * 网络发送数据到 PROC
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度
 * @return 成功返回0，否则返回-1
*/

int send_data_to_proc(void*buf,int len)
{
	return pipe_put(net2proc,buf,len);
}

/**
 *
 * PROC发送数据到网络
 * @param [in] buf 待发送数据
 * @param [in] len 数据长度
 * @return 成功返回0，否则返回-1
*/

int send_data_to_net(void*buf,int len)//发送数据到 NET
{
	return pipe_put(proc2net,buf,len);
}

/**
 *
 * 网络从PROC获取数据
 * @param [in] len 数据长度
 * @param [out] buf 获取数据
 * @return 成功返回0，否则返回-1
*/

int get_data_from_uipro_thread(void*buf,int len)//接收PROC发送到网络的数据
{
	return pipe_get(uipro2cache,buf,len);
}

int send_data_to_uipro_cache(void*buf,int len)//接收PROC发送到网络的数据
{
	return pipe_put(uipro2cache,buf,len);
}

/**
 *
 * 网络从PROC获取数据
 * @param [in] len 数据长度
 * @param [out] buf 获取数据
 * @return 成功返回0，否则返回-1
*/

int get_data_from_proc(void*buf,int len)//接收PROC发送到网络的数据
{
	return pipe_get(proc2net,buf,len);
}


/**
 *
 * 创建ajb_data类型数据包
 * @param [in] net_cmd 网络协议命令字
 * @param [in] cmd 串口协议命令字
 * @param [in] dest_No 四位目标房号
 * @param [in] src_No 四位源房号
 * @param [out] data 返回的ajb_data类型数据
*/

void ajb_send_data_build(ajb_data *data,const unsigned int net_cmd,const unsigned char cmd,\
		const unsigned char dest_No[4],const unsigned char src_No[4])
{
	data->net_cmd=net_cmd;
	data->cmd=cmd;
	memcpy(data->dest_No,dest_No,4);
	memcpy(data->src_No,src_No,4);
	/*data->check=(data->cmd+data->dest_No[0]+data->dest_No[1]+data->dest_No[2]+data->dest_No[3]\
			+data->src_No[0]+data->src_No[1]+data->src_No[2]+data->src_No[3])&0xff;*/
}


/**
 *
 * 创建ajb_data_ret类型数据包
 * @param [in] net_cmd 网络协议命令字
 * @param [in] cmd 串口协议命令字
 * @param [in] allow 允许标志 0表示运行，-1表示不允许
 * @param [in] oper_type 操作类型
 * @param [in] resstatus 资源占用标志
 * @param [out] data 返回的ajb_data_ret类型数据
*/

void ajb_ret_data_build(ajb_data_ret *data,const unsigned int net_cmd,const unsigned char cmd,\
		const short allow,const unsigned int oper_type,int resstatus)
{
	data->net_cmd=net_cmd;
	data->cmd=cmd;
	data->allow=allow;
	data->oper_type=oper_type;
	data->resstatus=resstatus;
}

/**
 *
 * 解析ajb_data类型数据包
 * @param [in] data 输入ajb_data类型数据
 * @param [out] net_cmd 网络协议命令字
 * @param [out] cmd 串口协议命令字
 * @param [out] dest_No 四位目标房号
 * @param [out] src_No 四位源房号
*/

int ajb_send_data_split(unsigned int *net_cmd,unsigned char *cmd,unsigned char **dest_No,unsigned char **src_No,const ajb_data *data)
{
	*net_cmd=data->net_cmd;
	*cmd=data->cmd;
	memcpy(*dest_No,data->dest_No,4);
	memcpy(*src_No,data->src_No,4);
	return 0;
}

/**
 *
 * 解析ajb_data_ret类型数据包
 * @param [in]  data 输入ajb_data_ret类型数据
 * @param [out] net_cmd 网络协议命令字
 * @param [out] cmd 串口协议命令字
 * @param [out] allow 允许标志 0表示运行，-1表示不允许
 * @param [out] oper_type 操作类型
 * @param [out] resstatus 资源占用标志
*/

void ajb_ret_data_split(unsigned int *net_cmd, unsigned char *cmd,short *allow,\
		unsigned int *oper_type,int *resstatus,ajb_data_ret *data)
{
	*net_cmd=data->net_cmd;
	*cmd=data->cmd;
	*allow=data->allow;
	*oper_type=data->oper_type;
	*resstatus=data->resstatus;
}

/**
 *
 * 创建ajb_msg类型数据包
 * @param [in] net_cmd 网络协议命令字
 * @param [in] cmd 串口协议命令字
 * @param [in] dest_No 四位目标房号
 * @param [in] src_No 四位源房号
 * @param [in] flag_addr 数据类型标志位
 * @param [out] msg 返回的ajb_msg类型数据
*/

void ajb_msg_build(ajb_msg *msg,const unsigned int net_cmd,const unsigned char cmd,\
		const unsigned char dest_No[4],const unsigned char src_No[4],char flag_addr)
{
	msg->data.net_cmd=net_cmd;
	msg->data.cmd=cmd;
	memcpy(msg->data.dest_No,dest_No,4);
	memcpy(msg->data.src_No,src_No,4);
	msg->data.check=(msg->data.cmd+msg->data.dest_No[0]+msg->data.dest_No[1]+msg->data.dest_No[2]+msg->data.dest_No[3]\
			+msg->data.src_No[0]+msg->data.src_No[1]+msg->data.src_No[2]+msg->data.src_No[3])&0xff;
	msg->flag_addr=flag_addr;
}

/**
 *
 * 创建ajb_msg类型数据包
 * @param [in] net_cmd 网络协议命令字
 * @param [in] cmd 串口协议命令字
 * @param [in] dest_No 四位目标房号
 * @param [in] src_No 四位源房号
 * @param [in] oper_type 操作类型
 * @param [in] resstatus 资源占用
 * @param [in] flag_addr 数据类型标志位
 * @param [out] msg 返回的ajb_msg类型数据
*/
void ajb_msg_build_whole(ajb_msg *msg,const unsigned int net_cmd,const unsigned char cmd,\
		const unsigned char dest_No[4],const unsigned char src_No[4],unsigned int oper_type,\
		int resstatus,char flag_addr)
{
	msg->data.net_cmd=net_cmd;
	msg->data.cmd=cmd;
	memcpy(msg->data.dest_No,dest_No,4);
	memcpy(msg->data.src_No,src_No,4);
	msg->data.oper_type=oper_type;
	msg->data.resstatus=resstatus;
	msg->data.check=(msg->data.cmd+msg->data.dest_No[0]+msg->data.dest_No[1]+msg->data.dest_No[2]+msg->data.dest_No[3]\
			+msg->data.src_No[0]+msg->data.src_No[1]+msg->data.src_No[2]+msg->data.src_No[3])&0xff;
	msg->flag_addr=flag_addr;
}

/**
 *
 * 创建ajb_msg类型数据包,针对518命令中不带目标房号的数据发送
 * @param [in] net_cmd 网络协议命令字
 * @param [in] cmd 串口协议命令字
 * @param [in] dest_No 四位目标房号
 * @param [in] src_No 四位源房号
 * @param [in] data 518数据中串口协议命令后面的数据
 * @param [in] data_len 518数据串口协议命令后面的数据的长度
 * @param [in] flag_addr 数据类型标志位
 * @param [out] msg 返回的ajb_msg类型数据
*/

void ajb_glj_msg_build(ajb_msg *msg,const unsigned int net_cmd,const unsigned char cmd,\
		const unsigned char dest_No[4],const void *data,char data_len,char flag_addr)
{
	msg->data.net_cmd=net_cmd;
	msg->data.cmd=cmd;
	memcpy(msg->data.dest_No,dest_No,4);
	msg->data.src_No[0]=data_len;
	memcpy(&msg->data.src_No[1],data,data_len);
	msg->flag_addr=flag_addr;
}

/**
 *
 * 创建ajb_msg类型数据包,针对518命令回复密码开锁
 * @param [in] net_cmd 网络协议命令字
 * @param [in] cmd 串口协议命令字
 * @param [in] dest_No 四位目标房号
 * @param [in] src_No 四位源房号
 * @param [in] data 518数据中串口协议命令后面的数据
 * @param [in] data_len 518数据串口协议命令后面的数据的长度
 * @param [in] flag_addr 数据类型标志位
 * @param [out] msg 返回的ajb_msg类型数据
*/

void ajb_glj_respond_msg_build(ajb_msg *msg,const unsigned int net_cmd,const unsigned char cmd,\
		const unsigned char dest_No[4],const void *data,char data_len,char flag_addr,unsigned int num)
{
	msg->data.net_cmd=net_cmd;
	msg->data.cmd=cmd;
	memcpy(msg->data.dest_No,dest_No,4);
	msg->data.src_No[0]=data_len;
	memcpy(&msg->data.src_No[1],data,data_len);
	msg->flag_addr=flag_addr;
	msg->num=num;
}

/*void ajb_glj_msg_split(unsigned int *net_cmd,unsigned char *cmd,unsigned char **dest_No,unsigned char **src_No,\
		char *flag_addr,const ajb_msg *msg)
{
	*net_cmd=msg->data.net_cmd;
	*cmd=msg->data.cmd;
	memcpy(*dest_No,msg->data.dest_No,4);
	memcpy(*src_No,msg->data.src_No,4);
	*flag_addr=msg->flag_addr;
}*/

/**
 *
 * 解析ajb_msg类型数据包
 * @param [in] msg 输入ajb_data_ret类型数据
 * @param [out] net_cmd 网络协议命令字
 * @param [out] cmd 串口协议命令字
 * @param [out] dest_No 目标房号
 * @param [out] src_No 源房号
 * @param [out] flag_addr 数据类型标志位
*/

void ajb_msg_split(unsigned int *net_cmd,unsigned char *cmd,unsigned char **dest_No,unsigned char **src_No,\
		char *flag_addr,const ajb_msg *msg)
{
	*net_cmd=msg->data.net_cmd;
	*cmd=msg->data.cmd;
	memcpy(*dest_No,msg->data.dest_No,4);
	memcpy(*src_No,msg->data.src_No,4);
	*flag_addr=msg->flag_addr;
}

/**
 *
 * 解析ajb_msg类型数据包
 * @param [in] msg 输入ajb_data_ret类型数据
 * @param [out] net_cmd 网络协议命令字
 * @param [out] cmd 串口协议命令字
 * @param [out] dest_No 目标房号
 * @param [out] src_No 源房号
 * @param [out] oper_type 操作类型
 * @param [out] resstatus 资源占用
 * @param [out] flag_addr 数据类型标志位
*/
void ajb_msg_split_whole(unsigned int *net_cmd,unsigned char *cmd,unsigned char **dest_No,unsigned char **src_No,\
		unsigned int *oper_type,int *resstatus,char *flag_addr,const ajb_msg *msg)
{
	*net_cmd=msg->data.net_cmd;
	*cmd=msg->data.cmd;
	memcpy(*dest_No,msg->data.dest_No,4);
	memcpy(*src_No,msg->data.src_No,4);
	*oper_type=msg->data.oper_type;
	*resstatus=msg->data.resstatus;
	*flag_addr=msg->flag_addr;
}

/**
 *
 * 创建ajb_msg_ret类型数据包,针对518命令回复密码开锁
 * @param [in] net_cmd 网络协议命令字
 * @param [in] cmd 串口协议命令字
 * @param [in] allow 允许标志 0表示运行，-1表示不允许
 * @param [in] oper_type 操作类型
 * @param [in] resstatus 资源占用
 * @param [out] ret_msg 返回的ajb_msg类型数据
*/

void ajb_msg_ret_build(ajb_msg_ret *ret_msg,const unsigned int net_cmd,const unsigned char cmd,\
		const short allow,const unsigned int oper_type,int resstatus,char flag_addr)
{
	ret_msg->data.net_cmd=net_cmd;
	ret_msg->data.cmd=cmd;
	ret_msg->data.allow=allow;
	ret_msg->data.oper_type=oper_type;
	ret_msg->data.resstatus=resstatus;
	ret_msg->flag_addr=flag_addr;
}


/**
 *
 * 解析ajb_msg_ret类型数据包
 * @param [in] msg 输入ajb_msg_ret类型数据
 * @param [out] net_cmd 网络协议命令字
 * @param [out] cmd 串口协议命令字
 * @param [out] allow 允许标志 0表示运行，-1表示不允许
 * @param [out] oper_type 操作类型
 * @param [out] resstatus 资源占用
 * @param [out] flag_addr 数据类型标志位
*/

void ajb_msg_ret_split(unsigned int *net_cmd, unsigned char *cmd,short *allow,\
		unsigned int *oper_type,int *resstatus,char *flag_addr,ajb_msg_ret *msg)
{
	*net_cmd=msg->data.net_cmd;
	*cmd=msg->data.cmd;
	*allow=msg->data.allow;
	*oper_type=msg->data.oper_type;
	*resstatus=msg->data.resstatus;
	*flag_addr=msg->flag_addr;
}

/**
 *
 * 解析ajb_msg_ret类型数据包
 * @param [in] msg 输入ajb_msg_ret类型数据
 * @param [out] net_cmd 网络协议命令字
 * @param [out] cmd 串口协议命令字
 * @param [out] allow 允许标志 0表示运行，-1表示不允许
 * @param [out] oper_type 操作类型
 * @param [out] resstatus 资源占用
 * @param [out] dev_type 设备类型
 * @param [out] flag_addr 数据类型标志位
*/

void ajb_msg_ret_split_whole(unsigned int *net_cmd, unsigned char *cmd,short *allow,\
		unsigned int *oper_type,int *resstatus,int *dev_type,char *flag_addr,ajb_msg_ret *msg)
{
	*net_cmd=msg->data.net_cmd;
	*cmd=msg->data.cmd;
	*allow=msg->data.allow;
	*oper_type=msg->data.oper_type;
	*resstatus=msg->data.resstatus;
	*dev_type=msg->data.dev_type;
	*flag_addr=msg->flag_addr;
}

void net_addr_msg_split(unsigned int *net_cmd,unsigned int *list_num,unsigned int *num,int *data_len,net_addr_msg*msg)
{
	*net_cmd=msg->net_cmd;
	*list_num=msg->list_num;
	*num=msg->num;
	*data_len=msg->data_len;
}


/**
 *
 * 设置网络参数
 * @param [in] DevNo 四位本机房号
 * @param [in] DevIp 四位本机IP
 * @param [in] DevType 本机类型
*/

void set_net_para(unsigned char DevNo[4],unsigned char DevIp[4],unsigned short DevType)//设置网络参数
{
	memcpy(my_devlist.DevNo,DevNo,4);
	memcpy(my_devlist.DevIp,DevIp,4);
	my_devlist.DevType=DevType;
	my_devlist.DevStatus=0;
	my_devlist.Default=DEFAULT_JDM365;
	app_debug(DBG_INFO, "My DEVNO : %x,%x,%x,%x\n",my_devlist.DevNo[0],my_devlist.DevNo[1],\
			my_devlist.DevNo[2],my_devlist.DevNo[3]);
}


