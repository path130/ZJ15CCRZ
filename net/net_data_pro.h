/*
 * net_data_pro.h
 *
 *  Created on: 2012-6-25
 *
 */

#ifndef NET_DATA_PRO_H_
#define NET_DATA_PRO_H_
#include "net_com_struct.h"
#include "global_data.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define DEVNO_NOT_IN_LIST (-1)

#define    UPDATE_NAME		"exec_update.elf"
#define    UPDATE_APP		"exec_update.elf"
#define    UPDATE_RES		"res_update.tar.gz"

#define	UpdateSetupPro	0x01
#define	UpdateMainPro		0x02
#define	UpdateResData		0x03
#define	Program_max_len	        (5*1024*1024)

#define    PRAG_TYPE_ERR		-1
#define    VERSION_ERR		-2
#define    VERSION_LOW		-3
#define    OTHER_ERR			-4

#define	ARPCALLREQ		0
#define	ARPINFOREQ		1

typedef struct ip_parameter_t{
	uint32_t addr;
	uint16_t port;
}socket_addr_t;

typedef struct net_mem_t
{
    unsigned char net_mem[MAX_LEN];
    int data_count;
    int mem_num;
    struct net_mem_t *next;
}net_mem;

struct net_cmd_struct
{
    char  *string;                          /* ASCII command string     */
    int   id;                               /* Command ID               */
};

int request_list_head_mem(net_mem **head_item);
void delete_list_mem(net_mem *head_item);
int add_net_mem(net_mem **current_node,int num);//为接收链表增加长度


void del_all_net_mem(net_mem **head);//释放链表内存空间

void tcp_process_task(connect_node *);
void tcp_process_task_t(connect_node *net_runtime);
void tcp_pc_process_task(connect_node *net_runtime);
void tcp_pc_json_process_task(connect_node *net_runtime);
void udp_process_task(int socketfd);
void ui_data_pro(char *buf,int len);
void ui_data_pro_thread_cache(char *buf,int len);//处理接收到的UI数据

int respond_to_socket(int sock,const void *buf,int length);
void close_socket(struct connect_node_t* runtime);

int send_task(unsigned int dest_addr,char *buf,int length);
#ifndef HAVE_TONGHAO
int get_dest_ip_by_arp(unsigned char *dest_dev_No,char *ip, signed long req_dev_status,unsigned short addr_way);
#else
//int get_dest_ip_by_arp(unsigned char *dest_dev_No,void *ip, signed long req_dev_status,unsigned short addr_way);
int get_dest_ip_by_arp(unsigned char *dest_dev_No,DevList *dev, signed long req_dev_status,\
		unsigned short addr_way);
int create_dev_arrary(void);
void clear_com_dev_arrary(void);
#endif
int get_net_mem_data(net_mem *node,void *buf);//从链表中读取数据
int get_net_mem_size(net_mem *node);//查询链表中数据长度
unsigned int get_ip_addr();
void clr_ip_flag();
//void set_ip_addr(unsigned char *ip);
void set_ip_flag();
int is_get_ip();
int send_get_village_data_to_server(unsigned int net_cmd,void *buf,int len);
int send_json_addr_to_server(unsigned int net_cmd,void *buf,int len);

#if 1
void ui_udp_data_pro(char *buf,int len);//处理接收到的UI数据
#endif





#if defined (__cplusplus)
}
#endif

#endif /* NET_DATA_PRO_H_ */
