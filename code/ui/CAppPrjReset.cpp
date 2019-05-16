#include "CCtrlModules.h"

class CPrjResetApp : public CAppBase
{
public:
	CPrjResetApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CPrjResetApp(void)
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
			if((wParam == m_idBack)
				|| (wParam == m_idCancel))
			{
				DPPostMessage(MSG_START_APP, SYSTEM_SET_APPID, 0, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idOK)
			{
				DPPostMessage(MSG_SYSTEM, RESET_MACH, 0, 0);
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("prjreset.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("ok", &m_idOK);
		GetCtrlByName("cancel", &m_idCancel);
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idOK;
	DWORD m_idCancel;
};

CAppBase* CreatePrjResetApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CPrjResetApp* pApp = new CPrjResetApp(wParam);
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