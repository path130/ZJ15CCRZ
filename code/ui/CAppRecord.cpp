#include "CCtrlModules.h"

class CRecordApp : public CAppBase
{
public:
	CRecordApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CRecordApp(void)
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch( uMsg )
		{
		case TIME_MESSAGE:
			if(m_dwTimeout < m_screenoff)
			{
				m_dwTimeout++;
				if(m_dwTimeout == m_screenoff)
					DPPostMessage(MSG_START_FROM_ROOT, BLACKSCREEN_APPID, 0, 0);
			}
			break;
		case TOUCH_MESSAGE:
			if(wParam == m_idBack)
			{
				DPPostMessage(MSG_START_APP, MAIN_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[0])
			{
				DPPostMessage(MSG_START_APP, REC_MESSAGE_APPID, RECORD_APPID, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[1])
			{
				DPPostMessage(MSG_START_APP, REC_SAFE_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[2])
			{
				DPPostMessage(MSG_START_APP, REC_ALARM_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[3])
			{
				DPPostMessage(MSG_START_APP, REC_PHOTO_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[4])
			{
				DPPostMessage(MSG_START_APP, REC_CALL_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idButton[5])
			{
				DPPostMessage(MSG_START_APP, REC_LIUYING_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("record.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("message", &m_idButton[0]);
		GetCtrlByName("defenseRec", &m_idButton[1]);
		GetCtrlByName("alarmRec", &m_idButton[2]);
		GetCtrlByName("photoRec", &m_idButton[3]);
		GetCtrlByName("callRec", &m_idButton[4]);
		GetCtrlByName("leaveMsgRec", &m_idButton[5]);
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idButton[7];
};

CAppBase* CreateRecordApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CRecordApp* pApp = new CRecordApp(wParam);
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