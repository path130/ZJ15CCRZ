#include "roomlib.h"
#include "ipfunc.h"
#include "dpxmlmessage.h"
#include "DPXmlParse.h"

#define	SYNC_UPDATE_APP			1
#define	SYNC_UPDATE_NETCFG		2
#define SYNC_UPDATE_MSG			4
#define SYNC_UPDATE_WEATHER     8

typedef struct
{
	char	DriverInfo[32];
	UINT64	DriverVer;
	char	AppInfo[32];
	UINT64	AppVer;
} SYSVER_T;

#define	SYNC_MSGQ				"NEWSYNCMSG"
#define	XML_SYNC_PORT			  0x8881

static BOOL g_bSyncXmlStart = FALSE;
static HANDLE g_hSyncXmlSendQ = NULL;
static HANDLE g_hSyncXmlThread = NULL;
static HANDLE m_hUpgradeSemp = NULL;
static HardInfo_T g_HardInfo;
static SYSVER_T g_SoftInfo;
static StaticLock g_NewSyncMsgCS;

char* GetAppInfo()
{
	return g_SoftInfo.AppInfo;
}

char* GetSoftVer()
{
	return g_SoftInfo.DriverInfo;
}

void SetSoftVer(char* str)
{
	strcpy(g_SoftInfo.DriverInfo, str);
}

void GetHardInfo(char *hardInfo)
{
	strcpy(hardInfo, g_HardInfo.HardInfo);
}

void GetCompanyInfo(int language, char* companyInfo)
{
	if(language == LANGUAGE_CH)
		strcpy(companyInfo, g_HardInfo.CompInfo);
	else
		strcpy(companyInfo, &g_HardInfo.CompInfoEn[17]);
}

static int GetManagerIP()
{
	ip_get pget;
	if(!ManagerGet(&pget))
	{
		DBGMSG(DPINFO, "ManagerGet fail!\r\n");
		return 0;
	}

	int ip = pget.param[0].ip;
	free(pget.param);
	return ip;
}

static void UnicodeConvertUTF8(char* unicode, int len)
{
	char *pbuf = (char*)malloc(len*3);
	unicode2utf8((BYTE*)pbuf, (wchar_t*)unicode);
	strcpy(unicode, pbuf);
	free(pbuf);
}

void BuildOpenLockCmd(MSGHEAD* pCmd, DWORD type)
{
	char localcode[16] = {0};
	GetTermId(localcode);

	char content[512];
	sprintf(content, "<msg><type>%d</type><unlock>%s</unlock></msg>", type, localcode);

	utf82unicode((WORD*)pCmd->content, (BYTE*)content);
	pCmd->head = XMLMSG_CHECKID;
	pCmd->length = sizeof(MSGHEAD) + (strlen(content) + 1) * 2;
}

void BuildSafeStatusCmd(MSGHEAD* pCmd, BOOL bSafe)
{
	char localcode[16] = {0};
	GetTermId(localcode);

	char content[512];
	printf(content, "<msg><type>1102</type><id>%s</id><security>%d</security></msg>", localcode, bSafe);

	utf82unicode((WORD*)pCmd->content, (BYTE*)content);
	pCmd->head = XMLMSG_CHECKID;
	pCmd->length = sizeof(MSGHEAD) + (strlen(content) + 1) * 2;
}

void BuildDisalarmCmd(MSGHEAD* pCmd, DWORD disalarmtype)
{
	char localcode[16] = {0};
	GetTermId(localcode);

	SYSTEMTIME systime;
	DPGetLocalTime(&systime);

	char content[512];
	sprintf(content, 
	"<msg><type>1007</type><device><devtype>%s</devtype><id>%s</id></device><clearalarm><type>%d</type><time>%04d-%02d-%02d %02d:%02d:%02d</time></clearalarm></msg>",
	GetAppInfo(), localcode, disalarmtype, systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	utf82unicode((WORD*)pCmd->content, (BYTE*)content);
	pCmd->head = XMLMSG_CHECKID;
	pCmd->length = sizeof(MSGHEAD) + (strlen(content) + 1) * 2;
}

void BuildAlarmCmd(MSGHEAD* pCmd, DWORD alarmType, DWORD alarmMsg)
{
	char localcode[16] = {0};
	GetTermId(localcode);

	SYSTEMTIME systime;
	DPGetLocalTime(&systime);

	int pos, delay;
	delay = alarmMsg/100;
	pos = alarmMsg - delay*100;
	pos +=10;

	char content[512];
	sprintf(content, 
		"<msg><type>1005</type><device><devtype>%s</devtype><id>%s</id></device><alarm><time>%04d-%02d-%02d %02d:%02d:%02d</time><type>%d</type><pos>%d</pos><delay>%d</delay></alarm></msg>",
		GetAppInfo(), localcode, systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, 
		alarmType, pos, delay);

	utf82unicode((WORD*)pCmd->content, (BYTE*)content);
	pCmd->head = XMLMSG_CHECKID;
	pCmd->length = sizeof(MSGHEAD) + (strlen(content) + 1) * 2;
}

static char* UpdateRequest(char* uptype)
{
	char pbuf[512];
	char strUptype[64] = "";
	char strNumber[32];
	MSGHEAD * phead = (MSGHEAD*)pbuf;
	CTcpClientSock tcpClient;
	if(strcmp(uptype, "appver") == 0)
	{
		strcpy(strUptype, GetSoftVer());
	}
	else if(strcmp(uptype, "infotime") == 0)
	{
		strcpy(strUptype, GetMsgTime());
	}
	else if(strcmp(uptype, "wea") == 0)
	{
		strcpy(strUptype, GetWeatherTime());
	}
	else if(strcmp(uptype, "netcfg") == 0)
	{
		char filename[128];
		sprintf(filename, "%s/%s", USERDIR, IPTABLE_NAME);
		if(-1 != DPGetFileAttributes(filename))
			CalFileMd5(strUptype, filename);
		else
		{
			sprintf(filename, "%s/%s", FLASHDIR, IPTABLE_NAME);
			if(-1 != DPGetFileAttributes(filename))
				CalFileMd5(strUptype, filename);
		}
	}

	tcpClient.SetTimeout(500);
	int ip = GetManagerIP();
	if(ip == 0)
		return NULL;

	if(!tcpClient.Connect(ip, XML_SYNC_PORT))
	{
		DBGMSG(DPINFO, "UpdataRequest Connect manager %x fail\r\n", ip);
		return NULL;
	}
	tcpClient.SetTimeout(1000);

	GetTermId(strNumber);
	char content[512];
	sprintf(content, 
		"<msg><type>%d</type><devtype>%s</devtype><id>%s</id><%s>%s</%s></msg>",
			MC_TYPE_UPDATE, GetAppInfo(), strNumber, uptype, strUptype, uptype);

	DPCHAR *pContent = (DPCHAR*)phead->content;
	utf82unicode((WORD*)phead->content, (BYTE*)content);
	phead->head = XMLMSG_CHECKID;
	phead->length = sizeof(MSGHEAD) + (strlen(content) + 1) * 2;

	if(tcpClient.Send((char*)phead, phead->length))
	{
		if(tcpClient.Recv((char*)phead, sizeof(MSGHEAD))
			&& (XMLMSG_CHECKID == phead->head)
			&& (phead->length > sizeof(MSGHEAD)))
		{
			phead->length -= sizeof(MSGHEAD);
			char* pbuf = (char*)malloc(phead->length * 3);
			if(tcpClient.Recv((char*)pbuf, phead->length))
			{
				pContent = (DPCHAR*)pbuf;
				*(DPCHAR*)(pbuf + phead->length) = 0;
				UnicodeConvertUTF8(pbuf, phead->length);
				return pbuf;
			}
		}
	}
	return NULL;
}

static void UpdateApp(char* pbuf)
{
	DBGMSG(DPINFO, "UpdateApp start\r\n");
	CXmlParse parse;
	char* pcontent;

	parse.Init(pbuf);
	if(!parse.RootCheck("msg"))
		goto endprocess;

	pcontent = parse.GetNodeContent("type");
	if(pcontent == NULL)
		goto endprocess;
	if(strtol(pcontent, NULL, 10) != MC_TYPE_UPDATE_ACK)
		goto endprocess;

	if(!parse.SetLocate("ver"))
		goto endprocess;

	pcontent = parse.GetNodeContent("devtype");
	if(pcontent == NULL)
		goto endprocess;
	if(strcmp(pcontent, GetAppInfo()) != 0)
		goto endprocess;

	pcontent = parse.GetNodeContent("appver");
	if(pcontent == NULL)
		goto endprocess;
	if(strcmp(pcontent, GetSoftVer()) <= 0)
		goto endprocess;

	pcontent = parse.GetNodeContent("url");
	if(pcontent == NULL)
		goto endprocess;

	if(!HttpDownloadFile(DOWNLOAD_IMAGE_PATH, pcontent))
	{
		goto endprocess;
	}
	//DP2000的升级方式为image.dd升级。管理中心发过来的dd文件必须去掉pc工具打包时的消息头才是程序真正的dd文件。
	//两者的md5值肯定不同，所以不能进行两者的md5校验。
#ifdef NANDFLASH
	CalFileMd5(wszNumber, DOWNLOAD_IMAGE_PATH);
	pcontent = parse.GetNodeContent("md5");
	if(pcontent == NULL)
	{
		wprintf("GetNodeContent  pcontent is NULL \r\n");
		::DeleteFile(DOWNLOAD_IMAGE_PATH);
		goto endprocess;
	}
	if(wcscmp(pcontent, wszNumber) != 0)
	{
		wprintf("compare not same pcontent is %s, wcsNumber is %s\r\n",pcontent, wszNumber);
		::DeleteFile(DOWNLOAD_IMAGE_PATH);
		goto endprocess;
	}
	if(UnpackFile(DOWNLOAD_IMAGE_PATH))
	{
		wprintf("----UnpackFileOk--------\r\n");
		MoveFileW("\\FlashDev\\root",L"\\FlashDev\\root_del");
		MoveFileW("\\FlashDev\\root_update",L"\\FlashDev\\root");
	}
	else
		wprintf("-----UnpackFileFail ---------\r\n");
	::DeleteFile(DOWNLOAD_IMAGE_PATH);
	DPPostMessage(MSG_SYSTEM, REBOOT_MACH, 0, 0);
#else
	DPPostMessage(MSG_SYSTEM, WATCHDOG_CHANGE, FALSE, 0);
	SetSoftVer(parse.GetNodeContent("appver"));
	DPPostMessage(MSG_EXTSTART_APP, UPGRATE_APPID, 0, 0);

#endif
endprocess:
	DBGMSG(DPINFO, "UpdateApp endprocess-------\r\n");
	return;
}

static void UpdateNetCfg(char* pbuf)
{
	DBGMSG(DPINFO, "UpdateNetCfg start\r\n");

	CXmlParse parse;
	char* pcontent;
	char wszNumber[33];
	char wszPath1[64];
	char wszPath2[64];
	sprintf(wszPath1, "%s/%s",USERDIR, IPTABLE_BAK);
	sprintf(wszPath2, "%s/%s", USERDIR,IPTABLE_NAME);
	parse.Init(pbuf);
	if(!parse.RootCheck("msg"))
		goto endprocess;

	pcontent = parse.GetNodeContent("type");
	if(pcontent == NULL)
		goto endprocess;
	if(strtol(pcontent, NULL, 10) != MC_TYPE_UPDATE_ACK)
		goto endprocess;

	if(!parse.SetLocate("netcfg"))
		goto endprocess;

	pcontent = parse.GetNodeContent("netcfgurl");
	if(pcontent == NULL)
		goto endprocess;
	if(!HttpDownloadFile(wszPath1, pcontent))
		goto endprocess;

	CalFileMd5(wszNumber, wszPath1);
	pcontent = parse.GetNodeContent("md5");
	if(pcontent == NULL)
	{
		DPDeleteFile(wszPath1);
		goto endprocess;
	}

	if(strcmp(pcontent, wszNumber) != 0)
	{
		DBGMSG(DPINFO, "download netcfg error, delete it\r\n");
		DPDeleteFile(wszPath1);
		goto endprocess;
	}
	DPDeleteFile(wszPath2);
	DPMoveFile(wszPath2, wszPath1);
	DPPostMessage(MSG_SYSTEM, UPDATE_NETCFG, 0, 0);
endprocess:
	return;
}

static void UpdateAreaMsg(char* pbuf)
{
	DBGMSG(DPINFO, "UpdateAreaMsg start\r\n");
	CXmlParse parse;
	char* pcontent;

	parse.Init(pbuf);
	if(!parse.RootCheck("msg"))
		goto endprocess;

	pcontent = parse.GetNodeContent("type");
	if(pcontent == NULL)
		goto endprocess;
	if(strtol(pcontent, NULL, 10) != MC_TYPE_UPDATE_ACK)
		goto endprocess;

	if(!parse.SetLocate("infos"))
	{
		if(!parse.SetLocate("info"))
			goto endprocess;
		else
		{
			AddMessage(parse.GetNodeContent("time"),
				parse.GetNodeContent("title"),
				parse.GetNodeContent("body"),
				parse.GetNodeContent("jpg"),
				TRUE);
			//DBGMSG(DPINFO, "AreaMsg recv time is %s\r\n, GetMsgTime is %s\r\n", parse.GetNodeContent("time"), GetMsgTime());
		}
	}
	else
	{
		if(parse.SetLocate("info"))
		{
			UpdateMessageEn(FALSE);
			while(strcmp(parse.GetLocateName(), "info") == 0)
			{	
				AddMessage(parse.GetNodeContent("time"),
					parse.GetNodeContent("title"),
					parse.GetNodeContent("body"),
					parse.GetNodeContent("jpg"),
					TRUE);	
				//DBGMSG(DPINFO, "AreaMsg recv time is %s \r\n, GetMsgTime is %s\r\n", parse.GetNodeContent("time"), GetMsgTime());
				if(!parse.NextNode())
					break;
			}
			UpdateMessageEn(TRUE);
		}	
	}
endprocess:
	return;
}

static void UpdateWeather(char* pbuf)
{
	DBGMSG(DPINFO, "UpdateWeather start\r\n");
	CXmlParse parse;
	char* pcontent;

	parse.Init(pbuf);
	if(!parse.RootCheck("msg"))
		goto endprocess;

	pcontent = parse.GetNodeContent("type");
	if(pcontent == NULL)
		goto endprocess;
	if(strtol(pcontent, NULL, 10) != MC_TYPE_UPDATE_ACK)
		goto endprocess;

	if(parse.SetLocate("wea"))
	{
		if(!parse.GetNodeContent("time"))
			goto endprocess;
		else
		{
			if(strcmp(parse.GetNodeContent("time"),GetWeatherTime()) < 0)
			{
				DBGMSG(DPINFO, "parse weatime %s cmp %s\r\n",parse.GetLocateName(), GetWeatherTime());
				goto endprocess;
			}
			else
				SetWeatherTime(parse.GetNodeContent("time"));
		}
		if(!parse.SetLocate("info"))
			goto endprocess;
		else
		{
			int nImage;
			char* ptr = parse.GetNodeContent("image");
			if(ptr)
			{
				nImage = strtol(ptr, NULL, 10);
				SetWeatherPng(nImage);
			}
			DBGMSG(DPINFO, "-----------nImage is %d\r\n",nImage);
			SetWeatherHum(parse.GetNodeContent("hum"));
			SetWeatherTemp(parse.GetNodeContent("temp"));
			DPPostMessage(MSG_BROADCAST, WEATHER_CHAGE, 0, 0);

		}
	}
	else
	{
		DBGMSG(DPINFO, "updateweathertime error not get wea\r\n");
	}
endprocess:
	return;
}

static DWORD UpdateThread(HANDLE pParam)
{
	DWORD updatetask = (DWORD)pParam;
	char* pbuf;

	if(updatetask & SYNC_UPDATE_APP)
	{
		pbuf = UpdateRequest("appver");
		if(pbuf != NULL)
		{
			UpdateApp(pbuf);
			free(pbuf);
		}
	}
	else if(updatetask & SYNC_UPDATE_NETCFG)
	{
		pbuf = UpdateRequest("netcfg");
		if(pbuf != NULL)
		{
			UpdateNetCfg(pbuf);
			free(pbuf);
		}
	}
	if(updatetask & SYNC_UPDATE_MSG)
	{
		pbuf = UpdateRequest("infotime");
		if(pbuf != NULL)
		{
			UpdateAreaMsg(pbuf);
			free(pbuf);
		}
	}
	if(updatetask & SYNC_UPDATE_WEATHER)
	{
		pbuf = UpdateRequest("wea");
		if(pbuf != NULL)
		{
			UpdateWeather(pbuf);
			free(pbuf);
		}
	}
	DPSetSemaphore(m_hUpgradeSemp);
	return 0;
}

int HandleSynchMsg(char* pSynchMsg)
{
	int dwRet = 0;
	CXmlParse parse;
	char* pcontent;
	int msgtype;
	DWORD updatetask = 0;
	//DBGMSG(DPINFO, "HandleSyncMsg %s\r\n", pSynchMsg);
	parse.Init(pSynchMsg);
	if(!parse.RootCheck("msg"))
		return -1;

	pcontent = parse.GetNodeContent("type");
	if(pcontent == NULL)
		return -1;

	msgtype = strtol(pcontent, NULL, 10);
	if(msgtype != MC_TYPE_SYNCHRO_ACK)
	{
		DBGMSG(DPINFO, "HandleSynchMsg type error:%s\r\n", pcontent);
		return -1;
	}

	pcontent = parse.GetNodeContent("time");
	if(pcontent != NULL)
	{
		SYSTEMTIME localTime = {0};
		if(6 == sscanf(pcontent, "%d-%d-%d %d:%d:%d",&localTime.wYear,&localTime.wMonth,
			&localTime.wDay,&localTime.wHour,&localTime.wMinute,&localTime.wSecond))
		{
			DPSetLocalTime(&localTime);
			//DBGMSG(DPINFO, "SetLocalTime:%d-%d-%d %d:%d:%d\r\n",localTime.wYear,localTime.wMonth, localTime.wDay,localTime.wHour,localTime.wMinute,localTime.wSecond);
		}
		else
		{
			DBGMSG(DPINFO, "HandleSynchMsg time error:%s\r\n",pcontent);
		}
	}

	pcontent = parse.GetNodeContent("appver");
	if((pcontent != NULL)
		&& (strcmp(pcontent, GetSoftVer()) > 0))
	{
		updatetask |= SYNC_UPDATE_APP;
	}

	pcontent = parse.GetNodeContent("netcfg");
	if((pcontent != NULL)
		&& (strcmp(pcontent, GetNetCfgMD5()) != 0))
	{
		updatetask |= SYNC_UPDATE_NETCFG;
	}
	pcontent = parse.GetNodeContent("infotime");
	if(pcontent != NULL)
	{
		if(strcmp(pcontent, GetMsgTime()) > 0)
			updatetask |= SYNC_UPDATE_MSG;
	}
	pcontent = parse.GetNodeContent("weatime");
	if(pcontent != NULL)
	{
		if(strcmp(pcontent, GetWeatherTime()) > 0)
			updatetask |= SYNC_UPDATE_WEATHER;
	}
	if(updatetask != 0)
	{
		// 清楚信号量
		DPGetSemaphore(m_hUpgradeSemp, 0);
		DPThreadCreate(0x4000, UpdateThread, (void*)updatetask, FALSE, 5);
	}
	return dwRet;
}

static BOOL SyncManager()
{
	char roomId[16];
	char pbuf[512];
	MSGHEAD* phead = (MSGHEAD*)pbuf;

	int ip = GetManagerIP();
	if(ip == 0)
		return FALSE;

	DBGMSG(DPINFO, "SyncManager %x\r\n", ip);

	CTcpClientSock tcpClient;
	tcpClient.SetTimeout(500);
	if(!tcpClient.Connect(ip, XML_SYNC_PORT))
		return FALSE;
	tcpClient.SetTimeout(1000);

	GetTermId(roomId);

	char content[512];
	sprintf(content,
		"<msg><type>%d</type><devtype>%s</devtype><id>%s</id><netcfg>%s</netcfg><appver>%s</appver><infotime>%s</infotime><weatime>%s</weatime></msg>",
		MC_TYPE_SYNCHRO, GetAppInfo(), roomId, GetNetCfgMD5(), GetSoftVer(), GetMsgTime(), GetWeatherTime());

	//DBGMSG(DPINFO, "SyncManager %s\r\n", content);

	DPCHAR *pContent = (DPCHAR*)phead->content;
	utf82unicode((WORD*)phead->content, (BYTE*)content);
	phead->head = XMLMSG_CHECKID;
	phead->length = sizeof(MSGHEAD) + (strlen(content) + 1) * 2;

	if(tcpClient.Send((char*)phead, phead->length))
	{
		MSGHEAD recvHead;
		char* pData;
		if(tcpClient.Recv((char*)&recvHead, sizeof(MSGHEAD))
			&& (XMLMSG_CHECKID == recvHead.head))
		{
			recvHead.length -= sizeof(MSGHEAD);
			pData = (char*)malloc(recvHead.length * 3);
			if(tcpClient.Recv(pData, recvHead.length))
			{
				*(DPCHAR*)(pData + recvHead.length) = 0;
				UnicodeConvertUTF8(pData, recvHead.length);
				HandleSynchMsg(pData);
			}
			free(pData);
		}
	}
	else
		DBGMSG(DPINFO, "SyncManager Get Sync data fail\r\n");
	return TRUE;
}

void DoSendXmlCmd(DWORD ip, MSGHEAD *pCmd)
{
	CTcpClientSock CSock;
	CSock.SetTimeout(500);
	if(CSock.Connect(ip, XML_SYNC_PORT))
	{
		CSock.SetTimeout(1000);
		if(!CSock.Send((char*)pCmd, pCmd->length))
		{
			DBGMSG(DPERROR, "DoSendXmlCmd send fail:%d\r\n", ip, DPGetLastError());
		}
	}
}

static DWORD SyncSendThread(HANDLE hRecvQ)
{
	DWORD lastTick = DPGetTickCount() - 120 * 1000 + (GetLocalIp() >> 24) * 1000;		
	DWORD curTick;
	SyncMSG smsg;
	FILE *fd;

	char filename[64];
	sprintf(filename, "%s/CompanyInfo.ver", FLASHDIR);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		HardInfo_T hardInfo;
		if(fread(&hardInfo, 1, sizeof(HardInfo_T), fd) != sizeof(HardInfo_T))
			memset(&g_HardInfo, 0, sizeof(HardInfo_T));
		else
		{
			unicode2utf8((BYTE*)g_HardInfo.CompInfo, (wchar_t*)hardInfo.CompInfo);
			unicode2utf8((BYTE*)g_HardInfo.CompInfoEn, (wchar_t*)hardInfo.CompInfoEn);
			unicode2utf8((BYTE*)g_HardInfo.HardInfo, (wchar_t*)hardInfo.HardInfo);
		}
		fclose(fd);

	}
	else
		memset(&g_HardInfo, 0, sizeof(HardInfo_T));

	sprintf(filename, "%s/SoftVersion.ver", FLASHDIR);
	fd = fopen(filename, "rb");
	if(fd != NULL)
	{
		SYSVER_T softInfo;
		if(fread(&softInfo, 1, sizeof(SYSVER_T), fd) != sizeof(SYSVER_T))
			memset(&g_SoftInfo, 0, sizeof(SYSVER_T));
		else
		{
			unicode2utf8((BYTE*)g_SoftInfo.AppInfo, (wchar_t*)softInfo.AppInfo);
			g_SoftInfo.DriverVer = softInfo.DriverVer;
			g_SoftInfo.AppVer = softInfo.AppVer;
			sprintf((char*)g_SoftInfo.DriverInfo, "%d", (DWORD)(g_SoftInfo.AppVer&0x00000000FFFFFFFF)); 
		}
		fclose(fd);
	}
	else
		memset(&g_SoftInfo, 0, sizeof(SYSVER_T));


	DBGMSG(DPINFO, "SyncXmlThread start\r\n");
	while(g_bSyncXmlStart)
	{
		if(DPReadMsgQueue(hRecvQ, &smsg, sizeof(SyncMSG), 1000))
		{
			char* buf[512];
			int ip = 0;
			MSGHEAD *pCmd = (MSGHEAD*)buf;

			switch(smsg.msg)
			{
				case OPEN_LOCK:
					DBGMSG(DPINFO, "Open Lock ip is %x\r\n", smsg.wParam);
					ip = smsg.wParam;
					BuildOpenLockCmd(pCmd, MC_TYPE_OPEN_LOCK);
					break;
				case OPEN_LOCKTWO:
					DBGMSG(DPINFO, "Open Lock 2 ip is %x\r\n", smsg.wParam);
					ip = smsg.wParam;
					BuildOpenLockCmd(pCmd, MC_TYPE_OPEN_LOCKTWO);
					break;
				case ALARM_REPORT:
					ip = GetManagerIP();
					DBGMSG(DPINFO, "Send Alarm %x\r\n", ip);
					BuildAlarmCmd(pCmd, smsg.wParam, smsg.lParam);
					break;
				case DISALARM_REPORT:
					ip = GetManagerIP();
					DBGMSG(DPINFO, "Send Disalarm %x\r\n", ip);
					BuildDisalarmCmd(pCmd, smsg.wParam);
					break;
				case SET_DEFENSE_REPORT:
					ip = GetManagerIP();
					DBGMSG(DPINFO, "Send Set Defense %x\r\n", ip);
					BuildSafeStatusCmd(pCmd, TRUE);
					break;
				case CANCEL_DEFENSE_REPORT:
					ip = GetManagerIP();
					DBGMSG(DPINFO, "Send Cancel Defense %x\r\n", ip);
					BuildSafeStatusCmd(pCmd, FALSE);
					break;
			}

			if(ip != 0)
			{
				DoSendXmlCmd(ip, pCmd);
			}
		}

		curTick = DPGetTickCount();
		if((curTick - lastTick) > 120 * 1000)
		{
			ip_get pget;
			if(ManagerGet(&pget))
			{
				free(pget.param);
				if(DPGetSemaphore(m_hUpgradeSemp, 0))
				{
					DPSetSemaphore(m_hUpgradeSemp);
					if(!SyncManager())
					{
						//DBGMSG(DPINFO, "SyncManaer fail\r\n");
					}
				}
			}
			lastTick = curTick;
		}
	}
	DPCloseMsgQueue(hRecvQ);
	DBGMSG(DPINFO, "SynXmlMsgProc end\r\n");
	return 0;
}

BOOL StartSyncSendServer(void)
{
	DBGMSG(DPINFO, "StartSyncSendServer start\r\n");
	m_hUpgradeSemp = DPCreateSemaphore(1, 1);
	g_NewSyncMsgCS.lockon();
	if(!g_bSyncXmlStart)
	{
		g_bSyncXmlStart = TRUE;
		HANDLE hRecvQ;
		DPCreateMsgQueue(SYNC_MSGQ, 100, sizeof(SyncMSG), &hRecvQ, &g_hSyncXmlSendQ);
		g_hSyncXmlThread = DPThreadCreate(0x4000, SyncSendThread, hRecvQ, TRUE, 5);
	}
	g_NewSyncMsgCS.lockoff();
	return TRUE;
}

void StopSyncSendServer(void)
{
	DBGMSG(DPINFO, "StopSyncSendServer start\r\n");

	SyncMSG smsg;
	g_NewSyncMsgCS.lockon();
	if(g_bSyncXmlStart)
	{
		g_bSyncXmlStart = FALSE;
		smsg.msg = 0xffffffff;
		DPWriteMsgQueue(g_hSyncXmlSendQ, &smsg, sizeof(SyncMSG), 0);
		DPThreadJoin(g_hSyncXmlThread);
		DPCloseMsgQueue(g_hSyncXmlSendQ);
	}
	g_NewSyncMsgCS.lockoff();
}

BOOL SendXmlSyncMsg(DWORD msg, DWORD wParam, DWORD lParam, UINT64 roomid)
{
	BOOL ret = TRUE;
	SyncMSG smsg;

	g_NewSyncMsgCS.lockon();
	smsg.msg = msg;
	smsg.wParam = wParam;
	smsg.lParam  = lParam;
	smsg.roomid = roomid;
	if(!DPWriteMsgQueue(g_hSyncXmlSendQ, &smsg, sizeof(SyncMSG), 0))
	{
		ret = FALSE;
		DBGMSG(DPERROR, "SendXmlSyncMsg fail\r\n");
	}
	g_NewSyncMsgCS.lockoff();
	return ret;
}
