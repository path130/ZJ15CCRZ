/*
 * info_container.c
 *
 *  Created on: 2013-8-22 下午2:47:57
 *  
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "net_com.h"
#include "dev_pro.h"
#include "udp_cmd.h"
#include "data_com_ui.h"
#include "info_container.h"
#include "net_data_pro.h"
#include "my_debug.h"
#include "sqlite_data.h"
#include "dev_config.h"

#define INFO_NAME "info_data.bin"

#pragma pack(1)
typedef struct INFO_CONTAINER_T
{
	short int info_val;
	short int info_amount;//信息存储器数目
	short int fang_amount;//房间总数
	info_code_t *code_prt;
	int code_len;
}info_container_t;
#pragma pack()

static info_container_t info_container_data;
static pthread_rwlock_t info_lock=PTHREAD_RWLOCK_INITIALIZER;


int load_info_container_data(void)
{
/*	if(pthread_rwlock_rdlock(&info_lock)<0)
	{
		return -1;
	}*/

	info_container_data.info_val=0;
	info_container_data.info_amount=0;
	info_container_data.fang_amount=0;
	info_container_data.code_len=0;
	info_container_data.code_prt=NULL;

	int fd=open(INFO_NAME,O_RDONLY);
	if(fd<0)
	{
		perror("open file for info_data.bin:\n");
	//	pthread_rwlock_unlock(&info_lock);
		goto open_err;
	}
	int file_len=lseek(fd,0, SEEK_END);
	if(file_len>6)
	{
		printf("###info_container_data.code_prt len=%d\n",file_len);
		info_container_data.code_prt=malloc(file_len);
		if(	info_container_data.code_prt==NULL)
		{
			perror("malloc for info_container_data.code_prt:");
			//pthread_rwlock_unlock(&info_lock);
			exit(1);
		}
		info_container_data.code_len=file_len-6;
	}
	lseek(fd,0, SEEK_SET);
	char *buf=alloca(file_len+32);
	read(fd,buf,file_len);
	char *ptr=buf;//info_container_data.code_prt;
	info_container_data.info_val   =*(short int* )ptr;
	ptr+=2;
	info_container_data.info_amount=*(short int* )ptr;
	ptr+=2;
	info_container_data.fang_amount=*(short int* )ptr;
	ptr+=2;
	if(file_len-6>0)
	{
	memcpy(info_container_data.code_prt,ptr,file_len-6);
	close(fd);
	//pthread_rwlock_unlock(&info_lock);
	return 0;
	}
open_err:
	save_info_container_data(&info_container_data,sizeof(short int)*3);
	return 0;
}


int save_info_container_data(void *buf,int len)
{
	return do_save_inf_container(buf, len);
	if(pthread_rwlock_wrlock(&info_lock)<0)
	{
		return -1;
	}
	int fd=open(INFO_NAME,O_WRONLY|O_CREAT,0664);
	if(fd<0)
	{
		perror("open file for info_data.bin:\n");
		pthread_rwlock_unlock(&info_lock);
		exit(1);
	}
	write(fd,buf,len);
	fsync(fd);
	close(fd);

	char *ptr=buf;
	info_container_data.info_val   =*(short int *)ptr;
	ptr+=2;
	info_container_data.info_amount=*(short int *)ptr;
	ptr+=2;
	info_container_data.fang_amount=*(short int *)ptr;
	ptr+=2;
	if(len-6>0)
	{
		info_container_data.code_prt=realloc(info_container_data.code_prt,len-6);
		info_container_data.code_len=len;
		memcpy(info_container_data.code_prt,ptr,len-6);
/*
		int i=0;
			printf("##~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
			for(i=0;i<len-6;i++)
			{
				printf("%02x ",*(ptr+i));
			}
			printf("\n##~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
*/

	}
	printf("###info_container_data.info_amount=%d\n",info_container_data.info_amount);
	printf("###info_container_data.fang_amount=%d\n",info_container_data.fang_amount);


	pthread_rwlock_unlock(&info_lock);
	return 0;

}

int get_all_info_container_data(void *buf,int len)//****获取门禁卡列表
{
	int ret=0;
	return	do_load_info_container(buf, len);
	
	if(pthread_rwlock_rdlock(&info_lock)<0)
	{
		return ret;
	}
	char *ptr=buf;
	memcpy(ptr,&info_container_data.info_val,2);
	ptr+=2;
	ret+=2;
	memcpy(ptr,&info_container_data.info_amount,2);
	ptr+=2;
	ret+=2;
	memcpy(ptr,&info_container_data.fang_amount,2);
	ptr+=2;
	ret+=2;
	int copy_len=(info_container_data.code_len<(len-6))?info_container_data.code_len:(len-6);
	memcpy(ptr,info_container_data.code_prt,copy_len);
/*
	int i=0;
		printf("###~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		for(i=0;i<copy_len;i++)
		{
			printf("%02x ",*(ptr+i));
		}
		printf("\n###~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

*/

	ret+=copy_len;
	debug_info("ret=%d, copy_len=%d\n", ret, copy_len);
	pthread_rwlock_unlock(&info_lock);
	return ret;
}


/**********************************************************************************
*Function name	: GET_CODE_FROM_CONTAINER_DATA
*Description		: 
*Input para		: 
*Output para		: 
*Return			: 
*Modify by		:
*Modify Date		: 
*Modify Note		: 
************************************************************************************/
short int get_code_from_container_data(char fjno[2])
{
	int ret = 0;
	controoler_sql_t inf_cntnr = {0,};

	if(!is_have_infostore()) {
		return 0;
	}
	if(0 == fjno[0]) {
		return 0;
	}
	debug_info("get sub controller,fjno:%02X%02X\n", fjno[0],fjno[1]);
	memcpy(&inf_cntnr.data.ext_id, fjno, 2);
	ret = query_info_controller(1, &inf_cntnr);
	if(ret > 0) {
		ret = inf_cntnr.next->data.sub_id;
		debug_info("sub controller is %02X\n", ret);
		free_controllers_data(inf_cntnr.next);
	} else {
		ret = 0;
	}
	
	return ret;
}

#ifdef CFG_SUPPORT_INFO_STORE

struct glj_arp_struct_t
{
	int current_pos;
	socket_addr_t dest_addr;
	DevList com_devlist;
};

static struct glj_arp_struct_t arp_arp_array;
static sem_t glj_sem;
static pthread_t info_arp_thread,info_arp_replay_thread;
extern int get_com_ip_list(DevList *dev_list);

void gli_arp_replay_data_process(void)
{
	sem_wait(&glj_sem);
    printf("gli_arp_replay_data_process \n");
	int loop=ARP_RETYR_TIMES*ARP_DELAY/10000;
	unsigned int ip_addr=0;
	char send_buf[100];
	char *send_ptr=send_buf;
	DevList th_addr_array[TONG_HAOIP_MAX];
	int count_ip = 0;
    while(loop--)
    {
        count_ip = get_com_ip_list(th_addr_array);
        if(count_ip > 0) {
            memcpy(&ip_addr, th_addr_array[0].DevIp, sizeof(ip_addr));
            break;
        }
        usleep(ARP_DELAY);
    }
   
    if(ip_addr>0)
    {
        /*
        struct in_addr ia;
        char  ias[32];
        ia.s_addr = ip_addr;
       
        strcpy(ias, inet_ntoa(ia));
        printf("gli_arp_replay_data_process ip_addr:%s\n", ias); 
        */ 
		//in_addr_t my_ip = inet_addr("192.168.23.20");
		//memcpy(&arp_arp_array.dest_addr.addr, &my_ip, 4);
		memcpy(send_ptr,&arp_arp_array.dest_addr,sizeof(socket_addr_t));


		send_ptr+=sizeof(socket_addr_t);
		*send_ptr=ARP_REPLY;
		send_ptr++ ;

		get_com_ip_list(th_addr_array);
		memcpy(send_ptr,th_addr_array, sizeof(DevList));
		/*
		debug_info("send_buf:\r\n");
        print_n_byte(send_buf, sizeof(socket_addr_t)+sizeof(DevList)+1); 
        debug_info("sent data to udp\n"); 
		*/
		send_data_to_udp(send_buf,sizeof(socket_addr_t)+sizeof(DevList)+1);

    }
}

#include "common.h"
void ui_udp_process(void *buf)
{

	unsigned int net_cmd;
	unsigned int list_num;
	unsigned int num;
	int data_len;

	net_addr_msg *msg=(net_addr_msg *)buf;
	//debug_info( "#NET flag= 0x%x\n ",msg->flag_addr);
	switch(msg->flag_addr)
	{
	case GLJ_RQ_ADDR://  (0x44)//请求数据
	{
		net_addr_msg_split(&net_cmd,&list_num,&num,&data_len,(net_addr_msg*)msg);
		switch(net_cmd)
		{
		case 0x992:
		{
			char buf[data_len+1];
			if(get_list_data(list_num,buf)>0)
			{
				char send_buf[data_len+100];
				char *send_ptr=send_buf;
				/*
				unsigned char devdest[4];
				printf("me is:%02x%02x%02x%02x\n",gData.DoorNo[0],gData.DoorNo[1],0x00,gData.DoorNo[2]);
				*/
				DevList recv_devlist;
				memcpy(&recv_devlist,&buf[sizeof(socket_addr_t)+1],sizeof(DevList));
				unsigned char *dest_No_2=(unsigned char [4]){};
				memcpy(dest_No_2,recv_devlist.DevNo,4);
				short int fuhao=get_code_from_container_data((char *)&dest_No_2[2]);
				if(fuhao>0)
				{
					dest_No_2[3]=fuhao&0xff;
				}
				else
				{
					return;
				}

				dest_No_2[2]=0;
				/*
				debug_info("dest_No_2[3]=%d\n",dest_No_2[3]);
				printf("SEND ARPINFOREQ\n");
				*/
				memcpy(&arp_arp_array.dest_addr,buf,sizeof(socket_addr_t));
				socket_addr_t addr;
				addr.addr=get_boardcast_ip();//INADDR_BROADCAST;
				addr.port=UDP_PORT;
				DevList dest_devlist;
				send_ptr=send_buf;
				memcpy(send_ptr,&addr,sizeof(socket_addr_t));
				send_ptr+=sizeof(socket_addr_t);
				*send_ptr=ARP_REQUEST;
				send_ptr++ ;
				memcpy(dest_devlist.DevNo,dest_No_2,4);
				dest_devlist.DevIp[0]=dev_info->dev_No[0];
				dest_devlist.DevIp[1]=dev_info->dev_No[1];
				dest_devlist.DevIp[2]=dev_info->dev_No[2];
				dest_devlist.DevIp[3]=dev_info->dev_No[3];
				dest_devlist.DevStatus=ARPINFOREQ;

				memcpy(send_ptr,&dest_devlist,sizeof(dest_devlist));
				arp_arp_array.current_pos=0;
				/*
				debug_info("deset_devlist:\r\n");
				print_n_byte(&dest_devlist,sizeof(dest_devlist));
                */
                clear_com_dev_arrary();
				clr_ip_flag();
				sem_post(&glj_sem);
				send_data_to_udp(send_buf,sizeof(socket_addr_t)+sizeof(dest_devlist)+1);
			}
		}
		break;
		}
	}
	break;
	default:
	break;
	}
	debug_info("ui_udp_process exit!!!!!!!!!!!!\n");
}


void ui_udp_msg_send_thread(void *handle)
{
	printf( "ui_udp_msg_send_thread create!!!!!!!!!!!!\n");
	while(1)
	{
		char buf[MSG_LEN_PROC];
		if(get_ui_data_from_udp(buf,MSG_LEN_PROC)==0)
		{
			ui_udp_process(&buf[1]);
		}
	}

}

void glj_reply_process_thread()
{
	printf( "glj_reply_process create!!!!!!!!!!!!\n");
	while(1)
	{
		gli_arp_replay_data_process();
	}
	printf( "glj_reply_process exit!!!!!!!!!!!!\n");
}


void init_info_arp_for_fenji(void)
{
    if (!is_have_infostore()) return;
	pthread_create(&info_arp_thread,NULL,(void *)&glj_reply_process_thread,NULL);
	pthread_create(&info_arp_replay_thread,NULL,(void *)&ui_udp_msg_send_thread,NULL);
}
#endif
