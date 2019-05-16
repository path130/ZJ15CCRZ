/*
 * dev_pro.c
 *
 *  Created on: 2012-6-21
 *
 */
#include "dev_pro.h"
#include "global_def.h"
#include <string.h>
#include <pthread.h>
#include "global_data.h"


int is_call_me(unsigned char DevNo[4])
{
	int ret=0;
#if DEV_GATE|DEV_GLJKZQ
	/*app_debug(DBG_INFO, "MyNO: %2.2x,%2.2x,%2.2x,%2.2x DestNO:%2.2x,%2.2x,%2.2x,%2.2x\n",gData.DoorNo[0],gData.DoorNo[1],0x00,gData.DoorNo[2]\
		,DevNo[0],DevNo[1],DevNo[2],DevNo[3]);
    */
	ret=(gData.DoorNo[0]==DevNo[0])&&(gData.DoorNo[1]==DevNo[1]) &&(0x00==DevNo[2])&&(gData.DoorNo[2]==DevNo[3]);
	//ret=(gData.DoorNo[0]==DevNo[0])&&(gData.DoorNo[1]==DevNo[1]) &&(gData.DoorNo[2]==DevNo[3]);
	//printf("#################ret=%d\n",ret);
#elif DEV_CONTROL
/*
	//app_debug(DBG_INFO, "MyNO: %2.2x,%2.2x,%2.2x,%2.2x DestNO:%2.2x,%2.2x,%2.2x,%2.2x\n",dev_info->dev_No[0],dev_info->dev_No[1],dev_info->dev_No[2]\
	//		,dev_info->dev_No[3],DevNo[0],DevNo[1],DevNo[2],DevNo[3]);
*/

	ret=(dev_info->dev_No[0]==DevNo[0])&&(dev_info->dev_No[1]==DevNo[1]) &&(dev_info->dev_No[2]==DevNo[2])&&(dev_info->dev_No[3]==DevNo[3]);

#else// DEV_TERMINAL

/*
	//app_debug(DBG_INFO, "MyNO: %2.2x,%2.2x,%2.2x,%2.2x DestNO:%2.2x,%2.2x,%2.2x,%2.2x\n",gData.DoorNo[0],gData.DoorNo[1],gData.DoorNo[2]\
	//		,gData.DoorNo[3],DevNo[0],DevNo[1],DevNo[2],DevNo[3]);
*/

	ret=(gData.DoorNo[0]==DevNo[0])&&(gData.DoorNo[1]==DevNo[1]) &&(gData.DoorNo[2]==DevNo[2])&&(gData.DoorNo[3]==DevNo[3]);

#endif
	return ret;
}

int is_call_my_buliding(unsigned char DevNo[4])
{
	int ret=0;
	if(is_have_infostore())
	{
		ret=(gData.DoorNo[0]==DevNo[0])&&(gData.DoorNo[1]==DevNo[1]);
	}
	return ret;
}

void get_mydev_ip(unsigned char *buf)
{

	memcpy(buf,my_devlist.DevIp,4);
}

int is_mydev_ip(unsigned int ip_addr)
{
	return (memcmp(&ip_addr,my_devlist.DevIp,4)?0:1);

}
