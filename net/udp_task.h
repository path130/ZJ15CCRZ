/*
 * udp_pro.h
 *
 *  Created on: 2012-6-18
 *
 */

#ifndef UDP_PRO_H_
#define UDP_PRO_H_

#include "data_struct.h"
#include "net_com_struct.h"
#include "net_data_pro.h"
#include "udp_cmd.h"
#include "global_def.h"
#include "udp_pc_task.h"

#if defined (__cplusplus)
extern "C" {
#endif

void udp_task_create(void);
void udp_task_delete(void);
int udp_time_request_task_create(void);
int set_udp_board_ip(const void *destip,int len);
int udp_client_send_data(unsigned char *buf,int len);
int send_data_to_udp_node(void *buf,size_t len,uint32_t addr,uint16_t port);
ssize_t send_udp_data_to_socket(void *data,ssize_t len,unsigned int dest_addr,unsigned short port);

/*
int get_dest_ip_by_arp(unsigned char *dev_No,char *ip, signed long req_dev_status,unsigned short addr_way);
int udp_master_send_data(unsigned char * buf,int len);
int set_arp_board_ip(char *destip,int len);
int udp_client_send_data(unsigned char *buf,int len);
void udp_start(void);
void udp_stop();*/

#if defined (__cplusplus)
}
#endif

#endif /* UDP_PRO_H_ */
