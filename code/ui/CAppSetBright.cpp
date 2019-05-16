#include "CCtrlModules.h"

class CSetBrightApp : public CAppBase
{
public:
	CSetBrightApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CSetBrightApp(void)
	{
	}

	BOOL DoPause()
	{
		SetScreenOnOff(TRUE);
		return CAppBase::DoPause();
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
			if(wParam == m_idBack || wParam == m_idCancel)
			{
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 1, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idOK)
			{
				DWORD dispParam[7];
				GetDisplayParam(dispParam);
				if(dispParam[0] != m_dwBright)
				{
					dispParam[0] = m_dwBright;
					SetDisplayParam(dispParam);
				}
				DPPostMessage(MSG_START_APP, USER_SET_APPID, 1, 0);
				DPPostMessage(MSG_END_APP, (DWORD)this, 0, 0);
			}
			else if(wParam == m_idAdd)
			{
				if(m_dwBright == 100)
					m_dwBright = 0;
				else 
					m_dwBright++;
				m_pBright->SetSrc(m_dwBright);
				m_pBright->Show(TRUE);

				AdjustScreen(m_dwBright, m_dwContrast, m_dwSaturation);
			}
			else if(wParam == m_idSub)
			{
				if(m_dwBright == 0)
					m_dwBright = 100;
				else 
					m_dwBright--;
				m_pBright->SetSrc(m_dwBright);
				m_pBright->Show(TRUE);

				AdjustScreen(m_dwBright, m_dwContrast, m_dwSaturation);
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("setbright.xml");
		GetCtrlByName("back", &m_idBack);
		GetCtrlByName("cancel", &m_idCancel);
		GetCtrlByName("ok", &m_idOK);
		GetCtrlByName("add", &m_idAdd);
		GetCtrlByName("sub", &m_idSub);
		m_pBright = (CDPStatic*)GetCtrlByName("vol");

		DWORD dispParam[7];
		GetDisplayParam(dispParam);
		m_dwBright = dispParam[0];
		m_dwContrast = dispParam[1];
		m_dwSaturation = dispParam[2];

		m_pBright->SetSrc(m_dwBright);
		m_pBright->Show(TRUE);

		return TRUE;
	}
private:
	DWORD m_idBack;
	DWORD m_idCancel;
	DWORD m_idOK;
	DWORD m_idAdd;
	DWORD m_idSub;
	CDPStatic* m_pBright;
	DWORD m_dwBright;
	DWORD m_dwContrast;
	DWORD m_dwSaturation;
};

CAppBase* CreateSetBrightApp(DWORD wParam, DWORD lParam, DWORD zParam)
{
	CSetBrightApp* pApp = new CSetBrightApp(wParam);
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