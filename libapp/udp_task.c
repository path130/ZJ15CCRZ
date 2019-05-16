#include <stdlib.h>
#include <strings.h>
#include "data_com.h"
#include "net_run.h"
#include "udp_task.h"
#include "public.h"

static int udp_send_run=0;
static int udp_recv_run=0;
static int udp_pc_recv_run=0;
static int udp_time_send=0;

static pthread_t thread_udp_recv;
static pthread_t thread_udp_pc_recv;
static pthread_t thread_arp_send;

int udp_fd[2];
int pipe_udp_c[2];
volatile struct sockaddr_in rec_client,arp_client,time_client;

void udp_recv_task()         //UDP接收处理函数
{
	app_debug(DBG_INFO, "udp_rec_task starting!\n");
	struct sockaddr_in recv_server;
	int udp_sock_recv=-1;
	if ((udp_sock_recv=socket(AF_INET,SOCK_DGRAM,0))<0)
		{
		app_debug(DBG_FATAL, "socket() udp_sock_recv error\n");
		exit(EXIT_FAILURE);
		}
	int reuse=1;
	setsockopt(udp_sock_recv, SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(reuse));
	bzero(&recv_server,sizeof(recv_server));
	recv_server.sin_family = AF_INET;
	recv_server.sin_port =htons(UDP_PORT);
	recv_server.sin_addr.s_addr = htonl (INADDR_ANY);

	net_bind(udp_sock_recv,&recv_server);
	udp_recv_run=1;
	while(udp_recv_run)
	{
		udp_process_task(udp_sock_recv);
	}
	app_debug(DBG_INFO, "udp_recv_task closed!\n");
}

void udp_pc_recv_task()         //UDP接收处理函数
{
	app_debug(DBG_INFO, "udp_rec_task starting!\n");
	struct sockaddr_in recv_server;
	int udp_pc_recv;
	if ((udp_pc_recv=socket(AF_INET,SOCK_DGRAM,0))<0)
	{
		app_debug(DBG_FATAL, "socket() udp_pc_recv error\n");
		exit(EXIT_FAILURE);
	}
	int reuse=1;
	setsockopt(udp_pc_recv, SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(reuse));
	bzero(&recv_server,sizeof(recv_server));
	recv_server.sin_family = AF_INET;
	recv_server.sin_port =htons(UDP_PC_PORT);
	recv_server.sin_addr.s_addr = htonl (INADDR_ANY);

	net_bind(udp_pc_recv,&recv_server);
	udp_pc_recv_run=1;
	while(udp_pc_recv_run)
	{
		udp_pc_process_task(udp_pc_recv);
	}
	app_debug(DBG_INFO, "udp_recv_task closed!\n");
}


int set_udp_board_ip(const void *destip,int len)//设置广播地址
{
	int ret;
	ret=pipe_write(pipe_udp_c[PIPE_WRITE],destip,len);
	return ret;
}
int get_udp_board_ip(void *buf,size_t len)    //获取udp广播IP
{
	int ret;
	ret=pipe_read(pipe_udp_c[PIPE_READ],buf,len);
	return ret;
}

int udp_client_get_data(unsigned char *buf)  //获取待发送UDP数据
{
	return pipe_read(udp_fd[PIPE_READ],buf,sizeof(DevList)+1);
}

int udp_client_send_data(unsigned char *buf,int len)//传送待发送ARP数据
{
	int ret;
	ret = pipe_write(udp_fd[PIPE_WRITE], buf, len);
	return ret - len;
}

/**
 *
 * 网络处理UI发送过来的数据
 * @param [in] buf UI发送过来的数据
 * @param [in] len 数据长度,此函数的长度需要固定为21，否则会出现意想不到的问题。
 * @param [in] port 目标端口
*/

int send_data_to_udp_node(void *buf,size_t len,uint32_t addr,uint16_t port)
{
	struct ip_parameter_t ip;
	ip.addr=addr;
	ip.port=port;
	if(set_udp_board_ip(&ip,sizeof(struct ip_parameter_t))!=sizeof(struct ip_parameter_t))
	{
		return -1;
        }
	return udp_client_send_data(buf,len);
}


void udp_boardcast_send_task()                   //ARP发送线程任务函数
{
	app_debug(DBG_INFO, "udp_boardcast_send_task starting!\n");
	int udp_broad = 1;
	int send_len;
	int udp_sock_send;
	unsigned char send_buf[1024]={0};

	if ((udp_sock_send=socket(AF_INET, SOCK_DGRAM,0))<0)
	{
		app_debug(DBG_INFO, "socket() error\n");
		exit(EXIT_FAILURE);
	}

	bzero((void *)(&arp_client),sizeof(arp_client));
	arp_client.sin_family = AF_INET;
	arp_client.sin_port =htons(UDP_PORT);
	if(setsockopt(udp_sock_send, SOL_SOCKET,SO_BROADCAST, &udp_broad, sizeof(udp_broad))<0)
	{
		perror("Set Broadcast:");
		exit(EXIT_FAILURE);
	}


	while(udp_send_run)
	{
		struct ip_parameter_t ip;
		if(get_udp_board_ip(&ip,sizeof(struct ip_parameter_t))!=sizeof(struct ip_parameter_t))
			continue;
		arp_client.sin_addr.s_addr=ip.addr;
		arp_client.sin_port=htons(ip.port);
		app_debug(DBG_INFO, "boardcast ip:%s\n",inet_ntoa(arp_client.sin_addr));
		int len=0;
		len=udp_client_get_data(send_buf);
		if(len<=0)
			continue;
		send_len=send_data_to(udp_sock_send,send_buf,len,(struct sockaddr *)&arp_client);
	}
	net_close(&udp_sock_send);
	app_debug(DBG_INFO, "udp_boardcast_send_task closed!\n");
}

int udp_time_request_task_create()
{

	if ((udp_time_send=socket(AF_INET, SOCK_DGRAM,0))<0)
	{
		fprintf(stderr, "socket() error\n");
		exit(EXIT_FAILURE);
	}
	int udp_broad = 1;
	if(setsockopt(udp_time_send, SOL_SOCKET,SO_BROADCAST, &udp_broad, sizeof(udp_broad))<0)
	{
		perror("Set Broadcast:");
		exit(EXIT_FAILURE);
	}

	bzero((void *)(&time_client),sizeof(time_client));
	time_client.sin_family = AF_INET;
	time_client.sin_port =htons(UDP_PORT);
	return 0;
}

ssize_t send_udp_data_to_socket(void *data,ssize_t len,unsigned int dest_addr,unsigned short port)
{

	time_client.sin_family = AF_INET;
	time_client.sin_port =htons(port);
	time_client.sin_addr.s_addr=dest_addr;
	return send_data_to(udp_time_send,data,len,(struct sockaddr *)&time_client);
}


void udp_task_create()
{
	int ret;
	udp_recv_run=1;
	udp_pc_recv_run=1;
	udp_send_run=1;

	pipe_creat(pipe_udp_c);
	pipe_creat(udp_fd);
	udp_time_request_task_create();

	ret=pthread_create(&thread_udp_recv,NULL,(void *)&udp_recv_task,NULL);
	if(ret<0)
	{
		perror("Create pthread_udp_recv!");
		exit(EXIT_FAILURE);
	}
	pthread_detach(thread_udp_recv);

	ret=pthread_create(&thread_udp_pc_recv,NULL,(void *)&udp_pc_recv_task,NULL);
	if(ret<0)
	{
		perror("Create pthread_udp_recv!");
		exit(EXIT_FAILURE);
	}
	pthread_detach(thread_udp_pc_recv);

	ret=pthread_create(&thread_arp_send,NULL,(void *)&udp_boardcast_send_task,NULL);
	if(ret<0)
	{
		perror("Create thread_udp_send!");
		exit(EXIT_FAILURE);
	}
	pthread_detach(thread_arp_send);
}


void udp_task_delete()
{
	udp_recv_run=0;
	udp_send_run=0;
	udp_pc_recv_run=0;
	close(udp_time_send);
	udp_time_send=0;
/*	if(thread_udp_recv)
		pthread_join(thread_udp_recv,NULL);
	if(thread_arp_send)
		pthread_join(thread_arp_send,NULL);*/
	pipe_close(udp_fd);
	pipe_close(pipe_udp_c);
}

