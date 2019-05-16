#include "CCtrlModules.h"

class CSetCleanScreenApp : public CAppBase
{
public:
	CSetCleanScreenApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSetCleanScreenApp(void)
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
			if(m_dwPercent > 0)
			{
				m_dwPercent++;
				if(m_dwPercent - 1 <= 10)
				{
					char buf[32];
					sprintf(buf, "%d0%%", m_dwPercent - 1);
					m_pPercent->SetSrc(buf);
					m_pPercent->Show(TRUE);
				}
				else
				{
					DPPostMessage(MSG_START_APP, USER_SET_APPID, 1, 0);
					DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
				}
			}
			break;
		case TOUCH_MESSAGE:
			if(wParam == m_idBack || wParam == m_idCancel)
			{
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 1, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idOK)
			{
				m_dwPercent = 1;
				m_pPercent->SetSrc("0%");
				m_pPercent->Show(TRUE);
				m_pCleaning->SwitchLay(TRUE);
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("setcleanscreen.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("cancel", &m_idCancel);
		GetCtrlByName("ok", &m_idOK);

		m_dwPercent = 0;
		m_pCleaning = (CLayOut*)GetCtrlByName("cleaning");
		m_pPercent = (CDPStatic*)GetCtrlByName("percent");
		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idCancel;
	DWORD m_idOK;
	CDPStatic* m_pPercent;
	CLayOut* m_pCleaning;
	DWORD m_dwPercent;
};

CAppBase* CreateSetCleanScreenApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSetCleanScreenApp* pApp = new CSetCleanScreenApp(wParam);
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