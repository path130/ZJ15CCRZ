#include "CCtrlModules.h"
#include "PhoneApp.h"
#include "ipfunc.h"
#include "dpsession.h"

extern BOOL SafeSetting(int mode);

class CMainApp : public CAppBase
{
public:
	CMainApp(DWORD hWnd) : CAppBase(hWnd)
	{

	}

	~CMainApp()
	{

	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch(uMsg)
		{
		case TIME_MESSAGE:
			m_pTime->UpdataDateTime();
			if(m_dwTimeout < m_screenoff)
			{
				m_dwTimeout++;
				if(m_dwTimeout == m_screenoff)
					DPPostMessage(MSG_START_FROM_ROOT, BLACKSCREEN_APPID, 0, 0);
			}
			if(m_dwConflict > 0)
			{
				m_dwConflict--;
				if(m_dwConflict == 0)
				{
					if(!CheckIPConflict(GetLocalIp()))
					{
						SetNetState(FALSE);
						RefreshSignTip();
					}
					else
					{
						m_dwConflict = 5;		// 5秒检测一次IP是否仍然冲突
					}
				}
			}
			break;
		case MSG_BROADCAST:
			RefreshSignTip();
			break;
		case TOUCH_MESSAGE:
			if(wParam == m_idMute)
			{
				if(GetRingVol() > 0)
					SetRingVol(0);
				else
					SetRingVol(6);
				UpdateMuteTip();
			}
			else if(wParam == m_idSafe)
			{
				if(GetDefenseStatus())
					DPPostMessage(MSG_SHOW_STATUE, 2016, 0, 0);		// 已布防
				else if(GetDefenseIsAlarming())	
					DPPostMessage(MSG_SHOW_STATUE, 2018, 0, 0);		// 正在报警中,请先撤防!
				else
				{
					DPPostMessage(MSG_START_APP, SAFE_SET_APPID, MAIN_APPID, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}
			else if(wParam == m_idElevator)
			{
				if(CallElevator())
					DPPostMessage(MSG_SHOW_STATUE, 2026, 0, 0);	// 呼梯成功！
				else
					DPPostMessage(MSG_SHOW_STATUE, 2027, 0, 0);	// 呼梯失败！
			}
			else if(wParam == m_idMonitor)
			{
				DPPostMessage(MSG_START_FROM_ROOT, TALKING_APPID, GetPhonePkt(MONITOR_TYPE, NULL), 0);
			}
			else if(wParam == m_idManager)
			{
				if(!RequestTalk())
					DPPostMessage(MSG_SHOW_STATUE, 606, 0, 0);		// 正在通话中
				else
					DPPostMessage(MSG_START_FROM_ROOT, TALKING_APPID, GetPhonePkt(GUARD_TYPE, NULL), 0);
			}
			else if(wParam == m_idMessage)
			{
				DPPostMessage(MSG_START_APP, REC_MESSAGE_APPID, MAIN_APPID, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[0])
			{
				//DPTransLiuyanStart(inet_addr("127.0.0.1"), 15100, 15100);
				DPPostMessage(MSG_START_APP, SMART_HOME_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[1])
			{
				//DPTransLiuyanStop();
				DPPostMessage(MSG_START_APP, RECORD_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[2])
			{
				DPPostMessage(MSG_START_APP, SAFE_MAIN_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[3])
			{
				DPPostMessage(MSG_START_APP, PWD_INPUT_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[4])
			{
				DPPostMessage(MSG_START_APP, CALL_MAIN_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[5])
			{
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[6])
			{
				DPPostMessage(MSG_START_APP, MONITOR_MAIN_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			break;
		}
		return TRUE;	
	}

	void RefreshSignTip()
	{
		int ptr = 0;
		switch(GetNetState())
		{
			case 1:
				m_dwConflict = 5;
				m_pSingTip[ptr]->SetSrc("sign_netconflict.png");
				break;
			case 3:
				m_pSingTip[ptr]->SetSrc("sign_netconnect.png");
				break;
			default:
				m_pSingTip[ptr]->SetSrc("sign_netdisconnect.png");
				break;
		}
		m_pSingTip[ptr++]->Show( TRUE );

		if(GetDefenseIsAlarming())
			m_pSingTip[ptr]->SetSrc("sign_alarming.png");
		else
		{
			if( GetDefenseStatus() )
				m_pSingTip[ptr]->SetSrc("sign_setdefense.png");
			else
				m_pSingTip[ptr]->SetSrc("sign_canceldefense.png");
		}
		m_pSingTip[ptr++]->Show(TRUE);

		if(GetLiuyingUnreadCount() > 0)
		{
			m_pSingTip[ptr]->SetSrc("sign_leavepicture.png");
			m_pSingTip[ptr++]->Show(TRUE);
		}
		if(GetMessageUnreadCount() > 0)
		{
			m_pSingTip[ptr]->SetSrc("sign_message.png");
			m_pSingTip[ptr++]->Show(TRUE);
		}
		if(GetCallRecordUnread())
		{
			m_pSingTip[ptr]->SetSrc("sign_misscall.png");
			m_pSingTip[ptr++]->Show(TRUE);
		}

		for(;ptr < 8; ptr++)
			m_pSingTip[ptr]->Show(FALSE);
	}

	void UpdateMuteTip()
	{
		if(GetRingVol() != 0)
		{
			m_pMute->SetSrcpng("ring.png");
			m_pMute->SetSrcText(GetStringByID(20008));	// 关闭铃声
			m_pMute->Show(STATUS_NORMAL);
		}
		else
		{
			m_pMute->SetSrcpng("mute.png");
			m_pMute->SetSrcText(GetStringByID(20001));	// 开启铃声
			m_pMute->Show(STATUS_NORMAL);
		}
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("main.xml");
		GetCtrlByName("smartHome", &m_idButton[0]);
		GetCtrlByName("record", &m_idButton[1]);
		GetCtrlByName("defense", &m_idButton[2]);
		GetCtrlByName("systemSetting", &m_idButton[3]);
		GetCtrlByName("callmain", &m_idButton[4]);
		GetCtrlByName("userSetting", &m_idButton[5]);
		GetCtrlByName("monitormain", &m_idButton[6]);

		// 快捷键
		GetCtrlByName("callelevator", &m_idElevator);
		GetCtrlByName("monitordoor", &m_idMonitor);
		GetCtrlByName("callmanager", &m_idManager);
		GetCtrlByName("message", &m_idMessage);
		GetCtrlByName("setdefense", &m_idSafe);
		m_pMute  = (CDPButton*)GetCtrlByName("mute", &m_idMute);
		UpdateMuteTip();

		// 提示图标
		m_pSingTip[0]  = (CDPStatic*)GetCtrlByName("sign_tip1");
		m_pSingTip[1]  = (CDPStatic*)GetCtrlByName("sign_tip2");
		m_pSingTip[2]  = (CDPStatic*)GetCtrlByName("sign_tip3");
		m_pSingTip[3]  = (CDPStatic*)GetCtrlByName("sign_tip4");
		m_pSingTip[4]  = (CDPStatic*)GetCtrlByName("sign_tip5");
		m_pSingTip[5]  = (CDPStatic*)GetCtrlByName("sign_tip6");
		m_pSingTip[6]  = (CDPStatic*)GetCtrlByName("sign_tip7");
		m_pSingTip[7]  = (CDPStatic*)GetCtrlByName("sign_tip8");
		RefreshSignTip();

		// 时间
		m_dwConflict = 0;
		m_pTime = (CTimeDate*)GetCtrlByName("time");
		return TRUE;
	}

private:
	DWORD m_idButton[7];

	DWORD m_idElevator;
	DWORD m_idMonitor;
	DWORD m_idManager;
	DWORD m_idMessage;
	DWORD m_idSafe;
	DWORD m_idMute;
	CDPButton* m_pMute;

	CDPStatic* m_pSingTip[8];
	CTimeDate* m_pTime;

	DWORD m_dwConflict;
};

CAppBase* CreateMainApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CMainApp* pApp = new CMainApp(wParam);
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