/*
 * net_event.c
 *
 *  Created on: 2012-9-20 上午10:27:06
 *  
 */
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "net_event.h"
#include "net_com_struct.h"
#include "data_com.h"


static int coon_counter=0;

typedef struct
{
	int write_pos;
	connect_node_destip current_node_destip[COON_QUANTITY*2];
}node_destip_arry_t;

static node_destip_arry_t node_destip_arry;

struct tcp_connect_array_t{
	connect_node tcp_connect_node;
	int used;
};

static struct tcp_connect_array_t tcp_connect_array[COON_LIST_QUANTITY];
static pthread_mutex_t reg_node_lock=PTHREAD_MUTEX_INITIALIZER;


void * malloc_connect_node(int size)
{
	int ret=-1;
	ret = pthread_mutex_lock(&reg_node_lock);
	if (ret < 0){
		perror("Pthread_lock failed");
		return NULL;
	}
	connect_node *new_item=NULL;
	int i=0;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if(tcp_connect_array[i].used==0){
			new_item=(connect_node *)&tcp_connect_array[i].tcp_connect_node;
			tcp_connect_array[i].used=1;
			coon_counter++;
            app_debug(DBG_INFO, "coon_counter increase to %d !\n",coon_counter);            
			break;
		}
	}

	pthread_mutex_unlock(&reg_node_lock);
	return new_item;
}


int unregister_connect_node(connect_node *net_runtime)
{
	app_debug(DBG_INFO,"unregister_connect_node!\n");
	int ret = pthread_mutex_lock(&reg_node_lock);
	if (ret < 0){
		perror("Pthread_lock failed");
		return -1;
	}
	int i=0;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if(tcp_connect_array[i].used==1){
			if(net_runtime->event_num==tcp_connect_array[i].tcp_connect_node.event_num)
			{
				tcp_connect_array[i].used=0;
				coon_counter--;
				memset(&tcp_connect_array[i].tcp_connect_node,0xff,sizeof(connect_node));
	            app_debug(DBG_INFO, "coon_counter decrease to  %d !\n",coon_counter);  //  3				
				break;
			}
		}

	}
	pthread_mutex_unlock(&reg_node_lock);
	return 0;
}

int save_dest_ip(unsigned int dest_ip,unsigned int event_num)
{
    int ret = pthread_mutex_lock(&reg_node_lock);
	if (ret < 0){
		perror("Pthread_lock failed");
		return -1;
	}    
	node_destip_arry.current_node_destip[node_destip_arry.write_pos].dest_ip=dest_ip;
	node_destip_arry.current_node_destip[node_destip_arry.write_pos].event_num=event_num;
	node_destip_arry.write_pos++;
	if(node_destip_arry.write_pos>=COON_QUANTITY*2)
		node_destip_arry.write_pos=0;
	pthread_mutex_unlock(&reg_node_lock);	
	return 0;
}

unsigned int get_current_connect_ip(unsigned int event_num)
{
	int pos=0;
	uint32_t dest_ip=INADDR_NONE;
	for(pos=0;pos<COON_QUANTITY*2;pos++)
	{
		if(event_num==node_destip_arry.current_node_destip[pos].event_num){
			dest_ip=node_destip_arry.current_node_destip[pos].dest_ip;
			break;
		}
	}
	return dest_ip;
}

int find_connect_node(unsigned int event_num,connect_node **net_runtime)
{
	int get_node=-1;
	int ret = pthread_mutex_lock(&reg_node_lock);
	if (ret < 0){
		perror("Pthread_lock failed");
		return -1;
	}
	int i=0;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if(tcp_connect_array[i].used==1){
			if(event_num==tcp_connect_array[i].tcp_connect_node.event_num){
				(*net_runtime)=&tcp_connect_array[i].tcp_connect_node;
				app_debug(DBG_INFO,"tcp_connect_list is %d\n",i);
				get_node=0;
				break;
			}
		}
	}
	pthread_mutex_unlock(&reg_node_lock);
	return get_node;
}

int is_event_num_equal(unsigned int event_num)
{
	int get_node=0;
	int ret = pthread_mutex_lock(&reg_node_lock);
	if (ret < 0){
		perror("Pthread_lock failed");
		return -1;
	}
	int i=0;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
			if((event_num==tcp_connect_array[i].tcp_connect_node.event_num)||\
					(get_current_connect_ip(event_num)!=INADDR_NONE)){
				get_node++;
				break;
			}
	}
	pthread_mutex_unlock(&reg_node_lock);
	return get_node;
}


int create_net_connect_manager()
{
	app_debug(DBG_INFO,"net_connect_manager_thread creating!\n");
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	srand ((unsigned)time(NULL));
	int i=0;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		tcp_connect_array[i].used=0;
		memset(&tcp_connect_array[i].tcp_connect_node,0xff,sizeof(connect_node));
	}
	app_debug(DBG_INFO,"net_connect_manager_thread create success!\n");
	return 0;
}

void delete_net_connect_manager()
{
	pthread_attr_destroy(&attr);
	app_debug(DBG_INFO,"net_man_thread exited!\n");
}

