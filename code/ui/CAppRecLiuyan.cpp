#include "CCtrlModules.h"

class CRecLiuyanApp : public CAppBase
{
public:
	CRecLiuyanApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CRecLiuyanApp(void)
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

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("recliuyan.xml");
		GetCtrlByName("back", &m_idBack);
		return TRUE;
	}
private:
	DWORD m_idBack;
};

CAppBase* CreateRecLiuyanApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CRecLiuyanApp* pApp = new CRecLiuyanApp(wParam);
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