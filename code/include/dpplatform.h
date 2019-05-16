/************************************************************************
此头文件为
公共接口函数
无关 WinCE、Linux 平台
************************************************************************/

#pragma once

//#ifdef DPLINUX
#include "dpdef_linux.h"
//#endif

//#ifdef DPCE
//#include "dpdef_ce.h"
//#endif

// dpbase
VOID DPInitCriticalSection(CRITICAL_SECTION* pcsCriticalSection);
VOID DPDeleteCriticalSection(CRITICAL_SECTION* pcsCriticalSection);
VOID DPEnterCriticalSection(CRITICAL_SECTION* pcsCriticalSection);
VOID DPLeaveCriticalSection(CRITICAL_SECTION* pcsCriticalSection);

HANDLE DPCreateSemaphore(DWORD dwInitCount, DWORD dwMaxCount);
void DPSetSemaphore(HANDLE hSem);
BOOL DPGetSemaphore(HANDLE hSem, DWORD timeout);

BOOL DPCreateMsgQueue(const char* name, DWORD vol, DWORD itemsize, HANDLE* pRead, HANDLE* pWrite);
BOOL DPWriteMsgQueue(HANDLE hMsg, void* data, DWORD len, DWORD timeout);
BOOL DPReadMsgQueue(HANDLE hMsg, void* data, DWORD len, DWORD timeout);
void DPCloseMsgQueue(HANDLE hMsg);
HANDLE DPThreadCreate(int stacksize, DWORD (*func) (void *), void *arg, BOOL joinable, DWORD level);
BOOL DPThreadJoin(HANDLE hThread, DWORD* dPret);

DWORD DPGetLastError();

// dptime
DWORD DPGetTickCount(void);
void DPSleep(DWORD dwMilliseconds);
void DPGetLocalTime(SYSTEMTIME* lpSystemTime);
BOOL DPSetLocalTime(SYSTEMTIME *lpSystemTime);
BOOL DPSystemTimeToFileTime(SYSTEMTIME* lpSystemTime, FILETIME* lpFileTime); 
BOOL DPFileTimeToSystemTime(FILETIME* lpFileTime, SYSTEMTIME* lpSystemTime); 

// dplib
void DPCreateTouchEvent();
void DPCreateTimeEvent();

// dpdevice
BOOL AdjustScreen(DWORD bright, DWORD contrast, DWORD saturation);
BOOL AdjustCscparam(DWORD bright, DWORD contrast, DWORD saturation, DWORD hue);

HANDLE InitGpio(DWORD pin, DWORD inout, DWORD value);
UINT8 GetGpioVal(HANDLE hDev);
void SetGpioVal(HANDLE hDev, UINT8 value);
void DeinitGpio(HANDLE hDev);

BOOL StartWatchDog(void);
void StopWatchDog(void);
void FeedWatchDog(void);
void RebootSystem(void);

HANDLE OpenSafeDev(void);
BOOL ReadSafeDev(HANDLE hSafe, BYTE* data);
void CloseSafeDev(HANDLE hSafe);

#define UPDATE_FILE_ERR			0
#define UPDATE_FILE_OK			1
#define UPDATE_FILE_NOTEXSIT	2
#define UPDATE_FILE_TOO_BIG		3
#define UPDATE_ALLOC_MEM_FAIL	4
#define UPDATE_READ_FILE_ERROR	5
#define UPDATE_ERASE_SEC_FAIL	6
#define UPDATE_FILE_NOT_NEW		7
#define UPDATE_FILE_WRONG		8
#define	UPDATE_NO_DEVICE		9
void StartUpdate(char* filename, DWORD id);

typedef struct _Block_Req
{
	DWORD*	viraddr;			// 内存起始地址
	POINT	lt;					// 屏幕上的左上点，对于
	SIZE	win;				// block的大小
	HANDLE	rethd;			
	DWORD	id;
	struct	_Block_Req* pnext;
	BOOL	isHard;				// 硬件相关frame,说明本块在显示内存中
	BOOL 	issystem;			// 系统相关的frame,主要是用于缓存png图片,设置该属性后，不会被CloseBlk释放
} Block_Req;
HANDLE InitSpr(void);
void DeinitSpr(HANDLE);
void DPSprCtrl(HANDLE h_hspr, DWORD ioctl, void* data, DWORD len);
void DPPhisycalFree(HANDLE h_hspr, Block_Req* frame);
BOOL DPPhisycalAlloc(HANDLE h_hspr, Block_Req* frame, DWORD memsize, DWORD prop);
void* DPVirtualAlloc(DWORD size);
void DPVirtualFree(void* pMem);

// dpfile
void DPDeleteFile(const char* pfile);
void DPMoveFile(const char* dstfile, const char* srcfile);
void DPCopyFile(const char* dstfile, const char* srcfile);
BOOL DPCopyFileEx(const char* dstfile, const char* srcfile);
HANDLE DPFindFirstFile(char* dir, char* pfile);
BOOL DPFindNextFile(HANDLE hFind, char* pfile);
void DPFindClose(HANDLE hFind);
BOOL CheckAndCreateDir(const char *cDir);
UINT64 CheckSpareSpace(const char* dir);
int DPGetFileAttributes(const char* filename);
void DPUnloadFile(const char* pfile);

void StopLogo(void);
void ScanDisk(void);
void DumpPhysicalMemory();
DWORD DumpMemory(DWORD* total, DWORD* avail);
DWORD SetObjectMemorySpace_GWF(int nSize,DWORD dwNewPage);						

// dpnetwork
#define	WIRE_PLUGIN		1
#define	ADDRESS_SET		2

BOOL SetIPAddress(int ip, int mask, int gw);
BOOL GetNetworkcardInfo(unsigned char * pMac);
BOOL StartNetMonitor(void);
DWORD GetIpAddress(void);
int GetNetState();
void SetNetState(BOOL bConflict);
void StopNetMonitor(void);
BOOL SetNetDhcp(void);

// dparp.cpp
//bool CheckIPConflict(int checkip);
