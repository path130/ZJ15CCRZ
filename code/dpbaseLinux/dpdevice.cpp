#include "roomlib.h"
#include "dpgpio.h"
#include <fcntl.h>
#include<linux/input.h>
#include <signal.h>
#include <sys/mman.h>

static int m_hWatchDog;
static DWORD refreshtime = 16000;

BOOL AdjustScreen(DWORD bright, DWORD contrast, DWORD saturation)
{
	BklParam curbkl;
	curbkl.bright = bright;
	curbkl.contrast = contrast;
	curbkl.saturation = saturation;
	printf("Set Screen %d %d %d\r\n",bright, contrast, saturation);
	int h_backlight = open("/dev/backlight", O_RDWR, 0);
	if(h_backlight < 0)
	{	
		printf("Open DIS1 fail error is %d\r\n",DPGetLastError());
		return FALSE;
	}

	ioctl(h_backlight,IOCTL_DISP_ADJUST_BKL,&curbkl);
	close(h_backlight);
	return TRUE;
}

BOOL AdjustCscparam(DWORD bright, DWORD contrast, DWORD saturation, DWORD hue)
{
	CscParam param;
	param.bright = bright;
	param.contrast = contrast;
	param.saturation = saturation;
	param.hue = hue;
	printf("Set Csc %d %d %d %d\r\n",bright, contrast, saturation, hue);
	int h_backlight = open("/dev/backlight", O_RDWR, 0);
	if(h_backlight < 0)
	{	
		printf("Open DIS1 fail error is %d\r\n",DPGetLastError());
		return FALSE;
	}

	ioctl(h_backlight,IOCTL_DISP_ADJUST_FE,&param);
	close(h_backlight);
	return TRUE;
}

void SetGpioVal(HANDLE hdev, UINT8 value)
{
	if(hdev == NULL)
		return;
	if(ioctl((int)hdev, IOCTRL_SET_GPIO_VALUE,&value))
	{
		printf("IOCTRL_SET_GPIO_VALUE  fail error \r\n");
		return;
	}
}

HANDLE InitGpio(DWORD pin, DWORD inout, DWORD value)
{
	int fd;
	GPIO_CFG gpio_cfg;

	fd = open("/dev/gpio", O_RDWR,0);
	if(fd < 0)
	{
		printf("open /dev/gpio  fail!\n");
		return INVALID_HANDLE_VALUE;
	}
	
	memset(&gpio_cfg,0,sizeof(GPIO_CFG));
	gpio_cfg.gpio_num= (WORD)pin;
	gpio_cfg.gpio_cfg = (BYTE)inout;
	gpio_cfg.gpio_pull = (BYTE)value;
	if(ioctl(fd,IOCTRL_GPIO_INIT,&gpio_cfg))
	{
		printf("IOCTRL_GPIO_INIT  fail error is \r\n");
		close(fd);
		return INVALID_HANDLE_VALUE;
	}
	return (HANDLE)fd;
}

void DeinitGpio(HANDLE hGpio)
{
	close((int)hGpio);
}

BOOL StartWatchDog(void)
{
	int ret;
	m_hWatchDog = open("/dev/watchdog", O_RDWR,0);
	if (m_hWatchDog < 0) {
		printf("open /dev/watchdog  fail!\n");
		return FALSE;
	}
	
	ret = ioctl(m_hWatchDog, IOCTL_START_WATCHDOG);
	if(ret)
	{
		printf("IOCTL_START_WATCHDOG fail\n");
		return FALSE;
	}
	
	ret = ioctl(m_hWatchDog, IOCTL_REFRESH_WATCHDOG, &refreshtime);
	if(ret)
	{
		printf("IOCTL_START_WATCHDOG fail\n");
		return FALSE;
	}
	
	return TRUE;
}

void StopWatchDog(void)
{
	ioctl(m_hWatchDog, IOCTL_STOP_WATCHDOG);
	close(m_hWatchDog);
}

void FeedWatchDog(void)
{
	ioctl(m_hWatchDog, IOCTL_REFRESH_WATCHDOG, &refreshtime);
}

void RebootSystem(void)
{
	int fd;
	int ret;
	unsigned int cmd;
	unsigned int timeout = 500;
	//500 	1000 	2000  3000  4000  5000  6000   8000  10000  12000  14000  16000
	//0.5s  1s  	2s    3s    4s    5s     6s     8s    10s    12s    14s    16s
	
	fd = open("/dev/watchdog", O_RDWR,0);
	if (fd < 0) {
		printf("open /dev/watchdog  fail!\n");
		//return NULL ;
	}
	
	sleep(1);
	cmd = IOCTL_START_WATCHDOG;
	ret = ioctl(fd,cmd);
	if(ret)
	{
		printf("IOCTL_START_WATCHDOG fail\n");
	}
	
	cmd = IOCTL_REFRESH_WATCHDOG;
	ret = ioctl(fd,cmd,&timeout);
	if(ret)
	{
		printf("IOCTL_START_WATCHDOG fail\n");
	}
	while(1);
}


HANDLE InitSpr(void)
{
	int h_hspr;
	DWORD bg = 0xC2BCBE;
	h_hspr = open("/dev/frame", O_RDWR, 0);
	if(h_hspr < 0)
	{
		printf("open /dev/frame fail\n");
		return NULL;
	}

	ioctl(h_hspr, IOCTL_SET_BKGRD, &bg);
	return (HANDLE)h_hspr;
}

void DeinitSpr(HANDLE h_hspr)
{
	close((int)h_hspr);
}


void DPSprCtrl(HANDLE h_hspr, DWORD ctlindex, void* data, DWORD len)
{
	ioctl((int)h_hspr, ctlindex, data);
}

void* DPVirtualAlloc(DWORD size)
{
	return malloc(size);
}

void DPVirtualFree(void* pMem)
{
	free(pMem);
}

BOOL DPPhisycalAlloc(HANDLE h_hspr, Block_Req* frame, DWORD memsize, DWORD prop)
{
	BOOL bret;

	if(prop == 0)
		bret = ioctl((int)h_hspr, IOCTL_BAR_REQUEST, frame);
	else if(prop == 1)
		bret = ioctl((int)h_hspr, IOCTL_FRM_REQUEST, frame);
	else 
		bret = ioctl((int)h_hspr, IOCTL_SPR_REQUEST, frame);
	if(bret < 0)
	{
		return FALSE;
	}
	memsize = (frame->win.cx * frame->win.cy * 4 + 0xfff) & 0xfffff000;
	frame->viraddr = (DWORD*)mmap(0, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, (int)h_hspr, 0); 
	if(frame->viraddr == MAP_FAILED)	
	{  
		printf("mmap fail\n");  
		return FALSE;
	}	
	memset(frame->viraddr, 0, memsize);
	return TRUE;
}

void DPPhisycalFree(HANDLE h_hspr, Block_Req* frame)
{
	int ret = 0;
	int memsize = (frame->win.cx * frame->win.cy * 4 + 0xfff) & 0xfffff000;

	munmap(frame->viraddr, memsize);  
	ret = ioctl((int)h_hspr, IOCTL_BLK_REMOVE, frame);
	if(ret < 0)
	{
		printf("IOCTL_BLK_REMOVE error\r\n");
	}
}

HANDLE OpenSafeDev(void)
{
	int hSafe = open("/dev/safe", O_RDWR, 0);

	if(hSafe < 0)
	{
		printf("%s(%d): Open device safe fail.\r\n", __FILE__, __LINE__);
		return NULL;
	}
	return (HANDLE)hSafe;
}

BOOL ReadSafeDev(HANDLE hSafe, BYTE *pData)
{
	read((int)hSafe, pData, 1);
	return TRUE;
}

void CloseSafeDev(HANDLE hSafe)
{
	close((int)hSafe);
}

#define BUF_SIZE (64*1024)
#define SPIUPDATE_MBRSIZE (16*1024)
#define SPIUPDATE_MBROFFSET ((256+64-16)*1024)//(240*1024)

void StartUpdate(char* filename, DWORD id)
{
	int fd;
	int ret;
	unsigned int cmd;
	FILE *fp = NULL;
	unsigned int filelen,retlen,updateprecent;
	unsigned char *buf = NULL;

	fd = open("/dev/spiupdate", O_RDWR,0);
	if (fd < 0)
	{
		printf("open /dev/spiupdate  fail!\n");
		DPPostMessage(MSG_PRIVATE, id, 0, 9);
	}

	buf = (unsigned char*)mmap(0, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,fd, 0); 
	if(buf == MAP_FAILED)  
	{
		printf("mmap fail\n");  
		DPPostMessage(MSG_PRIVATE, id, 0, 9);
		goto fail;
	}

	fp = fopen (filename,"r");
	if(fp == NULL)
	{
		printf("open image.dat fail\r\n");
		DPPostMessage(MSG_PRIVATE, id, 0, 9);
		goto fail;
	}

	fseek(fp,0,SEEK_END);
	filelen = ftell(fp);
	printf("filelen %d\n",filelen);
	//check image
	fseek(fp,SPIUPDATE_MBROFFSET,SEEK_SET);
	memset(buf,0,BUF_SIZE);
	retlen = fread(buf,1,SPIUPDATE_MBRSIZE,fp);
	if(retlen != SPIUPDATE_MBRSIZE)
	{
		printf("read MBR fail\n");
		DPPostMessage(MSG_PRIVATE, id, 0, 9);
		goto fail;
	}

	cmd = IOCTL_CHECK_UPDATAFILE;
	ret = ioctl(fd,cmd,&retlen);
	if(ret)
	{
		printf("IOCTL_CHECK_UPDATAFILE fail\n");
		DPPostMessage(MSG_PRIVATE, id, 0, 9);
		goto fail;
	}

	fseek(fp,0,SEEK_SET);

	while(1)
	{
		memset(buf,0,BUF_SIZE);
		retlen = fread(buf,1,BUF_SIZE,fp);
		if(retlen != BUF_SIZE)
			printf("read only %d\n",retlen);

		if(retlen == 0)
			break;
		cmd = IOCTL_UPDATA_IMAGE;
		ret = ioctl(fd,cmd,&retlen);
		if(ret)
		{
			printf("update fail\n");
		}

		cmd = IOCTL_GET_UPDATEPRERCET;
		ret = ioctl(fd,cmd,&updateprecent);
		if(!ret)
			DPPostMessage(MSG_PRIVATE, id, 1, updateprecent);
		if(updateprecent == 100)
		{
			DPPostMessage(MSG_PRIVATE, id, 0, UPDATE_FILE_OK);
			break;
		}
	}
	if(updateprecent == 100)
		printf("update end\n");
	else
		DPPostMessage(MSG_PRIVATE, id, 0, UPDATE_FILE_ERR);
fail:
	if(buf) 
		munmap(buf, BUF_SIZE);
	if(fp)	
		fclose(fp);
	close(fd);
}
