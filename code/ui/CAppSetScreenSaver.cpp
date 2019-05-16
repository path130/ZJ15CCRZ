#include "CCtrlModules.h"

class CSetScreenSaverApp : public CAppBase
{
public:
	CSetScreenSaverApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSetScreenSaverApp(void)
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
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 1, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 1, 0);
			}
			else if(wParam == m_idOK)
			{
				SetScreenType(m_type);
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 1, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 1, 0);
			}
			else if(wParam >= m_idScreenSaver[0] && wParam <= m_idScreenSaver[2])
			{
				m_pScreenSaver[m_type]->Show(STATUS_NORMAL);
				m_type = wParam - m_idScreenSaver[0];
				m_pScreenSaver[m_type]->Show(STATUS_FOCUS);
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("setscreensaver.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("ok", &m_idOK);
		m_pScreenSaver[SCREEN_SAVER_BLACK] = (CDPButton*)GetCtrlByName("black", &m_idScreenSaver[SCREEN_SAVER_BLACK]);
		m_pScreenSaver[SCREEN_SAVER_CLOCK] = (CDPButton*)GetCtrlByName("timeDate", &m_idScreenSaver[SCREEN_SAVER_CLOCK]);
		m_pScreenSaver[SCREEN_SAVER_TIME] = (CDPButton*)GetCtrlByName("clock", &m_idScreenSaver[SCREEN_SAVER_TIME]);

		m_type = GetScreenType();
		for(int i = 0; i < SCREEN_SAVER_MAX; i++)
		{
			if(m_type == i)
				m_pScreenSaver[i]->Show(STATUS_FOCUS);
			else
				m_pScreenSaver[i]->Show(STATUS_NORMAL);
		}
		
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idOK;
	DWORD m_idScreenSaver[SCREEN_SAVER_MAX];
	CDPButton* m_pScreenSaver[SCREEN_SAVER_MAX];
	DWORD m_type;
};

CAppBase* CreateSetScreenSaverApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSetScreenSaverApp* pApp = new CSetScreenSaverApp(wParam);
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