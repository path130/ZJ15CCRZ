/*
 * tcp_pc_task.h
 *
 *  Created on: 2012-8-6 上午11:58:19
 *  
 */

#ifndef TCP_PC_TASK_H_
#define TCP_PC_TASK_H_
#include "public.h"
#include "tcp_task.h"
#include "net_run.h"

#define NET_PC_BUF_SIZE NET_BUF_SIZE
#define NET_PC_BUF_NUM  NET_BUF_NUM

void tcp_pc_recv_task();
void tcp_pc_send_task();

int request_pc_send_buf(tcp_data **buf);
int send_data_to_pc(unsigned char *buf,int len);

void tcp_pc_task_create();
void tcp_pc_task_delete();

#endif /* TCP_PC_TASK_H_ */
