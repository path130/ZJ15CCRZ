/*
 * vlan_iptable.h
 *
 *  Created on: 2014-8-19 上午9:38:41
 *  
 */

#ifndef VLAN_IPTABLE_H_
#define VLAN_IPTABLE_H_

#include <stdint.h>
#include <stdlib.h>

#define MAX_VLANIP_QU (100000)

typedef struct{
	uint8_t dev_no[4]; 			       //设备编码
	uint8_t no_def[8]; 		          //预留
	uint8_t dev_ip[4]; 			       //IP
	uint8_t ip_def[2]; 		          //预留
}vlanip_table_t;

typedef struct
{
	uint8_t building_no;              //楼号
	uint32_t offset;
}building_search_t;

typedef struct BUILDING_SEARCH_LIST_T
{
	uint8_t building_no;              //楼号
	uint32_t offset;
	struct BUILDING_SEARCH_LIST_T *next;
}bld_srh_list_t;


typedef struct
{
	uint32_t seg_quantity;         //段的数量
	uint32_t max_seg_quantity;         //段的数量
	building_search_t *seg_arrary;

	uint32_t quantity;             //ip对应表中设备的总数量
	uint32_t def;                  //未使用，网络数据中有此4字节
	vlanip_table_t *table;         //ip对应表的起始地址
}vlanip_handel_t;


vlanip_handel_t* create_vlan_iptable(uint32_t quantity);

void delete_vlan_iptable(vlanip_handel_t*handle);

uint32_t search_vlanip_by_devno(const vlanip_handel_t*handle, const uint8_t *dev_no,\
		const int max_ip,uint32_t *ip_ret);

uint32_t get_vlan_iptable_quantity(vlanip_handel_t *handle);
const void* get_vlan_iptable_start_addr(vlanip_handel_t *handle,uint32_t *def);

int refresh_vlan_iptable(vlanip_handel_t *handle, const vlanip_table_t *table,uint32_t quantity,uint32_t def);


#endif /* VLAN_IPTABLE_H_ */
