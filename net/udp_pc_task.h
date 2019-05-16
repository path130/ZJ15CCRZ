/*
 * udp_pc_task.h
 *
 *  Created on: 2012-10-29 下午6:32:46
 *  
 */

#ifndef UDP_PC_TASK_H_
#define UDP_PC_TASK_H_
#include "net_com_struct.h"
#include "dev_info.h"

void InitOpdu(pdu_head * pHeader);
void udp_pc_process_task(int);

#endif /* UDP_PC_TASK_H_ */
