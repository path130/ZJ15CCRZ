/*
 * net_init.h
 *
 *  Created on: 2012-9-19 下午7:28:08
 *  
 */

#ifndef NET_INIT_H_
#define NET_INIT_H_

#include "net_parameter.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define ETH_DEV "eth0"
/*
 * NET 创建初始化
 */
extern pdu_head	udp_echo;
int inti_para_for_ipinstall();//初始化同ipinstall通信运行参数
void net_com_create(net_para *net_para_data, void *arg);
void net_com_delete();

void register_dev_to_pc();

int ui_refresh_vlan_iptable( void *table,uint32_t quantity,uint32_t def);
uint32_t ui_get_vlan_iptable_quantity(void);
const void* ui_get_vlan_iptable_start_addr(uint32_t *def);
uint32_t arp_search_vlanip_by_devno(const uint8_t dev_no[4],const int max_ip,uint32_t*ip_ret);
#if defined (__cplusplus)
}
#endif


#endif /* NET_INIT_H_ */
