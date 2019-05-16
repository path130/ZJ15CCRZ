/*
 * net_com_struct.h
 *
 *  Created on: 2012-9-19 上午11:00:04
 *  
 */

#ifndef NET_COM_STRUCT_H_
#define NET_COM_STRUCT_H_

#include <pthread.h>
#include "global_def.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#if defined (__cplusplus)
extern "C" {
#endif

typedef struct net_data_t
{
	unsigned int  event_num;
	unsigned int 	data_valid;
	unsigned char net_data[MAX_LEN];
	int data_len;
	struct net_data_t *next;
}net_data;


enum{
	RECV_TML_NODE,//终端接收终端
	SEND_TML_NODE,//终端发送终端
	RECV_PC_NODE, //终端接收PC
	SEND_PC_NODE, //终端发送PC
};

typedef struct connect_node_t{
	int socketfd;
	unsigned int connect_on;
	unsigned int dest_ip;
	unsigned int connect_type;
	struct sockaddr_in server,client;
	pthread_t thread_coon;							//当前节点接收和发送处理线程ID
	void (*process_fun)(struct connect_node_t*); //作为SERVER数据处理函数
	int (*respond_fun)(int sock,const void *buf,int length);    //回复函数
	//void (*close_fun)(struct connect_node_t*);    //关闭函数
	int  (*send_to_proc)(void *buf,int length);     //
	int  (*get_from_proc)(void *buf,int length);    //
	unsigned int  event_num;					  		//表示网络事件在链表中的存储序列号
	unsigned int  dev_type;
}connect_node;

//typedef connect_node* netnode_handle;

#if defined (__cplusplus)
}
#endif

#endif /* NET_COM_STRUCT_H_ */
