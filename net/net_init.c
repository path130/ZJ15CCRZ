/*
 * net_init.c
 *
 *  Created on: 2012-9-19 下午7:28:37
 *  
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#define __EXPORTED_HEADERS__
#include <linux/ethtool.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "data_com_ui.h"
#include "tcp_pc_task.h"
#include "net_data_pro.h"
#include "net_event.h"
#include "event_num.h"
#include "data_com.h"
#include "ring_buf.h"
#include "net_init.h"
#include "udp_task.h"
#include "tcp_task.h"
#include "ret_type.h"
#include "ajb_bus.h"
#include "cmd_def.h"
#include "public.h"
#include "character.h"
#include "udp_pc_task.h"
#include "dev_pro.h"
#include "call_event.h"
#include "info_container.h"
#include "dev_config.h"
#include <fcntl.h>
#include "vlan_iptable.h"

typedef struct{
	net_mem *list_head;
	int useing;
}net_mem_s;

net_mem_s list_head[LIST_QUANTITY];

static int run=1;
static pthread_t msg_thread,msg_thread_cache;
//#if DEV_GLJKZQ|DEV_TSGLJKZQ
#ifdef CFG_SUPPORT_INFO_STORE
static pthread_t msg_udp_thread;
#endif
static pthread_t net_detect;
static pthread_t mem_ts_thread;

int pipe_mem[2];
int pipe_req[2];


typedef struct{
	unsigned int msg_type;	//消息类型
	unsigned int head_addr;	//链表头地址
	int msg_pos;				//消息在数字中的位置
}mem_ts_msg;
vlanip_handel_t *vlanip_handel=NULL;

int ui_refresh_vlan_iptable( void *table,uint32_t quantity,uint32_t def)
{
	return refresh_vlan_iptable(vlanip_handel,table,quantity,def);
}

uint32_t ui_get_vlan_iptable_quantity(void)
{
	return get_vlan_iptable_quantity(vlanip_handel);
}

const void* ui_get_vlan_iptable_start_addr(uint32_t *def)
{
	return get_vlan_iptable_start_addr(vlanip_handel,def);
}


uint32_t arp_search_vlanip_by_devno(const unsigned char dev_no[4],const int max_ip,uint32_t*ip_ret)
{
	return search_vlanip_by_devno(vlanip_handel,dev_no,max_ip,ip_ret);
}
int is_vlan_enable(void)
{
	if(dev_info!=NULL)
	{
		return dev_info->vlan_enable;
	}
	return 0;
}
int get_list_data(int num,void *buf)
{
	int ret=0;
	if(num>=LIST_QUANTITY) return -1;
	if((list_head[num].list_head!=NULL)&&(list_head[num].useing==1))
	{
		 ret=get_net_mem_data(list_head[num].list_head,buf);
		 delete_list_mem(list_head[num].list_head);
	}
	else
		return -1;
	return ret;
}

int free_list_data(int num)
{
	int ret=0;
	if(num>=LIST_QUANTITY) return -1;
	if((list_head[num].list_head!=NULL)&&(list_head[num].useing==1))
	{
		 delete_list_mem(list_head[num].list_head);
	}
	else
		return -1;
	return ret;
}

int get_list_size(int num)
{
	if(num>=LIST_QUANTITY) return -1;
	if((list_head[num].list_head!=NULL)&&(list_head[num].useing==1))
		{
			return get_net_mem_size(list_head[num].list_head);
		}
	else
		return -1;
}

/**
 *
 * 申请网络发送缓冲区链表头
 * @param [out] head_item 返回链表头
 * @return 成功返回0，否则返回-1
*/
int request_list_head_mem(net_mem **head_item)
{
	app_debug(DBG_INFO, "request_list_head_mem!\n");
	int len;
	mem_ts_msg send_msg;
	send_msg.msg_type=REQUST_MEM;
	len=pipe_write(pipe_mem[PIPE_WRITE],&send_msg,sizeof(mem_ts_msg));
	if(len==sizeof(mem_ts_msg)){
		mem_ts_msg recv_msg;
		recv_msg.msg_type=REQUST_MEM;
		len=pipe_read(pipe_req[PIPE_READ],&recv_msg,sizeof(mem_ts_msg));
		if(len==sizeof(mem_ts_msg))
		{
			switch(recv_msg.msg_type)
			{
			case REQUST_MEM_OK:
				*head_item=(net_mem *)recv_msg.head_addr;
				return 0;
				break;
			case REQUST_MEM_FAILURE:
				*head_item=NULL;
				break;
			default:
				*head_item=NULL;
				break;
			}
		}
		else
		{
			app_debug(DBG_INFO, "request_list_head_mem read error!\n");
		}
	}
	return -1;
}

/**
 *
 * 释放链表
 * @param [in] head_item 待释放链表头
*/

void delete_list_mem(net_mem *head_item)
{
	int len;
	mem_ts_msg send_msg;
	send_msg.msg_type=FREE_MEM;
	send_msg.msg_pos=head_item->mem_num;
	len=pipe_write(pipe_mem[PIPE_WRITE],&send_msg,sizeof(mem_ts_msg));
	if(len==sizeof(mem_ts_msg)){
		app_debug(DBG_INFO, "delete_list_mem @ %d successful!\n",send_msg.msg_pos);
	}
	else{
		app_debug(DBG_ERROR, "delete_list_mem @ %d failure!\n",send_msg.msg_pos);
	}
}

/**
 *
 * 网络 UI数据传输管理此线程入口函数
 *
*/
void net_mem_transport_thread()
{
	app_debug(DBG_INFO, "net_mem_transport_thread create!\n");
	pipe_creat(pipe_mem);
	pipe_creat(pipe_req);
	while(run)
	{
		int recv_len;
		mem_ts_msg recv_msg;
		recv_len=pipe_read(pipe_mem[PIPE_READ],&recv_msg,sizeof(mem_ts_msg));
		if(recv_len==sizeof(mem_ts_msg))
		{
			switch(recv_msg.msg_type)
			{
			case REQUST_MEM:
			{
				mem_ts_msg send_msg;
				int i=0;
				for(i=0;i<LIST_QUANTITY;i++)
				{
					send_msg.msg_type=REQUST_MEM_FAILURE;
					if(list_head[i].useing==0)
					{
						list_head[i].list_head=(net_mem *)malloc(sizeof(net_mem));
						if(list_head[i].list_head==NULL)
						{
							app_debug(DBG_FATAL,"malloc mem for list_head[%d]->list_head failure!",i);
							//send_msg.msg_type=REQUST_MEM_FAILURE;
						}
						else{
							list_head[i].useing=1;
							send_msg.msg_type=REQUST_MEM_OK;
							send_msg.msg_pos=i;
							send_msg.head_addr=(int)list_head[i].list_head;
							list_head[i].list_head->mem_num=i;
							list_head[i].list_head->next=NULL;
						}
						break;
					}
				}
				pipe_write(pipe_req[PIPE_WRITE],&send_msg,sizeof(mem_ts_msg));
			}
				break;
			case FREE_MEM:
				//if(list_head[recv_msg.msg_pos].list_head>0)
				{
					net_mem *prev_head,*next;
					app_debug(DBG_FATAL,"recv_msg.msg_pos: %x !",recv_msg.msg_pos);
					prev_head=list_head[recv_msg.msg_pos].list_head;
				    while(prev_head!=NULL)
				    {
				        next=prev_head->next;
				        free(prev_head);
				        prev_head=next;
				    }
				    list_head[recv_msg.msg_pos].useing=0;
				}
				break;
			default:
				break;
			}
		}
		else
			continue;
	}
	pipe_close(pipe_mem);
	pipe_close(pipe_req);
	app_debug(DBG_INFO, "net_mem_transport_thread exit!\n");
}

/**
 *
 * 网络接搜UI数据线程入口函数
 *
*/
void msg_recv_thread()//UI数据接收
{
	app_debug(DBG_INFO, "msg_rev_thread create!!!!!!!!!!!!\n");
	while(run)
	{
		//ajb_msg msg_get;
		char msg_get[MSG_LEN_NET+10];
		if(get_data_from_proc(msg_get,MSG_LEN_NET)==0){
			//printf("get data ok!\n");
			//for(i=0;i<sizeof(ajb_msg);i++)
			//	printf("%2X ",msg_get[1+i]);
			//	printf("\n");
			ui_data_pro(&msg_get[1],sizeof(ajb_msg));
		}
	}
	app_debug(DBG_INFO, "msg_rev_thread exit!!!!!!!!!!!!\n");
}

void msg_recv_cache_thread()//UI数据接收
{
	init_pipe_uipro2cache();
	while(run)
	{
		char msg_get[MSG_LEN_NET+10];
		if(get_data_from_uipro_thread(msg_get,sizeof(ajb_msg))==0)
		{
			ui_data_pro_thread_cache(msg_get,sizeof(ajb_msg));
		}
	}
}

//#if DEV_GLJKZQ|DEV_TSGLJKZQ
#ifdef CFG_SUPPORT_INFO_STORE

void msg_ui_udp_thread()//UI数据接收
{
	app_debug(DBG_INFO, "msg_rev_thread create!!!!!!!!!!!!\n");
	while(run)
	{
		char msg_get[MSG_LEN_NET+10];
		if(get_udp_data_from_ui(msg_get,MSG_LEN_NET)==0){
			ui_udp_data_pro(&msg_get[1],sizeof(ajb_msg));
		}
	}
	app_debug(DBG_INFO, "msg_rev_thread exit!!!!!!!!!!!!\n");
}
#endif

/**
 *
 * 探测网络是否接入状态线程入口函数
*/
void net_com_detect()
{
	app_debug(DBG_INFO, "net_detect_thread create!!!!!!!!!!!!\n");
	proc_msg_ret detect_data;
	unsigned char last_s=0xff;
	int detect_fd=0;
	struct ifreq ifr;
	struct ethtool_value edata;
	int eth_status;

	edata.cmd = ETHTOOL_GLINK;
	edata.data = 0;
	memset(&ifr, 0,sizeof(ifr));
	strncpy(ifr.ifr_name,"eth0",sizeof(ifr.ifr_name)- 1);
	ifr.ifr_data =(char *) &edata;
    if ((detect_fd= socket(AF_INET, SOCK_DGRAM, 0 )) < 0)
    {
    	perror("Socket open :");
    	exit(EXIT_FAILURE);
    }
    detect_data.msg_type=MSG_FROM_NET;
    detect_data.msg.flag_addr=MEM_DATA;
    detect_data.msg.data.net_cmd=STASC;
    detect_data.msg.data.cmd=SCODE;
    detect_data.msg.data.allow=0;
    detect_data.msg.data.oper_type=0;

	while(run)
	{
	    if(ioctl(detect_fd,SIOCETHTOOL,&ifr ) == -1)
	       break;
	    if(edata.data)
	    	eth_status= NET_RUN;
	    else
	    	eth_status= NET_OFF;

	    detect_data.msg.data.resstatus=eth_status;
	    if(last_s!=detect_data.msg.data.resstatus){
	    	app_debug(DBG_INFO,"eth_status=%d\n",eth_status);
	    	send_data_to_proc(&detect_data,MSG_LEN_PROC);//发送链接状态数据给UI
	    }

	    last_s=detect_data.msg.data.resstatus;
	    sleep(DETECT_TIME);
	}
    close(detect_fd);
	app_debug(DBG_INFO, "net_detect_thread exit!!!!!!!!!!!!\n");

}


/**
 *
 * 获取后台服务器IP
 * @return 返回后台服务器IP
*/
unsigned int get_server_ip()
{
	unsigned int server_ip;
	memcpy(&server_ip,gData.ServerIP,4);
	struct in_addr ip;
	ip.s_addr=server_ip;
	printf("serverip:%s\n",inet_ntoa(ip));
	return server_ip;

}

/**
 *
 * 获取本地IP
 * @return 返回本地IP
*/

unsigned int get_local_ip()
{
	unsigned int local_ip;
	if(dev_info!=NULL){
	memcpy(&local_ip,gData.MyIP,4);
	return local_ip;
	}
	else
		return 0;
}

////add for multi_lift_ctl
/**
 *
 * 获取呼梯控制器IP
 * @return 返回呼梯控制器IP
*/
unsigned int get_1st_lift_controller_ip()
{
	unsigned int kzq_ip;
	memcpy(&kzq_ip,gData.Dns2,4);
	struct in_addr ip;
	ip.s_addr=kzq_ip;
	app_debug(DBG_INFO,"1st controller ip:%s\n",inet_ntoa(ip));
	return kzq_ip;
}


unsigned int get_2nd_lift_controller_ip()
{
	unsigned int kzq_ip;
	memcpy(&kzq_ip,&dev_cfg.lift_ip[0][0],4);
	struct in_addr ip;
	ip.s_addr=kzq_ip;
	app_debug(DBG_INFO,"2nd controller ip:%s\n",inet_ntoa(ip));
	return kzq_ip;
}

unsigned int get_3rd_lift_controller_ip()
{
    unsigned int kzq_ip;
    memcpy(&kzq_ip,&dev_cfg.lift_ip[1][0],4);
    struct in_addr ip;
    ip.s_addr=kzq_ip;
    app_debug(DBG_INFO,"3rd controller ip:%s\n",inet_ntoa(ip));
    return kzq_ip;
}

unsigned int get_4th_lift_controller_ip()
{
    unsigned int kzq_ip;
    memcpy(&kzq_ip,&dev_cfg.lift_ip[2][0],4);
    struct in_addr ip;
    ip.s_addr=kzq_ip;
    app_debug(DBG_INFO,"4th controller ip:%s\n",inet_ntoa(ip));
    return kzq_ip;
}

unsigned int get_5th_lift_controller_ip()
{
    unsigned int kzq_ip;
    memcpy(&kzq_ip,&dev_cfg.lift_ip[3][0],4);
    struct in_addr ip;
    ip.s_addr=kzq_ip;
    app_debug(DBG_INFO,"5th controller ip:%s\n",inet_ntoa(ip));
    return kzq_ip;
}
///end

/**
 *
 * 获取MAC地址
 * @param [out] buf 返回MAC地址
 * @return 成功返回1，否则返回-1
*/

int get_local_mac(unsigned char buf[6])
{
	if(dev_info!=NULL){
		memcpy(buf,gData.MyMac,6);
		return 1;
	}else
		return 0;
}

/**
 *
 * 获取UDP广播地址
 * @return 返回UDP广播地址
*/

unsigned int get_boardcast_ip()
{
	unsigned board_ip=INADDR_NONE;
	if(dev_info!=NULL){
		board_ip=dev_info->board_ip;
	}
	return board_ip;
}

/**
 *
 * 获取本机网络设备参数
 * @param [out] addr 返回IP地址
 * @param [out] mask 返回掩码
 * @param [out] gate_w 返回网关
 * @param [out] boardcast 返回UDP广播地址
 * @param [out] ip_mac 返回MAC地址
 * @return 成功返回0
*/

int get_ifconfig(unsigned int *addr, unsigned int *mask, unsigned int *gate_w, unsigned int *boardcast,char *ip_mac)
{
    int    ret  = 0;
    int    sfd  = -1;
    unsigned long gate = INADDR_NONE;
    unsigned long dest = INADDR_NONE;

    struct ifreq    ifr;
    struct in_addr  in_addr_ip, in_addr_mask,broadaddr;
    char   rbuf[128];
    char   iface[16];
    FILE   *fp_route;

    in_addr_ip.s_addr   = INADDR_NONE;
    in_addr_mask.s_addr = INADDR_NONE;
    broadaddr.s_addr    = INADDR_NONE;
    memset(ip_mac,0x00,6);
    strcpy(ifr.ifr_name, ETH_DEV);
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ret |= (0x01|0x02|0x04);
        goto GET_GATEWAY;
    }
    if (ioctl(sfd, SIOCGIFADDR, &ifr)) {
        ret |= 0x01;
        goto GET_MASK;
    }
    in_addr_ip.s_addr = (((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr).s_addr;
GET_MASK:
    if (ioctl(sfd, SIOCGIFNETMASK, &ifr)) {
        ret |= 0x02;
        goto GET_MAC;
    }
    in_addr_mask.s_addr = (((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr).s_addr;
GET_MAC:
    if (ioctl(sfd, SIOCGIFHWADDR, &ifr)) {
        ret |= 0x08;
        goto GET_BOARD;
    }
    memcpy(ip_mac,(char *)(&ifr.ifr_hwaddr.sa_data[0]),6);
GET_BOARD:
    if (ioctl(sfd, SIOCGIFBRDADDR, &ifr)) {
    	 goto GET_GATEWAY;
    }
    broadaddr.s_addr=(((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr).s_addr;

GET_GATEWAY:
	fp_route = fopen("/proc/net/route", "r");
	if (fp_route == NULL) {
		ret |= 0x0100;
	}
	fgets(rbuf, sizeof(rbuf), fp_route);
	while (fgets(rbuf, sizeof(rbuf), fp_route)) {
		if ((sscanf(rbuf, "%s%lX\t%lX", iface, &dest, &gate) != 3)
				|| (dest != 0))
			continue;
		break;
	}
	if (fp_route != NULL)
		fclose(fp_route);
    if (sfd > 0)  close(sfd);
    if (ret & 0x01)
    	(*addr)=INADDR_NONE;
    else
    	(*addr)=in_addr_ip.s_addr;
    if (ret & 0x02)
    	(*mask)=INADDR_NONE;
    else
    	(*mask)=in_addr_mask.s_addr;
    if (ret & 0x08){
    	(*boardcast)=INADDR_NONE;
    }
    else {
    	(*boardcast)=broadaddr.s_addr;
    }
    if (ret & 0x0100){
    	(*gate_w)=INADDR_NONE;
    }
    else {
        (*gate_w)= gate;
    }

    return (0-ret);
}

#if 0
/**
 *
 * 初始化同ipinstall通信运行参数
 */
int init_para_for_ipinstall()
{
	pdu_head	UdpEcho;
	unsigned int	port;
	char *eth_mac=alloca(6);
	memset(eth_mac,0x00,6);
	static unsigned int eth_addr,eth_gw,eth_mask,eth_boardcast;
	memset(&UdpEcho,0x00,sizeof(UdpEcho));
	get_ifconfig(&eth_addr,&eth_mask,&eth_gw,&eth_boardcast,eth_mac);
	if(dev_info!=NULL){
		memcpy(dev_info->eth_para.ip_addr,&eth_addr,4);
		memcpy(dev_info->eth_para.subnet_mask,&eth_mask,4);
		memcpy(dev_info->eth_para.gateway_addr,&eth_gw,4);
		memcpy(dev_info->eth_para.mac_addr,eth_mac,6);
		memcpy(&dev_info->board_ip,&eth_boardcast,4);
		memcpy(&UdpEcho.dhcp_ebl,&dev_info->eth_para.dhcp_ebl,sizeof(eth_para_t));
	}

	UdpEcho.dhcp_ebl=dhcp_ebl_s;
	port = TCP_PC_PORT;
	memcpy(UdpEcho.http_port,&port,4);
	strcpy(UdpEcho.equ_description,equ_description_s);	//设备描述
	strcpy(UdpEcho.equ_name,equ_name_s);					//设备名称
	UdpEcho.equ_type = equ_type_s;							//设备类型
	strcpy(UdpEcho.prod_name,prod_name_s);					//生产商名称
	strcpy(UdpEcho.type_name,type_name_s);					//类型名称
	strcpy(UdpEcho.hardware_version,hardware_version_s);
	strcpy(UdpEcho.software_version,"1.0.2.12-11-01");
	strcpy(UdpEcho.position,position_s);
	strcpy(UdpEcho.description,description_s);			//描述

	memcpy(gData.MyMac,eth_mac,6);
	memcpy(gData.MyIP,&eth_addr,4);
	InitOpdu(&UdpEcho);
	return 0;
}
#endif

pdu_head    udp_echo;

int init_para_for_ipinstall()       //初始化同ipinstall通信运行参数
{
	if(dev_info!=NULL){
		memcpy(dev_info->eth_para.ip_addr,      gData.MyIP,    4);
		memcpy(dev_info->eth_para.subnet_mask,  gData.mdv_mask, 4);
		memcpy(dev_info->eth_para.gateway_addr, gData.mdv_gw,   4);
		memcpy(dev_info->eth_para.mac_addr,     gData.MyMac,    6);
        dev_info->eth_para.dhcp_ebl = 0;
	}
	memcpy(udp_echo.dns1,dev_cfg.dns1,4);
	memcpy(udp_echo.dns2,dev_cfg.dns2,4);

	InitOpdu(&udp_echo);
	return 0;
}

/*
 * NET创建初始化 关闭
 */

/**
 *
 * 判断是否为本机网关相同
 * @param [in] buf 待对比网关数据
 * @return 相同返回1，不相同返回0
*/

int is_my_gateway_addr(unsigned char *buf)
{
	if(memcmp( buf,dev_info->eth_para.gateway_addr,4) == 0)
			return 1;
		return 0;
}

/**
 *
 * 判断是否为本机掩码相同
 * @param [in] buf 待对比掩码数据
 * @return 相同返回1，不相同返回0
*/
int is_my_subnet_mask(unsigned char *buf)
{
	if(memcmp( buf,dev_info->eth_para.subnet_mask,4) == 0)
			return 1;
		return 0;
}

net_para *dev_info=NULL;

/**
 *
 * 初始化网络运行参数，启动网络的各个模块
 * @param [in] net_para_data 网络运行参数
 * @param [in] arg pipe句柄
*/

void net_com_create(net_para *net_para_data, void *arg)
{
	int ret;
	run=1;
	dev_info=(net_para*)calloc(sizeof(net_para),sizeof(char));
	if(dev_info==NULL)
		exit(EXIT_FAILURE);
	memcpy(dev_info,net_para_data,sizeof(net_para));
	load_global_data();
	init_para_for_ipinstall();
	memcpy(dev_info->eth_para.dns1,dev_cfg.dns1,4);
	memcpy(dev_info->eth_para.dns2,dev_cfg.dns2,4);

		uint32_t gw;

		memcpy(&gw,gData.addr_gw,4);
//INADDR_NONE指255.255.255.255,INADDR_ANY指0.0.0.0
		if((gw==INADDR_NONE)||(gw==INADDR_ANY))
		{
			dev_info->vlan_enable=0;
		}
		else
		{			
			dev_info->vlan_enable=1; 
		}

		printf("++++ dev_info->vlan_enable:%d\n",dev_info->vlan_enable);
        
#if DEV_GLJKZQ|DEV_GATE|DEV_TSGLJKZQ
	unsigned char dev_No[4];
	dev_No[3]=gData.DoorNo[2];
	dev_No[2]=0x00;
	dev_No[1]=gData.DoorNo[1];
	dev_No[0]=gData.DoorNo[0];
	set_net_para(dev_No,gData.MyIP,DEV_TYPE);
#elif DEV_CONTROL
	set_net_para(dev_info->dev_No,gData.MyIP,DEV_TYPE);
#else //DEV_TERMINAL
	set_net_para(gData.DoorNo,gData.MyIP,DEV_TYPE);
#endif
#ifdef HAVE_TONGHAO
	 create_dev_arrary();
#endif
            if(is_vlan_enable())
                {
			 vlanip_handel=create_vlan_iptable(0);
			 if(vlanip_handel==NULL)
			 {
				 fprintf(stderr, "create_vlan_iptable failure!\n");
				 exit(EXIT_FAILURE);
			 }
#if 0
			 char buf[MAX_VLANIP_QU*sizeof(vlanip_table_t)+1024];
			 uint32_t quantity;
			 uint32_t def;
	
			 int write_fd=-1;
			 write_fd=open("vlan_list.bin",O_RDONLY);
			 if(write_fd>-1)
			 {
				 int read_len=read(write_fd,buf,MAX_VLANIP_QU*sizeof(vlanip_table_t)+1024);
				 close(write_fd);
				 if(read_len>8)
				 {
					 char *ptr=buf;
					 memcpy(&quantity,ptr,4);
					 ptr+=4;
					 memcpy(&def,ptr,4);
					 ptr+=4;
					 ui_refresh_vlan_iptable(ptr,quantity,def);
				 }
			 }
			 else
			 {
				 perror("open for vlan_list.bin:");
			 }
	
	
#if 1
				socket_addr_t addr;
				addr.addr=inet_addr("192.168.1.1");
				uint8_t ip_addr[4]={0};
				memcpy(ip_addr,&addr.addr,4);
	
				char dev_no=0xef;
				vlanip_table_t *vlan_buf=malloc(2*MAX_VLANIP_QU*sizeof(vlanip_table_t));
				int i=0;
				int j=0;
				int k=0;
				int l=0;
	
				int p=1;
				int q=1;
				int r=ip_addr[1];
	
				int count=0;
				vlanip_table_t *vlan_buf_ptr=vlan_buf;
				for(i=1;i<0x99;i++)
				{
					for(j=1;j<0x15;j++)
					{
						for(k=1;k<0x15;k++)
						{
							for(l=1;l<0x15;l++)
							{
								if(vlan_buf_ptr!=NULL)
								{
									vlan_buf_ptr->dev_no[0]=(i);
									vlan_buf_ptr->dev_no[1]=(j);
									vlan_buf_ptr->dev_no[2]=(k);
									vlan_buf_ptr->dev_no[3]=(l);
	
									count++;
									p++;
									if(p>253)
									{
										q++;
										if(q>253)
										{
											r++;
											q=2;
										}
										p=2;
									}
									vlan_buf_ptr->dev_ip[0]=ip_addr[0];
									vlan_buf_ptr->dev_ip[1]=r;
									vlan_buf_ptr->dev_ip[2]=q;
									vlan_buf_ptr->dev_ip[3]=p;
									if(count>=MAX_VLANIP_QU)
									{
										i+=100;
										j+=100;
										k+=100;
										l+=100;
										break;
									}
									vlan_buf_ptr++;
								}
								else
								{
	
									i+=100;
									j+=100;
									k+=100;
									l+=100;
									break;
	
								}
	
							}
						}
					}
				}
	/*
	
				printf("~~######=%d  %d %d	%d	 count=%d\n",(vlan_buf_ptr->dev_no[0]),
						(vlan_buf_ptr->dev_no[1]),\
						(vlan_buf_ptr->dev_no[2]),\
						(vlan_buf_ptr->dev_no[3]),count);
	*/
				quantity=MAX_VLANIP_QU-1;
				ui_refresh_vlan_iptable(vlan_buf,quantity,def);
	
#endif
	
#endif
	
		 }
	get_msg_handle(arg);
//#if DEV_GLJKZQ|DEV_TSGLJKZQ
#ifdef CFG_SUPPORT_INFO_STORE
	if(init_pipe_udp_ui()<0){
		app_debug(DBG_FATAL,"Create init_pipe_udp_ui failure!");
		exit(EXIT_FAILURE);
	}
    init_info_arp_for_fenji();
#endif
	ret=create_net_connect_manager();
	if(ret<0){
		app_debug(DBG_FATAL, "create_net_connect_manager failure!\n");
		exit(EXIT_FAILURE);
	}

/*	ret=create_call_event_manager();
	if(ret<0){
			app_debug(DBG_FATAL, "create_call_event_manager failure!\n");
			exit(EXIT_FAILURE);
		}*/
#if 0
	DIR *dir_path=NULL;
	dir_path=opendir("./rpcpic");
	if(dir_path==NULL)
	{
		if(mkdir("./rpcpic",0664)<0)
			printf("no dir ./rpcpic\n");
	}
	else
	{
		closedir(dir_path);
	}
/*	dir_path=opendir("./rpctxt");
	if(dir_path==NULL)
	{
		if(mkdir("./rpctxt",0664)<0)
			printf("no dir ./rpctxt\n");
	}
	else
		closedir(dir_path);*/

#endif
	//UI数据接收
	ret=pthread_create(&msg_thread_cache,NULL,(void *)&msg_recv_cache_thread,NULL);
	if(ret<0)
	{
		perror("Create msg_rev_thread!");
		exit(EXIT_FAILURE);
	}
	pthread_detach(msg_thread_cache);
	
	create_call_list();
	ret=pthread_create(&msg_thread,NULL,(void *)&msg_recv_thread,NULL);
	if(ret<0)
	{
		perror("Create msg_rev_thread!");
		exit(EXIT_FAILURE);
	}
	pthread_detach(msg_thread);
//#if DEV_GLJKZQ|DEV_TSGLJKZQ
#ifdef CFG_SUPPORT_INFO_STORE
	ret=pthread_create(&msg_udp_thread,NULL,(void *)&msg_ui_udp_thread,NULL);
	if(ret<0)
	{
		perror("Create msg_rev_thread!");
		exit(EXIT_FAILURE);
	}
	pthread_detach(msg_udp_thread);
#endif

	ret=pthread_create(&net_detect,NULL,(void *)&net_com_detect,NULL);
	if(ret<0)
	{
		perror("Create net_detect_thread!");
		exit(EXIT_FAILURE);
	}
	pthread_detach(net_detect);

	ret=pthread_create(&mem_ts_thread,NULL,(void *)&net_mem_transport_thread,NULL);
	if(ret<0)
	{
		perror("Create mem_ts_thread!");
		exit(EXIT_FAILURE);
	}
	pthread_detach(mem_ts_thread);

/*	ret=pthread_create(&mem_ts_test,NULL,(void *)&test_fun,NULL);
	if(ret<0)
	{
		perror("Create mem_ts_test!");
		exit(EXIT_FAILURE);
	}*/
	tcp_task_create();
	udp_task_create();
	tcp_pc_task_create();
	tcp_pc_json_task_create();
}

void net_com_delete()
{
	run=0;
/*	if(msg_thread)
		pthread_join(msg_thread,NULL);
	if(net_detect)
		pthread_join(net_detect,NULL);*/
	if(dev_info!=NULL){
		free(dev_info);
		dev_info=NULL;
	}
	tcp_task_delete();
	udp_task_delete();
	tcp_pc_task_delete();
	//delete_call_event_manager();
	delete_net_connect_manager();
	if(dev_info!=NULL)
		free(dev_info);
}

/**
 * 向后台发送上电报道数据
 */
void register_dev_to_pc()
{
	unsigned char buf[40];
	buf[0] = TIME_ASK_REQUEST;
#if DEV_GATE|DEV_GLJKZQ|DEV_TSGLJKZQ
	my_devlist.DevNo[0]=gData.DoorNo[0];
	my_devlist.DevNo[1]=gData.DoorNo[1];
	my_devlist.DevNo[2]=0x00;
	my_devlist.DevNo[3]=gData.DoorNo[2];
#endif

#if DEV_CONTROL
	my_devlist.DevNo[0]=dev_info->dev_No[0];
	my_devlist.DevNo[1]=dev_info->dev_No[1];
	my_devlist.DevNo[2]=dev_info->dev_No[2];
	my_devlist.DevNo[3]=dev_info->dev_No[3];
#endif

#if DEV_TERMINAL
	my_devlist.DevNo[0]=gData.DoorNo[0];
	my_devlist.DevNo[1]=gData.DoorNo[1];
	my_devlist.DevNo[2]=gData.DoorNo[2];
	my_devlist.DevNo[3]=gData.DoorNo[3];
#endif
	memcpy(&buf[3], &my_devlist, sizeof(DevList));
	int len = sizeof(DevList);
	memcpy(&buf[1], &len, 2);
	//unsigned int socket_addr[2];
	//socket_addr[0] = get_server_ip();
	//socket_addr[1]=UDP_PORT;
	unsigned int dest_addr = get_server_ip();
	if (dest_addr > 0) {
		send_udp_data_to_socket(buf,len + 3,dest_addr,UDP_PORT);
	}
}
static pthread_mutex_t global_data_lock=PTHREAD_MUTEX_INITIALIZER;

/**
 * 保存全局数据结构
 */

int save_global_data()
{
	int ret = pthread_mutex_lock(&global_data_lock);
	if (ret < 0){
		perror("Pthread_lock failed");
		return -1;
	}
	if(dev_info->save_global_data_fun!=NULL)
	{
		pthread_mutex_unlock(&global_data_lock);
		return dev_info->save_global_data_fun();
	}
	else{
		pthread_mutex_unlock(&global_data_lock);
		return -1;
	}
}

int load_global_data()
{
	int ret = pthread_mutex_lock(&global_data_lock);
		if (ret < 0){
			perror("Pthread_lock failed");
			return -1;
		}
	if(dev_info->load_global_data_fun!=NULL)
	{
		pthread_mutex_unlock(&global_data_lock);
		return dev_info->load_global_data_fun();
	}
	else{
		pthread_mutex_unlock(&global_data_lock);
		return -1;
	}
}

int save_eth_para(eth_para_t *para)
{
	if(dev_info->save_eth_para_fun!=NULL)
		return dev_info->save_eth_para_fun(para);
	else
		return -1;
}

int save_dev_eth_para(eth_para_t *para)
{
	struct in_addr  in_addr_ip;
	if(dev_info!=NULL)
	{
		memcpy(&dev_info->eth_para,para,sizeof(eth_para_t));
		memcpy(&in_addr_ip.s_addr,para->ip_addr,4);
		return 0;
	}
	else
		return -1;
}
void get_dns1(unsigned char*dns)
{
	memcpy(dns,dev_info->eth_para.dns1,4);
	
}

void get_dns2(unsigned char*dns)
{
	memcpy(dns,dev_info->eth_para.dns2,4);
}
