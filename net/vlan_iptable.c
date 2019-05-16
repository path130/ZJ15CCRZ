/*
 * vlan_iptable.c
 *
 *  Created on: 2014-8-19 上午9:38:30
 *  
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vlan_iptable.h"


vlanip_handel_t* create_vlan_iptable(uint32_t quantity)
{
	if(quantity>MAX_VLANIP_QU)
	{
		fprintf(stderr,"err:quantity>MAX_VLANIP_QU\n");
		return NULL;
	}
	vlanip_handel_t *handle=calloc(1,sizeof(vlanip_handel_t));
	if(handle==NULL)
	{
		perror("calloc for handle failure:");
		return NULL;
	}
	handle->quantity=quantity;
	if(quantity>0)
	{
		handle->table=malloc(quantity*sizeof(vlanip_table_t));
		if(handle->table==NULL)
		{
			perror("malloc for handle->table failure:");
			if(handle!=NULL)
			{
				free(handle);
				handle=NULL;
			}
			return NULL;
		}
		else
		{
			memset(handle->table,0,quantity*sizeof(vlanip_table_t));
		}
	}

	handle->seg_arrary=malloc(100*sizeof(building_search_t));
	if(handle->seg_arrary==NULL)
	{
		perror("malloc for handle->seg_arrary failure:");
		if(handle->table!=NULL)
		{
			free(handle->table);
			handle->table=NULL;
		}
		if(handle!=NULL)
		{
			free(handle);
			handle=NULL;
		}

		handle->seg_quantity=0;
		return NULL;
	}
	else
	{
		handle->max_seg_quantity=100;
		handle->seg_quantity=0;
	}

	return handle;
}

void delete_vlan_iptable(vlanip_handel_t*handle)
{
	if(handle!=NULL)
	{
		if(handle->table!=NULL)
		{
			free(handle->table);
			handle->table=NULL;
		}

		if(handle->seg_arrary!=NULL)
		{
			free(handle->seg_arrary);
			handle->seg_arrary=NULL;
		}

		if(handle!=NULL)
		{
			free(handle);
			handle=NULL;
		}
	}
}

int refresh_vlan_iptable(vlanip_handel_t *handle, const vlanip_table_t *table,uint32_t quantity,uint32_t def)
{
	if((quantity>MAX_VLANIP_QU)||(table==NULL)||(handle==NULL))
	{
		fprintf(stderr,"err:quantity>MAX_VLANIP_QU\n");
		return -1;
	}
	int i=0;
	uint32_t seg_quantity=0;
	bld_srh_list_t seg_arrary_head;
	memset(&seg_arrary_head,0,sizeof(bld_srh_list_t));
	uint8_t building_no=0xef;
	if(handle->table==NULL)
	{
		handle->table=malloc(quantity*sizeof(vlanip_table_t));
		if(handle->table==NULL)
		{
			perror("refresh malloc for handle->table failure:");
			handle->quantity=0;
			return -1;
		}
	}
	else if(handle->quantity!=quantity)
	{
		handle->table=(vlanip_table_t*)realloc(handle->table,(quantity)*sizeof(vlanip_table_t));
		if(handle->table==NULL)
		{
			perror("realloc for handle->table failure:");
			handle->quantity=0;
			return -1;
		}
	}
	memcpy(handle->table,table,quantity*sizeof(vlanip_table_t));
	handle->quantity=quantity;
	handle->def=def;
	vlanip_table_t*ptr_table=handle->table;

	bld_srh_list_t *prv_seg_arrary_head=&seg_arrary_head;
	bld_srh_list_t *cur_seg_arrary_head=&seg_arrary_head;

	for(i=0;i<quantity;i++)
	{
		if(building_no!=ptr_table->dev_no[0])
		{
			building_no=ptr_table->dev_no[0];
			if(cur_seg_arrary_head!=NULL)
			{
				cur_seg_arrary_head->building_no=building_no;
				cur_seg_arrary_head->offset=i;
				seg_quantity++;
				prv_seg_arrary_head=cur_seg_arrary_head;
				cur_seg_arrary_head=cur_seg_arrary_head->next;

			}

			if(cur_seg_arrary_head==NULL)
			{
				prv_seg_arrary_head->next=calloc(1,sizeof(bld_srh_list_t));
				if(prv_seg_arrary_head->next==NULL)
				{
					perror("refresh calloc for prv_seg_arrary_head->next failure:");
					prv_seg_arrary_head=seg_arrary_head.next;
					cur_seg_arrary_head=seg_arrary_head.next;
					while(prv_seg_arrary_head!=NULL)
					{
						cur_seg_arrary_head=prv_seg_arrary_head->next;
						free(prv_seg_arrary_head);
						prv_seg_arrary_head=cur_seg_arrary_head;
					}
					return -1;
				}
				cur_seg_arrary_head=prv_seg_arrary_head->next;
			}
		}
		ptr_table++;
	}

	if(handle->seg_arrary==NULL)
	{
		handle->seg_arrary=malloc(seg_quantity*sizeof(building_search_t));
		if(handle->seg_arrary==NULL)
		{
			perror("malloc for handle->seg_start failure:");
			handle->seg_quantity=0;
			prv_seg_arrary_head=seg_arrary_head.next;
			cur_seg_arrary_head=seg_arrary_head.next;
			while(prv_seg_arrary_head!=NULL)
			{
				cur_seg_arrary_head=prv_seg_arrary_head->next;
				free(prv_seg_arrary_head);
				prv_seg_arrary_head=cur_seg_arrary_head;
			}
			return -1;
		}
		building_search_t* ptr_seg_arrary=handle->seg_arrary;
		cur_seg_arrary_head=&seg_arrary_head;
		for(i=0;i<seg_quantity;i++)
		{
			if(cur_seg_arrary_head!=NULL)
			{
				ptr_seg_arrary->building_no=cur_seg_arrary_head->building_no;
				ptr_seg_arrary->offset=cur_seg_arrary_head->offset;
				cur_seg_arrary_head=cur_seg_arrary_head->next;
				ptr_seg_arrary++;
			}
			else
			{
				break;
			}
		}
		handle->seg_quantity=seg_quantity;
	}
	else
	{
		if(seg_quantity>handle->max_seg_quantity)
		{
			handle->seg_arrary=realloc(handle->seg_arrary,seg_quantity*sizeof(building_search_t));
			if(handle->seg_arrary==NULL)
			{
				perror("realloc for handle->seg_start failure:");
				handle->seg_quantity=0;
				prv_seg_arrary_head=seg_arrary_head.next;
				cur_seg_arrary_head=seg_arrary_head.next;
				while(prv_seg_arrary_head!=NULL)
				{
					cur_seg_arrary_head=prv_seg_arrary_head->next;
					free(prv_seg_arrary_head);
					prv_seg_arrary_head=cur_seg_arrary_head;
				}
				return -1;
			}
			handle->max_seg_quantity=seg_quantity;
		}
		else
		{
			memset(handle->seg_arrary,0,seg_quantity*sizeof(building_search_t));
		}

		building_search_t* ptr_seg_arrary=handle->seg_arrary;
		cur_seg_arrary_head=&seg_arrary_head;
		for(i=0;i<seg_quantity;i++)
		{
			if(cur_seg_arrary_head!=NULL)
			{
				ptr_seg_arrary->building_no=cur_seg_arrary_head->building_no;
				ptr_seg_arrary->offset=cur_seg_arrary_head->offset;
				cur_seg_arrary_head=cur_seg_arrary_head->next;
				ptr_seg_arrary++;
			}
			else
			{
				break;
			}
		}
		handle->seg_quantity=seg_quantity;
	}

	prv_seg_arrary_head=seg_arrary_head.next;
	cur_seg_arrary_head=seg_arrary_head.next;
	int x=0;
	while(prv_seg_arrary_head!=NULL)
	{
		cur_seg_arrary_head=prv_seg_arrary_head->next;
		free(prv_seg_arrary_head);
		prv_seg_arrary_head=cur_seg_arrary_head;
		x++;
	}
	return 0;
}

uint32_t get_vlan_iptable_quantity(vlanip_handel_t *handle)
{
	return handle->quantity;
}

const void* get_vlan_iptable_start_addr(vlanip_handel_t *handle,uint32_t *def)
{
	*def=handle->def;
	return handle->table;
}

/*
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
*/

uint32_t search_vlanip_by_devno(const vlanip_handel_t*handle,const uint8_t *dev_no,\
		const int max_ip,uint32_t *ip_ret)
{
	int i=0;
	uint32_t ret=0;
	vlanip_table_t* ptr=handle->table;
	int ip_offset=-1;
	if((handle->seg_arrary!=NULL)&&(handle->table!=NULL)&&(handle->seg_quantity>0)&&(handle->quantity>0))
	{
		const building_search_t *ptr_seg_arrary=handle->seg_arrary;

		for(i=0;i<handle->seg_quantity;i++)
		{
			if(dev_no[0]==ptr_seg_arrary->building_no)
			{
				ip_offset=ptr_seg_arrary->offset;
				break;
			}
			else
			{
				ptr_seg_arrary++;
			}
		}

		if(ip_offset>-1)
		{

			ptr+=ip_offset;
			for(i=0;i<(handle->quantity-ip_offset);i++)
			{
			/*	printf("dev_no:%02x %02x %02x %02x ptr->dev_no:%02x %02x %02x %02x\n",dev_no[0],dev_no[1],dev_no[2],dev_no[3],\
						ptr->dev_no[0]&0xff,ptr->dev_no[1]&0xff,ptr->dev_no[2]&0xff,ptr->dev_no[3]&0xff);
*/
				if((ptr->dev_no[0]==dev_no[0])&&(ptr->dev_no[1]==dev_no[1])\
						&&(ptr->dev_no[2]==dev_no[2])&&(ptr->dev_no[3]==dev_no[3]))
				{
					memcpy(ip_ret+ret,ptr->dev_ip,4);
	/*				struct in_addr in_addr;
					in_addr.s_addr=ip_ret[ret];
					printf("###0ret:%s\n",inet_ntoa(in_addr));*/
					ret++;
					if(ret>max_ip)
					{
						fprintf(stderr,"error,too many ip!\n");
						break;
					}
				}
				ptr++;
			}
		}
	}
	return ret;
}
