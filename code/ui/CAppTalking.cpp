#include "CCtrlModules.h"
#include "PhoneApp.h"

#define CALL_IDLE	0
#define CALL_WAIT	1
#define CALL_RING	2
#define CALL_TALK	3
#define CALL_LIUYAN	4

#define MONITOR_TIMEOUT		60
#define TALK_TIMEOUT		180		
#define LIUYAN_TIMEOUT		45		

static DWORD g_dwButton[2];		// 用来保存 挂断、接听按钮button
void AutoPostMsg(BOOL bAccept)
{
	if(bAccept)
		DPPostMessage(TOUCH_MESSAGE, g_dwButton[1], 0, 0);
	else
		DPPostMessage(TOUCH_MESSAGE, g_dwButton[0], 0, 0);
}


class CTalkingApp : public CAppBase
{
public:
	CTalkingApp(DWORD pHwnd):CAppBase(pHwnd)
	{
		m_hRing = NULL;
		m_pFrameBuf = NULL;
		m_bExit = FALSE;
		PostSafeMessage(SMSG_ALARM_RING, FALSE, 0, 0);
	}

	~CTalkingApp(void)
	{
	}

	BOOL DoPause()
	{
		// 如果正在通话中，应该除了升级之外，不会再有其他界面突然进来，所以此界面不用挂断
		StopRing();

		if(m_dwRingVol != GetRingVol())
			SetRingVol(m_dwRingVol);

		if(m_dwTalkVol != GetTalkVol())
			SetTalkVol(m_dwTalkVol);

		if(m_PhonePkt.dwCallType != MONITOR_TYPE)
		{
			if(!m_bExit)
			{
				m_bExit = TRUE;
				ReleaseTalk();
				AddCallRecord(m_PhonePkt.code, m_PhonePkt.bCallOut, m_dwStatus == CALL_TALK);

				if(m_bDoorType
					&& (m_dwStatus != CALL_TALK))
				{
					// 保存留影
					if(m_dwFrameLen > 0)
						AddLiuying(m_PhonePkt.code, m_dwFrameLen, m_pFrameBuf, 0);
				}
			}
		}

		if(GetDefenseIsAlarming())
			PostSafeMessage(SMSG_ALARM_RING, TRUE, 0, 0);

		return CAppBase::DoPause();
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:
			if(m_dwTimeOut > 0)
			{
				m_dwTimeOut--;
				if(m_dwTimeOut == 0)
				{
					if((m_dwStatus == CALL_RING)
						&& m_bDoorType
						&& !m_PhonePkt.bCallOut)
					{
						StartLiuyan();
					}
					else
					{
						DPSessionUserOp(DPMSG_HANGUP, m_PhonePkt.dwSessionId);
						HangUp(GetStringByID(607));		// 通话结束
					}
				}

				char buf[32];
				sprintf(buf, "%02d:%02d", m_dwTimeOut / 60, m_dwTimeOut % 60);
				m_pTimeTip->SetSrc(buf);
				m_pTimeTip->Show(TRUE);
			}
			if(m_bPhoto)
			{
				char* pdata;
				int len = GetIFrameData(&pdata);
				if(len > 0)
				{
					AddPhoto(m_PhonePkt.code, len, pdata, 0);
					m_bPhoto = FALSE;
					m_pBottomBtn[1]->Show(STATUS_NORMAL);
				}
			}
			if(m_dwLiuying > 0)
			{
				m_dwLiuying--;
				if(m_dwLiuying == 0)
				{
					m_dwFrameLen = GetIFrameData(&m_pFrameBuf);
					if(m_dwFrameLen == 0)
					{
						m_dwLiuying++;	// 还没有截取到IFrame, 继续抓拍
					}
				}
			}
			if(m_dwEndTime > 0)
			{
				m_dwEndTime--;
				if(m_dwEndTime == 0)
				{
					DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}
			break;
		case TOUCH_MESSAGE:
			if(wParam == m_idBottomBtn[0])
			{
				DPSessionUserOp(DPMSG_HANGUP, m_PhonePkt.dwSessionId);
				HangUp(GetStringByID(607));		// 通话结束
			}
			else if(wParam == m_idBottomBtn[1])
			{
				if(m_PhonePkt.dwCallType == MONITOR_TYPE)
				{
					// 拍照
					m_bPhoto = TRUE;
				}
				else
				{
					// 接听
					DPSessionUserOp(DPMSG_ACCEPT, m_PhonePkt.dwSessionId);
					StopRing();

					char buf[32];
					sprintf(buf, "%s %s", m_strName, GetStringByID(606));	// 正在通话中
					m_pStatusTip->SetSrc(buf);
					m_pStatusTip->Show(TRUE);

					m_dwTimeOut = TALK_TIMEOUT;
					m_dwStatus = CALL_TALK;
					ShowVolume();
				}
			}
			else if(wParam == m_idRightBtn[0])
			{
				if(m_dwStatus == CALL_TALK)
				{
					m_dwTalkVol++;
					if(m_dwTalkVol == 16)
						m_dwTalkVol = 0;

					DPTransAudioSetVol(m_dwTalkVol * 0x11111111);
				}
				else
				{
					m_dwRingVol++;
					if(m_dwRingVol == 16)
						m_dwRingVol = 0;

					if(m_hRing)
						SetMp3Volume(m_hRing, m_dwRingVol * 0x11111111);
				}
				ShowVolume();
			}
			else if(wParam == m_idRightBtn[1])
			{
				if(m_dwStatus == CALL_TALK)
				{
					m_dwTalkVol--;
					if(m_dwTalkVol == -1)
						m_dwTalkVol = 15;

					DPTransAudioSetVol(m_dwTalkVol * 0x11111111);
				}
				else
				{
					m_dwRingVol--;
					if(m_dwRingVol == -1)
						m_dwRingVol = 15;

					if(m_hRing)
						SetMp3Volume(m_hRing, m_dwRingVol * 0x11111111);
				}
				ShowVolume();
			}
			else if(wParam == m_idRightBtn[2])
			{
				if(m_bDoorType)
				{
					// 开锁
					UINT64 id;
					Code2ID(m_PhonePkt.code, &id);
					SendXmlSyncMsg(OPEN_LOCK, m_PhonePkt.ip, 0, id);
				}
				else
				{
					// 视频开关
					if(m_bVideoEnable)
						m_pRightBtn[2]->SetSrcpng("videoon.png");
					else
						m_pRightBtn[2]->SetSrcpng("videooff.png");
					m_pRightBtn[2]->Show(STATUS_NORMAL);
					m_bVideoEnable = !m_bVideoEnable;

					DPTransVideoEnable(m_bVideoEnable);
				}
			}
			else if(wParam == m_idEmpty)
			{
				// 全屏
				DPTransVideoSetRect(0, 0, FRAME_WIDTH, FRAME_HEIGHT);
				SetTouchDirect(TRUE);
			}
			break;
		case TOUCH_RAW_MESSAGE:
			if(zParam == TOUCH_UP)
			{
				// 正常
				DPTransVideoSetRect(V_NORMAL_LEFT, V_NORMAL_TOP, V_NORMAL_WIDTH, V_NORMAL_HEIGHT);
				SetTouchDirect(FALSE);
			}
			break;
		case MSG_PHONECALL:
			if(wParam == DPMSG_HANGUP)
			{
				char buf[64];
				sprintf(buf, "%s %s", m_strName, GetStringByID(610));	// 对方主动挂断
				HangUp(buf);
			}
			else if(wParam == DPMSG_OFFLINE)
			{
				if(CallManager())
					break;

				HangUp(GetStringByID(608));		// 对方不在线
			}
			else if(wParam == DPMSG_BUSY)
			{
				char buf[64];
				sprintf(buf, "%s %s", m_strName, GetStringByID(609));	// 对方繁忙
				HangUp(buf);		
			}
			else if(wParam == DPMSG_IDLE)
			{
				if(m_PhonePkt.dwCallType != MONITOR_TYPE)
				{
					StartRing();
					m_dwStatus = CALL_RING;
					// m_dwTimeOut = 60;	响铃超时由被叫方判断
				}
			}
			else if(wParam == DPMSG_ACCEPT)
			{
				char buf[64];
				m_dwStatus = CALL_TALK;
				if(m_PhonePkt.dwCallType == MONITOR_TYPE)
				{
					m_dwTimeOut = MONITOR_TIMEOUT;
				}
				else
				{
					sprintf(buf, "%s %s", m_strName, GetStringByID(606));	// 正在通话中
					m_pStatusTip->SetSrc(buf);
					m_pStatusTip->Show(TRUE);

					StopRing();
					m_dwTimeOut = TALK_TIMEOUT;
					ShowVolume();
				}
			}
			break;
		case MSG_BROADCAST:
			if((wParam == ALARM_CHANGE)
				&& GetDefenseIsAlarming())
			{
				m_pAlarmTip->SetSrc(GetStringByID(628));		// 发生报警
				m_pAlarmTip->Show(TRUE);
			}
		}
		return TRUE;
	}

	void StartLiuyan()
	{
		char buf[64];
		sprintf(buf, "%s %s", m_strName, GetStringByID(627));	// 正在留言中
		m_pStatusTip->SetSrc(buf);
		m_pStatusTip->Show(TRUE);

		// 进入留言状态
		StopRing();
		DPSessionUserOp(DPMSG_LIUYAN, m_PhonePkt.dwSessionId);

		m_bLiuyan = TRUE;
		m_dwTimeOut = LIUYAN_TIMEOUT;
		m_dwStatus = CALL_LIUYAN;
	}

	BOOL CallManager()
	{
		if(m_PhonePkt.bCallOut 
			&& m_PhonePkt.dwCallType == GUARD_TYPE)
		{
			ip_get pGet = {0};
			ManagerGet(&pGet);
			if(pGet.num > 0)
			{
				strcpy(m_PhonePkt.code, pGet.param[0].code);
				m_PhonePkt.dwCallType = MANAGER_TYPE;
				m_PhonePkt.dwSessionId = GetSessionId();
				m_PhonePkt.ip = pGet.param[0].ip;
				free(pGet.param);

				m_dwStatus = CALL_WAIT;
				Code2Name(m_strName, m_PhonePkt.code);

				char buf[64];
				sprintf(buf, "%s %s", GetStringByID(604), m_strName);	// 正在呼叫
				m_pStatusTip->SetSrc(buf);
				m_pStatusTip->Show(TRUE);

				DPSessionUserOp(DPMSG_CALL, 0, (DWORD)&m_PhonePkt);
				return TRUE;
			}
		}
		return FALSE;
	}

	void HangUp(char* strTip)
	{
		StopRing();

		m_pStatusTip->SetSrc(strTip);		//	通话结束
		m_pStatusTip->Show(TRUE);
		m_dwEndTime = 1;
	}

	void ShowVolume()
	{
		int nVol = m_dwRingVol;
		if(m_dwStatus == CALL_TALK)
			nVol = m_dwTalkVol;

		m_pVolVal->SetSrc(nVol);
		m_pVolVal->Show(TRUE);
	}

	void InitCall(PhonePkt* pPkt)
	{
		memcpy(&m_PhonePkt, pPkt, sizeof(PhonePkt));

		m_bDoorType = FALSE;
		switch(m_PhonePkt.dwCallType)
		{
			case CELL_DOOR_TYPE:
			case SECOND_DOOR_TYPE:
			case ZONE_DOOR_TYPE:
			case AREA_DOOR_TYPE:
				m_bDoorType = TRUE;
				break;
		}

		m_dwTalkVol = GetTalkVol();
		m_dwRingVol = GetRingVol();

		m_bPhoto = FALSE;
		m_bLiuyan = FALSE;
		m_dwLiuying = 0;
		m_dwEndTime = 0;
		Code2Name(m_strName, m_PhonePkt.code);

		char buf[64];
		if(m_PhonePkt.bCallOut)
		{
			m_dwTimeOut = 0;
			m_dwStatus = CALL_WAIT;
			m_PhonePkt.dwSessionId = GetSessionId();

			if(m_PhonePkt.dwCallType == MONITOR_TYPE)
			{
				sprintf(buf, "%s %s", GetStringByID(605), m_strName);	// 正在监视

				m_pBottomBtn[0]->SetSrcText(GetStringByID(1027));	// 返回
				m_pBottomBtn[0]->Show(STATUS_NORMAL);
				m_pBottomBtn[1]->SetSrcText(GetStringByID(1050));	// 拍照
				m_pBottomBtn[1]->Show(STATUS_NORMAL);

				DPSessionUserOp(DPMSG_MONITOR, 0, (DWORD)&m_PhonePkt);
			}
			else
			{
				sprintf(buf, "%s %s", GetStringByID(604), m_strName);	// 正在呼叫

				m_pVolTip->SetSrc(GetStringByID(1053));				// 音量:
				m_pVolTip->Show(TRUE);
				ShowVolume();

				m_pBottomBtn[0]->SetSrcText(GetStringByID(40021));	// 挂断
				m_pBottomBtn[0]->Show(STATUS_NORMAL);
				m_pBottomBtn[1]->SetSrcText(GetStringByID(40011));	// 接听
				m_pBottomBtn[1]->Show(STATUS_UNACK);
			
				m_pRightBtn[0]->SetSrcText(GetStringByID(40041));	// 音量+
				m_pRightBtn[0]->SetSrcpng("voladd.png");
				m_pRightBtn[0]->Show(STATUS_NORMAL);
				m_pRightBtn[1]->SetSrcText(GetStringByID(40031));	// 音量-
				m_pRightBtn[1]->SetSrcpng("volsub.png");
				m_pRightBtn[1]->Show(STATUS_NORMAL);

				m_bVideoEnable = FALSE;
				DPTransVideoEnable(FALSE);
				m_pRightBtn[2]->SetSrcText(GetStringByID(40051));	// 视频
				m_pRightBtn[2]->SetSrcpng("videoon.png");
				m_pRightBtn[2]->Show(STATUS_NORMAL);

				DPSessionUserOp(DPMSG_CALL, 0, (DWORD)&m_PhonePkt);
			}
		}
		else
		{
#ifdef _DEBUG
			m_dwTimeOut = 5;
#else
			m_dwTimeOut = GetDelay(DELAY_CALLIN);
#endif
			m_dwStatus = CALL_RING;
			StartRing();

			sprintf(buf, "%s %s，%s", m_strName, GetStringByID(602), GetStringByID(603));		// 呼叫，请求接听

			m_pVolTip->SetSrc(GetStringByID(1053));				// 音量:
			m_pVolTip->Show(TRUE);
			ShowVolume();

			m_pBottomBtn[0]->SetSrcText(GetStringByID(40021));	// 挂断
			m_pBottomBtn[0]->Show(STATUS_NORMAL);
			m_pBottomBtn[1]->SetSrcText(GetStringByID(40011));	// 接听
			m_pBottomBtn[1]->Show(STATUS_NORMAL);

			m_pRightBtn[0]->SetSrcText(GetStringByID(40041));	// 音量+
			m_pRightBtn[0]->SetSrcpng("voladd.png");
			m_pRightBtn[0]->Show(STATUS_NORMAL);
			m_pRightBtn[1]->SetSrcText(GetStringByID(40031));	// 音量-
			m_pRightBtn[1]->SetSrcpng("volsub.png");
			m_pRightBtn[1]->Show(STATUS_NORMAL);
			if(m_bDoorType)
			{
				m_pRightBtn[2]->SetSrcText(GetStringByID(1038));	// 开锁
				m_pRightBtn[2]->SetSrcpng("unlock.png");
				m_pRightBtn[2]->Show(STATUS_NORMAL);

				m_dwLiuying = 3;	// 3秒之后再截取	防止录屏？！
				DPSessionUserOp(DPMSG_START_VIDEO, m_PhonePkt.dwSessionId);
			}
			else
			{
				m_bVideoEnable = FALSE;
				DPTransVideoEnable(FALSE);
				m_pRightBtn[2]->SetSrcText(GetStringByID(40051));	// 视频
				m_pRightBtn[2]->SetSrcpng("videoon.png");
				m_pRightBtn[2]->Show(STATUS_NORMAL);
			}
		}

		m_pStatusTip->SetSrc(buf);
		m_pStatusTip->Show(TRUE);
	}

	void StartRing()
	{
		if(m_hRing == NULL)
		{
			m_hRing = PlayMp3(GetRingName(m_PhonePkt.bCallOut), 1000);
			SetMp3Volume(m_hRing, m_dwRingVol * 0x11111111);
		}
	}

	void StopRing()
	{
		if(m_hRing)
		{
			StopMp3(m_hRing);
			m_hRing = NULL;
		}
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("talking.xml");
		GetCtrlByName("ebutton", &m_idEmpty);
		m_pStatusTip = (CDPStatic*)GetCtrlByName("calltip");
		m_pTimeTip = (CDPStatic*)GetCtrlByName("timetip");
		m_pVolTip = (CDPStatic*)GetCtrlByName("voltip");
		m_pVolVal = (CDPStatic*)GetCtrlByName("valtip");
		m_pAlarmTip = (CDPStatic*)GetCtrlByName("safetip");
		m_pBottomBtn[0] = (CDPButton*)GetCtrlByName("bottom1", &m_idBottomBtn[0]);
		m_pBottomBtn[1] = (CDPButton*)GetCtrlByName("bottom2", &m_idBottomBtn[1]);
		m_pRightBtn[0] = (CDPButton*)GetCtrlByName("right1", &m_idRightBtn[0]);
		m_pRightBtn[1] = (CDPButton*)GetCtrlByName("right2", &m_idRightBtn[1]);
		m_pRightBtn[2] = (CDPButton*)GetCtrlByName("right3", &m_idRightBtn[2]);

		PhonePkt* pPkt = (PhonePkt*)lParam;
		InitCall(pPkt);
		free(pPkt);

		if(GetDefenseIsAlarming())
		{
			m_pAlarmTip->SetSrc(GetStringByID(628));		// 发生报警
			m_pAlarmTip->Show(TRUE);
		}

		g_dwButton[0] = m_idBottomBtn[0];
		g_dwButton[1] = m_idBottomBtn[1];

		return TRUE;
	}
private:
	DWORD m_idEmpty;
	DWORD m_idBottomBtn[2];
	CDPButton* m_pBottomBtn[2];		// 底部两个按钮，挂断、接听、拍照	
	DWORD m_idRightBtn[3];
	CDPButton* m_pRightBtn[3];		// 右边三个按钮，音量+、音量-、视频开关、开锁
	CDPStatic *m_pStatusTip;
	CDPStatic *m_pTimeTip;
	CDPStatic *m_pVolTip;
	CDPStatic *m_pVolVal;
	CDPStatic *m_pAlarmTip;

	PhonePkt m_PhonePkt;
	BOOL m_bDoorType;
	char m_strName[64];
	DWORD m_dwStatus;

	DWORD m_dwTalkVol;
	DWORD m_dwRingVol;
	BOOL m_bVideoEnable;
	BOOL m_bPhoto;
	BOOL m_bLiuyan;
	DWORD m_dwLiuying;

	char* m_pFrameBuf;
	DWORD m_dwFrameLen;

	HANDLE m_hRing;
	DWORD m_dwTimeOut;
	DWORD m_dwEndTime;
	BOOL m_bExit;
};

CAppBase* CreateTalkingApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CTalkingApp* pApp = new CTalkingApp(wParam);
	if(pApp)
	{
		if(!pApp->Create(lParam, zParam))
		{
			delete pApp;
			pApp = NULL;		
		}
	}
	return pApp;
}