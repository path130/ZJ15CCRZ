/*
 * tcp_pc_task.c
 *
 *  Created on: 2012-8-6 上午11:57:50
 *  
 */

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include "global_def.h"
#include "cmd_def.h"
#include "net_com.h"
#include "data_com.h"
#include "tcp_pc_task.h"
#include "net_data_pro.h"
#include "global_def.h"
#include "public.h"
#include "ring_buf.h"
#include "net_event.h"
#include "net_init.h"
#include "dev_pro.h"

static int recv_pc_run=0;
static int send_pc_run=0;

static pthread_t thread_tcp_pc_recv;
static pthread_t thread_tcp_pc_send;
//static pthread_t thread_tcp_pc_exec;

ring_buf_s tcp_send_pc_buf;
int pipe_pc_tcp_data[2];


void tcp_pc_recv_task()//接收处理数字社区发送的数据
{
	app_debug(DBG_INFO, "tcp_pc_recv_server starting!\n");
	int tcp_sock_recv;
	struct sockaddr_in recv_server;
	struct sockaddr_in recv_client;
	int reuse=1;
	if ((tcp_sock_recv=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		app_debug(DBG_FATAL,"socket() failure !\n");
		exit(EXIT_FAILURE);
	}
	setsockopt(tcp_sock_recv, SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(reuse));
	bzero(&recv_server,sizeof(recv_server));
	recv_server.sin_family = AF_INET;
	recv_server.sin_port =htons(TCP_PC_PORT);
	recv_server.sin_addr.s_addr = htonl (INADDR_ANY);

	net_bind(tcp_sock_recv,&recv_server);
	net_listen(tcp_sock_recv,COON_QUANTITY);
	while(recv_pc_run)
	{
		connect_node *net_runtime=NULL;
		net_runtime=malloc_connect_node(sizeof(connect_node));
		if(net_runtime==NULL){
			usleep(1000);
			app_debug(DBG_FATAL, "NULL\n");
			continue;
		}
		do{
			net_runtime->event_num=rand()%0x7fffff;
		}
		while(get_current_connect_ip(net_runtime->event_num)!=INADDR_NONE);
		socklen_t client_len=sizeof(struct sockaddr);
		net_runtime->socketfd=accept(tcp_sock_recv,(struct sockaddr *)&recv_client,&client_len);
		//net_runtime->socketfd=net_accept(tcp_sock_recv,(struct sockaddr *)&recv_client);
		if(net_runtime->socketfd>0)
		{
			app_debug(DBG_INFO, "PC is connected!!!!!!!!!!!!!\n");
			net_runtime->connect_type=RECV_PC_NODE;
			net_runtime_creat(net_runtime,TYPE_TCP_PC);
		}
		else
		{
			perror("accept error!");
			usleep(5000);
		}
	}
	net_close(&tcp_sock_recv);
	app_debug(DBG_INFO, "tcp_pc_recv_server closed!\n");
}


int request_pc_send_buf(tcp_data **buf)
{
	*buf=(tcp_data *)get_write_pos(&tcp_send_pc_buf);
	if(buf==NULL)
		return -1;
	return 0;
}

int get_tcp_data_to_pc(void *buf)
{
	int ret;
	ret=pipe_read(pipe_pc_tcp_data[PIPE_READ],buf,sizeof(int));
	return ret;
}

int send_tcp_data_to_pc(void *buf,int len)
{
	int ret;
	ret=pipe_write(pipe_pc_tcp_data[PIPE_WRITE],buf,len);
	return ret-len;
}

int again_fd;
int send_data_again(void *buf,int len)
{
	return write(again_fd,buf,len);
}
void tcp_pc_send_task()
{
	app_debug(DBG_INFO, "tcp_pc_send_client starting!\n");
	struct sockaddr_in  send_client;
	int ret;
	pipe_creat(pipe_pc_tcp_data);
	while(send_pc_run)
	{
        struct in_addr inaddr;	
        int items;        
		int recv_len=get_tcp_data_to_pc(&items);
		if(recv_len==sizeof(int))
		{
			tcp_data *tr_data;
			tr_data=(tcp_data *)get_read_pos(&tcp_send_pc_buf);
			if((tr_data!=NULL)&&(tr_data->length>0)){			
				memcpy(&inaddr,&tr_data->dest_ip,4);
			}
			else{
				app_debug(DBG_ERROR, "get_send_data_to_pc error!\n");
				continue;
			}
			
            int tcp_sock_pc_send;
            if ((tcp_sock_pc_send = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            	perror("tcp_sock_pc_send socket err:");
            	usleep(500000);
            	exit(EXIT_FAILURE);
            	continue;
            }
            bzero(&send_client,sizeof(send_client));
            send_client.sin_family = AF_INET;
            send_client.sin_port =htons(TCP_PC_PORT);
			send_client.sin_addr=inaddr;
            app_debug(DBG_INFO,"connecting -> (%s) 18020\n",inet_ntoa(inaddr));            

            connect_node *net_runtime=NULL;
            net_runtime=malloc_connect_node(sizeof(connect_node));
            if(net_runtime==NULL){
            	usleep(100000);
            	net_close(&tcp_sock_pc_send);
            	app_debug(DBG_FATAL, "net_runtime is NULL\n");
            	continue;
            }
		do{
			net_runtime->event_num=rand()%0x7fffff;
		}
		while(get_current_connect_ip(net_runtime->event_num)!=INADDR_NONE);
				//ret=net_connect(tcp_sock_pc_send,(struct sockaddr*)&send_client);
                socklen_t len=sizeof(struct sockaddr);
				ret=connect_nonb(tcp_sock_pc_send,(struct sockaddr*)&send_client,len,CONNECT_TIMEOUT);
				if(ret==0)
				{
					again_fd=tcp_sock_pc_send;
					app_debug(DBG_INFO, "net_connect 18020 ok!\n");

					net_runtime->socketfd = tcp_sock_pc_send;
					net_runtime->connect_type = SEND_PC_NODE;
					net_runtime_creat(net_runtime, TYPE_TCP_PC);
					usleep(500);

					switch(items)
					{
					case ITEM_ONE:
					{
						send_data_to_socket(tcp_sock_pc_send,tr_data->buf,tr_data->length);
						if(tr_data->length>NET_BUF_SIZE)
							tr_data->buf=realloc(tr_data->buf,NET_BUF_SIZE);
						tr_data->length=0;
					}
					break;
					case ITEM_TWO:
					{
						send_data_to_socket(tcp_sock_pc_send,tr_data->cmd_buf,20);
						if(tr_data->length>0)
							send_data_to_socket(tcp_sock_pc_send,tr_data->buf,tr_data->length);
						if(tr_data->length>NET_BUF_SIZE)
							tr_data->buf=realloc(tr_data->buf,NET_BUF_SIZE);
						tr_data->length=0;
					}
					break;
					default :
						break;
					}
					app_debug(DBG_INFO, "package has been sent to pc server!\n");
				}else{
					app_debug(DBG_INFO, "net_connect 18020 err!\n");
					if(net_runtime!=NULL)
						unregister_connect_node(net_runtime);
					net_close(&tcp_sock_pc_send);
				}
		}
		else
		{
			usleep(10000);
            continue;
		}
	}
	app_debug(DBG_INFO, "tcp_pc_send_client exit!\n");
}


void tcp_pc_task_create()
{
	int ret=-1;
	recv_pc_run=1;
	send_pc_run=1;
	ret = ring_buf_creat(&tcp_send_pc_buf,sizeof(tcp_data), NET_PC_BUF_NUM);
	if (ret < 0) {
		app_debug(DBG_FATAL,"Create tcp_send_pc_buf failure!");
		exit(EXIT_FAILURE);
	}

	int i=0;
	for(i=0;i<NET_PC_BUF_NUM;i++)
	{
		tcp_data *tr_data;
		tr_data=(tcp_data *)get_write_pos(&tcp_send_pc_buf);
		tr_data->buf=malloc(NET_BUF_SIZE);
	}
	for(i=0;i<NET_PC_BUF_NUM;i++)
	{
		tcp_data *tr_data;
		tr_data=(tcp_data *)get_read_pos(&tcp_send_pc_buf);
	}

	ret=pthread_create(&thread_tcp_pc_recv,NULL,(void *)&tcp_pc_recv_task,NULL);
	if(ret<0)
	{
		app_debug(DBG_FATAL,"Create thread_tcp_recv failure!\n");
		exit(EXIT_FAILURE);
	}
	pthread_detach(thread_tcp_pc_recv);

	ret=pthread_create(&thread_tcp_pc_send,NULL,(void *)&tcp_pc_send_task,NULL);
	if(ret<0)
	{
		app_debug(DBG_FATAL,"Create thread_tcp_pc_send failure!\n");
		exit(EXIT_FAILURE);
	}
	pthread_detach(thread_tcp_pc_send);
}

void tcp_pc_task_delete()
{
	recv_pc_run = 0;
	send_pc_run = 0;

	if(recv_pc_run) recv_pc_run=0;
	if(send_pc_run) send_pc_run=0;

	int i=0;
	for(i=0;i<NET_PC_BUF_NUM;i++)
	{
		tcp_data *tr_data;
		tr_data=(tcp_data *)get_write_pos(&tcp_send_pc_buf);
		free(tr_data->buf);
	}
	ring_buf_delete(&tcp_send_pc_buf);
}

