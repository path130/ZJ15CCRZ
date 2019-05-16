#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "dpplatform.h"
#include "dpgpio.h"

#define	IOCTL_GRAPHIC_SNR			0x201
#define	IOCTL_SENSOR_START			CTL_CODE(IOCTL_GRAPHIC_SNR, 0x220, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_SENSOR_STOP		CTL_CODE(IOCTL_GRAPHIC_SNR, 0x221, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_SENSOR_CTRL			CTL_CODE(IOCTL_GRAPHIC_SNR, 0x222, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_SENSOR_READ			CTL_CODE(IOCTL_GRAPHIC_SNR, 0x223, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define MT9V139_AE_RULE_ALGO 0xa404
#define MT9V139_AE_RULE_AE_WEIGHT_TABLE_0_0  0xa407
#define MT9V139_CAM_FRAME_SCAN_CONTROL          0xC858

static int sensor_handle;
static unsigned char wbuf[4]={0};

int get_139_para(int fd,unsigned short regaddr)
{
    unsigned char rbuf[4]={0};
	int ret = 0;    
    
	memcpy(rbuf,&regaddr,2);
    ret = read(fd,rbuf,2);
    if(ret < 0)
	{
		printf("read %x fail\n",regaddr);
		return -1;
	}
    printf("get %x data = 0x%04x \n",regaddr,(rbuf[0]<< 8 | rbuf[1]));      
}

int set_139_para(int fd,unsigned short regaddr,unsigned short wdata)
{
    unsigned char rbuf[4]={0},buf[4]={0};
	int ret = 0;    
    
	/*memcpy(rbuf,&regaddr,2);
    ret = read(fd,rbuf,2);
    if(ret < 0)
	{
		printf("read %x fail\n",regaddr);
		return -1; 
	}
    printf("pre %x data = 0x%04x \n",regaddr,(rbuf[0]<< 8 | rbuf[1]));  */
	
    memcpy(buf,&regaddr,2);
    memcpy(buf+2,&wdata,2);
	ret = write(fd, buf, 4);
	if(ret < 0)
	{
		printf("write fail,%d\n",__LINE__);
		return -1; 
	}  
    
	memcpy(rbuf,&regaddr,2);
    ret = read(fd,rbuf,2);
    if(ret < 0)
	{
		printf("read %x fail\n",regaddr);
		return -1; 
	}
    printf("set %x data to 0x%04x \n",regaddr,(rbuf[0]<< 8 | rbuf[1]));   
    return 0;   
}

/*
is_night:0 - day ， 1 - night
call_kzq：0 - 呼叫普通设备， 1 - 呼叫信息存储器下模拟分机
is_call：0 - 人脸识别，1 - 对讲
*/
int ae_rule_algo_init(char is_night,char call_kzq,char is_call)
{
	int fd = 0;
	int ret = 0;
	unsigned char rbuf[4]={0},buf[4]={0};
	unsigned short regaddr,wdata;
		
    if(sensor_handle <= 0){
        return -1;
    }

    fd = sensor_handle;

    regaddr = MT9V139_AE_RULE_ALGO;
	memcpy(wbuf,&regaddr,2);
	
	if(is_call){
		wbuf[2] = 0x00;
	}
	else{
		if(is_night){
			wbuf[2] = 0x02;
		}
		else{
			wbuf[2] = 0x03;
		}
	}
	wbuf[3] = 0x00;
	ret = write(fd, wbuf, 4);
	if(ret < 0)
	{
		printf("write fail,%d\n",__LINE__);
		return -1;
	}
	

	regaddr = MT9V139_AE_RULE_ALGO;
	memcpy(rbuf,&regaddr,2);
    ret = read(fd,rbuf,2);
    if(ret < 0)
	{
		printf("read %x %x fail\n",rbuf[0],rbuf[1]);
		return -1;
	}
	printf("write finished, read ae_rule_algo = 0x%04x !\n",(rbuf[0]<< 8 | rbuf[1]));

    get_139_para(fd,0xC95C);
    if(call_kzq == 1){	
        if(set_139_para(fd,0xC95C,0x0234)<0){//降低色差值
        	return -1;
    	}
	}
	else{
        if(set_139_para(fd,0xC95C,0x0534)<0){
        	return -1;
    	}        
	}

    if(set_139_para(fd,0xC92C,0x4c30)<0){ //降低饱和度
    	return -1;	
	}

        
    return 0;
	
}

int ae_rule_algo_auto_sw(void)
{
	int ret = 0;
	unsigned char rbuf[4]={0};
	unsigned short regaddr;
		
	if(sensor_handle < 0)
	{
		printf("illegal fd!!!\n");
		return -1;
	}

    regaddr = MT9V139_AE_RULE_ALGO;
	memcpy(wbuf,&regaddr,2);
    if(wbuf[2] == 0x03){
        wbuf[2] = 0x02;
    }
    else{
        wbuf[2] = 0x03;
    }
	wbuf[3] = 0x00;
	ret = write(sensor_handle, wbuf, 4);
	if(ret < 0)
	{
		printf("write fail\n");
		return -1;
	}

	memcpy(rbuf,&regaddr,2);
    ret = read(sensor_handle,rbuf,2);
    if(ret < 0)
	{
		printf("read %x %x fail\n",rbuf[0],rbuf[1]);
		return -1;
	}
	printf("write finished, read ae_rule_algo = 0x%04x !\n",(rbuf[0]<< 8 | rbuf[1]));

    return 0;
}

int SensorStart(char is_night)
{
	sensor_handle = open("/dev/sensor", O_RDWR, 0);
	if(sensor_handle < 0) {
		printf("open /dev/sensor fail\n");
		return -1;
	}

	if (0 > ioctl(sensor_handle, IOCTL_SENSOR_START)) {
		printf("IOCTL_SENSOR_START fail\r\n");
		close(sensor_handle);
		return  -1;
	}
    //ae_rule_algo_init(is_night,0);
	return 0;
}

int SensorStop()
{
	printf("SensorStop\n");
    if(sensor_handle<0){
        printf("illegal fd\r\n");
        return  -1;
    }
	if (0 > ioctl(sensor_handle, IOCTL_SENSOR_STOP)) {
		printf("IOCTL_SENSOR_STOP fail\r\n");
		close(sensor_handle);
		return  -1;
	}
	close(sensor_handle);
	sensor_handle=-1;
	return 0;
}

int SensorRead(char *buf)
{
	int size = 320*240;
    if(buf==NULL || sensor_handle<0){return -1;}	
	memset(buf,0,size);
	if (0 > ioctl(sensor_handle, IOCTL_SENSOR_READ, buf)) {
		printf("read fail\n");
		usleep(10000);//sensor can't read right after open
		return -1;
	}
	return 0;
}



