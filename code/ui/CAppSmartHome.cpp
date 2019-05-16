#include "CCtrlModules.h"

class CSmartHomeApp : public CAppBase
{
public:
	CSmartHomeApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSmartHomeApp(void)
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
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lparam, DWORD zParam)
	{
		InitFrame("smarthome.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("button1", &m_idButton[0]);
		GetCtrlByName("button2", &m_idButton[1]);
		GetCtrlByName("button3", &m_idButton[2]);
		GetCtrlByName("button4", &m_idButton[3]);
		GetCtrlByName("button5", &m_idButton[4]);
		GetCtrlByName("button6", &m_idButton[5]);

		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idButton[6];
};

CAppBase* CreateSmartHomeApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSmartHomeApp* pApp = new CSmartHomeApp(wParam);
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