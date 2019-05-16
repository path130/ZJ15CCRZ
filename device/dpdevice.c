/*#include "dpdevice.h"

#include <fcntl.h>
#include<linux/input.h>
#include <signal.h>
#include <sys/mman.h>
#include "audio.h"
#include "dev_config.h"

static int m_hReset_Net;  //by mkq for songjian 20170907
static int m_hGpioFC;//by mkq 20170914
#define NULL 0
extern  void net_send_alarm(int alarm_type);
void SetGpioVal(int hdev, int value)
{
	if(hdev == NULL)
		return;
	if(ioctl((int)hdev, IOCTRL_SET_GPIO_VALUE,&value))
	{
		printf("IOCTRL_SET_GPIO_VALUE  fail error \r\n");
		return;
	}
}

 int GetGpioVal(int hdev)//add by mkq 20170914
{
	int data= 0;
	int bytesreturned;
	//if(!ioctl((int)hdev, IOCTRL_GET_GPIO_VALUE, &data))
	//{
	//  printf( "IOCTRL_GET_GPIO_VALUE  fail error %d \n",data);
	//	return 0;
	//}
	//return 1;
	ioctl((int)hdev, IOCTRL_GET_GPIO_VALUE, &data);
        return data;
}

int InitGpio(int pin, int inout, int value)
{
	int fd;
	GPIO_CFG gpio_cfg;

	fd = open("/dev/gpio", O_RDWR,0);
	if(fd < 0)
	{
		printf("open /dev/gpio  fail!\n");
		return 0;
	}
	
	memset(&gpio_cfg,0,sizeof(GPIO_CFG));
	gpio_cfg.gpio_num= (short)pin;
	gpio_cfg.gpio_cfg = (short)inout;
	gpio_cfg.gpio_pull = (short)value;
	if(ioctl(fd,IOCTRL_GPIO_INIT,&gpio_cfg))
	{
		printf("IOCTRL_GPIO_INIT  fail error is \r\n");
		close(fd);
		return 0;
	}
	return (int)fd;
}
void Reset_Net(void)//by mkq 20170907
{
  m_hReset_Net =InitGpio(17,1,1);
  SetGpioVal(m_hReset_Net,0);
  usleep(50*1000);
  SetGpioVal(m_hReset_Net,1);
  usleep(200*1000);
  
 }
void DeinitGpio(int hGpio)
{
	close((int)hGpio);
}


 void FCcheck(void)
{
        m_hGpioFC = InitGpio(199,0,1);
	 int Fanchai = 0;	// 低电平正常
	int cur = 0;
	//unsigned char msg[28];
	while(1){
	        usleep(60*1000);
		cur = GetGpioVal(m_hGpioFC);
		if(cur == 1)
		{
			printf("fang chai happening!!!\r\n");
			if(dev_cfg.en_fc_alarm){
			   net_send_alarm(0x50);
			   audio_play(PMT_ALARM,DEV_VOL_PLAY);
			  //msg[0] = MSG_FROM_KEY;
			  //msg[1] = KEY_FC;
			  //msg[2] = cur;
			  // pipe_put(pipe_proc,msg,MSG_LEN_PROC);
			}
			
			cur = 0;
			
		}
	
	}
	DeinitGpio(m_hGpioFC);
}

*/
