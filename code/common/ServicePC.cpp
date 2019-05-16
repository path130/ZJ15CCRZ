#include "roomlib.h"
#include "ipfunc.h"

#define	PCMSG_VERIFY			0x10000000
#define	PCMSG_SET_ROOM			0x10000001
#define	PCMSG_SET_TIME			0x10000002
#define	PCMSG_UPDATE_NETCFG		0x10000003
#define	PCMSG_UPDATE_IMAGEDD	0x10000004
#define	PCMSG_REBOOT			0x10000005
#define	PCMSG_RESET				0x10000006
#define	PCMSG_OPENFTP			0x10000007

#define	PCMSG_UPDATE_APP0		0x60000000

#define PCMSG_PORT				0x8866

typedef struct
{
	WORD devtype[32];
	DWORD type;
	DWORD length;
}PCMSG_DATA;

static BOOL g_bPCStart;
static HANDLE g_hPCThread;
static StaticLock g_CS;

BOOL DownLoadFile(SOCKET sockfd, char* filename, int nFileSize)
{
	FILE* pFile = fopen(filename, "wb");
	if(NULL == pFile)
		return FALSE;

	int nDownLoadSize = 0x10000;
	char* pdata = (char*)malloc(nDownLoadSize);
	int nRecvSize = 0;
	int nCompleteSize = 0;
	while(nCompleteSize < nFileSize)
	{
		nRecvSize = nFileSize - nCompleteSize;
		if(nRecvSize > nDownLoadSize)
			nRecvSize = nDownLoadSize;

		if(TcpRecvData(sockfd, pdata, nRecvSize, 2000) != nRecvSize)
			break;
	
		DBGMSG(DPINFO, "DownLoadFile %s %d %d\r\n", filename, nFileSize, nCompleteSize);

		nCompleteSize += nRecvSize;
		fwrite(pdata, 1, nRecvSize, pFile);
	}

	free(pdata);
	fclose(pFile);
	return (nCompleteSize == nFileSize);
}

static void HandleUpdateImage(SOCKET sockfd, PCMSG_DATA* pMsg)
{
	int ret = FALSE;
	do
	{
		SetObjectMemorySpace_GWF(IMAGE_OBJECT_SIZE + NORMAL_OBJECT_SIZE);

		if(CheckSpareSpace(WINDOWSDIR) < pMsg->length)
			break;

		DPPostMessage(MSG_SYSTEM, WATCHDOG_CHANGE, FALSE, 0);

		ret = DownLoadFile(sockfd, DOWNLOAD_IMAGE_PATH, pMsg->length);
	}while(0);

	TcpSendData(sockfd, (char*)&ret, 4, 1000);
	if(!ret)
	{
		DPDeleteFile(DOWNLOAD_IMAGE_PATH);
		SetObjectMemorySpace_GWF(NORMAL_OBJECT_SIZE);
		DPPostMessage(MSG_SYSTEM, WATCHDOG_CHANGE, TRUE, 0);
	}
	else
	{
		DPPostMessage(MSG_EXTSTART_APP, UPGRATE_APPID, 0, 0);
	}
}

static void HandleUpdateNetCfg(SOCKET sockfd, PCMSG_DATA* pMsg)
{
	int ret = FALSE;
	do
	{
		DWORD dwSize = CheckSpareSpace(USERDIR);
		if(dwSize < pMsg->length)
			break;

		char* pdata = (char*)malloc(pMsg->length + 1);
		if(pMsg->length != TcpRecvData(sockfd, pdata, pMsg->length, 5000))
		{
			DBGMSG(DPINFO, "UpdateNetCfg Recv fail:%d\r\n", DPGetLastError());
			free(pdata);
			break;
		}

		char fileName1[64];
		char fileName2[64];
		sprintf(fileName1, "%s/%s", USERDIR, IPTABLE_NAME);
		sprintf(fileName2, "%s/%s", USERDIR, IPTABLE_BAK);

		FILE* pFile = fopen(fileName2, "wb");
		if(pFile)
		{
			dwSize = fwrite(pdata, 1, pMsg->length, pFile);
			fclose(pFile);
		}

		free(pdata);
		if(dwSize != pMsg->length)
			break;

		DBGMSG(DPINFO, "UpdateNetCfg Start\r\n");
		DPDeleteFile(fileName1);
		DPMoveFile(fileName1, fileName2);
		DPPostMessage(MSG_SYSTEM, UPDATE_NETCFG, 0, 0);
		ret = TRUE;
	}while(0);

	TcpSendData(sockfd, (char*)&ret, 4, 1000);
}

static void HandleSetTime(SOCKET sockfd, PCMSG_DATA* pMsg)
{
	int ret = FALSE;
	do
	{
		if(pMsg->length != sizeof(SYSTEMTIME))
			break;

		SYSTEMTIME settime;
		if(TcpRecvData(sockfd, (char*)&settime, pMsg->length, 1000) != pMsg->length)
			break;

		DBGMSG(DPINFO, "PC SetTime %04d-%02d-%02d %02d:%02d:%02d\r\n", settime.wYear, settime.wMonth, settime.wDay, settime.wHour, settime.wMinute, settime.wSecond);
		ret = DPSetLocalTime(&settime);
	}while(0);

	TcpSendData(sockfd, (char*)&ret, 4, 1000);
}

static void HandleSetRoomCode(SOCKET sockfd, PCMSG_DATA* pMsg)
{
	int ret = FALSE;
	do
	{
		if(pMsg->length != 28)
			break;

		char pdata[64] = {0};
		if(TcpRecvData(sockfd, pdata, pMsg->length, 1000) != pMsg->length)
			break;

		char roomid[16];
		unicode2utf8((BYTE*)roomid, (wchar_t*)pdata);

		DBGMSG(DPINFO, "PC Set Room Code %s\r\n", roomid);
		ip_get pget;
		if(!TermGet(&pget, roomid))
			break;

		SetTermId(roomid, pget.param->ip);
		free(pget.param);
		DPPostMessage(MSG_SYSTEM, CODE_CHANGE, 0, 0);
		ret = TRUE;
	}while(0);

	TcpSendData(sockfd, (char*)&ret, 4, 1000);
}

static void HandleUpdateApp0(SOCKET sockfd, PCMSG_DATA* pMsg)
{
	int ret = FALSE;
	do
	{
		char* pdata = (char*)malloc(pMsg->length + 1);
		if(pMsg->length != TcpRecvData(sockfd, pdata, pMsg->length, 50000))
		{
			DBGMSG(DPINFO, "HandleUpdateApp0 Recv fail:%d\r\n", DPGetLastError());
			free(pdata);
			break;
		}

		DWORD dwSize = 0;
		FILE* pFile = fopen("/Windows/app0", "wb");
		if(pFile)
		{
			dwSize = fwrite(pdata, 1, pMsg->length, pFile);
			fclose(pFile);
			DBGMSG(DPINFO, "HandleUpdateApp0 Success\r\n");
		}

		free(pdata);
		if(dwSize != pMsg->length)
			break;

#ifdef DPLINUX
		chmod("/Windows/app0", 777);
#endif

		DPPostMessage(MSG_SYSTEM, WATCHDOG_CHANGE, FALSE, 0);
		ret = TRUE;
	}while(0);

	TcpSendData(sockfd, (char*)&ret, 4, 1000);
}

static DWORD PCThread(HANDLE pParam)
{
	DBGMSG(DPINFO, "PCThread start\r\n");

	SOCKET listenSock = TcpListen(NULL, PCMSG_PORT);
	if(INVALID_SOCKET == listenSock)
	{
		DBGMSG(DPERROR, "PCThread Socket TcpListen fail:%d\r\n", DPGetLastError());
		return 0;
	}

	PCMSG_DATA msg;
	
	while(g_bPCStart)
	{
		SOCKET sockfd = TcpAccpet(listenSock, 1000, NULL);
		if(INVALID_SOCKET == sockfd)
			continue;

		SocketUnblock(sockfd);
		int ret = TcpRecvData(sockfd, (char*)&msg, sizeof(PCMSG_DATA), 1000);
		if (ret != sizeof(PCMSG_DATA))
		{
			DBGMSG(DPWARNING, "PCThread recv length fail expect:%d ret:%d\r\n", sizeof(PCMSG_DATA), ret);
			SocketClose(sockfd);
			continue;
		}

		DBGMSG(DPINFO, "Recv PC Msg %x %x\r\n", msg.type, msg.length);
		switch(msg.type)
		{
#ifdef _DEBUG
			case 0xA1008:
				DPPostMessage(MSG_SHOW_STATUE, msg.length, 0, 0);
				break;
#endif
			case PCMSG_VERIFY:
				ret = 1;
				TcpSendData(sockfd, (char*)&ret, 4, 1000);
				break;
			case PCMSG_SET_ROOM:
				HandleSetRoomCode(sockfd, &msg);
				break;
			case PCMSG_SET_TIME:
				HandleSetTime(sockfd, &msg);
				break;
			case PCMSG_UPDATE_NETCFG:
				HandleUpdateNetCfg(sockfd, &msg);
				break;
			case PCMSG_UPDATE_IMAGEDD:
				HandleUpdateImage(sockfd, &msg);
				break;
			case PCMSG_REBOOT:
				ret = 1;
				TcpSendData(sockfd, (char*)&ret, 4, 1000);
				DPPostMessage(MSG_SYSTEM, REBOOT_MACH, 0, 0);
				break;
			case PCMSG_RESET:
				ret = 1;
				TcpSendData(sockfd, (char*)&ret, 4, 1000);
				DPPostMessage(MSG_SYSTEM, RESET_MACH, 0, 0);
				break;
			case PCMSG_OPENFTP:
				/************************************************************************/
				/*                                                                      */
				/************************************************************************/
				ret = 1;
				TcpSendData(sockfd, (char*)&ret, 4, 1000);
				break;
			case PCMSG_UPDATE_APP0:
				/************************************************************************/
				/*    µ˜ ‘”√                                                            */
				/************************************************************************/
				HandleUpdateApp0(sockfd, &msg);
				break;

		}

		SocketClose(sockfd);
	}

	return 0;
}

void StartPCServer(void)
{
	g_CS.lockon();
	DBGMSG(DPINFO, "StartPCServer start\r\n");

	if(!g_bPCStart)
	{
		g_bPCStart = TRUE;
		g_hPCThread = DPThreadCreate(0x4000, PCThread, NULL, TRUE, 5);
	}
	g_CS.lockoff();
}

void StopPCServer(void)
{
	g_CS.lockon();
	DBGMSG(DPINFO, "StopPCServer start\r\n");

	if(g_bPCStart)
		g_bPCStart = FALSE;

	if(g_hPCThread)
	{
		DPThreadJoin(g_hPCThread);
		g_hPCThread = NULL;
	}
	g_CS.lockoff();
}