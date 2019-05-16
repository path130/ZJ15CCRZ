/*
 * info_container.h
 *
 *  Created on: 2013-8-22 下午2:48:08
 *  
 */

#ifndef INFO_CONTAINER_H_
#define INFO_CONTAINER_H_
#pragma pack(1)
typedef struct INFO_CODE_T
{
	char fjno[2];
	short int info_num;
}info_code_t;
#pragma pack()

int load_info_container_data(void);
int save_info_container_data(void *buf,int len);
short int get_code_from_container_data(char fjno[2]);
int get_all_info_container_data(void *buf,int len);
void init_info_arp_for_fenji(void);


#endif /* INFO_CONTAINER_H_ */
