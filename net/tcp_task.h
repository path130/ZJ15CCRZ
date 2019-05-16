/*
 * tcp_task.h
 *
 *  Created on: 2012-9-19 下午3:52:01
 *  
 */

#ifndef TCP_TASK_H_
#define TCP_TASK_H_
#include "net_data_pro.h"
#include "net_com_struct.h"
#include "ring_buf.h"

#define NET_BUF_SIZE 1024
#define NET_BUF_NUM  64

#if defined (__cplusplus)
extern "C" {
#endif

enum{
    TCP_TML=0x8001,
    TCP_PC,
    TCP_PC_JSON
};

extern ring_buf_s tcp_send_pc_buf;
extern ring_buf_s tcp_send_pc_json_buf;

typedef struct {
	unsigned char *buf;
	char cmd_buf[20];
	int length;
	unsigned int dest_ip;
}tcp_data;

/*typedef struct{
	net_mem *list_head;
	int useing;
}net_mem_s;
net_mem_s list_head[LIST_QUANTITY];*/

typedef struct {
	int items;
	unsigned int start_addr;
	int all_len;
}tcp_tr_addr;

typedef struct {
	int length;
	unsigned int buf;
}tcp_tr_data;

void tcp_task_create();
void tcp_task_delete();

int request_send_buf(tcp_data **buf,int,int path);
//int send_tcp_data(unsigned int *dest_ip,tcp_tr_addr *mem_add);
int send_tcp_data_to_pc(void *buf,int len);
int send_tcp_data(int *item,int path);
int send_data_again(void *buf,int len);

inline int send_data_to_socket(int sock,void *buf,int len);
/*void tcp_process_task(connect_node *);
int send_task(unsigned int dest_addr,char *buf,int length);*/
#if defined (__cplusplus)
}
#endif


#endif /* TCP_TASK_H_ */
