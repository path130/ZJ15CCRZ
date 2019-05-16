/*
 * call_event.c
 *
 *  Created on: 2012-9-20 上午10:27:26
 *  
 */
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "call_event.h"
#include "public.h"
#include "data_com.h"

struct call_list_t
{
	int pos;
	call_item call_item_array[COON_LIST_QUANTITY];
};

struct dest_dev_type_t
{
	unsigned char dest_No[4];
	unsigned int  dest_dev_type;
	unsigned int 	ip_addr;
};

struct dest_dev_type_array
{
	int pos;
	struct dest_dev_type_t dev_type_array[COON_LIST_QUANTITY];
};

static struct dest_dev_type_array dest_type_list;
static struct call_list_t call_list;
pthread_mutex_t call_item_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dev_type_lock=PTHREAD_MUTEX_INITIALIZER;


#ifndef HAVE_TONGHAO

/**
 *
 * 在已经建立的呼叫链接列表中，通过远端房号查询远端的IP地址。
 * @param [in]  dest_No[4] 四位房号
 * @param [out] ip_addr 返回的IP地址
 * @return 成功返回0，否则返回-1
*/
void create_call_list()
{
	memset(&call_list,0x00,sizeof(call_list));
	int i=0;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		call_list.call_item_array[i].dest_No[0]=-1;
		call_list.call_item_array[i].dest_No[1]=-1;
		call_list.call_item_array[i].dest_No[2]=-1;
		call_list.call_item_array[i].dest_No[3]=-1;
	}
	memset(&dest_type_list,0x00,sizeof(dest_type_list));
}

/**
 *
 * 在已经建立的呼叫链接列表中，通过远端房号查询远端的IP地址。
 * @param [in]  dest_No[4] 四位房号
 * @param [out] ip_addr 返回的IP地址
 * @return 成功返回0，否则返回-1
*/

unsigned int get_ip_from_call_list(unsigned char dest_No[4],unsigned int *ip_addr)
{
	int i=0;
	unsigned int ret=-1;
	if (pthread_mutex_lock(&call_item_lock) < 0){
		app_debug(DBG_FATAL,"lock call_item_lock failure !");
		return ret;
	}
	printf("call_list.dest_No:%02x,%02x,%02x,%02x\n",dest_No[0],dest_No[1],dest_No[2],dest_No[3]);
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if((call_list.call_item_array[i].dest_No[0]==dest_No[0])&&(call_list.call_item_array[i].dest_No[1]==dest_No[1])\
				&&(call_list.call_item_array[i].dest_No[2]==dest_No[2])&&(call_list.call_item_array[i].dest_No[3]==dest_No[3]))
		{
			*ip_addr=call_list.call_item_array[i].ip_addr;
			ret=0;
			break;
		}
	}
	printf("call_list.call_item_array[i].ip_addr:%x\n",call_list.call_item_array[i].ip_addr);
	pthread_mutex_unlock(&call_item_lock);
	return ret;
}
/**
 *
 * 添加呼叫对讲条目
 * @param [in] dest_No 为四位远端房号
 * @param [in] ip_addr 为远端目标IP
*/

void add_call_item(unsigned char dest_No[4],unsigned int ip_addr)
{
	if (pthread_mutex_lock(&call_item_lock) < 0){
		app_debug(DBG_FATAL,"lock call_item_lock failure !");
		return ;
	}
	int i;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if((call_list.call_item_array[i].dest_No[0]==dest_No[0])&&(call_list.call_item_array[i].dest_No[1]==dest_No[1])\
				&&(call_list.call_item_array[i].dest_No[2]==dest_No[2])&&(call_list.call_item_array[i].dest_No[3]==dest_No[3]))
		{
			call_list.call_item_array[i].ip_addr=ip_addr;
			pthread_mutex_unlock(&call_item_lock);
			return;
		}
	}
	memcpy(call_list.call_item_array[call_list.pos].dest_No,dest_No,4);
	call_list.call_item_array[call_list.pos].ip_addr=ip_addr;
	call_list.pos++;
	if(call_list.pos==COON_LIST_QUANTITY)
		call_list.pos=0;
	pthread_mutex_unlock(&call_item_lock);
}


#else


void create_call_list()
{
	memset(&call_list,0x00,sizeof(call_list));
	int i=0,j=0;
	ip_list_t *cur=NULL,*next=NULL;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		call_list.call_item_array[i].dest_No[0]=0xef;
		call_list.call_item_array[i].dest_No[1]=0xef;
		call_list.call_item_array[i].dest_No[2]=0xef;
		call_list.call_item_array[i].dest_No[3]=0xef;
		call_list.call_item_array[i].ip_count=0;
		call_list.call_item_array[i].ip_max=TONG_HAOIP_MAX;
		for(j=0;j<TONG_HAOIP_MAX;j++)
		{
			switch(j)
			{
			case 0:
			{
				call_list.call_item_array[i].ip_head=malloc(sizeof(ip_list_t));
				cur=call_list.call_item_array[i].ip_head;
				if(cur==NULL)
				{
					fprintf(stderr,"malloc for ip_head failure!\n");
					exit(EXIT_FAILURE);
				}
				else
				{
					cur->ip_talk=TONGHAO_TOOFF;
				}
			}
			break;
			default:
			{
				next=malloc(sizeof(ip_list_t));
				if(next!=NULL)
				{
					next->ip_talk=TONGHAO_TOOFF;
					next->ip_addr=INADDR_NONE;
					cur->next=next;
					cur=cur->next;
				}
				else
				{
					fprintf(stderr,"malloc for ip_head  cur failure!\n");
					exit(EXIT_FAILURE);
				}
			}
			break;
			}
			//call_list.call_item_array[i].ip_head.next=malloc(ip_list_t);
		}
	}
	memset(&dest_type_list,0x00,sizeof(dest_type_list));
}


/**
 *
 * 通过四位房号获取对应IP
 * @param [in] dest_No  四位房号
 * @param [out] ip_addr 目标IP
 * @return 成功返回0，否则返回-1
*/

unsigned int get_ip_from_call_list(unsigned char dest_No[4],unsigned int *ip_addr)
{
	int i=0;
	unsigned int ret=-1;
	if (pthread_mutex_lock(&call_item_lock) < 0){
		fprintf(stderr,"lock call_item_lock failure !");
		return ret;
	}
	//printf("call_list.dest_No:%02x,%02x,%02x,%02x\n",dest_No[0],dest_No[1],dest_No[2],dest_No[3]);
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if((call_list.call_item_array[i].dest_No[0]==dest_No[0])&&(call_list.call_item_array[i].dest_No[1]==dest_No[1])\
				&&(call_list.call_item_array[i].dest_No[2]==dest_No[2])&&(call_list.call_item_array[i].dest_No[3]==dest_No[3])\
				&&(call_list.call_item_array[i].ip_count>0))
		{
			*ip_addr=call_list.call_item_array[i].ip_head->ip_addr;
			ip_list_t *cur=call_list.call_item_array[i].ip_head;
			int j=0;
			for(j=0;(j<call_list.call_item_array[i].ip_count&&(j<TONG_HAOIP_MAX));j++)
			{
				if(cur!=NULL)
				{
					if((INADDR_NONE!=cur->ip_addr)&&(cur->ip_talk==TONGHAO_CALLING))
					{
						*ip_addr=cur->ip_addr;
						pthread_mutex_unlock(&call_item_lock);
						return 0;
					}
				}
				else
				{
					break;
				}
				cur=cur->next;
			}
			ret=0;
			break;
		}
	}
	pthread_mutex_unlock(&call_item_lock);
	return ret;
}

/**
 *
 * 通过四位房号获取对应正在对讲的目标IP
 * @param [in] dest_No  四位房号
 * @param [out] ip_addr 目标IP
 * @return 成功返回1，否则返回 0
*/

unsigned int get_talkip_from_call_list(unsigned char dest_No[4],ip_tongh_t ip_addr[TONG_HAOIP_MAX])
{
	int i=0,th_c=0;
	unsigned int ret=0;
	ip_tongh_t *prt=ip_addr;
	if (pthread_mutex_lock(&call_item_lock) < 0){
		fprintf(stderr,"lock call_item_lock failure !");
		return ret;
	}
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if((call_list.call_item_array[i].dest_No[0]==dest_No[0])&&(call_list.call_item_array[i].dest_No[1]==dest_No[1])\
				&&(call_list.call_item_array[i].dest_No[2]==dest_No[2])&&(call_list.call_item_array[i].dest_No[3]==dest_No[3])\
				&&(call_list.call_item_array[i].ip_count>0))
		{
			int j=0;
			ip_list_t *cur=call_list.call_item_array[i].ip_head;
			for(j=0,th_c=0;(j<call_list.call_item_array[i].ip_count&&(th_c<TONG_HAOIP_MAX));j++)
			{
				if(cur!=NULL)
				{

					if((INADDR_NONE!=cur->ip_addr)&&(prt->ip_talk==TONGHAO_TON))
					{

						prt->ip_addr=cur->ip_addr;
						prt->ip_talk=cur->ip_talk;
						prt++;
						ret++;
						th_c++;
					}
				}
				cur=cur->next;
			}
		}
	}
	//printf("call_list.call_item_array[i].ip_addr:%x\n",call_list.call_item_array[i].talk_ip);
	pthread_mutex_unlock(&call_item_lock);
	return ret;
}

/**
 *
 * 通过四位房号获取对应正的目标IP列表
 * @param [in] dest_No  四位房号
 * @param [out] ip_addr 目标IP数组，最多TONG_HAOIP_MAX个
 * @return 成功返回IP数量，否则返回 0
*/

unsigned int get_iplist_from_call_list(unsigned char dest_No[4],unsigned char flag,ip_tongh_t ip_addr[TONG_HAOIP_MAX])
{
	int i=0,th_c=0;
	unsigned int ret=0;
	ip_tongh_t *prt=ip_addr;

	if (pthread_mutex_lock(&call_item_lock) < 0){
		fprintf(stderr,"lock call_item_lock failure !");
		return ret;
	}
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if((call_list.call_item_array[i].dest_No[0]==dest_No[0])&&(call_list.call_item_array[i].dest_No[1]==dest_No[1])\
				&&(call_list.call_item_array[i].dest_No[2]==dest_No[2])&&(call_list.call_item_array[i].dest_No[3]==dest_No[3])\
				&&(call_list.call_item_array[i].ip_count>0))
		{
			int j=0;
			ip_list_t *cur=call_list.call_item_array[i].ip_head;
			for(j=0,th_c=0;(j<call_list.call_item_array[i].ip_count&&(th_c<TONG_HAOIP_MAX));j++)
			{
				if(cur!=NULL)
				{
					/*printf("~~~~dest_No:%02x %02x %02x %02x ip_addr=%x ip_talk=%d\n",\
							call_list.call_item_array[i].dest_No[0],call_list.call_item_array[i].dest_No[1],\
							call_list.call_item_array[i].dest_No[2],\
							call_list.call_item_array[i].dest_No[3],cur->ip_addr,cur->ip_talk);*/
					if(INADDR_NONE!=cur->ip_addr)
					{
						switch(flag)
						{
						case SEND_NO://返回已发704但未提机对讲的IP
						{
							if((cur->ip_talk==TONGHAO_CALLING)||(cur->ip_talk==TONGHAO_CALLON))
							{
								prt->ip_addr=cur->ip_addr;
								prt->ip_talk=cur->ip_talk;
								prt++;
								ret++;
							}
						}
						break;
						case SEND_TA: //返回正在对讲的IP
						{
							if(cur->ip_talk==TONGHAO_TON)
							{
								prt->ip_addr=cur->ip_addr;
								prt->ip_talk=cur->ip_talk;
								prt++;
								ret++;
							}
						}
						break;
						case SEND_NC://返回未发送704的IP
						{
							if(cur->ip_talk==TONGHAO_ARP)
							{
								prt->ip_addr=cur->ip_addr;
								prt->ip_talk=cur->ip_talk;
								prt++;
								ret++;
							}
						}
						break;
						case SEND_RE://返回收到705回复
						{
							if((cur->ip_talk==TONGHAO_TON)||(cur->ip_talk==TONGHAO_CALLING))
							{
								prt->ip_addr=cur->ip_addr;
								prt->ip_talk=cur->ip_talk;
								prt++;
								ret++;
							}
						}
						break;

						case SEND_AL://返回所有IP
						default:
						{
							if((cur->ip_talk==TONGHAO_TON)||(cur->ip_talk==TONGHAO_CALLING)\
									||(cur->ip_talk==TONGHAO_CALLON))
							{
								prt->ip_addr=cur->ip_addr;
								prt->ip_talk=cur->ip_talk;
								prt++;
								ret++;
							}
						}
						break;
						}

						th_c++;
					}
					cur=cur->next;
				}
				else
				{
					break;
				}
			}

			break;
		}
	}
	pthread_mutex_unlock(&call_item_lock);
	return ret;
}

/**
 *
 * 通过IP判断收到705是否为当前发送的704的回复
 * @param [in] dest_No  四位房号
 * @param [out] ip_addr 当前收到705的ip地址
 * @return 是当前发送704的回复返回1，否则返回 0
*/

int is_rtelc_to_current_call(unsigned char dest_No[4],unsigned int ip_addr)
{
	ip_tongh_t ip_array[TONG_HAOIP_MAX];
    memset(ip_array, 0, sizeof(ip_array));
	unsigned int count_th=get_iplist_from_call_list(dest_No,SEND_AL,ip_array);
	if(count_th<=0)
	{
		return 0;
	}
	else
	{
		int i=0;
		for(i=0;i<count_th;i++)
		{
			if(ip_addr==ip_array[i].ip_addr)
			{
				return 1;
			}
		}
		return 0;
	}
	return 0;
}

/**
 *
 * 通过四位房号获取清空对应的IP列表
 * @param [in] dest_No  四位房号
*/
void clear_tonghaoip_from_call_list(unsigned char dest_No[4])
{
	int i=0;
	if (pthread_mutex_lock(&call_item_lock) < 0){
		fprintf(stderr,"lock call_item_lock failure !");
		return ;
	}
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if((call_list.call_item_array[i].dest_No[0]==dest_No[0])&&(call_list.call_item_array[i].dest_No[1]==dest_No[1])\
				&&(call_list.call_item_array[i].dest_No[2]==dest_No[2])&&(call_list.call_item_array[i].dest_No[3]==dest_No[3])\
				&&(call_list.call_item_array[i].ip_count>0))
		{
			int j=0;
			ip_list_t *cur=call_list.call_item_array[i].ip_head;
			for(j=0;j<call_list.call_item_array[i].ip_max;j++)
			{
				if(cur!=NULL)
				{
					cur->ip_addr=INADDR_NONE;
					cur->ip_talk=TONGHAO_TOOFF;
					cur=cur->next;
				}
				else
				{
					break;
				}

			}
			call_list.call_item_array[i].ip_count=0;
			call_list.call_item_array[i].dest_No[0]=0xef;
			call_list.call_item_array[i].dest_No[1]=0xef;
			call_list.call_item_array[i].dest_No[2]=0xef;
			call_list.call_item_array[i].dest_No[3]=0xef;
			break;
		}
	}
	pthread_mutex_unlock(&call_item_lock);
}

/**
 *
 * 通过四位房号获取清空对应的对讲IP
 * @param [in] dest_No  四位房号
*/
void clear_talkip_from_call_list(unsigned char dest_No[4])
{
	int i=0;
	if (pthread_mutex_lock(&call_item_lock) < 0){
		fprintf(stderr,"lock call_item_lock failure !");
		return ;
	}
	//printf("call_list.dest_No:%02x,%02x,%02x,%02x\n",dest_No[0],dest_No[1],dest_No[2],dest_No[3]);
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if((call_list.call_item_array[i].dest_No[0]==dest_No[0])&&(call_list.call_item_array[i].dest_No[1]==dest_No[1])\
				&&(call_list.call_item_array[i].dest_No[2]==dest_No[2])&&(call_list.call_item_array[i].dest_No[3]==dest_No[3]))
		{
			int j=0;
			ip_list_t *cur=call_list.call_item_array[i].ip_head;
			for(j=0;j<call_list.call_item_array[i].ip_count;j++)
			{
				if(cur!=NULL)
				{
					cur->ip_talk=TONGHAO_TOOFF;
					cur=cur->next;
				}
				else
				{
					break;
				}
			}
			call_list.call_item_array[i].dest_No[0]=0xef;
			call_list.call_item_array[i].dest_No[1]=0xef;
			call_list.call_item_array[i].dest_No[2]=0xef;
			call_list.call_item_array[i].dest_No[3]=0xef;

			break;
		}
	}
	pthread_mutex_unlock(&call_item_lock);
}


/**
 *
 * 添加呼叫对讲条目
 * @param [in] dest_No 为四位远端房号
 * @param [in] ip_addr 为远端目标IP
*/

void add_call_item(unsigned char dest_No[4],unsigned int ip_addr)
{
	if (pthread_mutex_lock(&call_item_lock) < 0){
		fprintf(stderr,"lock call_item_lock failure !");
		return ;
	}
	int i;
	/*printf("~~~~add_call_item:%02x %02x %02x %02x ip_addr=%x \n",\
			dest_No[0],dest_No[1],dest_No[2],dest_No[3],ip_addr);*/
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if((call_list.call_item_array[i].dest_No[0]==dest_No[0])&&(call_list.call_item_array[i].dest_No[1]==dest_No[1])\
				&&(call_list.call_item_array[i].dest_No[2]==dest_No[2])&&(call_list.call_item_array[i].dest_No[3]==dest_No[3])\
				&&(call_list.call_item_array[i].ip_count>0))
		{
			ip_list_t *cur=call_list.call_item_array[i].ip_head;
			int j;//=call_list.call_item_array[i].ip_count;
			for(j=0;(j<call_list.call_item_array[i].ip_count)&&(j<TONG_HAOIP_MAX);j++)
			{
				if((cur->ip_addr==ip_addr)||(cur->ip_addr==INADDR_NONE))
				{
					j=0;
					if(cur->ip_talk==TONGHAO_DOWN)
					{
						cur->ip_talk=TONGHAO_ARP;
					}

					break;
				}
				else
				{
					cur=cur->next;
				}
			}
			if((j>0)&&(cur!=NULL))
			{
				cur->ip_addr=ip_addr;
				cur->ip_talk=TONGHAO_ARP;
				//printf("call_list.pos=%d i=%d\n",call_list.pos,j);
				call_list.call_item_array[i].ip_count++;
				if(call_list.call_item_array[i].ip_count>TONG_HAOIP_MAX)
				{
					call_list.call_item_array[i].ip_count=TONG_HAOIP_MAX;
				}
			}
			pthread_mutex_unlock(&call_item_lock);
			return;
		}
	}
	//当前表中没有待添加IP
	memcpy(call_list.call_item_array[call_list.pos].dest_No,dest_No,4);
	if(call_list.call_item_array[call_list.pos].ip_head!=NULL)
	{

		call_list.call_item_array[call_list.pos].ip_head->ip_addr=ip_addr;
		call_list.call_item_array[call_list.pos].ip_head->ip_talk=TONGHAO_ARP;
		call_list.call_item_array[call_list.pos].ip_count=1;
		call_list.pos++;
		//printf("call_list.pos=%d i=%d\n",call_list.pos,0);

		if(call_list.pos==COON_LIST_QUANTITY)
		{
			call_list.pos=0;
		}
	}
	else
	{
		fprintf(stderr,"err: call_list.call_item_array[%d].ip_head is NULL!",call_list.pos);
	}

	pthread_mutex_unlock(&call_item_lock);
}



/**
 *
 * ARP阶段获取回复IP信息的目标设备的IP地址
 * @param [in] dev_ip 建立通话的设备IP
*/
void set_cur_talk_dev_ip(unsigned char dest_No[4],unsigned int dev_ip)
{
	if(pthread_mutex_lock (&call_item_lock)<0)
	{
		fprintf(stderr,"lock dev_arrary_list_lock failure!");
		return;
	}
	int i;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if((call_list.call_item_array[i].dest_No[0]==dest_No[0])&&(call_list.call_item_array[i].dest_No[1]==dest_No[1])\
				&&(call_list.call_item_array[i].dest_No[2]==dest_No[2])&&(call_list.call_item_array[i].dest_No[3]==dest_No[3])\
				)
		{
			ip_list_t *cur=NULL;
			cur=call_list.call_item_array[i].ip_head;
			int j;//=call_list.call_item_array[i].ip_count;
			for(j=0;j<call_list.call_item_array[i].ip_count;j++)
			{
				if(cur->ip_addr==dev_ip)
				{
					cur->ip_talk=TONGHAO_TON;
					pthread_mutex_unlock(&call_item_lock);
					return;
				}
				else
				{
					cur=cur->next;
				}
			}
			pthread_mutex_unlock(&call_item_lock);
			return;
		}
	}
	pthread_mutex_unlock(&call_item_lock);
}

/**
 *
 * 设置目标设备的状态
 * @param [in] dest_No 目标设备的房号
 * @param [in] dev_ip  目标设备的IP
 * @param [in] status  目标设备的状态
*/

void set_dest_dev_status(unsigned char dest_No[4],unsigned int dev_ip,th_status_t status)
{
	if(pthread_mutex_lock (&call_item_lock)<0)
	{
		fprintf(stderr,"lock dev_arrary_list_lock failure!");
		return;
	}
	int i;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if((call_list.call_item_array[i].dest_No[0]==dest_No[0])&&(call_list.call_item_array[i].dest_No[1]==dest_No[1])\
				&&(call_list.call_item_array[i].dest_No[2]==dest_No[2])&&(call_list.call_item_array[i].dest_No[3]==dest_No[3])\
				)
		{
			ip_list_t *cur=NULL;
			cur=call_list.call_item_array[i].ip_head;
			int j;
			for(j=0;j<call_list.call_item_array[i].ip_count;j++)
			{
				if(cur->ip_addr==dev_ip)
				{
					cur->ip_talk=status;
					pthread_mutex_unlock(&call_item_lock);
					return;
				}
				else
				{
					cur=cur->next;
				}
			}
			pthread_mutex_unlock(&call_item_lock);
			return;
		}
	}
	pthread_mutex_unlock(&call_item_lock);
}

#endif

#ifndef HAVE_TONGHAO
/**
 *
 * 通过远端IP添加远端设备的设备类型到本地设备类型列表中
 * @param [in] ip_addr 为远端目标IP
 * @param [in] type 为四位远端设备类型
*/

void add_dest_dev_type_by_ip(unsigned int ip_addr,unsigned int type)
{
	if (pthread_mutex_lock(&dev_type_lock) < 0){
		app_debug(DBG_FATAL,"lock call_item_lock failure !");
		return ;
	}
	printf("\nip_addr:0x%x type:%d\n",ip_addr,type);
	int i;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if(dest_type_list.dev_type_array[i].ip_addr==ip_addr)
		{
			dest_type_list.dev_type_array[i].dest_dev_type=type;
			pthread_mutex_unlock(&dev_type_lock);
			return;
		}
	}
	dest_type_list.dev_type_array[dest_type_list.pos].dest_dev_type=type;
	dest_type_list.dev_type_array[dest_type_list.pos].ip_addr=ip_addr;
	dest_type_list.pos++;
	if(dest_type_list.pos==COON_LIST_QUANTITY)
			dest_type_list.pos=0;
	pthread_mutex_unlock(&dev_type_lock);
}


/**
 *
 * 通过远端房号查询远端设备类型
 * @param [in] dest_No 为四位远端房号
 * @return 成功返回实际设备类型，默认返回TYPE_JDM365
*/

unsigned int get_dest_dev_type(unsigned char dest_No[4])
{
	unsigned int ret=TYPE_JDM365;
	if (pthread_mutex_lock(&dev_type_lock) < 0){
		app_debug(DBG_FATAL,"lock call_item_lock failure !");
		return ret;
	}
	printf("\ndest_No:0x%2.2x,0x%2.2x,0x%2.2x,0x%2.2x\n",dest_No[0],dest_No[1],dest_No[2],dest_No[3]);
	int i;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if((dest_type_list.dev_type_array[i].dest_No[0]==dest_No[0])&&(dest_type_list.dev_type_array[i].dest_No[1]==dest_No[1])\
				&&(dest_type_list.dev_type_array[i].dest_No[2]==dest_No[2])&&(dest_type_list.dev_type_array[i].dest_No[3]==dest_No[3]))
		{
			ret=dest_type_list.dev_type_array[i].dest_dev_type;
		}
	}
	pthread_mutex_unlock(&dev_type_lock);
	return ret;
}

/**
 *
 * 通过远端IP查询远端设备类型
 * @param [in] ip_addr 为远端ip
 * @return 成功返回实际设备类型，默认返回TYPE_JDM365
*/

unsigned int get_dest_dev_type_by_ip(unsigned int ip_addr)
{
	unsigned int ret=TYPE_JDM365;
	if (pthread_mutex_lock(&dev_type_lock) < 0){
		app_debug(DBG_FATAL,"lock call_item_lock failure !");
		return ret;
	}
	int i;
	for(i=0;i<COON_LIST_QUANTITY;i++)
	{
		if(dest_type_list.dev_type_array[i].ip_addr==ip_addr)
		{
			ret=dest_type_list.dev_type_array[i].dest_dev_type;
		}
	}
	pthread_mutex_unlock(&dev_type_lock);
	printf("\nget_dest_dev_type_by_ip ret:%d\n",ret);
	return ret;
}

unsigned int get_dest_dev_type_by_NO(unsigned char dest_No[4])
{
	unsigned int ip_addr=TYPE_JDM365;
	if(get_ip_from_call_list(dest_No,&ip_addr)<0)
		return ip_addr;
	return get_dest_dev_type_by_ip(ip_addr);
}
#endif

