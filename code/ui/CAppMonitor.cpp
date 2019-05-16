#include "CCtrlModules.h"

class CMonitorApp : public CAppBase
{
public:
	CMonitorApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CMonitorApp(void)
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch( uMsg )
		{
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
		InitFrame("talking.xml");
		GetCtrlByName("hungup", &m_idBack);

		return TRUE;
	}
private:
	DWORD m_idBack;
};

CAppBase* CreateMonitorApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CMonitorApp* pApp = new CMonitorApp(wParam);
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