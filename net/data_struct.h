/*
 * data_struct.h
 *
 *  Created on: 2012-6-20
 *data_struct.h
 */

#ifndef DATA_STRUCT_H_
#define DATA_STRUCT_H_

#include "dev_info.h"


#pragma pack(1)
typedef struct
{
	unsigned char dest_addr[4];
	unsigned char src_addr[4];
	unsigned int oper_type;
	int resstaus;
}ntel_data;

typedef struct
{
	short allow;
	unsigned int oper_type;
	unsigned int resstaus;
}rtel_data;

#pragma pack()


#endif /* DATA_STRUCT_H_ */
