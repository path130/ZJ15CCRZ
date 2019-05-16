/*
 * net_run.c
 *
 *  Created on: 2012-9-19 下午2:55:59
 *  
 */
#include <pthread.h>
#include "net_run.h"
#include "udp_task.h"
#include "tcp_task.h"
#include "public.h"
#include "data_com_ui.h"
#include "net_event.h"
pthread_attr_t attr;

int net_runtime_creat(connect_node *net_runtime,int type)
{
	int ret=-1;
	struct sockaddr_in peeraddr;
	socklen_t addrlen=sizeof(struct sockaddr_in);
	getpeername(net_runtime->socketfd,(struct sockaddr *)&peeraddr,&addrlen);
	net_runtime->dest_ip=peeraddr.sin_addr.s_addr;
	save_dest_ip(net_runtime->dest_ip,net_runtime->event_num);
	switch(type)
	{
	case TYPE_TCP:
	{
		app_debug(DBG_INFO, "create TYPE_TCP runtime.\n");
		net_runtime->process_fun=tcp_process_task;
		net_runtime->send_to_proc=send_data_to_proc;
		net_runtime->respond_fun=respond_to_socket;
		//net_runtime->close_fun=close_socket;
		net_runtime->connect_on=NET_START;
		ret=pthread_create(&(net_runtime->thread_coon),&attr,(void *)&tcp_process_task,net_runtime);
		if(ret<0){
			app_debug(DBG_ERROR, "pthread_create");
		}

	//	pthread_detach(net_runtime->thread_coon);
	}
		break;
	case TYPE_TCP_PC:
	{
		net_runtime->process_fun=tcp_pc_process_task;
		net_runtime->send_to_proc=send_data_to_proc;
		net_runtime->respond_fun=respond_to_socket;
		//net_runtime->close_fun=close_socket;
		net_runtime->connect_on=NET_START;
		ret=pthread_create(&(net_runtime->thread_coon),&attr,(void *)net_runtime->process_fun,net_runtime);
		if(ret<0){
			app_debug(DBG_ERROR, "pthread_create");
		}
	//	pthread_detach(net_runtime->thread_coon);
	}
		break;
	case TYPE_TCP_PC_JSON:
	{
		net_runtime->process_fun=tcp_pc_json_process_task;
		net_runtime->send_to_proc=send_data_to_proc;
		net_runtime->respond_fun=respond_to_socket;
		//net_runtime->close_fun=close_socket;
		net_runtime->connect_on=NET_START;
		ret=pthread_create(&(net_runtime->thread_coon),&attr,(void *)net_runtime->process_fun,net_runtime);
		if(ret<0){
			app_debug(DBG_ERROR, "pthread_create");
		}
	//	pthread_detach(net_runtime->thread_coon);
	}
		break;		
	default:
		ret=-1;
		break;
	}
	return ret;
}
