#include "CCtrlModules.h"
#include "PhoneApp.h"
#include "ipfunc.h"

class CCallMainApp : public CAppBase
{
public:
	CCallMainApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CCallMainApp(void)
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
			else if(wParam == m_idCallManager)
			{
				if(!RequestTalk())
					DPPostMessage(MSG_SHOW_STATUE, 606, 0, 0);		// 正在通话中
				else
					DPPostMessage(MSG_START_FROM_ROOT, TALKING_APPID, GetPhonePkt(GUARD_TYPE, NULL), 0);
			}
			else if(wParam == m_idCallRoom)
			{
				DPPostMessage(MSG_START_APP, CALL_ROOM_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idCallLine)
			{
				DPPostMessage(MSG_START_APP, CALL_LINE_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idTelBook)
			{
				DPPostMessage(MSG_START_APP, TELBOOK_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("callmain.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("callManager", &m_idCallManager);
		GetCtrlByName("callRoom", &m_idCallRoom);
		GetCtrlByName("callExt", &m_idCallLine);
		GetCtrlByName("telephoneBook", &m_idTelBook);

		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idCallManager;
	DWORD m_idCallRoom;
	DWORD m_idCallLine;
	DWORD m_idTelBook;
};

CAppBase* CreateCallMainApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CCallMainApp* pApp = new CCallMainApp(wParam);
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