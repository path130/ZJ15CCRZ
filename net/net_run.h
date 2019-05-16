/*
 * net_run.h
 *
 *  Created on: 2012-9-19 下午2:55:39
 *  
 */

#ifndef NET_RUN_H_
#define NET_RUN_H_
#include "net_com.h"
#include "net_com_struct.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define TYPE_TCP 0
#define TYPE_UDP 1
#define TYPE_TCP_PC 3
#define TYPE_TCP_PC_JSON 4
//connect_node
int net_runtime_creat(connect_node *net_runtime,int type);

#if defined (__cplusplus)
}
#endif

#endif /* NET_RUN_H_ */
