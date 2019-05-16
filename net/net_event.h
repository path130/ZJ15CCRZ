/*
 * net_event.h
 *
 *  Created on: 2012-9-20 上午10:28:47
 *  
 */

#ifndef NET_EVENT_H_
#define NET_EVENT_H_
#include <pthread.h>
#include "public.h"
#include "net_com_struct.h"
#include "event_num.h"
#if defined (__cplusplus)
extern "C" {
#endif

extern pthread_attr_t attr;

typedef struct connect_item_t
{
	connect_node current_item;
	struct connect_item_t *next_item;
}connect_item;

typedef struct event_msg_t
{
	int event_type;
	connect_node current_item;
}net_event_msg;

typedef struct connect_node_destip_t
{
	unsigned int  event_num;
	uint32_t dest_ip;
}connect_node_destip;

int create_net_connect_manager();
void delete_net_connect_manager();

int find_connect_node(unsigned int event_num,connect_node **net_runtime);
int is_event_num_equal(unsigned int event_num);
int save_dest_ip(unsigned int dest_ip,unsigned int event_num);
unsigned int get_current_connect_ip(unsigned int event_num);
//int register_connect_node(connect_item *net_runtime);
void * malloc_connect_node(int size);
int unregister_connect_node(connect_node *net_runtime);
//int send_connect_msg(const void *buf,int count);

#if defined (__cplusplus)
}
#endif

#endif /* NET_EVENT_H_ */
