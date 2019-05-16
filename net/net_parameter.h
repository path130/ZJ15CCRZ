/*
 * net_parameter.h
 *
 *  Created on: 2012-7-6
 *
 */

#ifndef NET_PARAMETER_H_
#define NET_PARAMETER_H_

#include <stdint.h>
#include "data_struct.h"
#include "dev_info.h"

typedef struct{
	unsigned char dev_No[4];		// 本机房号
	eth_para_t eth_para;
	unsigned int board_ip;		// 广播Ip
	int (*load_global_data_fun)();
	int (*save_global_data_fun)();
	int (*save_eth_para_fun)(eth_para_t*);
      uint32_t vlan_enable;
}net_para;


#endif /* NET_PARAMETER_H_ */
