#include "roomlib.h"

extern DWORD GetCurAppID();

#define	SAFE_MSGQ			"SAFESERVER"	
#define ALARMF_MP3_PATH		"/FlashDev/sound/alarmf.mp3"		// 预报警
#define ALARMS_MP3_PATH		"/FlashDev/sound/alarms.mp3"		// 报警

typedef struct  
{
	DWORD msg;
	DWORD wParam;
	DWORD lParam;
	DWORD zParam;
}SAFE_MSG;

static HANDLE g_hSafeDev = NULL;
static StaticLock* g_pSafeCS = NULL;
static HANDLE g_hSafeThread = NULL;
static HANDLE g_hSafeSendQ = NULL;
static SafeSet *g_pSafeSet = NULL;
static HANDLE g_hAlarmMp3 = NULL;
static DWORD g_dwMp3Start;
static DWORD g_dwDelayStart[SAFE_MAX_NUMBER];

static DWORD GetSafeGpio()
{
	DWORD trigger;
	ReadSafeDev(g_hSafeDev, (BYTE*)&trigger);
	return trigger;
}

static DWORD GetTriggerLevel()
{
	DWORD level = 0;
	g_pSafeCS->lockon();
	for(int i = 0; i < SAFE_MAX_NUMBER; i++)
	{
		if(g_pSafeSet->m_areaSet[i].Level == 0)
			level |= 1 << i;
	}
	g_pSafeCS->lockoff();
	return level;
}

static DWORD GetTriggerGpio()
{
	DWORD trigger = 0;
	g_pSafeCS->lockon();
	for(int i = 0; i < SAFE_MAX_NUMBER; i++)
	{
		if(g_pSafeSet->m_areaSet[i].OnOff)
			trigger |= 1 << i;
	}
	g_pSafeCS->lockoff();
	return trigger;
}

static void StopAlarmRing()
{
	if(g_hAlarmMp3 != NULL)
	{
		StopMp3(g_hAlarmMp3);
		g_hAlarmMp3 = NULL;
	}
}

static void StarAlarmRing(char* filename)
{
	if((g_hAlarmMp3 != NULL)
		&& (strcmp(filename, ALARMF_MP3_PATH) == 0))
	{
		return;
	}

	StopAlarmRing();
	g_hAlarmMp3 = PlayMp3(filename, 3000);
	if(g_hAlarmMp3 != NULL)
	{
		SetMp3Volume(g_hAlarmMp3, 0xAFFFFFFF);
	}
}

static void DisAlarmProc(DWORD disAlarmType, DWORD index)
{
	StopAlarmRing();

	if(disAlarmType == SMSG_HOSTAGE)
		SendXmlSyncMsg(ALARM_REPORT, STYPE_HOSTAGE, 99-10, 0);
	else
		SendXmlSyncMsg(DISALARM_REPORT, 1, 0 ,0);

	DPPostMessage(MSG_BROADCAST, ALARM_CHANGE, index, FALSE);
}

static void AlarmProc(int index)
{
	if(g_pSafeSet->m_areaSet[index].Type == STYPE_URGENT)
	{
		SendXmlSyncMsg(ALARM_REPORT, g_pSafeSet->m_areaSet[index].Type, g_pSafeSet->m_areaSet[index].Area, 0);
		AddAlarmRecord(g_pSafeSet->m_areaSet[index].Area, g_pSafeSet->m_areaSet[index].Type);
	}
	else if(g_pSafeSet->m_areaSet[index].Type < STYPE_MAGNETIC || g_pSafeSet->m_areaSet[index].Type > STYPE_MAX)
	{
		if(GetCurAppID() != TALKING_APPID)
			StarAlarmRing(ALARMS_MP3_PATH);
		SendXmlSyncMsg(ALARM_REPORT, g_pSafeSet->m_areaSet[index].Type, g_pSafeSet->m_areaSet[index].Area, 0);
		AddAlarmRecord(g_pSafeSet->m_areaSet[index].Area, g_pSafeSet->m_areaSet[index].Type);
		g_pSafeSet->m_areaSet[index].Status = SSTATUS_ALARMING;
		g_dwMp3Start = DPGetTickCount();
	}
	else
	{
		if(GetCurAppID() != TALKING_APPID)
			StarAlarmRing(ALARMF_MP3_PATH);
		g_pSafeSet->m_areaSet[index].Status = SSTATUS_ALARM_DELAY;
		g_dwDelayStart[index] = DPGetTickCount();
		SendXmlSyncMsg(ALARM_REPORT, g_pSafeSet->m_areaSet[index].Type, g_pSafeSet->m_areaSet[index].Area + g_pSafeSet->m_alarmDelay * 100, 0);
	}

	if(g_pSafeSet->m_areaSet[index].Type != STYPE_URGENT)
		DPPostMessage(MSG_BROADCAST, ALARM_CHANGE, index, TRUE);
}

static DWORD SafeThread(HANDLE pParam)
{
	SAFE_MSG safeMsg;
	DWORD lastTick = DPGetTickCount();

	g_hSafeDev = OpenSafeDev();

	GetSafeSet(&g_pSafeSet);
	DWORD dwCurStatus = GetSafeGpio();

	DWORD dwLastStatus = dwCurStatus;
	DWORD dwCmpStatus = GetTriggerLevel();					
	DWORD dwGpioTrigger = GetTriggerGpio();
	DWORD dwChangeStatus = 0;
	DWORD dwLastChangeStatus = 0;

	HANDLE hSafeRecvQ = (HANDLE)pParam;
	while(1)
	{
		if(DPReadMsgQueue(hSafeRecvQ, &safeMsg, sizeof(SAFE_MSG), 100))
		{
			switch(safeMsg.msg)
			{
			case SMSG_SETTING_CHANGE:
				dwGpioTrigger = GetTriggerGpio();
				dwCmpStatus = GetTriggerLevel();
				break;
			case SMSG_DISALARM:
			case SMSG_HOSTAGE:
				DisAlarmProc(safeMsg.msg, safeMsg.wParam);
				break;
			case SMSG_ALARM_RING:
				// 恢复报警声
				if(safeMsg.wParam)
				{
					// 如果没有响报警铃
					if(g_dwMp3Start == 0)
					{
						for(int i = 0; i < SAFE_MAX_NUMBER; i++)
						{
							// 是否还需要响预报警
							if(g_dwDelayStart[i] != 0)
							{
								StarAlarmRing(ALARMF_MP3_PATH);
								break;
							}
						}
					}
					else if(DPGetTickCount() - g_dwMp3Start < g_pSafeSet->m_alarmDuration)
					{
						StarAlarmRing(ALARMS_MP3_PATH);
					}
				}
				else
				{
					// 暂停报警声
					StopAlarmRing();
				}
				break;
			case SMSG_SOS:
				SendXmlSyncMsg(ALARM_REPORT, STYPE_URGENT, 0, 0);
				break;
			case SMSG_DELAY:
				g_pSafeSet->m_setDelay = safeMsg.wParam;
				g_pSafeSet->m_alarmDelay = safeMsg.lParam;
				g_pSafeSet->m_alarmDuration = safeMsg.zParam * 60 * 1000;
				break;
			case 0xFFFFFFFF:
				goto safeEnd;
			}
		}

		//if(ReadSafeDev(g_hSafeDev, (BYTE*)&dwCurStatus))
		dwCurStatus = GetSafeGpio();
		{
			if(dwCurStatus == dwLastStatus)	//去除抖动
			{
				g_pSafeCS->lockon();
				dwChangeStatus = dwCurStatus ^ dwCmpStatus;
				dwChangeStatus = dwChangeStatus & dwGpioTrigger;
				if(dwChangeStatus ^ dwLastChangeStatus)	//有探头被触发
				{
					DBGMSG(DPINFO, "Detect Gpio change %x %x %x\r\n", dwCmpStatus, dwCurStatus, dwGpioTrigger);
					for(int i = 0; i < SAFE_MAX_NUMBER; i++)
					{
						if(dwChangeStatus & (1 << i))
						{
							switch(g_pSafeSet->m_areaSet[i].Status)
							{
							case SSTATUS_CANCEL_DEFENSE:	//未布防，不处理
								break;
							case SSTATUS_SET_DEFENSE:	//报警
								AlarmProc(i);
								break;
							case SSTATUS_ALARMING:	//已经在报警中，不处理
								break;
							case SSTATUS_ALARM_DELAY:
								break;
							default:
								DBGMSG(DPINFO, "areaSet Status error, this should not be happening!!!" );
								break;
							}
						}
					}
					dwLastChangeStatus = dwChangeStatus;
				}
				else
				{
				}
				g_pSafeCS->lockoff();
			}
			else
			{
				dwLastStatus = dwCurStatus;
			}
		}

		DWORD tick = DPGetTickCount();
		for(int i = 0; i < SAFE_MAX_NUMBER; i++)
		{
			if(g_pSafeSet->m_areaSet[i].Status == SSTATUS_ALARM_DELAY)
			{
				if(tick - g_dwDelayStart[i] > g_pSafeSet->m_alarmDelay*1000)
				{
					if(GetCurAppID() != TALKING_APPID)
						StarAlarmRing(ALARMS_MP3_PATH);
					g_pSafeSet->m_areaSet[i].Status = SSTATUS_ALARMING;
					g_dwDelayStart[i] = 0;
					g_dwMp3Start = tick;

					AddAlarmRecord(g_pSafeSet->m_areaSet[i].Area, g_pSafeSet->m_areaSet[i].Type);
					DPPostMessage(MSG_BROADCAST, ALARM_CHANGE, i, 0);
				}
			}
		}

		if(g_dwMp3Start)
		{
			if(DPGetTickCount() - g_dwMp3Start > g_pSafeSet->m_alarmDuration)
			{
				StopAlarmRing();
				g_dwMp3Start = 0;
			}
		}
	}

safeEnd:
	DPCloseMsgQueue(hSafeRecvQ);
	return 0;
}

void StarSafeServer(void)
{
	g_pSafeCS = (StaticLock*)GetSafeSetCS();
	g_pSafeCS->lockon();
	if(g_hSafeThread == NULL)
	{
		HANDLE hSafeRecvQ = NULL;
		DPCreateMsgQueue(SAFE_MSGQ, 100, sizeof(SAFE_MSG), &hSafeRecvQ, &g_hSafeSendQ);
		g_hSafeThread = DPThreadCreate(0x4000, SafeThread, hSafeRecvQ, TRUE, 5);
	}
	g_pSafeCS->lockoff();
}

void StopSafeServer(void)
{
	g_pSafeCS->lockon();
	if(g_hSafeThread)
	{
		SAFE_MSG msg = {-1, 0, 0, 0};
		if(!DPWriteMsgQueue(g_hSafeSendQ, &msg, sizeof(SAFE_MSG), 0))
		{
			DBGMSG(SRV_MOD|DPERROR, "StopSafeServer WriteMsgQueue error:%d\n", DPGetLastError());
		}
		DPThreadJoin(g_hSafeThread);
		DPCloseMsgQueue(g_hSafeSendQ);
	}
	g_pSafeCS->lockoff();
}

BOOL PostSafeMessage(DWORD msg, DWORD wParam, DWORD lParam, DWORD zParam)
{
	BOOL ret = FALSE;
	g_pSafeCS->lockon(); 

	if(g_hSafeThread)
	{
		SAFE_MSG safeMsg = {msg, wParam, lParam, zParam};
		if(DPWriteMsgQueue(g_hSafeSendQ, &safeMsg, sizeof(SAFE_MSG), 0))
		{
			ret = TRUE;
		}
		else
		{
			DBGMSG(SRV_MOD|DPERROR, "PostSafeMessage error:%d\n", DPGetLastError());
		}
	}
	g_pSafeCS->lockoff(); 
	return ret;
}

BOOL CheckSafeGpio()
{
	// 检查是否有探头被触发
	return ((GetTriggerLevel() ^ GetTriggerGpio()) & GetTriggerGpio());
}