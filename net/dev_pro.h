/*
 * dev_pro.h
 *
 *  Created on: 2012-6-21
 *
 */

#ifndef DEV_PRO_H_
#define DEV_PRO_H_
#include "data_com_ui.h"
#include "global_def.h"


int is_call_me(unsigned char devno[4]);
//int is_call_on();
void set_call_on();
void set_call_off();

int is_call_my_buliding(unsigned char DevNo[4]);
void get_mydev_ip(unsigned char *buf);
int is_mydev_ip(unsigned int ip_addr);
unsigned int get_local_ip();
unsigned int get_server_ip();
int get_local_mac(unsigned char buf[6]);
unsigned int get_boardcast_ip();

#endif /* DEV_PRO_H_ */
