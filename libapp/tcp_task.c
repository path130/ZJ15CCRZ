/*
 * tcp_task.c
 *
 *  Created on: 2012-9-19 下午3:52:19
 *  
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "data_com.h"
#include "tcp_task.h"
#include "net_run.h"
#include "net_event.h"
#include "public.h"
#include "ring_buf.h"



static int recv_run=0;
static int send_run=0;

static pthread_t thread_tcp_recv;
static pthread_t thread_tcp_send;
static ring_buf_s tcp_send_buf;

int pipe_tcp_data[2];

void tcp_recv_task()
{
	app_debug(DBG_INFO,"tcp_recv_task starting!\n");
	int tcp_sock_rec;
	struct sockaddr_in rec_server;
	struct sockaddr_in rec_client;
	int reuse=1;
	if ((tcp_sock_rec=socket(AF_INET,SOCK_STREAM,0))<0)
		{
		app_debug(DBG_FATAL,"socket() failure\n");
		exit(EXIT_FAILURE);
	}
	setsockopt(tcp_sock_rec, SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(reuse));
	bzero(&rec_server,sizeof(rec_server));
	rec_server.sin_family = AF_INET;
	rec_server.sin_port =htons(TCP_PORT);
	rec_server.sin_addr.s_addr = htonl (INADDR_ANY);

	net_bind(tcp_sock_rec,&rec_server);
	net_listen(tcp_sock_rec,COON_QUANTITY);

	while(recv_run)
	{
		connect_node *net_runtime=NULL;
		net_runtime=malloc_connect_node(sizeof(connect_item));
		if(net_runtime==NULL){
			usleep(5000);
			app_debug(DBG_FATAL, "NULL\n");
			continue;
		}

		do{
			net_runtime->event_num=rand()%0x7fffff;
		}
		while(get_current_connect_ip(net_runtime->event_num)!=INADDR_NONE);
		socklen_t client_len=sizeof(struct sockaddr);
		net_runtime->socketfd=accept(tcp_sock_rec,(struct sockaddr *)&rec_client,&client_len);
		if(net_runtime->socketfd>0)
		{
			net_runtime->connect_type=RECV_TML_NODE;
			net_runtime_creat(net_runtime,TYPE_TCP);
		}
		else
		{
			usleep(5000);
		}
	}
	net_close(&tcp_sock_rec);
	app_debug(DBG_INFO,"tcp_recv_task closed!\n");
}

int get_send_data_to_tcp(void *buf)
{
	int ret;
	ret=pipe_read(pipe_tcp_data[PIPE_READ],buf,sizeof(int));
	return ret;
}

int send_data_to_tcp(void *buf,int len)
{
	int ret;
	ret=pipe_write(pipe_tcp_data[PIPE_WRITE],buf,len);
	return ret;
}



int request_send_buf(tcp_data **buf,int length,int path)
{
	switch(path)
	{
	case TCP_TML:
	{
		*buf = (tcp_data *) get_write_pos(&tcp_send_buf);
		if (*buf == NULL)
			return -1;
		if(length>NET_BUF_SIZE){
			(*buf)->buf=realloc((*buf)->buf,length);
		}
	}
	break;
	case TCP_PC:
	{
		*buf=(tcp_data *)get_write_pos(&tcp_send_pc_buf);
		if(*buf==NULL)
			return -1;
		if(length>NET_BUF_SIZE){
			(*buf)->buf=realloc((*buf)->buf,length);
		}
	}
	break;
	case TCP_PC_JSON:
	{
		*buf=(tcp_data *)get_write_pos(&tcp_send_pc_json_buf);
		if(*buf==NULL)
			return -1;
		if(length>NET_BUF_SIZE){
			(*buf)->buf=realloc((*buf)->buf,length);
		}
	}
	break;	
	default:
		return -1;
		break;
	}
	return 0;
}

int send_tcp_data(int *item, int path) {
	int ret = -1;
	switch (path) {
	case TCP_TML:
		ret = send_data_to_tcp((void*) item, sizeof(int));
		if (ret < 0) {
			app_debug(DBG_ERROR, "creat_tcp_client failure!\n");
			return -1;
		}
		break;
	case TCP_PC:
		ret = send_tcp_data_to_pc((void*) item, sizeof(int));
		if (ret < 0) {
			app_debug(DBG_ERROR, "creat_tcp_client failure!\n");
			return -1;
		}
		break;
	case TCP_PC_JSON:
		ret = send_tcp_json_data_to_pc((void*) item, sizeof(int));
		if (ret < 0) {
			app_debug(DBG_ERROR, "creat_tcp_client failure!\n");
			return -1;
		}
		break;		
	default:
		return -1;
		break;
	}
	return 0;
}

int send_data_to_socket(int sock,void *buf,int len)
{
	int send_item=COUNT_ITEM(len);
	int ret=0,send_len=0,i=0;
	for(i=0;i<send_item-1;i++){
		send_len=send_data(sock,buf+MAX_LEN*i,MAX_LEN);
		ret+=send_len;
	}
	send_len=send_data(sock,buf+MAX_LEN*i,len-MAX_LEN*i);
	ret+=send_len;
	return ret;
}

void tcp_send_task()
{
	app_debug(DBG_INFO,"tcp_send_task starting!\n");
	struct sockaddr_in  send_client;
	bzero(&send_client,sizeof(send_client));
	send_client.sin_family = AF_INET;
	pipe_creat(pipe_tcp_data);
	while(send_run)
	{
		int ret_len;
		int tcp_sock_send=-1;
		int items;
		ret_len=get_send_data_to_tcp(&items);
		if(ret_len==sizeof(int))
		{

			tcp_data *tr_data;
			int loop=3;
			do{
				tr_data=(tcp_data *)get_read_pos(&tcp_send_buf);
				if(tr_data==NULL)
				{
					//pthread_yield();
					usleep(25000);
					continue;
				}
				else
				{
					break;
				}
			}while(loop--);

			if(tr_data==NULL)
			{
				app_debug(DBG_ERROR, "get_send_data_to_tcp error!\n");
				continue;
			}

			if ((tcp_sock_send = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				perror("tcp_sock_send socket err: ");
				usleep(50000);
				exit(EXIT_FAILURE);
				continue;
			}
			struct in_addr inaddr;
			memcpy(&inaddr,&tr_data->dest_ip,4);
			send_client.sin_port =htons(TCP_PORT);
			send_client.sin_addr=inaddr;
			app_debug(DBG_INFO,"connecting -> (%s) 18022\n",inet_ntoa(inaddr));
			int ret;
			connect_node *net_runtime=NULL;
			net_runtime=malloc_connect_node(sizeof(connect_item));
			if(net_runtime==NULL){
				usleep(50000);
				app_debug(DBG_FATAL, "net_runtime is NULL\n");
				continue;
			}
			do{
				net_runtime->event_num=rand()%0x7fffff;
			}
			while(get_current_connect_ip(net_runtime->event_num)!=INADDR_NONE);
			//ret=net_connect(tcp_sock_send,(struct sockaddr*)&send_client);
			socklen_t len=sizeof(struct sockaddr);
			ret=connect_nonb(tcp_sock_send,(struct sockaddr*)&send_client,len,CONNECT_TIMEOUT);
			if(ret==0)
			{
				app_debug(DBG_INFO,"connect ok\n");
				net_runtime->socketfd=tcp_sock_send;
				net_runtime->connect_type=SEND_TML_NODE;
				net_runtime_creat(net_runtime,TYPE_TCP);

				switch(items)
				{
				case ITEM_ONE:
				{
					if(tr_data->length>0){
						send_data_to_socket(tcp_sock_send,tr_data->buf,tr_data->length);
						if(tr_data->length>NET_BUF_SIZE)
							tr_data->buf=realloc(tr_data->buf,NET_BUF_SIZE);
						tr_data->length=0;
					}
				}
					break;
				case ITEM_TWO:
				{
					send_data_to_socket(tcp_sock_send,tr_data->cmd_buf,20);
					if(tr_data->length>0)
						send_data_to_socket(tcp_sock_send,tr_data->buf,tr_data->length);
					if(tr_data->length>NET_BUF_SIZE)
						tr_data->buf=realloc(tr_data->buf,NET_BUF_SIZE);
					tr_data->length=0;
				}
					break;
				default :
					break;
				}
				app_debug(DBG_INFO, "package has been sent to tcp(%s)!\n",inet_ntoa(inaddr));
			}
			else
			{
				app_debug(DBG_INFO, "net_connect err!\n");
				if(net_runtime!=NULL)
					unregister_connect_node(net_runtime);
				net_close(&tcp_sock_send);
			}
		}
		else{
			usleep(10000);
			continue;
		}

	}
	pipe_close(pipe_tcp_data);
	app_debug(DBG_INFO,"tcp_send_task closed!\n");
}


void tcp_task_create()
{
	int ret=-1;
	recv_run = 1;
	send_run = 1;
	ret = ring_buf_creat(&tcp_send_buf,sizeof(tcp_data), NET_BUF_NUM);
	if (ret < 0) {
		app_debug(DBG_FATAL,"Create tcp_send_buf failure!");
		exit(EXIT_FAILURE);
	}

	int i=0;
	for(i=0;i<NET_BUF_NUM;i++)
	{
		tcp_data *tr_data;
		tr_data=(tcp_data *)get_write_pos(&tcp_send_buf);
		tr_data->buf=malloc(NET_BUF_SIZE);
	}
	for(i=0;i<NET_BUF_NUM;i++)
	{
		tcp_data *tr_data;
		tr_data=(tcp_data *)get_read_pos(&tcp_send_buf);
	}
	ret = pthread_create(&thread_tcp_recv, NULL, (void *) &tcp_recv_task, NULL);
	if (ret < 0) {
		perror("Create thread_tcp_recv!");
		exit(EXIT_FAILURE);
	}
	//pthread_detach(thread_tcp_recv);

	ret = pthread_create(&thread_tcp_send, NULL, (void *) &tcp_send_task, NULL);
	if (ret < 0) {
		perror("Create thread_tcp_send!");
		exit(EXIT_FAILURE);
	}
	//pthread_detach(thread_tcp_send);
}

void tcp_task_delete()
{
	recv_run = 0;
	send_run = 0;

	if (thread_tcp_recv){
		thread_tcp_recv = 0;
		pthread_join(thread_tcp_recv,NULL);
	}
	if (thread_tcp_send){
		thread_tcp_send = 0;
		pthread_join(thread_tcp_send,NULL);
	}
	int i=0;
	for(i=0;i<NET_BUF_NUM;i++)
	{
		tcp_data *tr_data;
		tr_data=(tcp_data *)get_write_pos(&tcp_send_buf);
		free(tr_data->buf);
	}
	ring_buf_delete(&tcp_send_buf);
}

