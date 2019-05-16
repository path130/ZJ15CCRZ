#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <linux/input.h>
#include <errno.h> 
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "rtc.h"

#define IOCTL_READ_RTC		0x1234567b
#define IOCTL_SET_RTC		0x1234567c

int set_rtc_time(struct tm __tm)
{
    int ret;
    struct timeval tv;
    time_t timepl;
    struct tm tnow; 
    int fd = open("/dev/rtc", O_RDWR,0);
	if (fd < 0) {
		printf("open /dev/rtc  fail!\n");
		return -1 ;
	}    
    printf("SET Time to (%d/%d/%d,%d:%d:%d-%d)\r\n",__tm.tm_year+1900, __tm.tm_mon+1,__tm.tm_mday,\
		__tm.tm_hour, __tm.tm_min, __tm.tm_sec, __tm.tm_wday);
	ret = ioctl(fd,IOCTL_SET_RTC,&__tm);
	if(ret != 0)
	{
        printf("set time fail\n");
        close(fd);
        return -1;
	}
    ret = ioctl(fd,IOCTL_READ_RTC,&tnow);
    if(ret != 0)
    {
            printf("read time fail\n");
            close(fd);
            return -1;
    }
    printf("OEMGetRealTime (%d/%d/%d,%d:%d:%d-%d)\r\n",tnow.tm_year+1900, tnow.tm_mon+1,tnow.tm_mday,\
		tnow.tm_hour, tnow.tm_min, tnow.tm_sec, tnow.tm_wday);
#if 0	
	timepl = mktime(&__tm);
	tv.tv_sec = timepl;
	tv.tv_usec = 0;
	//设置系统时间和rtc时间一致
	settimeofday(&tv,(struct timezone*)0);
#endif
    close(fd);
    return 0;
}

int syn_with_rtc_time(void)    
{	
	//系统启动之后第一次获取时间通过RTC获取系统时间，并设置系统时间和rtc时间一致
 	struct tm __tm;  
    int ret;
    struct timeval tv;
    time_t timepl;	
    int fd = open("/dev/rtc", O_RDWR,0);
	if (fd < 0) {
		printf("open /dev/rtc  fail!\n");
		return -1 ;
	}        
    ret = ioctl(fd,IOCTL_READ_RTC,&__tm);
    if(ret != 0)
    {
            printf("read time fail\n");
            close(fd);
            return -1;
    }
    printf("OEMGetRealTime (%d/%d/%d,%d:%d:%d-%d)\r\n",__tm.tm_year+1900, __tm.tm_mon+1,__tm.tm_mday,\
		__tm.tm_hour, __tm.tm_min, __tm.tm_sec, __tm.tm_wday);
    timepl = mktime(&__tm);
#if 0
	tv.tv_sec = timepl;
	tv.tv_usec = 0;
	//设置系统时间和rtc时间一致
	settimeofday(&tv,(struct timezone*)0);
#else
    if(stime(&timepl) != 0) {
       close(fd);
       perror("stime error:");
       return -1;
    }
    else{
        printf("%s: set time ok\n",__func__);
    }
#endif
    close(fd);
    return 0;
}    
	




