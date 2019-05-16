#include "CCtrlModules.h"

#define STATUS_TIMEOUT		3

class CStatusApp : public CAppBase
{
public:
	CStatusApp(DWORD pHwnd):CAppBase(pHwnd)
	{
	}

	~CStatusApp(void)
	{
	}

	BOOL DoProcess(DWORD uMsg, DWORD wParam, DWORD lParam, DWORD zParam)
	{
		switch( uMsg )
		{
		case TIME_MESSAGE:
			if(m_dwTimeOut < STATUS_TIMEOUT)
			{
				m_dwTimeOut++;
				if(m_dwTimeOut == STATUS_TIMEOUT)
					m_layoutTip->SwitchLay(FALSE);
			}
			break;
		case MSG_SHOW_STATUE:
			m_pTip->SetSrc(GetStringByID(wParam));
			m_pTip->Show(TRUE);
			m_layoutTip->SwitchLay( TRUE );
			m_dwTimeOut = 0;
			break;
		case TOUCH_MESSAGE:
		case KBD_MESSAGE:
			if(m_dwTimeOut < STATUS_TIMEOUT)
			{
				m_dwTimeOut == STATUS_TIMEOUT;
				m_layoutTip->SwitchLay(FALSE);
			}
			break;
		}
		return TRUE;
	}

	BOOL Create(DWORD lParam, DWORD zParam)
	{
		InitFrame("status.xml");
		m_layoutTip = (CLayOut*)GetCtrlByName("mainlayer");
		m_pTip = (CDPStatic*)GetCtrlByName( "tip" );
		m_dwTimeOut == STATUS_TIMEOUT;
		return TRUE;
	}
private:
	CLayOut* m_layoutTip;
	CDPStatic* m_pTipTitle;
	CDPStatic* m_pTip;
	DWORD m_dwTimeOut;
};

CAppBase* CreateStatusApp(DWORD pHwnd, DWORD lParam, DWORD zParam)
{
	CStatusApp* pMain;
	pMain = new CStatusApp(pHwnd);
	if(pMain != NULL)
	{
		if(!pMain->Create(lParam, zParam))
		{
			delete pMain;
			pMain = NULL;
		}
	}

	return pMain;
}

